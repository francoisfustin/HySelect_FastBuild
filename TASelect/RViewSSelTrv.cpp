#include "stdafx.h"
#include "afxctl.h"

#include "TASelect.h"
#include "MainFrm.h"
#include "Hydronic.h"

#include "DlgLeftTabSelManager.h"
#include "ProductSelectionParameters.h"
#include "RViewSSelTrv.h"

CRViewSSelTrv *pRViewSSelTrv = NULL;

CRViewSSelTrv::CRViewSSelTrv() : CRViewSSelSS( CMainFrame::RightViewList::eRVSSelTrv, false )
{
	m_pclIndSelTRVParams = NULL;
	_Init();
	pRViewSSelTrv = this;
}

CRViewSSelTrv::~CRViewSSelTrv()
{
	pRViewSSelTrv = NULL;
}

void CRViewSSelTrv::Reset()
{
	_Init();
	CRViewSSelSS::Reset();
}

void CRViewSSelTrv::Suggest( CProductSelelectionParameters *pclProductSelectionParameters, LPARAM lpParam )
{
	CRViewSSelSS::Suggest( pclProductSelectionParameters, lpParam );

	if( NULL == pclProductSelectionParameters || NULL == dynamic_cast<CIndSelTRVParams*>( pclProductSelectionParameters ) 
			|| NULL == lpParam )
	{
		return;
	}

	m_pclIndSelTRVParams = dynamic_cast<CIndSelTRVParams*>( pclProductSelectionParameters );
	m_rSuggestParams.m_dMaxDpSV = ( (SuggestParams*)lpParam )->m_dMaxDpSV;
	m_rSuggestParams.m_dMaxDpRV = ( (SuggestParams*)lpParam )->m_dMaxDpRV;
	m_rSuggestParams.m_bIsDifferentDpOnSVExist = ( (SuggestParams*)lpParam )->m_bIsDifferentDpOnSVExist;
	m_rSuggestParams.m_bIsDifferentDpOnRVExist = ( (SuggestParams*)lpParam )->m_bIsDifferentDpOnRVExist;

	// To remove all current displayed sheets.
	Reset();

	BeginWaitCursor();
	CWnd::SetRedraw( FALSE );

	m_uiSDActuatorID = ( true == m_pclIndSelTRVParams->m_bIsThermostaticHead ) ? SD_SVThermoActuator : SD_SVElectroActuator;

	long lSupplyValveRowSelected = -1;
	long lReturnValveRowSelected = -1;
	CDB_TAProduct *pEditedSupplyValve = NULL; 
	CDB_TAProduct *pEditedReturnValve = NULL; 
	
	CDS_SSelRadSet *pSelectedTrv = reinterpret_cast<CDS_SSelRadSet*>( m_pclIndSelTRVParams->m_SelIDPtr.MP );
	
	if( NULL != pSelectedTrv )
	{
		pEditedSupplyValve = dynamic_cast<CDB_TAProduct *>( pSelectedTrv->GetSupplyValveIDPtr().MP );
		pEditedReturnValve = dynamic_cast<CDB_TAProduct *>( pSelectedTrv->GetReturnValveIDPtr().MP );
	}

	// If user doesn't want insert or wants insert but Heimeier.
	if( RadiatorValveType::RVT_Inserts != m_pclIndSelTRVParams->m_eValveType || RIT_Heimeier == m_pclIndSelTRVParams->m_eInsertType )
	{
		// HYS-1305 : Add insert with automatic flow limiter
		if( ( RVT_WithFlowLimitation != m_pclIndSelTRVParams->m_eValveType ) 
			&& ( false == m_pclIndSelTRVParams->m_bIsFLCVInsert ) )
		{
			lSupplyValveRowSelected = _FillSupplyValveRows( pEditedSupplyValve );
		}
		else
		{
			lSupplyValveRowSelected = _FillSupplyFlowLimitedRows( pEditedSupplyValve );
		}

		lReturnValveRowSelected = _FillReturnValveRows( pEditedReturnValve );
	}
	else
	{
		// Create first group 'Other insert'.
		_CreateOtherInsertGroup( pSelectedTrv );
		
		// Create return valve group.
		lReturnValveRowSelected = _FillReturnValveRows( pEditedReturnValve );
	}
	
	CSheetDescription *pclSheetDescriptionSupplyValve;
	
	// HYS-1305 : Add insert with automatic flow limiter
	if(( RVT_WithFlowLimitation != m_pclIndSelTRVParams->m_eValveType ) 
		&& ( false == m_pclIndSelTRVParams->m_bIsFLCVInsert ) )
	{
		pclSheetDescriptionSupplyValve = m_ViewDescription.GetFromSheetDescriptionID( SD_SupplyValve );
	}
	else
	{
		pclSheetDescriptionSupplyValve = m_ViewDescription.GetFromSheetDescriptionID( SD_SupplyFlowLimited );
	}

	// If we are in edition mode, we must simulate a click on the supply valve.
	if( true == m_pclIndSelTRVParams->m_bEditModeRunning )
	{
		// Verify if sheet description has been well created.
		// Remark: 'pclSheetDescriptionSupplyValve' will be NULL if we are in 'Other insert' mode.
		if( NULL != pclSheetDescriptionSupplyValve && NULL != pclSheetDescriptionSupplyValve->GetSSheetPointer() )
		{
			CSSheet *pclSSheet = pclSheetDescriptionSupplyValve->GetSSheetPointer();	
			
			if( lSupplyValveRowSelected > -1 )	
			{
				CCellDescriptionProduct *pclCDSupplyValve = FindCDProduct( lSupplyValveRowSelected, (LPARAM)pEditedSupplyValve, pclSheetDescriptionSupplyValve );
				
				if( NULL != pclCDSupplyValve && NULL != pclCDSupplyValve->GetProduct() )
				{
					// If we are in edition mode we simulate a click on the product.
					OnClickProduct( pclSheetDescriptionSupplyValve, pclCDSupplyValve, pclSheetDescriptionSupplyValve->GetActiveColumn(), lSupplyValveRowSelected );

					// Allow to check if we need to change the 'Show all priorities' button or not.
					CheckShowAllPrioritiesButtonState( pclSheetDescriptionSupplyValve, lSupplyValveRowSelected );

					// Verify accessories on supply valve.
					CAccessoryList *pclSupplyValveAccessoryList = pSelectedTrv->GetSupplyValveAccessoryList();
					CAccessoryList::AccessoryItem rAccessoryItem = pclSupplyValveAccessoryList->GetFirst();
					
					while( NULL != rAccessoryItem.IDPtr.MP )
					{
						VerifyCheckboxAccessories( (CDB_Product *)rAccessoryItem.IDPtr.MP, true, &m_vecSupplyValveAccessoryList );
						rAccessoryItem = pclSupplyValveAccessoryList->GetNext();
					}

					// Verify if user has selected an actuator.
					IDPTR ActuatorIDPtr = pSelectedTrv->GetSupplyValveActuatorIDPtr();
					CSheetDescription *pclSheetDescriptionSVActuator = m_ViewDescription.GetFromSheetDescriptionID( m_uiSDActuatorID );
					
					if( _NULL_IDPTR != ActuatorIDPtr && NULL != ActuatorIDPtr.MP && NULL != pclSheetDescriptionSVActuator )
					{
						CDB_Actuator *pEditedActuator = dynamic_cast<CDB_Actuator*>( ActuatorIDPtr.MP );
						
						if( NULL != pEditedActuator )
						{
							// Find row number where is the	actuator.
							long lActuatorRowSelected = _GetRowOfEditedSVActuator( pclSheetDescriptionSVActuator, pEditedActuator );
							
							if( lActuatorRowSelected != -1 )
							{
								CCellDescriptionProduct *pclCDActuator = FindCDProduct( lActuatorRowSelected, (LPARAM)pEditedActuator, pclSheetDescriptionSVActuator );
								
								if( NULL != pclCDActuator && NULL != pclCDActuator->GetProduct() )
								{
									// Simulate a click on the edited actuator.
									OnClickProduct( pclSheetDescriptionSVActuator, pclCDActuator, pclSheetDescriptionSVActuator->GetActiveColumn(), lActuatorRowSelected );

									// Allow to check if we need to change the 'Show all priorities' button or not.
									CheckShowAllPrioritiesButtonState( pclSheetDescriptionSVActuator, lActuatorRowSelected );

 									// HYS- 987: Verify accessories on actuator for Trv.
 									CAccessoryList* pclActuatorAccessoryList = pSelectedTrv->GetSVActuatorAccessoryList();
 									IDPTR idptr = pclActuatorAccessoryList->GetFirst().IDPtr;
 									while( NULL != idptr.MP )
 									{
 										VerifyCheckboxAccessories( (CDB_Product *)idptr.MP, true, &m_vecSVActuatorAccessoryList );
 										idptr = pclActuatorAccessoryList->GetNext().IDPtr;
 									}
								}
							}
						}
					}
				}
			}
		}
		
		// Verify if sheet description has been well created.
		CSheetDescription *pclSheetDescriptionReturnValve = m_ViewDescription.GetFromSheetDescriptionID( SD_ReturnValve );
		
		if( NULL != pEditedReturnValve && NULL != pclSheetDescriptionReturnValve && NULL != pclSheetDescriptionReturnValve->GetSSheetPointer() )
		{
			lReturnValveRowSelected = _GetRowOfEditedReturnValve( pclSheetDescriptionReturnValve, pEditedReturnValve );
			
			if( lReturnValveRowSelected > -1 )	
			{
				CCellDescriptionProduct *pclCDReturnValve = FindCDProduct( lReturnValveRowSelected, (LPARAM)pEditedReturnValve, pclSheetDescriptionReturnValve );
				
				if( NULL != pclCDReturnValve && NULL != pclCDReturnValve->GetProduct() )
				{
					// If we are in edition mode we simulate a click on the product.
					OnClickProduct( pclSheetDescriptionReturnValve, pclCDReturnValve, pclSheetDescriptionReturnValve->GetActiveColumn(), lReturnValveRowSelected );

					// Allow to check if we need to change the 'Show all priorities' button or not.
					CheckShowAllPrioritiesButtonState( pclSheetDescriptionReturnValve, lReturnValveRowSelected );

					// Verify accessories on return valve.
					CAccessoryList *pclReturnValveAccessoryList = pSelectedTrv->GetReturnValveAccessoryList();
					CAccessoryList::AccessoryItem rAccessoryItem = pclReturnValveAccessoryList->GetFirst();
					
					while( NULL != rAccessoryItem.IDPtr.MP )
					{
						VerifyCheckboxAccessories( (CDB_Product *)rAccessoryItem.IDPtr.MP, true, &m_vecReturnValveAccessoryList );
						rAccessoryItem = pclReturnValveAccessoryList->GetNext();
					}
				}
			}
		}
	}
	else
	{
		if( NULL != pclSheetDescriptionSupplyValve )
		{
			// Define the first product row as the active cell and set a visual focus.
			PrepareAndSetNewFocus( pclSheetDescriptionSupplyValve, CD_SupplyValve_Name, RD_SupplyValve_FirstAvailRow, 0 );
		}
	}

	// Move different sheets to correct position in the right view.
	SetSheetSize();

	CWnd::SetRedraw( TRUE );
	m_bInitialised = true;

	// Pay attention: 'Invalidate' must be called after 'SetRedraw' because 'Invalidate' has no effect if 'SetRedraw' is set to FALSE.
	Invalidate();
	
	// Force a paint now.
	UpdateWindow();

	EndWaitCursor();
}

void CRViewSSelTrv::FillInSelected( CDS_SelProd *pSelectedProductToFill )
{
	if( NULL == m_pclIndSelTRVParams || NULL == m_pclIndSelTRVParams->m_pclSelectSupplyValveList )
	{
		ASSERT_RETURN;
	}

	CDS_SSelRadSet *pSelectedTrvToFill = dynamic_cast<CDS_SSelRadSet *>( pSelectedProductToFill );

	if( NULL == pSelectedTrvToFill )
	{
		return;
	}

	// Clear previous supply valve accessories selected.
	CAccessoryList *pclSupplyValveAccessoryListToFill = pSelectedTrvToFill->GetSupplyValveAccessoryList();

	// Clear previous supply valve actuators accessories selected.
	CAccessoryList *pclSVActuatorsAccessoryListToFill = pSelectedTrvToFill->GetSVActuatorAccessoryList();
	
	// Clear previous return valve accessories selected.
	CAccessoryList *pclReturnValveAccessoryListToFill = pSelectedTrvToFill->GetReturnValveAccessoryList();

	bool bIsEditionMode = false;

	if( false == m_pclIndSelTRVParams->m_bEditModeRunning )
	{
		pclSupplyValveAccessoryListToFill->Clear();
		pclSVActuatorsAccessoryListToFill->Clear();
		pclReturnValveAccessoryListToFill->Clear();
	}
	else
	{
		bIsEditionMode = true;
	}
	// Fill data for supply valve.
	CDB_ControlValve *pclCurrentSupplyValveSelected = GetCurrentSupplyValveSelected();
	
	if( NULL != pclCurrentSupplyValveSelected )
	{
		// Find corresponding CSelectedTAP.
		CSelectedValve *pSelectedTAP = GetSelectProduct<CSelectedValve>( pclCurrentSupplyValveSelected, m_pclIndSelTRVParams->m_pclSelectSupplyValveList );
		ASSERT( NULL != pSelectedTAP );

		// Fill Dp computed for supply valve.
		pSelectedTrvToFill->SetSupplyValveDp( pSelectedTAP->GetDp() );

		// Fill opening computed for supply valve.
		pSelectedTrvToFill->SetSupplyValveOpening( pSelectedTAP->GetH() );

		// Save supply valve IDPtr.
		pSelectedTrvToFill->SetSupplyValveIDPtr( pclCurrentSupplyValveSelected->GetIDPtr() );

		if( false == bIsEditionMode )
		{
			// Retrieve selected accessory and add it.
			for( vecCDCAccessoryListIter vecIter = m_vecSupplyValveAccessoryList.begin(); vecIter != m_vecSupplyValveAccessoryList.end(); vecIter++ )
			{
				CCDBCheckboxAccessory *pCDBCheckboxAccessory = *vecIter;

				if( NULL != pCDBCheckboxAccessory && true == pCDBCheckboxAccessory->GetCheckStatus() && NULL != pCDBCheckboxAccessory->GetAccessoryPointer() )
				{
					pclSupplyValveAccessoryListToFill->Add( pCDBCheckboxAccessory->GetAccessoryPointer()->GetIDPtr(), CAccessoryList::AccessoryType::_AT_Accessory, pCDBCheckboxAccessory->GetRuledTable() );
				}
			}
		}
		else
		{
			UpdateAccessoryList( m_vecSupplyValveAccessoryList, pclSupplyValveAccessoryListToFill, CAccessoryList::_AT_Accessory );
		}

		// Save actuator.
		IDPTR ActIDPtr = _NULL_IDPTR;
		CDB_Actuator *pActuator = GetCurrentSVActuatorSelected();

		if( NULL != pActuator )
		{
			ActIDPtr = pActuator->GetIDPtr();
		}
		
		pSelectedTrvToFill->SetSupplyValveActuatorIDPtr( ActIDPtr );

		if( false == bIsEditionMode )
		{
			// Retrieve selected accessory on the actuators and add it.
			for( vecCDCAccessoryListIter vecIter = m_vecSVActuatorAccessoryList.begin(); vecIter != m_vecSVActuatorAccessoryList.end(); vecIter++ )
			{
				CCDBCheckboxAccessory *pCDBCheckboxAccessory = *vecIter;

				if( NULL != pCDBCheckboxAccessory && true == pCDBCheckboxAccessory->GetCheckStatus() && NULL != pCDBCheckboxAccessory->GetAccessoryPointer() )
				{
					pclSVActuatorsAccessoryListToFill->Add( pCDBCheckboxAccessory->GetAccessoryPointer()->GetIDPtr(), CAccessoryList::AccessoryType::_AT_Accessory, pCDBCheckboxAccessory->GetRuledTable() );
				}
			}
		}
		else
		{
			UpdateAccessoryList( m_vecSVActuatorAccessoryList, pclSVActuatorsAccessoryListToFill, CAccessoryList::_AT_Accessory );
		}

		// Selected pipe informations.
		m_pclIndSelTRVParams->m_pclSelectSupplyValveList->GetSelectPipeList()->GetMatchingPipe( pclCurrentSupplyValveSelected->GetSizeKey(), 
				*pSelectedTrvToFill->GetpSelPipe() );
	}
	else if( RadiatorValveType::RVT_Inserts == m_pclIndSelTRVParams->m_eValveType 
			&& RadiatorInsertType::RIT_ByKv == m_pclIndSelTRVParams->m_eInsertType )
	{
		// If user is in 'Other insert' mode, we put only the Dp on the insert.
		pSelectedTrvToFill->SetSupplyValveDp( m_dSVOtherInsertDp );
	}

	// Fill data for balancing valve.
	CDB_TAProduct *pclCurrentReturnValveSelected = GetCurrentReturnValveSelected();
	
	if( NULL != pclCurrentReturnValveSelected )
	{
		if( NULL == m_pclIndSelTRVParams->m_pclSelectSupplyValveList->GetReturnValveList() 
				|| NULL == m_pclIndSelTRVParams->m_pclSelectSupplyValveList->GetReturnValveList()->GetSelectPipeList() )
		{
			ASSERT_RETURN;
		}

		// Find corresponding CSelectedTAP.
		CSelectedValve *pSelectedTAP = GetSelectProduct<CSelectedValve>( pclCurrentReturnValveSelected, m_pclIndSelTRVParams->m_pclSelectSupplyValveList->GetReturnValveList() );
		ASSERT( NULL != pSelectedTAP );

		// Fill Dp computed for return valve.
		pSelectedTrvToFill->SetReturnValveDp( pSelectedTAP->GetDp() );

		// Fill opening computed for return valve.
		pSelectedTrvToFill->SetReturnValveOpening( pSelectedTAP->GetH() );

		// Save return valve IDPtr.
		pSelectedTrvToFill->SetReturnValveIDPtr( pclCurrentReturnValveSelected->GetIDPtr() );

		if( false == bIsEditionMode )
		{
			// Retrieve selected accessory and add it.
			for( vecCDCAccessoryListIter vecIter = m_vecReturnValveAccessoryList.begin(); vecIter != m_vecReturnValveAccessoryList.end(); vecIter++ )
			{
				CCDBCheckboxAccessory *pCDBCheckboxAccessory = *vecIter;

				if( NULL != pCDBCheckboxAccessory && true == pCDBCheckboxAccessory->GetCheckStatus() && NULL != pCDBCheckboxAccessory->GetAccessoryPointer() )
				{
					pclReturnValveAccessoryListToFill->Add( pCDBCheckboxAccessory->GetAccessoryPointer()->GetIDPtr(),
						CAccessoryList::AccessoryType::_AT_Accessory, pCDBCheckboxAccessory->GetRuledTable() );
				}
			}
		}
		else
		{
			UpdateAccessoryList( m_vecReturnValveAccessoryList, pclReturnValveAccessoryListToFill, CAccessoryList::_AT_Accessory );
		}

		// Selected Pipe informations.
		m_pclIndSelTRVParams->m_pclSelectSupplyValveList->GetReturnValveList()->GetSelectPipeList()->GetMatchingPipe( 
				pclCurrentReturnValveSelected->GetSizeKey(), *pSelectedTrvToFill->GetpSelPipe() );
	}
}

void CRViewSSelTrv::OnNewDocument( CDS_IndSelParameter *pclIndSelParameter )
{
	_ReadAllColumnWidth( pclIndSelParameter );
}

void CRViewSSelTrv::SaveSelectionParameters( CDS_IndSelParameter *pclIndSelParameter )
{
	_WriteAllColumnWidth( pclIndSelParameter );
}

CDB_ControlValve *CRViewSSelTrv::GetCurrentSupplyValveSelected()
{
	CDB_ControlValve *pclCurrentSupplyValveSelected = NULL;
	CSheetDescription *pclSheetDescriptionSv = NULL;

	// HYS-1305 : Add insert with automatic flow limiter
	if( ( RVT_WithFlowLimitation != m_pclIndSelTRVParams->m_eValveType ) 
		&& ( false == m_pclIndSelTRVParams->m_bIsFLCVInsert ) )
	{
		pclSheetDescriptionSv = m_ViewDescription.GetFromSheetDescriptionID( SD_SupplyValve );
	}
	else
	{
		pclSheetDescriptionSv = m_ViewDescription.GetFromSheetDescriptionID( SD_SupplyFlowLimited );
	}

	if( NULL != pclSheetDescriptionSv )
	{
		// Retrieve the current selected supply valve.
		CCellDescriptionProduct *pclCDCurrentSupplyValveSelected = NULL;
		LPARAM lpPointer;

		if( true == pclSheetDescriptionSv->GetUserVariable( _SDUV_SELECTEDPRODUCT, lpPointer ) )
		{
			pclCDCurrentSupplyValveSelected = (CCellDescriptionProduct*)lpPointer;
		}

		if( NULL != pclCDCurrentSupplyValveSelected && NULL != pclCDCurrentSupplyValveSelected->GetProduct() )
		{
			pclCurrentSupplyValveSelected = dynamic_cast<CDB_ControlValve*>( (CData*)pclCDCurrentSupplyValveSelected->GetProduct() );
		}
	}

	return pclCurrentSupplyValveSelected;
}

void CRViewSSelTrv::SetCurrentSupplyValveSelected( CCellDescriptionProduct *pclCDCurrentSupplyValveSelected )
{
	// Try to retrieve sheet description linked to supply valve.
	CSheetDescription *pclSheetDescriptionSv = NULL;
	// HYS-1305 : Add insert with automatic flow limiter
	if( ( RVT_WithFlowLimitation != m_pclIndSelTRVParams->m_eValveType )
		&& ( false == m_pclIndSelTRVParams->m_bIsFLCVInsert ) )
	{
		pclSheetDescriptionSv = m_ViewDescription.GetFromSheetDescriptionID( SD_SupplyValve );
	}
	else
	{
		pclSheetDescriptionSv = m_ViewDescription.GetFromSheetDescriptionID( SD_SupplyFlowLimited );
	}

	if( NULL != pclSheetDescriptionSv )
	{
		pclSheetDescriptionSv->SetUserVariable( _SDUV_SELECTEDPRODUCT, (LPARAM)pclCDCurrentSupplyValveSelected );
	}
}

void CRViewSSelTrv::UpdateSupplyValveActuators()
{
	CSheetDescription *pclSheetDescription = NULL;
	CRect rectFocus( 0, 0, 0, 0);
	
	UINT uiSDID = -1;
	GetCurrentFocus( pclSheetDescription, rectFocus );
	
	if( NULL != pclSheetDescription )
	{
		 uiSDID = pclSheetDescription->GetSheetDescriptionID();
	}

	// Remark: 'm_uiSDActuatorID' was refreshed in 'Suggest' with current selected actuator type in 'DlgIndSelTrv'.
	if( true == m_ViewDescription.IsSheetDescriptionExist( m_uiSDActuatorID ) )
	{
		// If focus was on this group...
		if( m_uiSDActuatorID == uiSDID )
		{
			// To be sure to clean all previous data concerning focus.
			KillCurrentFocus();
		}

		// Clean previous actuator on supply valve.
		m_lSVActuatorSelectedRow = 0;
		m_pCDBExpandCollapseRowsSVActuator = NULL;
		SetCurrentSVActuatorSelected( NULL );
	
		m_ViewDescription.RemoveAllSheetAfter( m_uiSDActuatorID, true );
	}

	BeginWaitCursor();
	CWnd::SetRedraw( FALSE );

	_FillSupplyValveActuatorRows();

	// If there is actuators and focus was previously on this group...
	bool bFocusSet = false;

	if( true == m_ViewDescription.IsSheetDescriptionExist( m_uiSDActuatorID ) && m_uiSDActuatorID == uiSDID )
	{
		CSheetDescription *pclNextSheetDescription = m_ViewDescription.GetFromSheetDescriptionID( m_uiSDActuatorID );

		if( NULL != pclNextSheetDescription )
		{
			PrepareAndSetNewFocus( pclNextSheetDescription, pclNextSheetDescription->GetFirstFocusedColumn(), pclNextSheetDescription->GetFirstSelectableRow() );		
			bFocusSet = true;
		}
	}
	if( false == bFocusSet && m_uiSDActuatorID == uiSDID )
	{
		// Set to first group.
		CSheetDescription *pclFirstSheetDescription = m_ViewDescription.GetTopSheetDescription();

		if( NULL != pclFirstSheetDescription )
		{
			PrepareAndSetNewFocus( pclFirstSheetDescription, pclFirstSheetDescription->GetFirstFocusedColumn(), pclFirstSheetDescription->GetFirstSelectableRow() );		
		}
	}

	// Move different sheets to correct position in the right view.
	SetSheetSize();

	CWnd::SetRedraw( TRUE );

	// Pay attention: 'Invalidate' must be called after 'SetRedraw' because 'Invalidate' has no effect if 'SetRedraw' is set to FALSE.
	Invalidate();
	
	// Force a paint now.
	UpdateWindow();

	EndWaitCursor();
}

CDB_Actuator *CRViewSSelTrv::GetCurrentSVActuatorSelected( void )
{
	CDB_Actuator *pclCurrentActuatorSelected = NULL;

	// Remark: 'm_uiSDActuatorID' was refreshed in 'Suggest' with current selected actuator type in 'DlgIndSelTrv'.
	CSheetDescription *pclSheetDescriptionActuator = m_ViewDescription.GetFromSheetDescriptionID( m_uiSDActuatorID );

	if( NULL != pclSheetDescriptionActuator )
	{
		// Retrieve the current selected actuator.
		CCellDescriptionProduct *pclCDCurrentActuatorSelected = NULL;
		LPARAM lpPointer;

		if( true == pclSheetDescriptionActuator->GetUserVariable( _SDUV_SELECTEDPRODUCT, lpPointer ) )
		{
			pclCDCurrentActuatorSelected = (CCellDescriptionProduct*)lpPointer;
		}

		if( NULL != pclCDCurrentActuatorSelected && NULL != pclCDCurrentActuatorSelected->GetProduct() )
		{
			pclCurrentActuatorSelected = dynamic_cast<CDB_Actuator*>( (CData*)pclCDCurrentActuatorSelected->GetProduct() );
		}
	}

	return pclCurrentActuatorSelected;
}

void CRViewSSelTrv::SetCurrentSVActuatorSelected( CCellDescriptionProduct *pclCDCurrentActuatorSelected )
{
	// Try to retrieve sheet description linked to actuator.
	// Remark: 'm_uiSDActuatorID' was refreshed in 'Suggest' with current selected actuator type in 'DlgIndSelTrv'.
	CSheetDescription *pclSheetDescriptionActuator = m_ViewDescription.GetFromSheetDescriptionID( m_uiSDActuatorID );

	if( NULL != pclSheetDescriptionActuator )
	{
		pclSheetDescriptionActuator->SetUserVariable( _SDUV_SELECTEDPRODUCT, (LPARAM)pclCDCurrentActuatorSelected );
	}
}

CDB_TAProduct *CRViewSSelTrv::GetCurrentReturnValveSelected()
{
	CDB_TAProduct *pclCurrentReturnValveSelected = NULL;

	CSheetDescription *pclSheetDescriptionRv = m_ViewDescription.GetFromSheetDescriptionID( SD_ReturnValve );

	if( NULL != pclSheetDescriptionRv )
	{
		// Retrieve the current selected return valve.
		CCellDescriptionProduct *pclCDCurrentReturnValveSelected = NULL;
		LPARAM lpPointer;

		if( true == pclSheetDescriptionRv->GetUserVariable( _SDUV_SELECTEDPRODUCT, lpPointer ) )
		{
			pclCDCurrentReturnValveSelected = (CCellDescriptionProduct*)lpPointer;
		}

		if( NULL != pclCDCurrentReturnValveSelected && NULL != pclCDCurrentReturnValveSelected->GetProduct() )
		{
			pclCurrentReturnValveSelected = dynamic_cast<CDB_TAProduct*>( (CData*)pclCDCurrentReturnValveSelected->GetProduct() );
		}
	}

	return pclCurrentReturnValveSelected;
}

void CRViewSSelTrv::SetCurrentReturnValveSelected( CCellDescriptionProduct *pclCDCurrentReturnValveSelected )
{
	// Try to retrieve sheet description linked to return valve.
	CSheetDescription *pclSheetDescriptionRv = m_ViewDescription.GetFromSheetDescriptionID( SD_ReturnValve );

	if( NULL != pclSheetDescriptionRv )
	{
		pclSheetDescriptionRv->SetUserVariable( _SDUV_SELECTEDPRODUCT, (LPARAM)pclCDCurrentReturnValveSelected );
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PROTECTED MEMBERS
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CRViewSSelTrv::OnTextTipFetch( CSheetDescription *pclSheetDescription, long lColumn, long lRow, WORD *pwMultiLine, SHORT *pnTipWidth, 
		TCHAR *pstrTipText, BOOL *pbShowTip )
{
	if( NULL == m_pclIndSelTRVParams || NULL == m_pclIndSelTRVParams->m_pclSelectSupplyValveList )
	{
		ASSERTA_RETURN( false );
	}

	if( false == m_bInitialised || NULL == pclSheetDescription )
	{
		return false;
	}

	// First, we must call base class.
	// Remark: if base class has displayed a tooltip, it's not necessary to go further in this method.
	if( true == CRViewSSelSS::OnTextTipFetch( pclSheetDescription, lColumn, lRow, pwMultiLine, pnTipWidth, pstrTipText, pbShowTip ) )
	{
		return true;
	}

	CString str;
	bool bReturnValue = false;
	*pbShowTip = false;
	*pwMultiLine = SS_TT_MULTILINE_AUTO;
	
	CDB_TAProduct *pTAP = dynamic_cast<CDB_TAProduct *>( (CData *)GetCDProduct( lColumn, lRow, pclSheetDescription ) );
	CSSheet *pclSSheet = pclSheetDescription->GetSSheetPointer();
	CDS_TechnicalParameter *pclTechParam = m_pclIndSelTRVParams->m_pTADS->GetpTechParams();

	switch( pclSheetDescription->GetSheetDescriptionID() )
	{
		// Mouse cursor has passed over a supply valve.
		case SD_SupplyValve:
			{
				CDB_ThermostaticValve *pSupplyValve = dynamic_cast<CDB_ThermostaticValve *>( pTAP );
				CSelectedValve *pSelectedTAP = GetSelectProduct<CSelectedValve>( pTAP, m_pclIndSelTRVParams->m_pclSelectSupplyValveList );
				
				// Check if it's in the good column range.
				if( CD_SupplyValve_Setting == lColumn )
				{
					// Check what is the color of the text.
					if( NULL != pSupplyValve && NULL != pclSSheet && _ORANGE == pclSSheet->GetForeColor( lColumn, lRow ) )
					{
						// Case when setting is below the minimum recommended setting in the valve characteristic.
						CDB_ValveCharacteristic *pValveChar = pSupplyValve->GetValveCharacteristic();
						
						if( NULL != pValveChar )
						{
							CString str1;
							CString str2 = pValveChar->GetSettingString( pValveChar->GetMinRecSetting() );
							str1.Format( _T("%s < %s"), (LPCTSTR)pclSSheet->GetCellText( CD_SupplyValve_Setting, lRow ), (LPCTSTR)str2 );
							FormatString( str, IDS_SSHEETSSEL_SETTINGERROR, str1 );
						}
					}
				}
				else if( CD_SupplyValve_Dp == lColumn )
				{
					if( NULL != pSelectedTAP && NULL != pclSSheet && _ORANGE == pclSSheet->GetForeColor( lColumn, lRow ) )
					{
						// Check what is the error code.
						CString str2;
						double dDpTotal, dDpSv, dDpRv;
						int iErrorCode = pSelectedTAP->GetFlagError( CSelectedBase::eDp );
						
						switch( iErrorCode )
						{
							case CSelectTrvList::FlagError::DpNotFound:

								// If user has chosen a pressure drop and supply valve is presettable...
								if( true == m_pclIndSelTRVParams->m_bDpEnabled && true == m_pclIndSelTRVParams->m_bIsTrvTypePreset )
								{
									dDpTotal = m_pclIndSelTRVParams->m_dDp;
									dDpSv = pSelectedTAP->GetDp();
									dDpRv = -1.0;
									CSelectList *pSelectRVList = m_pclIndSelTRVParams->m_pclSelectSupplyValveList->GetReturnValveList();
									
									if( NULL != pSelectRVList )
									{
										// If pressure drop on all return valves is the same...
										if( false == m_rSuggestParams.m_bIsDifferentDpOnRVExist )
										{
											CSelectedValve *pFirstRv = pSelectRVList->GetFirst<CSelectedValve>();

											if( NULL != pFirstRv )
											{
												dDpRv = pFirstRv->GetDp();
											}
										}
										else if( NULL != GetCurrentReturnValveSelected() )
										{
											CSelectedValve *pSelectedRV = GetSelectProduct<CSelectedValve>( GetCurrentReturnValveSelected(), pSelectRVList );

											if( NULL != pSelectedRV )
											{
												dDpRv = pSelectedRV->GetDp();
											}
										}
									}

									if( ( dDpRv != -1.0 ) && ( dDpSv + dDpRv > dDpTotal ) )
									{
										CString str2 = WriteCUDouble( _U_DIFFPRESS, dDpSv );
										str2 += CString( _T(" + ") ) + WriteCUDouble( _U_DIFFPRESS, dDpRv );
										str2 += CString( _T(" > ") ) + WriteCUDouble( _U_DIFFPRESS, dDpTotal, true );
										FormatString( str, IDS_SSHEETSSELTRV_USERDPTOLOWERROR, str2 );
									}
								}

								if( true == str.IsEmpty() )
								{
									str = TASApp.LoadLocalizedString( ( true == m_pclIndSelTRVParams->m_bIsTrvTypePreset ) ? IDS_SSHEETSSELTRV_DPPRESETERROR 
											: IDS_SSHEETSSELTRV_DPNOPRESETERROR );
								}

								break;
								
							case CSelectTrvList::FlagError::DpAuthority:
								dDpTotal = ( true == m_pclIndSelTRVParams->m_bDpEnabled ) ? m_pclIndSelTRVParams->m_dDp : pclTechParam->GetTrvDefDpTot();
								str2 = WriteCUDouble( _U_DIFFPRESS, pSelectedTAP->GetDp() );
								str2 += CString( _T(" < ") ) + WriteDouble( pclTechParam->GetTrvMinDpRatio(), 2, 0 );
								str2 += CString( _T(" * ") ) + WriteCUDouble( _U_DIFFPRESS, dDpTotal, true );
								FormatString( str, IDS_SSHEETSSELTRV_DPAUTHERROR, str2 );
								break;

							case CSelectTrvList::FlagError::DpMaxReached:
								str2 = WriteCUDouble( _U_DIFFPRESS, pSelectedTAP->GetDp() );
								str2 += CString( _T(" > ") ) + WriteCUDouble( _U_DIFFPRESS, pSupplyValve->GetDpmax(), true );
								FormatString( str, IDS_SSHEETSSELTRV_DPERRORH, str2 );
								break;

							// Case when valve is non presettable and user has input a wanted Dp. Even before user has clicked on a return valve,
							// Dp on supply is already bigger than the Dp wanted.
							case CSelectTrvList::FlagError::DpAboveUser:
								str2 = WriteCUDouble( _U_DIFFPRESS, pSelectedTAP->GetDp() );
								str2 += CString( _T(" > ") ) + WriteCUDouble( _U_DIFFPRESS, m_pclIndSelTRVParams->m_dDp, true );
								FormatString( str, ( true == m_pclIndSelTRVParams->m_bIsTrvTypePreset ) ? IDS_SSHEETSSELTRV_DPVALVEERROR2 : IDS_SSHEETSSELTRV_DPVALVEERROR, str2 );
								break;

							default:
								break;
						}
					}
				}
				else if( CD_SupplyValve_TemperatureRange == lColumn )
				{
					if( NULL != pSelectedTAP && true == pSelectedTAP->IsFlagSet( CSelectedBase::eTemperature ) )
					{
						FormatString( str, IDS_SSHEETSSEL_TEMPERROR, pSupplyValve->GetTempRange() );
					}
				}
				else if( CD_SupplyValve_PipeLinDp == lColumn )
				{
					// Check what is the color of the text.
					if( NULL != pclSSheet && _ORANGE == pclSSheet->GetForeColor( lColumn, lRow ) )
					{
						CString str2 = CString( _T("[") ) + WriteCUDouble( _U_LINPRESSDROP, pclTechParam->GetPipeMinDp() );
						str2 += CString( _T(" - ") ) + WriteCUDouble( _U_LINPRESSDROP, pclTechParam->GetPipeMaxDp() );
						str2 += CString( _T("]") );
						FormatString( str, IDS_SSHEETSSEL_PIPELINDPERROR, str2 );
					}
				}
				else if( CD_SupplyValve_PipeV == lColumn )
				{
					// Check what is the color of the text.
					if( NULL != pclSSheet && _ORANGE == pclSSheet->GetForeColor( lColumn, lRow ) )
					{
						CString str2 = CString( _T("[") ) + WriteCUDouble( _U_VELOCITY, pclTechParam->GetPipeMinVel() );
						str2 += CString( _T(" - ") ) + WriteCUDouble( _U_VELOCITY, pclTechParam->GetPipeMaxVel() );
						str2 += CString( _T("]") );
						FormatString( str, IDS_SSHEETSSEL_PIPEVERROR, str2 );
					}
				}

				if( NULL != pTAP && true == str.IsEmpty() )
				{
					if( 0 != _tcslen( pTAP->GetComment() ) )
					{
						str = pTAP->GetComment();
					}
				}
			}
			break;

		// Mouse cursor has passed over a supply flow limited control valve.
		case SD_SupplyFlowLimited:
			{
				CDB_FlowLimitedControlValve *pSupplyValve = dynamic_cast<CDB_FlowLimitedControlValve *>( pTAP );
				CSelectedValve *pSelectedTAP = GetSelectProduct<CSelectedValve>( pTAP, m_pclIndSelTRVParams->m_pclSelectSupplyValveList );

				// Check if it's in the good column range.
				if( CD_SupplyFlowLimited_FlowRange == lColumn )
				{
					// Check what is the color of the text.
					if( NULL != pSupplyValve && NULL != pclSSheet && _ORANGE == pclSSheet->GetForeColor( lColumn, lRow ) )
					{
						// Case when flow is out of the recommended flow set in the valve characteristic.
						CDB_FLCVCharacteristic *pValveChar = pSupplyValve->GetFLCVCharacteristic();
						
						if( NULL != pValveChar )
						{
							// Selected flow is out of the recommended flow range ([%1])
							CString str1;
							str1.Format( _T("[%u-%u]"), pValveChar->GetQLFmin(), pValveChar->GetQNFmax() );
							FormatString( str, IDS_SSHEETSSELTRV_FLOWRANGEERROR, str1 );
						}
					}
				}
				else if( CD_SupplyFlowLimited_TemperatureRange == lColumn )
				{
					if( NULL != pSelectedTAP && true == pSelectedTAP->IsFlagSet( CSelectedBase::eTemperature ) )
					{
						FormatString( str, IDS_SSHEETSSEL_TEMPERROR, pSupplyValve->GetTempRange() );
					}
				}
				else if( CD_SupplyFlowLimited_PipeLinDp == lColumn )
				{
					// Check what is the color of the text.
					if( NULL != pclSSheet && _ORANGE == pclSSheet->GetForeColor( lColumn, lRow ) )
					{
						CString str2 = CString( _T("[") ) + WriteCUDouble( _U_LINPRESSDROP, pclTechParam->GetPipeMinDp() );
						str2 += CString( _T(" - ") ) + WriteCUDouble( _U_LINPRESSDROP, pclTechParam->GetPipeMaxDp() );
						str2 += CString( _T("]") );
						FormatString( str, IDS_SSHEETSSEL_PIPELINDPERROR, str2 );
					}
				}
				else if( CD_SupplyFlowLimited_PipeV == lColumn )
				{
					// Check what is the color of the text.
					if( NULL != pclSSheet && _ORANGE == pclSSheet->GetForeColor( lColumn, lRow ) )
					{
						CString str2 = CString( _T("[") ) + WriteCUDouble( _U_VELOCITY, pclTechParam->GetPipeMinVel() );
						str2 += CString( _T(" - ") ) + WriteCUDouble( _U_VELOCITY, pclTechParam->GetPipeMaxVel() );
						str2 += CString( _T("]") );
						FormatString( str, IDS_SSHEETSSEL_PIPEVERROR, str2 );
					}
				}

				if( NULL != pTAP && true == str.IsEmpty() )
				{
					if( 0 != _tcslen( pTAP->GetComment() ) )
					{
						str = pTAP->GetComment();
					}
				}
			}
			break;

		// Mouse cursor has passed over a supply valve accessory.
		case SD_SupplyValveAccessories:
			if( lColumn > CD_SupplyValveAccessory_FirstColumn && lColumn < CD_SupplyValveAccessory_LastColumn )
			{
				TextTipFetchEllipsesHelper( lColumn, lRow, pclSheetDescription, pnTipWidth, &str );
			}
			break;
	
		// Mouse cursor has passed over a return valve.
		case SD_ReturnValve:
			{
				CDB_RegulatingValve *pReturnValve = dynamic_cast<CDB_RegulatingValve *>( pTAP );
				CSelectedValve *pSelectedTAP = GetSelectProduct<CSelectedValve>( pTAP, m_pclIndSelTRVParams->m_pclSelectSupplyValveList->GetReturnValveList() );
				
				// Check if it's in the good column range.
				if( NULL != pReturnValve && CD_ReturnValve_Preset == lColumn )
				{
					// Check what is the color of the text.
					if( NULL != pclSSheet && _ORANGE == pclSSheet->GetForeColor( lColumn, lRow ) )
					{
						// Case when setting is below the minimum recommended setting in the valve characteristic.
						CDB_ValveCharacteristic *pValveChar = pReturnValve->GetValveCharacteristic();
						
						if( NULL != pValveChar )
						{
							CString str1;
							CString str2 = pValveChar->GetSettingString( pValveChar->GetMinRecSetting() );
							str1.Format( _T("%s < %s"), (LPCTSTR)pclSSheet->GetCellText( CD_SupplyValve_Setting, lRow ), (LPCTSTR)str2 );
							FormatString( str, IDS_SSHEETSSEL_SETTINGERROR, str1 );
						}
					}
				}
				else if( CD_ReturnValve_Dp == lColumn )
				{
					// Check what is the color of the text.
					if( NULL != pSelectedTAP && NULL != pclSSheet && _ORANGE == pclSSheet->GetForeColor( lColumn, lRow ) )
					{
						CString str2;
						int iErrorCode = pSelectedTAP->GetFlagError( CSelectedBase::eDp );
						
						switch( iErrorCode )
						{
							case CSelectList::BvFlagError::DpBelowMinDp:
								str2 = WriteCUDouble( _U_DIFFPRESS, pSelectedTAP->GetDp() );
								str2 += CString( _T(" < ") ) + WriteCUDouble( _U_DIFFPRESS, pclTechParam->GetValvMinDp( pTAP->GetTypeIDPtr().ID ), true );

								// "Dp is below the minimum value defined in technical parameters ( %1 )"
								FormatString( str, IDS_SSHEETSSEL_DPERRORL, str2 );
								break;

							case CSelectList::BvFlagError::DpAboveMaxDp:
							{
								// Either the max Dp is defined for the valve or we take the max Dp defined in the technical parameters.
								// This is why the message is different in regards to these both cases.
								double dDpMax = pReturnValve->GetDpmax();

								// IDS_SSHEETSSEL_DPERRORH: "Dp is above the maximum value defined in technical parameters ( %1 )"
								// IDS_SSHEETSSEL_DPERRORH2: "Dp is above the maximum value defined for this valve ( %1 )"
								int iMsgID = ( dDpMax <= 0.0 ) ? IDS_SSHEETSSEL_DPERRORH : IDS_SSHEETSSEL_DPERRORH2;

								str2 = WriteCUDouble( _U_DIFFPRESS, pSelectedTAP->GetDp() );
								str2 += CString( _T(" > ") ) + WriteCUDouble( _U_DIFFPRESS, pclTechParam->VerifyValvMaxDp(pTAP), true );
								FormatString( str, iMsgID, str2 );
								break;
							}

							case CSelectList::BvFlagError::DpToReachTooLow:
								{
									double dDpTotal = m_pclIndSelTRVParams->m_dDp;
									CSelectTrvList *pSelectTrvList = m_pclIndSelTRVParams->m_pclSelectSupplyValveList;

									if( NULL == pSelectTrvList )
									{
										break;
									}

									// Retrieve current supply valve selected
									CDB_ControlValve *pclCurrentSupplyValve = GetCurrentSupplyValveSelected();
									CSelectedValve *pSelectedSV = NULL;
									
									if( NULL != pclCurrentSupplyValve )
									{
										pSelectedSV = GetSelectProduct<CSelectedValve>( pclCurrentSupplyValve, m_pclIndSelTRVParams->m_pclSelectSupplyValveList );
									}
									else
									{
										// No supply valve selected. If pressure drop are all the same, we can take first supply valve.
										if( false == m_rSuggestParams.m_bIsDifferentDpOnSVExist )
										{
											pSelectedSV = pSelectTrvList->GetFirst<CSelectedValve>();
										}
									}
									
									if( NULL == pSelectedSV )
									{
										break;
									}
											
									double dDpSV = pSelectedSV->GetDp();

									if( dDpSV < dDpTotal )
									{
										double dAvailableRV = dDpTotal - dDpSV;
										double dDpRV = pSelectTrvList->GetDpRV();
										
										str2 = WriteCUDouble( _U_DIFFPRESS, dAvailableRV );
										str2 += CString( _T(" < ") ) + WriteCUDouble( _U_DIFFPRESS, dDpRV, true );
										FormatString( str, IDS_SSHEETSSELTRV_DPRVERROR, str2 );
									}
									else
									{
										str2 = WriteCUDouble( _U_DIFFPRESS, dDpSV );
										str2 += CString( _T(" > ") ) + WriteCUDouble( _U_DIFFPRESS, dDpTotal, true );
										FormatString( str, IDS_SSHEETSSELTRV_DPRVERROR2, str2 );
									}
								}
								break;

							case CSelectList::BvFlagError::DpQOTooLow:
							case CSelectList::BvFlagError::DpFOTooHigh:
								ASSERT( 0 );
								break;
						}
					}
				}
				else if( CD_ReturnValve_TemperatureRange == lColumn )
				{
					if( NULL != pSelectedTAP && true == pSelectedTAP->IsFlagSet( CSelectedBase::eTemperature ) )
					{
						FormatString( str, IDS_SSHEETSSEL_TEMPERROR, pReturnValve->GetTempRange() );
					}
				}
				else if( CD_ReturnValve_PipeLinDp == lColumn )
				{
					// Check what is the color of the text.
					if( NULL != pclSSheet && _ORANGE == pclSSheet->GetForeColor( lColumn, lRow ) )
					{
						CString str2 = CString( _T("[") ) + WriteCUDouble( _U_LINPRESSDROP, pclTechParam->GetPipeMinDp() );
						str2 += CString( _T(" - ") ) + WriteCUDouble( _U_LINPRESSDROP, pclTechParam->GetPipeMaxDp() );
						str2 += CString( _T("]") );
						FormatString( str, IDS_SSHEETSSEL_PIPELINDPERROR, str2 );
					}
				}
				else if( CD_ReturnValve_PipeV == lColumn )
				{
					// Check what is the color of the text.
					if( NULL != pclSSheet && _ORANGE == pclSSheet->GetForeColor( lColumn, lRow ) )
					{
						CString str2 = CString( _T("[") ) + WriteCUDouble( _U_VELOCITY, pclTechParam->GetPipeMinVel() );
						str2 += CString( _T(" - ") ) + WriteCUDouble( _U_VELOCITY, pclTechParam->GetPipeMaxVel() );
						str2 += CString( _T("]") );
						FormatString( str, IDS_SSHEETSSEL_PIPEVERROR, str2 );
					}
				}

				if( NULL != pTAP && true == str.IsEmpty() )
				{
					if( 0 != _tcslen( pTAP->GetComment() ) )
					{
						str = pTAP->GetComment();
					}
				}
			}
			break;

		// Mouse cursor has passed over a return valve accessory.
		case SD_ReturnValveAccessories:
			if( lColumn > CD_ReturnValveAccessory_FirstColumn && lColumn < CD_ReturnValveAccessory_LastColumn )
			{
				TextTipFetchEllipsesHelper( lColumn, lRow, pclSheetDescription, pnTipWidth, &str );
			}
			break;
		//HYS-726: Show tooltip error
		case SD_SVThermoActuator:
			if( ( CD_SVThrmActuator_Name == lColumn || CD_SVThrmActuator_MaxTemp == lColumn ) && _RED == pclSSheet->GetForeColor( lColumn, lRow ) )
			{
				str = TASApp.LoadLocalizedString( IDS_SSHEETTRVACT_TT_MAXTEMPERROR );
			}
			break;

		case SD_SVElectroActuator:
			if( ( CD_SVElecActuator_Name == lColumn || CD_SVElecActuator_MaxTemp == lColumn ) && _RED == pclSSheet->GetForeColor( lColumn, lRow ) )
			{
				str = TASApp.LoadLocalizedString( IDS_SSHEETTRVACT_TT_MAXTEMPERROR );
			}
			break;
	}

	if( false == str.IsEmpty() )
	{
		*pnTipWidth = (SHORT)pclSSheet->GetTipTextWidth( str );
		wcsncpy_s( pstrTipText, SS_TT_TEXTMAX, (LPCTSTR)str, SS_TT_TEXTMAX );
		*pbShowTip = true;
		bReturnValue = true;
	}
	return bReturnValue;
}

bool CRViewSSelTrv::OnClickProduct( CSheetDescription *pclSheetDescription, CCellDescriptionProduct *pclCellDescriptionProduct, long lColumn, long lRow )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() || NULL == pclCellDescriptionProduct )
	{
		return false;
	}

	bool bNeedRefresh = true;
	CSSheet *pclSSheet = pclSheetDescription->GetSSheetPointer();
	pclSSheet->SetBool( SSB_REDRAW, FALSE );

	CDB_ControlValve *pSupplyValve = dynamic_cast<CDB_ControlValve *>( (CData *)pclCellDescriptionProduct->GetProduct() );
	CDB_Actuator *pSupplyValveActuator = dynamic_cast<CDB_Actuator *>( (CData *)pclCellDescriptionProduct->GetProduct() );
	CDB_RegulatingValve *pReturnValve = dynamic_cast<CDB_RegulatingValve *>( (CData *)pclCellDescriptionProduct->GetProduct() );

	if( NULL != pSupplyValve )
	{
		// User has clicked on a supply valve.
		_ClickOnSupplyValve( pclSheetDescription, pclCellDescriptionProduct, lColumn, lRow );
	}
	else if( NULL != pSupplyValveActuator )
	{
		// User has clicked on a supply valve.
		_ClickOnSupplyValveActuator( pclSheetDescription, pSupplyValveActuator, pclCellDescriptionProduct, lColumn, lRow );
	}
	else if( NULL != pReturnValve )
	{
		// User has clicked on a return valve.
		_ClickOnReturnValve( pclSheetDescription, pReturnValve, pclCellDescriptionProduct, lColumn, lRow );
	}
	else
	{
		bNeedRefresh = false;
	}

	pclSSheet->SetBool( SSB_REDRAW, TRUE );
	return bNeedRefresh;
}

bool CRViewSSelTrv::ResetColumnWidth( short nSheetDescriptionID )
{
	TSpread clTSpread;

	if( FALSE == clTSpread.Create( WS_CHILD, CRect( 0, 0, 0, 0 ), this, 0 ) )
	{
		ASSERT( 0 );
		return false;
	}

	switch( nSheetDescriptionID )
	{
		case SD_SupplyValve:
			m_mapSSheetColumnWidth[SD_SupplyValve][CD_SupplyValve_FirstColumn] = clTSpread.ColWidthToLogUnits( 4 );
			m_mapSSheetColumnWidth[SD_SupplyValve][CD_SupplyValve_Box] = clTSpread.ColWidthToLogUnits( 2 );
			m_mapSSheetColumnWidth[SD_SupplyValve][CD_SupplyValve_CheckBox] = clTSpread.ColWidthToLogUnits( 2 );
			m_mapSSheetColumnWidth[SD_SupplyValve][CD_SupplyValve_Name] = clTSpread.ColWidthToLogUnits( 16 );
			m_mapSSheetColumnWidth[SD_SupplyValve][CD_SupplyValve_Connection] = clTSpread.ColWidthToLogUnits( 15 );
			m_mapSSheetColumnWidth[SD_SupplyValve][CD_SupplyValve_Version] = clTSpread.ColWidthToLogUnits( 17 );
			m_mapSSheetColumnWidth[SD_SupplyValve][CD_SupplyValve_PN] = clTSpread.ColWidthToLogUnits( 6 );
			m_mapSSheetColumnWidth[SD_SupplyValve][CD_SupplyValve_Size] = clTSpread.ColWidthToLogUnits( 10 );
			m_mapSSheetColumnWidth[SD_SupplyValve][CD_SupplyValve_Setting] = clTSpread.ColWidthToLogUnits( 10 );
			m_mapSSheetColumnWidth[SD_SupplyValve][CD_SupplyValve_Dp] = clTSpread.ColWidthToLogUnits( 10 );
			m_mapSSheetColumnWidth[SD_SupplyValve][CD_SupplyValve_DpFullOpening] = clTSpread.ColWidthToLogUnits( 10 );
			m_mapSSheetColumnWidth[SD_SupplyValve][CD_SupplyValve_Kv] = clTSpread.ColWidthToLogUnits( 6 );
			m_mapSSheetColumnWidth[SD_SupplyValve][CD_SupplyValve_TemperatureRange] = clTSpread.ColWidthToLogUnits( 8 );
			m_mapSSheetColumnWidth[SD_SupplyValve][CD_SupplyValve_Separator] = clTSpread.ColWidthToLogUnits( 1 );
			m_mapSSheetColumnWidth[SD_SupplyValve][CD_SupplyValve_PipeSize] = clTSpread.ColWidthToLogUnits( 8 );
			m_mapSSheetColumnWidth[SD_SupplyValve][CD_SupplyValve_PipeLinDp] = clTSpread.ColWidthToLogUnits( 8 );
			m_mapSSheetColumnWidth[SD_SupplyValve][CD_SupplyValve_PipeV] = clTSpread.ColWidthToLogUnits( 8 );
			m_mapSSheetColumnWidth[SD_SupplyValve][CD_SupplyValve_Pointer] = clTSpread.ColWidthToLogUnits( 1 );
			break;

		case SD_SupplyFlowLimited:
			m_mapSSheetColumnWidth[SD_SupplyFlowLimited][CD_SupplyFlowLimited_FirstColumn] = clTSpread.ColWidthToLogUnits( 4 );
			m_mapSSheetColumnWidth[SD_SupplyFlowLimited][CD_SupplyFlowLimited_CheckBox] = clTSpread.ColWidthToLogUnits( 2 );
			m_mapSSheetColumnWidth[SD_SupplyFlowLimited][CD_SupplyFlowLimited_Name] = clTSpread.ColWidthToLogUnits( 16 );
			m_mapSSheetColumnWidth[SD_SupplyFlowLimited][CD_SupplyFlowLimited_Connection] = clTSpread.ColWidthToLogUnits( 15 );
			m_mapSSheetColumnWidth[SD_SupplyFlowLimited][CD_SupplyFlowLimited_Version] = clTSpread.ColWidthToLogUnits( 17 );
			m_mapSSheetColumnWidth[SD_SupplyFlowLimited][CD_SupplyFlowLimited_PN] = clTSpread.ColWidthToLogUnits( 6 );
			m_mapSSheetColumnWidth[SD_SupplyFlowLimited][CD_SupplyFlowLimited_Size] = clTSpread.ColWidthToLogUnits( 10 );
			m_mapSSheetColumnWidth[SD_SupplyFlowLimited][CD_SupplyFlowLimited_Setting] = clTSpread.ColWidthToLogUnits( 10 );
			m_mapSSheetColumnWidth[SD_SupplyFlowLimited][CD_SupplyFlowLimited_Dpmin] = clTSpread.ColWidthToLogUnits( 10 );
			m_mapSSheetColumnWidth[SD_SupplyFlowLimited][CD_SupplyFlowLimited_FlowRange] = clTSpread.ColWidthToLogUnits( 10 );
			m_mapSSheetColumnWidth[SD_SupplyFlowLimited][CD_SupplyFlowLimited_TemperatureRange] = clTSpread.ColWidthToLogUnits( 8 );
			m_mapSSheetColumnWidth[SD_SupplyFlowLimited][CD_SupplyFlowLimited_Separator] = clTSpread.ColWidthToLogUnits( 1 );
			m_mapSSheetColumnWidth[SD_SupplyFlowLimited][CD_SupplyFlowLimited_PipeSize] = clTSpread.ColWidthToLogUnits( 8 );
			m_mapSSheetColumnWidth[SD_SupplyFlowLimited][CD_SupplyFlowLimited_PipeLinDp] = clTSpread.ColWidthToLogUnits( 8 );
			m_mapSSheetColumnWidth[SD_SupplyFlowLimited][CD_SupplyFlowLimited_PipeV] = clTSpread.ColWidthToLogUnits( 8 );
			m_mapSSheetColumnWidth[SD_SupplyFlowLimited][CD_SupplyFlowLimited_Pointer] = clTSpread.ColWidthToLogUnits( 1 );
			break;

		case SD_SVThermoActuator:
			m_mapSSheetColumnWidth[SD_SVThermoActuator][CD_SVThrmActuator_FirstColumn] = clTSpread.ColWidthToLogUnits( 4 );
			m_mapSSheetColumnWidth[SD_SVThermoActuator][CD_SVThrmActuator_Box] = clTSpread.ColWidthToLogUnits( 2 );
			m_mapSSheetColumnWidth[SD_SVThermoActuator][CD_SVThrmActuator_CheckBox] = clTSpread.ColWidthToLogUnits( 2 );
			m_mapSSheetColumnWidth[SD_SVThermoActuator][CD_SVThrmActuator_Name] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_SVThermoActuator][CD_SVThrmActuator_Description] = clTSpread.ColWidthToLogUnits( 40 );
			m_mapSSheetColumnWidth[SD_SVThermoActuator][CD_SVThrmActuator_SettingRange] = clTSpread.ColWidthToLogUnits( 10 );
			m_mapSSheetColumnWidth[SD_SVThermoActuator][CD_SVThrmActuator_CapillaryLength] = clTSpread.ColWidthToLogUnits( 13 );
			m_mapSSheetColumnWidth[SD_SVThermoActuator][CD_SVThrmActuator_Hysteresis] = clTSpread.ColWidthToLogUnits( 10 );
			m_mapSSheetColumnWidth[SD_SVThermoActuator][CD_SVThrmActuator_FrostProtection] = clTSpread.ColWidthToLogUnits( 10 );
			m_mapSSheetColumnWidth[SD_SVThermoActuator][CD_SVThrmActuator_MaxTemp] = clTSpread.ColWidthToLogUnits( 8 );
			m_mapSSheetColumnWidth[SD_SVThermoActuator][CD_SVThrmActuator_Pointer] = clTSpread.ColWidthToLogUnits( 1 );
			break;

		case SD_SVElectroActuator:
			m_mapSSheetColumnWidth[SD_SVElectroActuator][CD_SVElecActuator_FirstColumn] = clTSpread.ColWidthToLogUnits( 4 );
			m_mapSSheetColumnWidth[SD_SVElectroActuator][CD_SVElecActuator_Box] = clTSpread.ColWidthToLogUnits( 2 );
			m_mapSSheetColumnWidth[SD_SVElectroActuator][CD_SVElecActuator_CheckBox] = clTSpread.ColWidthToLogUnits( 2 );
			m_mapSSheetColumnWidth[SD_SVElectroActuator][CD_SVElecActuator_Name] = clTSpread.ColWidthToLogUnits( 30 );
			m_mapSSheetColumnWidth[SD_SVElectroActuator][CD_SVElecActuator_CloseOffValue] = clTSpread.ColWidthToLogUnits( 10 );
			m_mapSSheetColumnWidth[SD_SVElectroActuator][CD_SVElecActuator_ActuatingTime] = clTSpread.ColWidthToLogUnits( 10 );
			m_mapSSheetColumnWidth[SD_SVElectroActuator][CD_SVElecActuator_IP] = clTSpread.ColWidthToLogUnits( 10 );
			m_mapSSheetColumnWidth[SD_SVElectroActuator][CD_SVElecActuator_PowSupply] = clTSpread.ColWidthToLogUnits( 20 );
			m_mapSSheetColumnWidth[SD_SVElectroActuator][CD_SVElecActuator_InputSig] = clTSpread.ColWidthToLogUnits( 20 );
			m_mapSSheetColumnWidth[SD_SVElectroActuator][CD_SVElecActuator_OutputSig] = clTSpread.ColWidthToLogUnits( 20 );
			m_mapSSheetColumnWidth[SD_SVElectroActuator][CD_SVElecActuator_MaxTemp] = clTSpread.ColWidthToLogUnits( 8 );
			m_mapSSheetColumnWidth[SD_SVElectroActuator][CD_SVElecActuator_Pointer] = clTSpread.ColWidthToLogUnits( 1 );
			break;

		case SD_ReturnValve:
			m_mapSSheetColumnWidth[SD_ReturnValve][CD_ReturnValve_FirstColumn] = clTSpread.ColWidthToLogUnits( 4 );
			m_mapSSheetColumnWidth[SD_ReturnValve][CD_ReturnValve_CheckBox] = clTSpread.ColWidthToLogUnits( 2 );
			m_mapSSheetColumnWidth[SD_ReturnValve][CD_ReturnValve_Name] = clTSpread.ColWidthToLogUnits( 16 );
			m_mapSSheetColumnWidth[SD_ReturnValve][CD_ReturnValve_Connection] = clTSpread.ColWidthToLogUnits( 15 );
			m_mapSSheetColumnWidth[SD_ReturnValve][CD_ReturnValve_Version] = clTSpread.ColWidthToLogUnits( 17 );
			m_mapSSheetColumnWidth[SD_ReturnValve][CD_ReturnValve_PN] = clTSpread.ColWidthToLogUnits( 6 );
			m_mapSSheetColumnWidth[SD_ReturnValve][CD_ReturnValve_Size] = clTSpread.ColWidthToLogUnits( 10 );
			m_mapSSheetColumnWidth[SD_ReturnValve][CD_ReturnValve_Preset] = clTSpread.ColWidthToLogUnits( 10 );
			m_mapSSheetColumnWidth[SD_ReturnValve][CD_ReturnValve_Dp] = clTSpread.ColWidthToLogUnits( 10 );
			m_mapSSheetColumnWidth[SD_ReturnValve][CD_ReturnValve_TemperatureRange] = clTSpread.ColWidthToLogUnits( 8 );
			m_mapSSheetColumnWidth[SD_ReturnValve][CD_ReturnValve_Separator] = clTSpread.ColWidthToLogUnits( 1 );
			m_mapSSheetColumnWidth[SD_ReturnValve][CD_ReturnValve_PipeSize] = clTSpread.ColWidthToLogUnits( 8 );
			m_mapSSheetColumnWidth[SD_ReturnValve][CD_ReturnValve_PipeLinDp] = clTSpread.ColWidthToLogUnits( 8 );
			m_mapSSheetColumnWidth[SD_ReturnValve][CD_ReturnValve_PipeV] = clTSpread.ColWidthToLogUnits( 8 );
			m_mapSSheetColumnWidth[SD_ReturnValve][CD_ReturnValve_Pointer] = clTSpread.ColWidthToLogUnits( 1 );
			break;
	}

	if( NULL != clTSpread.GetSafeHwnd() )
	{
		clTSpread.DestroyWindow();
	}

	return true;
}

bool CRViewSSelTrv::IsSelectionReady()
{
	if( NULL == m_pclIndSelTRVParams )
	{
		ASSERTA_RETURN( false );
	}

	bool bReturn = false;
	CDB_ControlValve *pclSVSelected = GetCurrentSupplyValveSelected();
	CDB_TAProduct *pclRVSelected = GetCurrentReturnValveSelected();
	
	if( ( RadiatorReturnValveMode::RRVM_Nothing != m_pclIndSelTRVParams->m_eReturnValveMode && NULL != pclSVSelected && NULL != pclRVSelected) ||
		( RadiatorValveType::RVT_Inserts == m_pclIndSelTRVParams->m_eValveType && RadiatorInsertType::RIT_ByKv == m_pclIndSelTRVParams->m_eInsertType ) )
	{
		// If both objects are selected, 'Select' button can be enabled.
		// If user is in 'Other insert' mode for supply valve, then we have only return valve.
		bReturn = true;

		if( NULL != pclSVSelected )
		{
			bReturn = ( false == pclSVSelected->IsDeleted() );
		}

		if( bReturn && NULL != pclRVSelected )
		{
			bReturn = ( false == pclRVSelected->IsDeleted() );
		}	

	}
	else if( RadiatorReturnValveMode::RRVM_Nothing == m_pclIndSelTRVParams->m_eReturnValveMode && NULL != pclSVSelected )
	{
		// In case of only a supply valve is selected...
		bReturn = ( false == pclSVSelected->IsDeleted() );
	}

	return bReturn;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PRIVATE MEMBERS
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CRViewSSelTrv::_Init( void )
{
	m_lSupplyValveSelectedRow = 0;
	m_lSVActuatorSelectedRow = 0;
	m_lReturnValveSelectedRow = 0;
	m_vecSupplyValveAccessoryList.clear();
	m_vecSVActuatorAccessoryList.clear();
	m_vecReturnValveAccessoryList.clear();
	m_bMustFillSVSettingDp = false;
	m_bMustFillRVSettingDp = false;
	m_uiSDActuatorID = SD_SVThermoActuator;
	m_dSVOtherInsertDp = 0.0;
	m_pCDBExpandCollapseRowsSupplyValve = NULL;
	m_pCDBShowAllPrioritiesSupplyValve = NULL;
	m_pCDBExpandCollapseRowsSVActuator = NULL;
	m_pCDBExpandCollapseRowsReturnValve = NULL;
	m_pCDBShowAllPrioritiesReturnValve = NULL;
}

void CRViewSSelTrv::_ClickOnSupplyValve( CSheetDescription *pclSheetDescriptionSupplyValve, CCellDescriptionProduct *pclCellDescriptionProduct, 
		long lColumn, long lRow )
{
	if( NULL == m_pclIndSelTRVParams || NULL == m_pclIndSelTRVParams->m_pclSelectSupplyValveList )
	{
		ASSERT_RETURN;
	}
	
	CSSheet *pclSSheet = pclSheetDescriptionSupplyValve->GetSSheetPointer();

	// Memorize previous focus row.
	long lCurrentFocusedRow = GetCurrentFocusedRow();
	
	// To be sure to clean all previous data concerning focus.
	KillCurrentFocus();

	// Unselect row where current selection is set (yellow light).
	// Remark: 'KillCurrentFocus' unselect only rows where focus is on (blue light).
	pclSSheet->UnSelectMultipleRows();

	// Clean previous accessory list on supply and return valves.
	m_vecSupplyValveAccessoryList.clear();
	m_vecSVActuatorAccessoryList.clear();

	// Clean previous actuator on supply valve.
	m_lSVActuatorSelectedRow = 0;
	m_pCDBExpandCollapseRowsSVActuator = NULL;
	SetCurrentSVActuatorSelected( NULL );

	LPARAM lSupplyValveCount;
	pclSheetDescriptionSupplyValve->GetUserVariable( _SDUV_TOTALPRODUCT_COUNT, lSupplyValveCount );

	// Retrieve the current selected supply valve if exist.
	CDB_ControlValve *pclCurrentSupplyValveSelected = GetCurrentSupplyValveSelected();

	// If there is already one supply valve selected and user click on the current one...
	// Remark: 'm_pCDBExpandCollapseRowsSupplyValve' is not created if there is only one supply valve. Thus we need to check first if there is only one supply valve.
	//         Otherwise we have to check if user has clicked on the current one.
	if( NULL != pclCurrentSupplyValveSelected 
			&& ( ( 1 == lSupplyValveCount ) 
				|| ( NULL != m_pCDBExpandCollapseRowsSupplyValve && lRow == m_pCDBExpandCollapseRowsSupplyValve->GetCellPosition().y ) ) )
	{
		// Change focus state (selected to normal) and delete Expand/Collapse rows button.

		// Change box button to open state for the supply valve.
		if( true == pclCurrentSupplyValveSelected->IsPartOfaSet() )
		{
			CCDButtonBox *pCDButtonBox = GetCDButtonBox( CD_SupplyValve_Box, lRow, pclSheetDescriptionSupplyValve );

			if( NULL != pCDButtonBox )
			{
				pCDButtonBox->SetButtonState( CCDButtonBox::ButtonState::BoxOpened,  true );
			}
		}

		// Reset current product selected.
		SetCurrentSupplyValveSelected( NULL );

		// Uncheck checkbox.
		// HYS-1305 : Add insert with automatic flow limiter
		if( ( RVT_WithFlowLimitation != m_pclIndSelTRVParams->m_eValveType ) 
			&& ( false == m_pclIndSelTRVParams->m_bIsFLCVInsert ) )
		{
			pclSSheet->SetCheckBox( CD_SupplyValve_CheckBox, lRow, _T(""), false, true );
		}
		else
		{
			pclSSheet->SetCheckBox( CD_SupplyFlowLimited_CheckBox, lRow, _T(""), false, true );
		}

		// Delete Expand/Collapse rows button if exist.
		// Remark: 'm_pCDBExpandCollapseRowsSupplyValve' is set to NULL in method!
		DeleteExpandCollapseRowsButton( m_pCDBExpandCollapseRowsSupplyValve, pclSheetDescriptionSupplyValve );

		// Show Show/Hide all priorities button if exist.
		if( NULL != m_pCDBShowAllPrioritiesSupplyValve )
		{
			m_pCDBShowAllPrioritiesSupplyValve->ApplyInternalChange();
		}

		// Set focus on supply valve currently selected.
		PrepareAndSetNewFocus( pclSheetDescriptionSupplyValve, pclSheetDescriptionSupplyValve->GetActiveColumn(), lRow, 0 );

		// Remove all after supply valve group.
		m_ViewDescription.RemoveAllSheetAfter( SD_ReturnValve );
		
		m_lSupplyValveSelectedRow = 0;

		// Notify a change in the selection.
		SelectionHasChanged( IsSelectionReady() );

		// Clear 'Setting' and 'Dp' columns of all return valves.
		if( true == m_bMustFillRVSettingDp )
		{
			_UpdateReturnValveSettingDp( -1.0 );
		}
	}
	else
	{
		// If a supply valve is already selected...
		if( NULL != pclCurrentSupplyValveSelected )
		{
			// Change box button to open state for previous supply valve.
			if( true == pclCurrentSupplyValveSelected->IsPartOfaSet() )
			{
				CCDButtonBox *pCDButtonBox = GetCDButtonBox( CD_SupplyValve_Box, m_lSupplyValveSelectedRow, pclSheetDescriptionSupplyValve );

				if( NULL != pCDButtonBox )
				{
					pCDButtonBox->SetButtonState( CCDButtonBox::ButtonState::BoxOpened,  true );
				}
			}

			// Uncheck checkbox.
			// HYS-1305 : Add insert with automatic flow limiter
			if( ( RVT_WithFlowLimitation != m_pclIndSelTRVParams->m_eValveType )
				&& ( false == m_pclIndSelTRVParams->m_bIsFLCVInsert ) )
			{
				pclSSheet->SetCheckBox( CD_SupplyValve_CheckBox, m_lSupplyValveSelectedRow, _T(""), false, true );
			}
			else
			{
				pclSSheet->SetCheckBox( CD_SupplyFlowLimited_CheckBox, m_lSupplyValveSelectedRow, _T(""), false, true );
			}

			// Remove all after supply valve group.
			m_ViewDescription.RemoveAllSheetAfter( SD_ReturnValve );
		}

		// Save new supply valve selection.
		SetCurrentSupplyValveSelected( pclCellDescriptionProduct );
		pclCurrentSupplyValveSelected = GetCurrentSupplyValveSelected();

		// Select checkbox.
		// HYS-1305 : Add insert with automatic flow limiter
		if( ( RVT_WithFlowLimitation != m_pclIndSelTRVParams->m_eValveType )
			&& ( false == m_pclIndSelTRVParams->m_bIsFLCVInsert ) )
		{
			pclSSheet->SetCheckBox( CD_SupplyValve_CheckBox, lRow, _T(""), true, true );
		}
		else
		{
			pclSSheet->SetCheckBox( CD_SupplyFlowLimited_CheckBox, lRow, _T(""), true, true );
		}

		// Delete Expand/Collapse rows button if exist.
		if( NULL != m_pCDBExpandCollapseRowsSupplyValve )
		{
			// Remark: 'm_pCDBExpandCollapseRowsSupplyValve' is set to NULL in method!
			DeleteExpandCollapseRowsButton( m_pCDBExpandCollapseRowsSupplyValve, pclSheetDescriptionSupplyValve );
		}

		// Create Expand/Collapse rows button if needed...
		LPARAM lSupplyValveTotalCount;
		pclSheetDescriptionSupplyValve->GetUserVariable( _SDUV_TOTALPRODUCT_COUNT, lSupplyValveTotalCount );
		if( lSupplyValveTotalCount > 1 )
		{
			m_pCDBExpandCollapseRowsSupplyValve = CreateExpandCollapseRowsButton( CD_SupplyValve_FirstColumn, lRow, true, 
					CCDButtonExpandCollapseRows::ButtonState::CollapseRow, pclSheetDescriptionSupplyValve->GetFirstSelectableRow(), 
					pclSheetDescriptionSupplyValve->GetLastSelectableRow( false ), pclSheetDescriptionSupplyValve );

			// Show button.
			if( NULL != m_pCDBExpandCollapseRowsSupplyValve )
			{
				m_pCDBExpandCollapseRowsSupplyValve->SetShowStatus( true );
			}
		}

		// Select supply valve.
		pclSSheet->SelectOneRow( lRow, pclSheetDescriptionSupplyValve->GetSelectionFrom(), pclSheetDescriptionSupplyValve->GetSelectionTo() );

		// Fill accessories available for the current thermostatic valve.
		_FillSupplyValveAccessoryRows();

		// Fill actuators available for the current thermostatic valve.
		_FillSupplyValveActuatorRows();

		bool bSetFocus = true;

		// If user has selected Trv-Head set and we have only one actuator...
		// Remark: 'm_uiSDActuatorID' was refreshed in 'Suggest' with current selected actuator type in 'DlgIndSelTrv'.
		CSheetDescription *pSheetDescriptionActuator = m_ViewDescription.GetFromSheetDescriptionID( m_uiSDActuatorID );

		if( NULL != pSheetDescriptionActuator && true == pclCurrentSupplyValveSelected->IsPartOfaSet() )
		{
			// Verify first if we have only one actuator.
			LPARAM lActuatorTotalCount;
			pSheetDescriptionActuator->GetUserVariable( _SDUV_TOTALPRODUCT_COUNT, lActuatorTotalCount );
			
			if( 1 == lActuatorTotalCount )
			{
				// ... we can than automatically select actuator and close boxes in front of it.

				// Retrieve actuator.
				CCellDescriptionProduct *pclCDProduct = NULL;
				long lActuatorRow = pSheetDescriptionActuator->GetFirstSelectableRow();
				long lColParam = pSheetDescriptionActuator->GetFirstParameterColumn();
				LPARAM lProduct = GetCDProduct( lColParam, lActuatorRow, pSheetDescriptionActuator, &pclCDProduct );
				
				if( NULL != pclCDProduct )
				{
					// Simulate a click on the edited actuator.
					_ClickOnSupplyValveActuator( pSheetDescriptionActuator, dynamic_cast<CDB_Actuator*>( (CData*)lProduct ), pclCDProduct, 
							pSheetDescriptionActuator->GetSelectionFrom(), lActuatorRow, false );
					
					// Change box button to close state for supply valve.
					CCDButtonBox *pCDButtonBox = GetCDButtonBox( CD_SupplyValve_Box, lRow, pclSheetDescriptionSupplyValve );

					if( NULL != pCDButtonBox )
					{
						pCDButtonBox->SetButtonState( CCDButtonBox::ButtonState::BoxClosed,  true );
					}

					// Change box button to close state for actuator selected.
					int iBoxCol = ( SD_SVThermoActuator == m_uiSDActuatorID ) ? CD_SVThrmActuator_Box : CD_SVElecActuator_Box;
					pCDButtonBox = GetCDButtonBox( iBoxCol, lActuatorRow, pSheetDescriptionActuator );
					
					if( NULL != pCDButtonBox)
					{
						pCDButtonBox->SetButtonState( CCDButtonBox::ButtonState::BoxClosed, true );
					}

					bSetFocus = false;
				}
			}
		}

		if( true == bSetFocus )
		{
			// Try to set the focus on the next table but do not lose the focus if no other table exist.
			CSheetDescription *pclNextSheetDescription = NULL;
			long lNewFocusedRow;
		
			// For TRV it's a special case. We don't want to focus on return valve group when user has selected a supply valve (2nd condition).
			bool bShiftPressed;

			if( DNFERRC_FocusOnNextGroupOK == DefineNextFocusedRow( pclSheetDescriptionSupplyValve, CD_SupplyValve_FirstColumn, lRow, false, 
					lNewFocusedRow, pclNextSheetDescription, bShiftPressed ) 
					&& SD_ReturnValve != pclNextSheetDescription->GetSheetDescriptionID() )
			{
				long lNewFocusedCol = pclNextSheetDescription->GetFirstParameterColumn();
				PrepareAndSetNewFocus( pclNextSheetDescription, lNewFocusedCol, lNewFocusedRow );
			}
			else
			{
				// Because we have changed current selection background in yellow, if there is no other sheet following current one, we
				// must set focus on current selection.
				PrepareAndSetNewFocus( pclSheetDescriptionSupplyValve, lColumn, lRow );
			}
		}

		// Memorize supply valve row selected.
		m_lSupplyValveSelectedRow = lRow;

		// Notify a change in the selection.
		SelectionHasChanged( IsSelectionReady() );

		// Update 'Setting' and 'Dp' columns of all return valves if needed...
		if( true == m_bMustFillRVSettingDp )
		{
			CDB_TAProduct *pclReturnValve = dynamic_cast<CDB_TAProduct*>( (CData*)pclCellDescriptionProduct->GetProduct() );

			if( NULL != pclReturnValve )
			{
				CSelectedValve *pclSelectedSupplyValve = GetSelectProduct<CSelectedValve>( pclReturnValve, m_pclIndSelTRVParams->m_pclSelectSupplyValveList );

				if( NULL != pclSelectedSupplyValve )
				{
					_UpdateReturnValveSettingDp( pclSelectedSupplyValve->GetDp() );
				}
			}
		}
	}
}

long CRViewSSelTrv::_FillSupplyValveRows( CDB_TAProduct *pEditedSupplyValve )
{
	if( NULL == m_pclIndSelTRVParams || NULL == m_pclIndSelTRVParams->m_pclSelectSupplyValveList )
	{
		ASSERTA_RETURN( 0 );
	}

	CSelectedBase *pclSelectedProduct = m_pclIndSelTRVParams->m_pclSelectSupplyValveList->GetFirst<CSelectedBase>();

	if( NULL == pclSelectedProduct ) 
	{
		return 0;
	}

	// Call 'MultiSpreadBase' method to get a new SSheet.
	CSheetDescription *pclSheetDescriptionSupplyValve = CreateSSheet( SD_SupplyValve );

	if( NULL == pclSheetDescriptionSupplyValve || NULL == pclSheetDescriptionSupplyValve->GetSSheetPointer() )
	{
		return 0;
	}
	
	CSSheet *pclSSheet = pclSheetDescriptionSupplyValve->GetSSheetPointer();

	pclSSheet->SetBool( SSB_REDRAW, FALSE );

	pclSSheet->Init();
	pclSSheet->SetBool( SSB_ALLOWDRAGDROP, FALSE );
	pclSSheet->SetBool( SSB_HORZSCROLLBAR, FALSE );
	pclSSheet->SetBool( SSB_VERTSCROLLBAR, FALSE );
	pclSSheet->SetBool( SSB_MOVEACTIVEONFOCUS, FALSE );
	pclSSheet->SetTextTip( SS_TT_STATUS_FLOATING, 250, NULL, _LIGHTYELLOW, _TAH_BLACK );
	
	// Initialize supply valve sheet and fill header.
	_InitAndFillSupplyValveHeader( pclSheetDescriptionSupplyValve, pclSSheet );

	long lRetRow = -1;
	long lRow = RD_SupplyValve_FirstAvailRow;

	long lValveTotalCount = 0;
	long lValveNotPriorityCount = 0;
	bool bShowAllPrioritiesShown = false;
	long lShowAllPrioritiesButtonRow = 0;
	
	pclSheetDescriptionSupplyValve->SetUserVariable( _SDUV_TOTALPRODUCT_COUNT, 0 );
	pclSheetDescriptionSupplyValve->SetUserVariable( _SDUV_NOPRIORITYPRODUCT_COUNT, 0 );

	bool bAtLeastOneTrvPartOfASet = false;
	pclSheetDescriptionSupplyValve->RestartRemarkGenerator();
	
	for( CSelectedValve *pclSelectedSupplyValve = m_pclIndSelTRVParams->m_pclSelectSupplyValveList->GetFirst<CSelectedValve>(); 
			NULL != pclSelectedSupplyValve; pclSelectedSupplyValve = m_pclIndSelTRVParams->m_pclSelectSupplyValveList->GetNext<CSelectedValve>() )
	{
		CDB_ThermostaticValve *pSupplyValve = dynamic_cast<CDB_ThermostaticValve *>( pclSelectedSupplyValve->GetpData() );

		if( NULL == pSupplyValve ) 
		{
			continue;
		}

		if( true == pSupplyValve->IsPartOfaSet() )
		{
			bAtLeastOneTrvPartOfASet = true;
		}

		if( true == pclSelectedSupplyValve->IsFlagSet(CSelectedBase::eNotPriority) )
		{
			lValveNotPriorityCount++;
			
			if( false == bShowAllPrioritiesShown )
			{
				// Add one empty not selectable row.
				pclSheetDescriptionSupplyValve->AddRows( 1 );
				pclSheetDescriptionSupplyValve->RemoveSelectableRow( lRow );

				pclSSheet->SetCellBorder( CD_SupplyValve_CheckBox, lRow, CD_SupplyValve_Separator - 1, lRow, true, 
						SS_BORDERTYPE_TOP | SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );

				pclSSheet->SetCellBorder( CD_SupplyValve_PipeSize, lRow, CD_SupplyValve_Pointer - 1, lRow, true, 
						SS_BORDERTYPE_TOP | SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );

				lShowAllPrioritiesButtonRow = lRow++;
				bShowAllPrioritiesShown = true;
			}
		}

		long lRetVal = _FillOneSupplyValveRow( pclSheetDescriptionSupplyValve, pclSSheet, pclSelectedSupplyValve, lRow, pEditedSupplyValve );
		
		if( lRetVal > 0 )
		{
			lRetRow = lRetVal;
		}

		lRow++;
		lValveTotalCount++;
	}

	long lLastDataRow = lRow - 1;

	// If there is no thermostatic valve that belongs to a set...
	if( false == bAtLeastOneTrvPartOfASet )
	{
		pclSSheet->ShowCol( CD_SupplyValve_Box, FALSE );
	}

	pclSheetDescriptionSupplyValve->SetUserVariable( _SDUV_TOTALPRODUCT_COUNT, lValveTotalCount );
	pclSheetDescriptionSupplyValve->SetUserVariable( _SDUV_NOPRIORITYPRODUCT_COUNT, lValveNotPriorityCount );

	// Now we can create Show/Hide all priorities if needed.
	// Remarks: 2nd condition to avoid to have button if we have only no priority valves.
	if( true == bShowAllPrioritiesShown && lValveTotalCount > lValveNotPriorityCount )
	{
		// Create Show/Hide all priorities button in regards to current state!
		CCDButtonShowAllPriorities::ButtonState eState = ( true == GetShowAllPrioritiesFlag( SD_SupplyValve ) ) ? 
				CCDButtonShowAllPriorities::ButtonState::HidePriorities : CCDButtonShowAllPriorities::ButtonState::ShowPriorities;
		
		CCDButtonShowAllPriorities *pclShowAllButton = CreateShowAllPrioritiesButton( CD_SupplyValve_FirstColumn, lShowAllPrioritiesButtonRow, 
				bShowAllPrioritiesShown, eState, lShowAllPrioritiesButtonRow + 1, pclSSheet->GetMaxRows(), pclSheetDescriptionSupplyValve );

		// Here we must call 'ApplyInternalChange' to show or hide rows in regards to the current state!
		if( NULL != pclShowAllButton )
		{
			pclShowAllButton->ApplyInternalChange( );
		}
	}
	else
	{
		if( lShowAllPrioritiesButtonRow != 0 )
		{
			pclSSheet->ShowRow( lShowAllPrioritiesButtonRow, false );
		}
	}
	
	// Try to merge only if there is more than one supply valve.
	if( lValveTotalCount > 2 || ( 2 == lValveTotalCount && lValveNotPriorityCount != 1 ) )
	{
		vector<long> vecColumnList;
		vecColumnList.push_back( CD_SupplyValve_PipeSize );
		vecColumnList.push_back( CD_SupplyValve_PipeLinDp );
		vecColumnList.push_back( CD_SupplyValve_PipeV );
		pclSSheet->MergeRowsWithSameValues( RD_SupplyValve_FirstAvailRow, lLastDataRow, vecColumnList );
	}

	pclSSheet->SetCellBorder( CD_SupplyValve_CheckBox, lLastDataRow, CD_SupplyValve_Separator - 1, lLastDataRow, true, SS_BORDERTYPE_BOTTOM,
			SS_BORDERSTYLE_SOLID, _BLACK );

	// Needed by pipe merging.
	pclSSheet->SetCellBorder( CD_SupplyValve_PipeSize, lLastDataRow, CD_SupplyValve_Pointer - 1, lLastDataRow, true, SS_BORDERTYPE_BOTTOM,
			SS_BORDERSTYLE_SOLID, _BLACK );

	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

	// By default we add one row for remark.
	// Remark: 'false' to specify that this row can't be selected.
	pclSheetDescriptionSupplyValve->AddRows( 1, false );

	pclSSheet->AddCellSpanW( CD_SupplyValve_CheckBox, lRow, CD_SupplyValve_Separator - CD_SupplyValve_CheckBox, 1 );
	pclSSheet->AddCellSpanW( CD_SupplyValve_CheckBox, lRow + 1, CD_SupplyValve_Separator - CD_SupplyValve_CheckBox, 1 );
	
	// Add Remarks.
	// Note: these lines must be placed after creating Show/Hide all priorities button to avoid to take in count these lines when defining
	//       range of row to be Show/Hide.
	pclSheetDescriptionSupplyValve->SetUserVariable( _SDUV_REMARKROW, lRow );
	lRow = pclSheetDescriptionSupplyValve->WriteRemarks( lRow, CD_SupplyValve_CheckBox, CD_SupplyValve_Separator );

	pclSSheet->SetBool( SSB_REDRAW, TRUE );

	// Set that there is no selection at now.
	SetCurrentSupplyValveSelected( NULL );

	// Add possibility to change column size.
	// Remark: place this code after any initialization (like adding, removing, resizing, hiding/showing columns).
	CResizingColumnInfo *pclResizingColumnInfo = pclSheetDescriptionSupplyValve->GetResizingColumnInfo();
	pclResizingColumnInfo->ActivateFeature( true, false, RD_SupplyValve_FirstAvailRow, lLastDataRow );
	pclResizingColumnInfo->AddRangeColumn( CD_SupplyValve_Name, CD_SupplyValve_TemperatureRange, RD_SupplyValve_ColName, RD_SupplyValve_Unit );
	pclResizingColumnInfo->AddRangeColumn( CD_SupplyValve_PipeSize, CD_SupplyValve_PipeV, RD_SupplyValve_ColName, RD_SupplyValve_Unit );

	// Add possibility to click on title group to reset column width.
	CreateTitleGroupButtons( CD_SupplyValve_CheckBox, CD_SupplyValve_PipeV, RD_SupplyValve_GroupName, pclSheetDescriptionSupplyValve );

	return lRetRow;
}

void CRViewSSelTrv::_InitAndFillSupplyValveHeader( CSheetDescription *pclSheetDescriptionSupplyValve, CSSheet *pclSSheet )
{
	if( NULL == m_pclIndSelTRVParams || NULL == m_pclIndSelTRVParams->m_pPipeDB )
	{
		ASSERT_RETURN;
	}

	// Set title.
	pclSSheet->SetMaxRows( RD_SupplyValve_FirstAvailRow - 1 );
	
	// All cells are static by default and filled with _T("").
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	pclSSheet->FormatStaticText( -1, -1, -1, -1, _T("") );

	// Increase row height.
	double dRowHeight = 12.75;
	pclSSheet->SetRowHeight( RD_SupplyValve_FirstRow, dRowHeight * 0.5 );
	pclSSheet->SetRowHeight( RD_SupplyValve_GroupName, dRowHeight * 1.5 );
	pclSSheet->SetRowHeight( RD_SupplyValve_ColName, dRowHeight * 2 );
	pclSSheet->SetRowHeight( RD_SupplyValve_Unit, dRowHeight * 1.2 );
	
	// Set columns.
	pclSheetDescriptionSupplyValve->Init( 1, pclSSheet->GetMaxRows(), 1, CSheetDescription::SD_ParameterMode_Mono );
	pclSheetDescriptionSupplyValve->AddColumnInPixels( CD_SupplyValve_FirstColumn, m_mapSSheetColumnWidth[SD_SupplyValve][CD_SupplyValve_FirstColumn] );
	pclSheetDescriptionSupplyValve->AddColumnInPixels( CD_SupplyValve_Box, m_mapSSheetColumnWidth[SD_SupplyValve][CD_SupplyValve_Box] );
	pclSheetDescriptionSupplyValve->AddColumnInPixels( CD_SupplyValve_CheckBox, m_mapSSheetColumnWidth[SD_SupplyValve][CD_SupplyValve_CheckBox] );
	pclSheetDescriptionSupplyValve->AddColumnInPixels( CD_SupplyValve_Name, m_mapSSheetColumnWidth[SD_SupplyValve][CD_SupplyValve_Name] );
	pclSheetDescriptionSupplyValve->AddColumnInPixels( CD_SupplyValve_Connection, m_mapSSheetColumnWidth[SD_SupplyValve][CD_SupplyValve_Connection] );
	pclSheetDescriptionSupplyValve->AddColumnInPixels( CD_SupplyValve_Version, m_mapSSheetColumnWidth[SD_SupplyValve][CD_SupplyValve_Version] );
	pclSheetDescriptionSupplyValve->AddColumnInPixels( CD_SupplyValve_PN, m_mapSSheetColumnWidth[SD_SupplyValve][CD_SupplyValve_PN] );
	pclSheetDescriptionSupplyValve->AddColumnInPixels( CD_SupplyValve_Size, m_mapSSheetColumnWidth[SD_SupplyValve][CD_SupplyValve_Size] );
	pclSheetDescriptionSupplyValve->AddColumnInPixels( CD_SupplyValve_Setting, m_mapSSheetColumnWidth[SD_SupplyValve][CD_SupplyValve_Setting] );
	pclSheetDescriptionSupplyValve->AddColumnInPixels( CD_SupplyValve_Dp, m_mapSSheetColumnWidth[SD_SupplyValve][CD_SupplyValve_Dp] );
	pclSheetDescriptionSupplyValve->AddColumnInPixels( CD_SupplyValve_DpFullOpening, m_mapSSheetColumnWidth[SD_SupplyValve][CD_SupplyValve_DpFullOpening] );
	pclSheetDescriptionSupplyValve->AddColumnInPixels( CD_SupplyValve_Kv, m_mapSSheetColumnWidth[SD_SupplyValve][CD_SupplyValve_Kv] );
	pclSheetDescriptionSupplyValve->AddColumnInPixels( CD_SupplyValve_TemperatureRange, m_mapSSheetColumnWidth[SD_SupplyValve][CD_SupplyValve_TemperatureRange] );
	pclSheetDescriptionSupplyValve->AddColumnInPixels( CD_SupplyValve_Separator, m_mapSSheetColumnWidth[SD_SupplyValve][CD_SupplyValve_Separator] );
	pclSheetDescriptionSupplyValve->AddColumnInPixels( CD_SupplyValve_PipeSize, m_mapSSheetColumnWidth[SD_SupplyValve][CD_SupplyValve_PipeSize] );
	pclSheetDescriptionSupplyValve->AddColumnInPixels( CD_SupplyValve_PipeLinDp, m_mapSSheetColumnWidth[SD_SupplyValve][CD_SupplyValve_PipeLinDp] );
	pclSheetDescriptionSupplyValve->AddColumnInPixels( CD_SupplyValve_PipeV, m_mapSSheetColumnWidth[SD_SupplyValve][CD_SupplyValve_PipeV] );
	pclSheetDescriptionSupplyValve->AddColumnInPixels( CD_SupplyValve_Pointer, m_mapSSheetColumnWidth[SD_SupplyValve][CD_SupplyValve_Pointer] );

	// Set in which column parameter must be saved.
	pclSheetDescriptionSupplyValve->AddParameterColumn( CD_SupplyValve_Pointer );
	
	// Set the focus column.
	pclSheetDescriptionSupplyValve->SetActiveColumn( CD_SupplyValve_Name );

	// Set range for selection.
	pclSheetDescriptionSupplyValve->SetFocusColumnRange( CD_SupplyValve_CheckBox, CD_SupplyValve_TemperatureRange );

	// m_bIsTrvTypePreset = m_pclIndSelTRVParams->m_bIsTrvTypePreset
	// m_bDpEnabled = m_pclIndSelTRVParams->m_bDpEnabled
	// +-------+--------------------+--------------+---------+-----+---------------+
	// | Case  | m_bIsTrvTypePreset | m_bDpEnabled | Setting | Dp  | DpFullOpening |
	// +-------+--------------------+--------------+---------+-----+---------------+
	// |   1   |       0		    |      0       |    0    |  1  |       0       |
	// |   2   |       0		    |      1       |    0    |  1  |       0       |
	// |   3   |       1		    |      0       |    1    |  0  |       1       |
	// |   4   |       1		    |      1       |    1    |  1  |       1       |
	// +-------+--------------------+--------------+---------+-----+---------------+
		
	if( false == m_pclIndSelTRVParams->m_bIsTrvTypePreset )
	{
		// CASE 1 & 2: If supply valve has no preset we hide the 'Setting' column.
		pclSSheet->ShowCol( CD_SupplyValve_Setting, FALSE );
	}
	
	if( false == m_pclIndSelTRVParams->m_bDpEnabled )
	{
		// CASE 3: If supply valve has preset and user has not input Dp total, we hide 'Dp' column.
		pclSSheet->ShowCol( CD_SupplyValve_Dp, FALSE );
	}

	if( false == m_pclIndSelTRVParams->m_bIsTrvTypePreset )
	{
		// CASE 1-2: Hide Dp full opening as soon as a valve is not presettable.
		pclSSheet->ShowCol( CD_SupplyValve_DpFullOpening, FALSE );
	}

	// Hide columns corresponding to user selected combos.
	pclSSheet->ShowCol( CD_SupplyValve_Connection, ( true == m_pclIndSelTRVParams->m_strComboSVConnectID.IsEmpty() ) ? TRUE : FALSE );
	pclSSheet->ShowCol( CD_SupplyValve_Version, ( true == m_pclIndSelTRVParams->m_strComboSVVersionID.IsEmpty() ) ? TRUE : FALSE );

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Row name
	
	// Add the main title.
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)GetTitleBackgroundColor( pclSheetDescriptionSupplyValve ) );
	pclSheetDescriptionSupplyValve->SetUserVariable( _SDUV_TITLEFORECOLOR, ( pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor ) ) );
	pclSheetDescriptionSupplyValve->SetUserVariable( _SDUV_TITLEBACKCOLOR, ( pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor ) ) );

	pclSSheet->AddCellSpanW( CD_SupplyValve_CheckBox, RD_SupplyValve_GroupName, CD_SupplyValve_Separator - CD_SupplyValve_CheckBox, 1 );
	pclSSheet->SetStaticText( CD_SupplyValve_CheckBox, RD_SupplyValve_GroupName, IDS_SSHEETSSELTRV_SVGROUP );
	pclSSheet->AddCellSpanW( CD_SupplyValve_PipeSize, RD_SupplyValve_GroupName, CD_SupplyValve_Pointer - CD_SupplyValve_PipeSize, 1 );
	
	CTable *pclTable = dynamic_cast<CTable *>( m_pclIndSelTRVParams->m_pPipeDB->Get( m_pclIndSelTRVParams->m_strPipeSeriesID ).MP );
	ASSERT( NULL != pclTable );

	pclSSheet->SetStaticText( CD_SupplyValve_PipeSize, RD_SupplyValve_GroupName, pclTable->GetName() );
	
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

	pclSSheet->SetStaticText( CD_SupplyValve_Name, RD_SupplyValve_ColName, IDS_SSHEETSSEL_NAME );
	pclSSheet->SetStaticText( CD_SupplyValve_Connection, RD_SupplyValve_ColName, IDS_SSHEETSSEL_CONNECT );
	pclSSheet->SetStaticText( CD_SupplyValve_Version, RD_SupplyValve_ColName, IDS_SSHEETSSEL_VERSION );
	pclSSheet->SetStaticText( CD_SupplyValve_PN, RD_SupplyValve_ColName, IDS_SSHEETSSEL_PN );
	pclSSheet->SetStaticText( CD_SupplyValve_Size, RD_SupplyValve_ColName, IDS_SSHEETSSEL_SIZE );
	pclSSheet->SetStaticText( CD_SupplyValve_Setting, RD_SupplyValve_ColName, IDS_SSHEETSSEL_PRESET );
	pclSSheet->SetStaticText( CD_SupplyValve_Dp, RD_SupplyValve_ColName, IDS_SSHEETSSEL_DP );
	
	// For Kv column, we need to know if we are working with thermostatic head (Kv@2K) or another (KvS).
	int iTextID = ( true == m_pclIndSelTRVParams->m_bIsThermostaticHead ) ? IDS_SSHEETSSELTRV_COLKVAT2K : IDS_SSHEETSSELTRV_COLKVS;
	pclSSheet->SetStaticText( CD_SupplyValve_Kv, RD_SupplyValve_ColName, iTextID );

	pclSSheet->SetStaticText( CD_SupplyValve_DpFullOpening, RD_SupplyValve_ColName, IDS_SSHEETSSEL_DPFO );
	pclSSheet->SetStaticText( CD_SupplyValve_TemperatureRange, RD_SupplyValve_ColName, IDS_SSHEETSSEL_COLTEMPRANGE );

	pclSSheet->SetStaticText( CD_SupplyValve_Separator, RD_SupplyValve_ColName, _T("") );

	// Pipes.
	pclSSheet->SetStaticText( CD_SupplyValve_PipeSize, RD_SupplyValve_ColName, IDS_SSHEETSSEL_PIPESIZE );
	pclSSheet->SetStaticText( CD_SupplyValve_PipeLinDp, RD_SupplyValve_ColName, IDS_SSHEETSSEL_PIPELINDP );
	pclSSheet->SetStaticText( CD_SupplyValve_PipeV, RD_SupplyValve_ColName, IDS_SSHEETSSEL_PIPEV );
	//
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Row units
	CUnitDatabase* pUnitDB = CDimValue::AccessUDB();
	pclSSheet->SetStaticText( CD_SupplyValve_Setting, RD_SupplyValve_Unit, IDS_SHEETHDR_TURNSPOS );
	pclSSheet->SetStaticText( CD_SupplyValve_Dp, RD_SupplyValve_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ) ).c_str() );
	pclSSheet->SetStaticText( CD_SupplyValve_DpFullOpening, RD_SupplyValve_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ) ).c_str() );
	pclSSheet->SetStaticText( CD_SupplyValve_TemperatureRange, RD_SupplyValve_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_TEMPERATURE ) ).c_str() );
	pclSSheet->SetStaticText( CD_SupplyValve_PipeLinDp, RD_SupplyValve_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_LINPRESSDROP ) ).c_str() );
	pclSSheet->SetStaticText( CD_SupplyValve_PipeV, RD_SupplyValve_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_VELOCITY ) ).c_str() );
	//
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Draw black line below column header.
	pclSSheet->SetCellBorder( CD_SupplyValve_CheckBox, RD_SupplyValve_Unit, CD_SupplyValve_Separator - 1, RD_SupplyValve_Unit, true, SS_BORDERTYPE_BOTTOM );
	pclSSheet->SetCellBorder( CD_SupplyValve_PipeSize, RD_SupplyValve_Unit, CD_SupplyValve_Pointer - 1, RD_SupplyValve_Unit, true, SS_BORDERTYPE_BOTTOM );
}

long CRViewSSelTrv::_FillOneSupplyValveRow( CSheetDescription *pclSheetDescriptionSupplyValve, CSSheet *pclSSheet, CSelectedValve *pSelectedTAP, 
		long lRow, CDB_TAProduct *pEditedSupplyValve )
{
	if( NULL == m_pclIndSelTRVParams || NULL == m_pclIndSelTRVParams->m_pTADB || NULL == m_pclIndSelTRVParams->m_pclSelectSupplyValveList 
			|| NULL == m_pclIndSelTRVParams->m_pclSelectSupplyValveList->GetSelectPipeList() )
	{
		ASSERTA_RETURN( 0 );
	}

	CDS_TechnicalParameter *pTechParam = m_pclIndSelTRVParams->m_pTADS->GetpTechParams();
	CDB_ThermostaticValve *pSupplyValve = dynamic_cast<CDB_ThermostaticValve *>( pSelectedTAP->GetpData() );

	CString str;
	long lRetRow = 0;
	bool bAtLeastOneError = false;
	bool bBest = pSelectedTAP->IsFlagSet(CSelectedBase::eBest );

	// Add this row.
	// 'true' to specify that this row can be selected.
	pclSheetDescriptionSupplyValve->AddRows( 1, true );

	// If user wants a Trv-head set, we add an opened box in front of the line.
	if( true == pSupplyValve->IsPartOfaSet() )
	{
		CCDButtonBox *pCDButtonBox = CreateCellDescriptionBox( CD_SupplyValve_Box, lRow, true, CCDButtonBox::ButtonState::BoxOpened, 
				pclSheetDescriptionSupplyValve );

		if( NULL != pCDButtonBox )
		{
			pCDButtonBox->ApplyInternalChange();
		}
	}

	// First columns will be set at the end!

	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

	// m_bIsTrvTypePreset = m_pclIndSelTRVParams->m_bIsTrvTypePreset
	// m_bDpEnabled = m_pclIndSelTRVParams->m_bDpEnabled
	// +-------+--------------------+--------------+---------+-----+---------------+
	// | Case  | m_bIsTrvTypePreset | m_bDpEnabled | Setting | Dp  | DpFullOpening |
	// +-------+--------------------+--------------+---------+-----+---------------+
	// |   1   |       0		    |      0       |    0    |  1  |       0       |
	// |   2   |       0		    |      1       |    0    |  1  |       0       |
	// |   3   |       1		    |      0       |    1    |  0  |       1       |
	// |   4   |       1		    |      1       |    1    |  1  |       1       |
	// +-------+--------------------+--------------+---------+-----+---------------+

	// Prepare Dp full opening.
	CString strDpFO = WriteDouble( CDimValue::SItoCU( _U_DIFFPRESS, pSelectedTAP->GetDpFullOpen() ), 3, 0 );

	// Set Setting and Dp.
	if( true == m_pclIndSelTRVParams->m_bDpEnabled && true == m_pclIndSelTRVParams->m_bIsTrvTypePreset 
			&& RadiatorReturnValveMode::RRVM_IMI == m_pclIndSelTRVParams->m_eReturnValveMode && true == m_rSuggestParams.m_bIsDifferentDpOnRVExist 
			&& false == pSelectedTAP->IsFlagSet( CSelectedBase::eValveFullODp ) )
	{
		// CASE 4 with empty values. We have return valves with different pressure drop. It it thus impossible at now to compute setting and Dp on
		// the supply valves.
		// Remark: why last condition? Because if pressure drop on supply valve when fully opened is already bigger than the Dp user wants, it is 
		//         impossible to do better. Than in this case we show setting and Dp.
		pclSSheet->SetStaticText( CD_SupplyValve_Setting, lRow, _T("-") );
		pclSSheet->SetStaticText( CD_SupplyValve_Dp, lRow, _T("-") );
		pclSSheet->SetStaticText( CD_SupplyValve_DpFullOpening, lRow, strDpFO );
		m_bMustFillSVSettingDp = true;
	}
	else
	{
		// Case 2 and 4: write setting.
		if( true == m_pclIndSelTRVParams->m_bIsTrvTypePreset && NULL != pSupplyValve->GetThermoCharacteristic() )
		{
			str = pSupplyValve->GetThermoCharacteristic()->GetSettingString( pSelectedTAP->GetH() );

			if( true == pSelectedTAP->IsFlagSet( CSelectedBase::eValveMaxSetting ) )
			{
				pclSheetDescriptionSupplyValve->WriteTextWithFlags( str, CD_SupplyValve_Setting, lRow, CSheetDescription::RemarkFlags::FullOpening );
			}
			else
			{
				// Opening.
				if( true == pSelectedTAP->IsFlagSet( CSelectedBase::eValveSetting ) )
				{
					pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_ORANGE );
				}
				
				pclSSheet->SetStaticText( CD_SupplyValve_Setting, lRow, str );
				pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );
			}
		}

		// Case 1, 3 and 4: write Dp.
		if( true == m_pclIndSelTRVParams->m_bDpEnabled || false == m_pclIndSelTRVParams->m_bIsTrvTypePreset )
		{
			if( true == pSelectedTAP->IsFlagSet( CSelectedBase::eDp ) || true == pSelectedTAP->IsFlagSet( CSelectedBase::eValveFullODp ) )
			{
				// Remark: We don't set in red with supply valves that are not presettable and when choosing a specific Dp.
				//         In that case, we have computed Dp on supply valve at full opening and we don't show at this moment in red. Because it should
				//         be possible to reach Dp wanted with pressure drop on return valve.
				//         But if pressure drop is already above the wanted Dp, it's impossible to find a solution!
				double dDpTotal = m_pclIndSelTRVParams->m_dDp;
				
				if( !( true == m_pclIndSelTRVParams->m_bDpEnabled && false == m_pclIndSelTRVParams->m_bIsTrvTypePreset 
						&& true == m_rSuggestParams.m_bIsDifferentDpOnSVExist ) || pSelectedTAP->GetDp() > dDpTotal )
				{
					pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_ORANGE );

					// In this particular case, we change flag error to well show message in 'OnTextTipFetch'.
					if( true == m_pclIndSelTRVParams->m_bDpEnabled && pSelectedTAP->GetDp() > dDpTotal )
					{
						pSelectedTAP->SetFlag( CSelectedBase::Flags::eDp, true, CSelectTrvList::FlagError::DpAboveUser );
					}
				}
			}
			
			str = WriteCUDouble( _U_DIFFPRESS, pSelectedTAP->GetDp() );
			pclSSheet->SetStaticText( CD_SupplyValve_Dp, lRow, str );
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );
		}

		// Case 2 and 4: write Dp full opening.
		if( true == m_pclIndSelTRVParams->m_bIsTrvTypePreset )
		{
			pclSSheet->SetStaticText( CD_SupplyValve_DpFullOpening, lRow, strDpFO );
		}
	}

	// Add checkbox.
	pclSSheet->SetCheckBox( CD_SupplyValve_CheckBox, lRow, _T(""), false, true );

	// Set the Kv at DT 2K or Kvs.
	double dKv = -1.0;
	int iDeltaT = ( true == m_pclIndSelTRVParams->m_bIsThermostaticHead ) ? 2 : 0;

	CDB_ThermoCharacteristic *pclThermoCharacteristic = pSupplyValve->GetThermoCharacteristic();

	if( NULL != pclThermoCharacteristic )
	{
		dKv = pclThermoCharacteristic->GetKv( pSelectedTAP->GetH(), iDeltaT );
	}

	pclSSheet->SetStaticText( CD_SupplyValve_Kv, lRow, ( dKv != -1.0 ) ? WriteCUDouble( _C_KVCVCOEFF, dKv ) : TASApp.LoadLocalizedString( IDS_ERROR ) );

	// Set the temperature range.
	if( true == pSelectedTAP->IsFlagSet( CSelectedBase::eTemperature ) )
	{
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_RED );
		bAtLeastOneError = true;
	}
	
	pclSSheet->SetStaticText( CD_SupplyValve_TemperatureRange, lRow, ((CDB_TAProduct *)pSelectedTAP->GetProductIDPtr().MP)->GetTempRange() );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

	// Now we can set first columns in regards to current status (error, best or normal).
	if( true == bAtLeastOneError )
	{
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_RED );
	}
	else if( true == pSelectedTAP->IsFlagSet( CSelectedBase::eBest ) )
	{
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_GREEN );
	}
	
	// In addition of current flag, we have possibility that valve has the flag 'Not available' or 'Deleted'. In that case, we show valve name in red with "*" around it and
	// symbol '!' or '!!' after.
	if( true == pSupplyValve->IsDeleted() )
	{
		pclSheetDescriptionSupplyValve->WriteTextWithFlags( CString( pSupplyValve->GetName() ), CD_SupplyValve_Name, lRow, 
				CSheetDescription::RemarkFlags::Deleted, _T(""), _RED );
	}
	else if( false == pSupplyValve->IsAvailable() )
	{
		pclSheetDescriptionSupplyValve->WriteTextWithFlags( CString( pSupplyValve->GetName() ), CD_SupplyValve_Name, lRow, 
				CSheetDescription::RemarkFlags::NotAvailable, _T(""), _RED );
	}
	else
	{
		pclSSheet->SetStaticText( CD_SupplyValve_Name, lRow, pSupplyValve->GetName() );
	}

	if( true == bAtLeastOneError )
	{
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );
	}
	else if( true == pSelectedTAP->IsFlagSet( CSelectedBase::eBest ) )
	{
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_GREEN );
	}

	pclSSheet->SetStaticText( CD_SupplyValve_Connection, lRow, pSupplyValve->GetConnect() );
	pclSSheet->SetStaticText( CD_SupplyValve_Version, lRow, pSupplyValve->GetVersion() );
	pclSSheet->SetStaticText( CD_SupplyValve_PN, lRow, pSupplyValve->GetPN().c_str() );
	pclSSheet->SetStaticText( CD_SupplyValve_Size, lRow, pSupplyValve->GetSize() );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

	// Set pipe size.
	CSelectPipe selPipe( m_pclIndSelTRVParams );
	m_pclIndSelTRVParams->m_pclSelectSupplyValveList->GetSelectPipeList()->GetMatchingPipe( pSupplyValve->GetSizeKey(), selPipe );
	pclSSheet->SetStaticText( CD_SupplyValve_PipeSize, lRow, selPipe.GetpPipe()->GetSize( m_pclIndSelTRVParams->m_pTADB ) );
		
	// Set the LinDp to orange if it is above or below the technical parameters limits.
	if( selPipe.GetLinDp() > pTechParam->GetPipeMaxDp() || selPipe.GetLinDp() < pTechParam->GetPipeMinDp())
	{
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_ORANGE );
	}
	
	pclSSheet->SetStaticText( CD_SupplyValve_PipeLinDp, lRow, WriteCUDouble( _U_LINPRESSDROP, selPipe.GetLinDp() ) );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

	// Set the Velocity to orange if it is above the technical parameter limit.
	// Orange if it is below the dMinVel.
	if( selPipe.GetU() > pTechParam->GetPipeMaxVel() || selPipe.GetU() < pTechParam->GetPipeMinVel() )
	{
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_ORANGE );
	}
	
	pclSSheet->SetStaticText( CD_SupplyValve_PipeV, lRow, WriteCUDouble( _U_VELOCITY, selPipe.GetU() ) );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

	// Save parameter.
	CreateCellDescriptionProduct( pclSheetDescriptionSupplyValve->GetFirstParameterColumn(), lRow, (LPARAM)pSupplyValve, pclSheetDescriptionSupplyValve );
	
	if( NULL != pEditedSupplyValve && pEditedSupplyValve == pSupplyValve )
	{
		lRetRow = lRow;
	}

	pclSSheet->SetCellBorder( CD_SupplyValve_CheckBox, lRow, CD_SupplyValve_Separator - 1, lRow, true, SS_BORDERTYPE_BOTTOM,SS_BORDERSTYLE_SOLID, _GRAY );
	pclSSheet->SetCellBorder( CD_SupplyValve_PipeSize, lRow, CD_SupplyValve_Pointer - 1, lRow, true, SS_BORDERTYPE_BOTTOM,SS_BORDERSTYLE_SOLID, _GRAY );

	return lRetRow;
}

long CRViewSSelTrv::_FillSupplyFlowLimitedRows( CDB_TAProduct *pEditedSupplyValve )
{
	if( NULL == m_pclIndSelTRVParams || NULL == m_pclIndSelTRVParams->m_pclSelectSupplyValveList )
	{
		ASSERTA_RETURN( 0 );
	}

	CSelectedBase *pclSelectedProduct = m_pclIndSelTRVParams->m_pclSelectSupplyValveList->GetFirst<CSelectedBase>();

	if( NULL == pclSelectedProduct ) 
	{
		return 0;
	}

	// Call 'MultiSpreadBase' method to get a new SSheet.
	CSheetDescription *pclSheetDescriptionSupplyValve = CreateSSheet( SD_SupplyFlowLimited );

	if( NULL == pclSheetDescriptionSupplyValve || NULL == pclSheetDescriptionSupplyValve->GetSSheetPointer() )
	{
		return 0;
	}
	
	CSSheet *pclSSheet = pclSheetDescriptionSupplyValve->GetSSheetPointer();

	pclSSheet->SetBool( SSB_REDRAW, FALSE );

	pclSSheet->Init();
	pclSSheet->SetBool( SSB_ALLOWDRAGDROP, FALSE );
	pclSSheet->SetBool( SSB_HORZSCROLLBAR, FALSE );
	pclSSheet->SetBool( SSB_VERTSCROLLBAR, FALSE );
	pclSSheet->SetBool( SSB_MOVEACTIVEONFOCUS, FALSE );
	pclSSheet->SetTextTip( SS_TT_STATUS_FLOATING, 250, NULL, _LIGHTYELLOW, _TAH_BLACK );
	
	// Initialize supply flow limited control valve sheet and fill header.
	_InitAndFillSupplyFlowLimitedHeader( pclSheetDescriptionSupplyValve, pclSSheet );

	long lRetRow = -1;
	long lRow = RD_SupplyValve_FirstAvailRow;

	long lValveTotalCount = 0;
	long lValveNotPriorityCount = 0;
	bool bShowAllPrioritiesShown = false;
	long lShowAllPrioritiesButtonRow = 0;
	
	pclSheetDescriptionSupplyValve->SetUserVariable( _SDUV_TOTALPRODUCT_COUNT, 0 );
	pclSheetDescriptionSupplyValve->SetUserVariable( _SDUV_NOPRIORITYPRODUCT_COUNT, 0 );

	pclSheetDescriptionSupplyValve->RestartRemarkGenerator();
	
	for( CSelectedValve *pclSelectedSupplyValve = m_pclIndSelTRVParams->m_pclSelectSupplyValveList->GetFirst<CSelectedValve>(); 
			NULL != pclSelectedSupplyValve; pclSelectedSupplyValve = m_pclIndSelTRVParams->m_pclSelectSupplyValveList->GetNext<CSelectedValve>() )
	{
		CDB_FlowLimitedControlValve *pclSupplyValve = dynamic_cast<CDB_FlowLimitedControlValve *>( pclSelectedSupplyValve->GetpData() );

		if( NULL == pclSupplyValve ) 
		{
			continue;
		}

		if( true == pclSelectedSupplyValve->IsFlagSet(CSelectedBase::eNotPriority) )
		{
			lValveNotPriorityCount++;
			
			if( false == bShowAllPrioritiesShown )
			{
				// Add one empty not selectable row.
				pclSheetDescriptionSupplyValve->AddRows( 1 );
				pclSheetDescriptionSupplyValve->RemoveSelectableRow( lRow );

				pclSSheet->SetCellBorder( CD_SupplyFlowLimited_CheckBox, lRow, CD_SupplyFlowLimited_Separator - 1, lRow, true, 
						SS_BORDERTYPE_TOP | SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );

				pclSSheet->SetCellBorder( CD_SupplyFlowLimited_PipeSize, lRow, CD_SupplyFlowLimited_Pointer - 1, lRow, true, 
						SS_BORDERTYPE_TOP | SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );

				lShowAllPrioritiesButtonRow = lRow++;
				bShowAllPrioritiesShown = true;
			}
		}

		long lRetVal = _FillOneSupplyFlowLimitedRow( pclSheetDescriptionSupplyValve, pclSSheet, pclSelectedSupplyValve, lRow, pEditedSupplyValve );
		
		if( lRetVal > 0 )
		{
			lRetRow = lRetVal;
		}

		lRow++;
		lValveTotalCount++;
	}

	long lLastDataRow = lRow - 1;

	pclSheetDescriptionSupplyValve->SetUserVariable( _SDUV_TOTALPRODUCT_COUNT, lValveTotalCount );
	pclSheetDescriptionSupplyValve->SetUserVariable( _SDUV_NOPRIORITYPRODUCT_COUNT, lValveNotPriorityCount );

	// Now we can create Show/Hide all priorities if needed.
	// Remarks: 2nd condition to avoid to have button if we have only no priority valves.
	if( true == bShowAllPrioritiesShown && lValveTotalCount > lValveNotPriorityCount )
	{
		// Create Show/Hide all priorities button in regards to current state!
		CCDButtonShowAllPriorities::ButtonState eState = ( true == GetShowAllPrioritiesFlag( SD_SupplyFlowLimited ) ) ? 
				CCDButtonShowAllPriorities::ButtonState::HidePriorities : CCDButtonShowAllPriorities::ButtonState::ShowPriorities;

		CCDButtonShowAllPriorities *pclShowAllButton = CreateShowAllPrioritiesButton( CD_SupplyFlowLimited_FirstColumn, lShowAllPrioritiesButtonRow, 
				bShowAllPrioritiesShown, eState, lShowAllPrioritiesButtonRow + 1, pclSSheet->GetMaxRows(), pclSheetDescriptionSupplyValve );

		// Here we must call 'ApplyInternalChange' to show or hide rows in regards to the current state!
		if( NULL != pclShowAllButton )
		{
			pclShowAllButton->ApplyInternalChange( );
		}
	}
	else
	{
		if( lShowAllPrioritiesButtonRow != 0 )
		{
			pclSSheet->ShowRow( lShowAllPrioritiesButtonRow, false );
		}
	}
	
	// Try to merge only if there is more than one supply valve.
	if( lValveTotalCount > 2 || ( 2 == lValveTotalCount && lValveNotPriorityCount != 1 ) )
	{
		vector<long> vecColumnList;
		vecColumnList.push_back( CD_SupplyFlowLimited_PipeSize );
		vecColumnList.push_back( CD_SupplyFlowLimited_PipeLinDp );
		vecColumnList.push_back( CD_SupplyFlowLimited_PipeV );
		pclSSheet->MergeRowsWithSameValues( RD_SupplyValve_FirstAvailRow, lLastDataRow, vecColumnList );
	}

	pclSSheet->SetCellBorder( CD_SupplyFlowLimited_CheckBox, lLastDataRow, CD_SupplyFlowLimited_Separator - 1, lLastDataRow, true, 
			SS_BORDERTYPE_BOTTOM,SS_BORDERSTYLE_SOLID, _BLACK );

	// Needed by pipe merging.
	pclSSheet->SetCellBorder( CD_SupplyFlowLimited_PipeSize, lLastDataRow, CD_SupplyFlowLimited_Pointer - 1, lLastDataRow, true, 
			SS_BORDERTYPE_BOTTOM,SS_BORDERSTYLE_SOLID, _BLACK );

	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

	// By default we add one row for remark.
	// Remark: 'false' to specify that this row can't be selected.
	pclSheetDescriptionSupplyValve->AddRows( 1, false );

	pclSSheet->AddCellSpanW( CD_SupplyFlowLimited_CheckBox, lRow, CD_SupplyFlowLimited_Separator - CD_SupplyFlowLimited_CheckBox, 1 );
	pclSSheet->AddCellSpanW( CD_SupplyFlowLimited_CheckBox, lRow + 1, CD_SupplyFlowLimited_Separator - CD_SupplyFlowLimited_CheckBox, 1 );
	
	// Add Remarks.
	// Note: these lines must be placed after creating Show/Hide all priorities button to avoid to take in count these lines when defining
	//       range of row to be Show/Hide.
	pclSheetDescriptionSupplyValve->SetUserVariable( _SDUV_REMARKROW, lRow );
	lRow = pclSheetDescriptionSupplyValve->WriteRemarks( lRow, CD_SupplyFlowLimited_CheckBox, CD_SupplyFlowLimited_Separator );

	pclSSheet->SetBool( SSB_REDRAW, TRUE );

	// Set that there is no selection at now.
	SetCurrentSupplyValveSelected( NULL );

	// Add possibility to change column size.
	// Remark: place this code after any initialization (like adding, removing, resizing, hiding/showing columns).
	CResizingColumnInfo *pclResizingColumnInfo = pclSheetDescriptionSupplyValve->GetResizingColumnInfo();
	pclResizingColumnInfo->ActivateFeature( true, false, RD_SupplyValve_FirstAvailRow, lLastDataRow );
	pclResizingColumnInfo->AddRangeColumn( CD_SupplyFlowLimited_Name, CD_SupplyFlowLimited_TemperatureRange, RD_SupplyValve_ColName, RD_SupplyValve_Unit );
	pclResizingColumnInfo->AddRangeColumn( CD_SupplyFlowLimited_PipeSize, CD_SupplyFlowLimited_PipeV, RD_SupplyValve_ColName, RD_SupplyValve_Unit );

	// Add possibility to click on title group to reset column width.
	CreateTitleGroupButtons( CD_SupplyFlowLimited_CheckBox, CD_SupplyFlowLimited_PipeV, RD_SupplyValve_GroupName, pclSheetDescriptionSupplyValve );

	return lRetRow;
}

void CRViewSSelTrv::_InitAndFillSupplyFlowLimitedHeader( CSheetDescription *pclSheetDescriptionSupplyValve, CSSheet *pclSSheet )
{
	if( NULL == m_pclIndSelTRVParams || NULL == m_pclIndSelTRVParams->m_pPipeDB )
	{
		ASSERT_RETURN;
	}

	// No need to do sanity check, it is already done in '_FillSupplyFlowLimitedRows'.

	// Set the title.
	pclSSheet->SetMaxRows( RD_SupplyValve_FirstAvailRow - 1 );
	
	// All cells are static by default and filled with _T("").
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	pclSSheet->FormatStaticText( -1, -1, -1, -1, _T("") );

	// Increase row height.
	double dRowHeight = 12.75;
	pclSSheet->SetRowHeight( RD_SupplyValve_FirstRow, dRowHeight * 0.5 );
	pclSSheet->SetRowHeight( RD_SupplyValve_GroupName, dRowHeight * 1.5 );
	pclSSheet->SetRowHeight( RD_SupplyValve_ColName, dRowHeight * 2 );
	pclSSheet->SetRowHeight( RD_SupplyValve_Unit, dRowHeight * 1.2 );
	
	// Set columns.
	pclSheetDescriptionSupplyValve->Init( 1, pclSSheet->GetMaxRows(), 1, CSheetDescription::SD_ParameterMode_Mono );
	pclSheetDescriptionSupplyValve->AddColumnInPixels( CD_SupplyFlowLimited_FirstColumn, m_mapSSheetColumnWidth[SD_SupplyFlowLimited][CD_SupplyFlowLimited_FirstColumn] );
	pclSheetDescriptionSupplyValve->AddColumnInPixels( CD_SupplyFlowLimited_CheckBox, m_mapSSheetColumnWidth[SD_SupplyFlowLimited][CD_SupplyFlowLimited_CheckBox] );
	pclSheetDescriptionSupplyValve->AddColumnInPixels( CD_SupplyFlowLimited_Name, m_mapSSheetColumnWidth[SD_SupplyFlowLimited][CD_SupplyFlowLimited_Name] );
	pclSheetDescriptionSupplyValve->AddColumnInPixels( CD_SupplyFlowLimited_Connection, m_mapSSheetColumnWidth[SD_SupplyFlowLimited][CD_SupplyFlowLimited_Connection] );
	pclSheetDescriptionSupplyValve->AddColumnInPixels( CD_SupplyFlowLimited_Version, m_mapSSheetColumnWidth[SD_SupplyFlowLimited][CD_SupplyFlowLimited_Version] );
	pclSheetDescriptionSupplyValve->AddColumnInPixels( CD_SupplyFlowLimited_PN, m_mapSSheetColumnWidth[SD_SupplyFlowLimited][CD_SupplyFlowLimited_PN] );
	pclSheetDescriptionSupplyValve->AddColumnInPixels( CD_SupplyFlowLimited_Size, m_mapSSheetColumnWidth[SD_SupplyFlowLimited][CD_SupplyFlowLimited_Size] );
	pclSheetDescriptionSupplyValve->AddColumnInPixels( CD_SupplyFlowLimited_Setting, m_mapSSheetColumnWidth[SD_SupplyFlowLimited][CD_SupplyFlowLimited_Setting] );
	pclSheetDescriptionSupplyValve->AddColumnInPixels( CD_SupplyFlowLimited_Dpmin, m_mapSSheetColumnWidth[SD_SupplyFlowLimited][CD_SupplyFlowLimited_Dpmin] );
	pclSheetDescriptionSupplyValve->AddColumnInPixels( CD_SupplyFlowLimited_FlowRange, m_mapSSheetColumnWidth[SD_SupplyFlowLimited][CD_SupplyFlowLimited_FlowRange] );
	pclSheetDescriptionSupplyValve->AddColumnInPixels( CD_SupplyFlowLimited_TemperatureRange, m_mapSSheetColumnWidth[SD_SupplyFlowLimited][CD_SupplyFlowLimited_TemperatureRange] );
	pclSheetDescriptionSupplyValve->AddColumnInPixels( CD_SupplyFlowLimited_Separator, m_mapSSheetColumnWidth[SD_SupplyFlowLimited][CD_SupplyFlowLimited_Separator] );
	pclSheetDescriptionSupplyValve->AddColumnInPixels( CD_SupplyFlowLimited_PipeSize, m_mapSSheetColumnWidth[SD_SupplyFlowLimited][CD_SupplyFlowLimited_PipeSize] );
	pclSheetDescriptionSupplyValve->AddColumnInPixels( CD_SupplyFlowLimited_PipeLinDp, m_mapSSheetColumnWidth[SD_SupplyFlowLimited][CD_SupplyFlowLimited_PipeLinDp] );
	pclSheetDescriptionSupplyValve->AddColumnInPixels( CD_SupplyFlowLimited_PipeV, m_mapSSheetColumnWidth[SD_SupplyFlowLimited][CD_SupplyFlowLimited_PipeV] );
	pclSheetDescriptionSupplyValve->AddColumnInPixels( CD_SupplyFlowLimited_Pointer, m_mapSSheetColumnWidth[SD_SupplyFlowLimited][CD_SupplyFlowLimited_Pointer] );

	// Set in which column parameter must be saved.
	pclSheetDescriptionSupplyValve->AddParameterColumn( CD_SupplyFlowLimited_Pointer );
	
	// Set the focus column.
	pclSheetDescriptionSupplyValve->SetActiveColumn( CD_SupplyFlowLimited_Name );

	// Set range for selection.
	pclSheetDescriptionSupplyValve->SetFocusColumnRange( CD_SupplyFlowLimited_CheckBox, CD_SupplyFlowLimited_TemperatureRange );

	// Hide columns corresponding to user selected combos.
	pclSSheet->ShowCol( CD_SupplyFlowLimited_Connection, ( m_pclIndSelTRVParams->m_strComboSVConnectID.IsEmpty() ) ? TRUE : FALSE );
	pclSSheet->ShowCol( CD_SupplyFlowLimited_Version, ( m_pclIndSelTRVParams->m_strComboSVVersionID.IsEmpty() ) ? TRUE : FALSE );

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Row names.
	
	// Add the main title.
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)GetTitleBackgroundColor( pclSheetDescriptionSupplyValve ) );
	pclSheetDescriptionSupplyValve->SetUserVariable( _SDUV_TITLEFORECOLOR, ( pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor ) ) );
	pclSheetDescriptionSupplyValve->SetUserVariable( _SDUV_TITLEBACKCOLOR, ( pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor ) ) );

	pclSSheet->AddCellSpanW( CD_SupplyFlowLimited_Name, RD_SupplyValve_GroupName, CD_SupplyFlowLimited_Separator - CD_SupplyFlowLimited_Name, 1 );
	pclSSheet->SetStaticText( CD_SupplyFlowLimited_Name, RD_SupplyValve_GroupName, IDS_SSHEETSSELTRV_SVGROUP );
	pclSSheet->AddCellSpanW( CD_SupplyFlowLimited_PipeSize, RD_SupplyValve_GroupName, CD_SupplyFlowLimited_Pointer - CD_SupplyFlowLimited_PipeSize, 1 );
	
	CTable *pclTable = dynamic_cast<CTable *>( m_pclIndSelTRVParams->m_pPipeDB->Get( m_pclIndSelTRVParams->m_strPipeSeriesID ).MP );
	ASSERT( NULL != pclTable );

	pclSSheet->SetStaticText( CD_SupplyFlowLimited_PipeSize, RD_SupplyValve_GroupName, pclTable->GetName() );
	
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

	pclSSheet->SetStaticText( CD_SupplyFlowLimited_Name, RD_SupplyValve_ColName, IDS_SSHEETSSEL_NAME );
	pclSSheet->SetStaticText( CD_SupplyFlowLimited_Connection, RD_SupplyValve_ColName, IDS_SSHEETSSEL_CONNECT );
	pclSSheet->SetStaticText( CD_SupplyFlowLimited_Version, RD_SupplyValve_ColName, IDS_SSHEETSSEL_VERSION );
	pclSSheet->SetStaticText( CD_SupplyFlowLimited_PN, RD_SupplyValve_ColName, IDS_SSHEETSSEL_PN );
	pclSSheet->SetStaticText( CD_SupplyFlowLimited_Size, RD_SupplyValve_ColName, IDS_SSHEETSSEL_SIZE );
	pclSSheet->SetStaticText( CD_SupplyFlowLimited_Setting, RD_SupplyValve_ColName, IDS_SSHEETSSEL_PRESET );
	pclSSheet->SetStaticText( CD_SupplyFlowLimited_Dpmin, RD_SupplyValve_ColName, IDS_SHEETHDR_DPMIN );
	pclSSheet->SetStaticText( CD_SupplyFlowLimited_FlowRange, RD_SupplyValve_ColName, IDS_SSHEETSSELTRV_FLOWRANGE );
	pclSSheet->SetStaticText( CD_SupplyFlowLimited_TemperatureRange, RD_SupplyValve_ColName, IDS_SSHEETSSEL_COLTEMPRANGE );

	pclSSheet->SetStaticText( CD_SupplyFlowLimited_Separator, RD_SupplyValve_ColName, _T("") );

	// Pipes.
	pclSSheet->SetStaticText( CD_SupplyFlowLimited_PipeSize, RD_SupplyValve_ColName, IDS_SSHEETSSEL_PIPESIZE );
	pclSSheet->SetStaticText( CD_SupplyFlowLimited_PipeLinDp, RD_SupplyValve_ColName, IDS_SSHEETSSEL_PIPELINDP );
	pclSSheet->SetStaticText( CD_SupplyFlowLimited_PipeV, RD_SupplyValve_ColName, IDS_SSHEETSSEL_PIPEV );
	//
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Row units.
	CUnitDatabase* pUnitDB = CDimValue::AccessUDB();
	pclSSheet->SetStaticText( CD_SupplyFlowLimited_Setting, RD_SupplyValve_Unit, IDS_SHEETHDR_TURNSPOS );
	pclSSheet->SetStaticText( CD_SupplyFlowLimited_Dpmin, RD_SupplyValve_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ) ).c_str() );
	pclSSheet->SetStaticText( CD_SupplyFlowLimited_FlowRange, RD_SupplyValve_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_FLOW ) ).c_str() );
	pclSSheet->SetStaticText( CD_SupplyFlowLimited_TemperatureRange, RD_SupplyValve_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_TEMPERATURE ) ).c_str() );
	pclSSheet->SetStaticText( CD_SupplyFlowLimited_PipeLinDp, RD_SupplyValve_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_LINPRESSDROP ) ).c_str() );
	pclSSheet->SetStaticText( CD_SupplyFlowLimited_PipeV, RD_SupplyValve_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_VELOCITY ) ).c_str() );
	//
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Draw black line below column header.
	pclSSheet->SetCellBorder( CD_SupplyFlowLimited_CheckBox, RD_SupplyValve_Unit, CD_SupplyFlowLimited_Separator - 1, RD_SupplyValve_Unit, true, 
			SS_BORDERTYPE_BOTTOM );

	pclSSheet->SetCellBorder( CD_SupplyFlowLimited_PipeSize, RD_SupplyValve_Unit, CD_SupplyFlowLimited_Pointer - 1, RD_SupplyValve_Unit, true, 
			SS_BORDERTYPE_BOTTOM );
}

long CRViewSSelTrv::_FillOneSupplyFlowLimitedRow( CSheetDescription *pclSheetDescriptionSupplyValve, CSSheet *pclSSheet, 
		CSelectedValve *pSelectedTAP, long lRow, CDB_TAProduct *pEditedSupplyValve )
{
	if( NULL == m_pclIndSelTRVParams || NULL == m_pclIndSelTRVParams->m_pTADB || NULL == m_pclIndSelTRVParams->m_pclSelectSupplyValveList
			|| NULL == m_pclIndSelTRVParams->m_pclSelectSupplyValveList->GetSelectPipeList() )
	{
		ASSERTA_RETURN( 0 );
	}

	CDS_TechnicalParameter *pTechParam = m_pclIndSelTRVParams->m_pTADS->GetpTechParams();
	CDB_FlowLimitedControlValve *pSupplyValve = dynamic_cast<CDB_FlowLimitedControlValve *>( pSelectedTAP->GetpData() );

	CString str;
	long lRetRow = 0;
	bool bAtLeastOneError = false;
	bool bBest = pSelectedTAP->IsFlagSet(CSelectedBase::eBest );

	// Add this row.
	// 'true' to specify that this row can be selected.
	pclSheetDescriptionSupplyValve->AddRows( 1, true );

	// First columns will be set at the end!

	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

	// Set setting.
	if( NULL != pSupplyValve->GetFLCVCharacteristic() )
	{
		str = pSupplyValve->GetFLCVCharacteristic()->GetSettingString( pSelectedTAP->GetH() );

		if( true == pSelectedTAP->IsFlagSet( CSelectedBase::eValveMaxSetting ) )
		{
			pclSheetDescriptionSupplyValve->WriteTextWithFlags( str, CD_SupplyFlowLimited_Setting, lRow, CSheetDescription::RemarkFlags::FullOpening );
		}
		else
		{
			// Opening.
			pclSSheet->SetStaticText( CD_SupplyFlowLimited_Setting, lRow, str );
		}
	}

	// Add checkbox.
	pclSSheet->SetCheckBox( CD_SupplyFlowLimited_CheckBox, lRow, _T(""), false, true );

	// Set Dpmin.
	double dDpmin = -1.0;
	CDB_FLCVCharacteristic *pclFLCVCharacteristic = pSupplyValve->GetFLCVCharacteristic();

	if( NULL != pclFLCVCharacteristic )
	{
		dDpmin = pclFLCVCharacteristic->GetDpmin( pSelectedTAP->GetH() );
	}

	pclSSheet->SetStaticText( CD_SupplyFlowLimited_Dpmin, lRow, ( dDpmin != -1.0 ) ? WriteCUDouble( _U_DIFFPRESS, dDpmin ) : GetDashDotDash() );

	// Set flow range.
	CString strFlowRange( GetDashDotDash() );
	
	if( NULL != pclFLCVCharacteristic )
	{
		CString strQLFMin = WriteCUDouble( _U_FLOW, pclFLCVCharacteristic->GetQLFmin() );
		CString strQNFmax = WriteCUDouble( _U_FLOW, pclFLCVCharacteristic->GetQNFmax() );
		strFlowRange.Format( _T("%s/%s"), strQLFMin, strQNFmax );

		if( m_pclIndSelTRVParams->m_dFlow < pclFLCVCharacteristic->GetQLFmin() || m_pclIndSelTRVParams->m_dFlow > pclFLCVCharacteristic->GetQNFmax() )
		{
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_ORANGE );
		}
	}
	
	pclSSheet->SetStaticText( CD_SupplyFlowLimited_FlowRange, lRow, strFlowRange );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

	// Set the temperature range.
	if( true == pSelectedTAP->IsFlagSet( CSelectedBase::eTemperature ) )
	{
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_RED );
		bAtLeastOneError = true;
	}
	
	pclSSheet->SetStaticText( CD_SupplyFlowLimited_TemperatureRange, lRow, ((CDB_TAProduct *)pSelectedTAP->GetProductIDPtr().MP)->GetTempRange() );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

	// Now we can set first columns in regards to current status (error, best or normal).
	if( true == bAtLeastOneError )
	{
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_RED );
	}
	else if( true == pSelectedTAP->IsFlagSet( CSelectedBase::eBest ) )
	{
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_GREEN );
	}
	
	// In addition of current flag, we have possibility that valve has the flag 'Not available' or 'Deleted'.
	// In that case, we show valve name in red with "*" around it and symbol '!' or '!!' after.
	if( true == pSupplyValve->IsDeleted() )
	{
		pclSheetDescriptionSupplyValve->WriteTextWithFlags( CString( pSupplyValve->GetName() ), CD_SupplyFlowLimited_Name, lRow, 
				CSheetDescription::RemarkFlags::Deleted, _T(""), _RED );
	}
	else if( false == pSupplyValve->IsAvailable() )
	{
		pclSheetDescriptionSupplyValve->WriteTextWithFlags( CString( pSupplyValve->GetName() ), CD_SupplyFlowLimited_Name, lRow, 
				CSheetDescription::RemarkFlags::NotAvailable, _T(""), _RED );
	}
	else
	{
		pclSSheet->SetStaticText( CD_SupplyFlowLimited_Name, lRow, pSupplyValve->GetName() );
	}

	if( true == bAtLeastOneError )
	{
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );
	}
	else if( true == pSelectedTAP->IsFlagSet( CSelectedBase::eBest ) )
	{
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor,(LPARAM) _GREEN );
	}

	pclSSheet->SetStaticText( CD_SupplyFlowLimited_Connection, lRow, pSupplyValve->GetConnect() );
	pclSSheet->SetStaticText( CD_SupplyFlowLimited_Version, lRow, pSupplyValve->GetVersion() );
	pclSSheet->SetStaticText( CD_SupplyFlowLimited_PN, lRow, pSupplyValve->GetPN().c_str() );
	pclSSheet->SetStaticText( CD_SupplyFlowLimited_Size, lRow, pSupplyValve->GetSize() );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

	// Set pipe size.
	CSelectPipe selPipe( m_pclIndSelTRVParams );
	m_pclIndSelTRVParams->m_pclSelectSupplyValveList->GetSelectPipeList()->GetMatchingPipe( pSupplyValve->GetSizeKey(), selPipe );
	pclSSheet->SetStaticText( CD_SupplyFlowLimited_PipeSize, lRow, selPipe.GetpPipe()->GetSize( m_pclIndSelTRVParams->m_pTADB ) );
		
	// Set the LinDp to orange if it is above or below the technical parameters limits.
	if( selPipe.GetLinDp() > pTechParam->GetPipeMaxDp() || selPipe.GetLinDp() < pTechParam->GetPipeMinDp())
	{
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_ORANGE );
	}
	
	pclSSheet->SetStaticText( CD_SupplyFlowLimited_PipeLinDp, lRow, WriteCUDouble( _U_LINPRESSDROP, selPipe.GetLinDp() ) );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

	// Set the Velocity to orange if it is above the technical parameter limit.
	// Orange if it is below the dMinVel.
	if( selPipe.GetU() > pTechParam->GetPipeMaxVel() || selPipe.GetU() < pTechParam->GetPipeMinVel() )
	{
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_ORANGE );
	}
	
	pclSSheet->SetStaticText( CD_SupplyFlowLimited_PipeV, lRow, WriteCUDouble( _U_VELOCITY, selPipe.GetU() ) );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

	// Save parameter.
	CreateCellDescriptionProduct( pclSheetDescriptionSupplyValve->GetFirstParameterColumn(), lRow, (LPARAM)pSupplyValve, pclSheetDescriptionSupplyValve );
	
	if( NULL != pEditedSupplyValve && pEditedSupplyValve == pSupplyValve )
	{
		lRetRow = lRow;
	}

	pclSSheet->SetCellBorder( CD_SupplyFlowLimited_CheckBox, lRow, CD_SupplyFlowLimited_Separator - 1, lRow, true, SS_BORDERTYPE_BOTTOM,
			SS_BORDERSTYLE_SOLID, _GRAY );

	pclSSheet->SetCellBorder( CD_SupplyFlowLimited_PipeSize, lRow, CD_SupplyFlowLimited_Pointer - 1, lRow, true, SS_BORDERTYPE_BOTTOM,
			SS_BORDERSTYLE_SOLID, _GRAY );

	return lRetRow;
}

void CRViewSSelTrv::_FillSupplyValveAccessoryRows( )
{
	if( NULL == m_pclIndSelTRVParams || NULL == m_pclIndSelTRVParams->m_pTADB )
	{
		ASSERT_RETURN;
	}

	// Check current supply valve selected.
	CDB_ControlValve *pclCurrentSupplyValveSelected = GetCurrentSupplyValveSelected();

	if( NULL == pclCurrentSupplyValveSelected )
	{
		return;
	}

	CDB_RuledTable *pRuledTable = (CDB_RuledTable *)( pclCurrentSupplyValveSelected->GetAccessoriesGroupIDPtr().MP );

	if( NULL == pRuledTable )
	{
		return;
	}

	CRank rList;
	int iCount = m_pclIndSelTRVParams->m_pTADB->GetAccessories( &rList, pRuledTable, m_pclIndSelTRVParams->m_eFilterSelection );
	
	if( 0 == iCount )
	{
		return;
	}

	// Call 'MultiSpreadBase' method to get a new SSheet.
	CSheetDescription *pclSheetDescriptionSVAccessory = CreateSSheet( SD_SupplyValveAccessories );
	
	if( NULL == pclSheetDescriptionSVAccessory || NULL == pclSheetDescriptionSVAccessory->GetSSheetPointer() )
	{
		return;
	}
	
	CSSheet *pclSSheet = pclSheetDescriptionSVAccessory->GetSSheetPointer();

	pclSSheet->SetBool( SSB_REDRAW, FALSE );

	pclSSheet->Init();
	pclSSheet->SetBool( SSB_ALLOWDRAGDROP, FALSE );
	pclSSheet->SetBool( SSB_HORZSCROLLBAR, FALSE );
	pclSSheet->SetBool( SSB_VERTSCROLLBAR, FALSE );
	pclSSheet->SetTextTip( SS_TT_STATUS_FLOATING, 250, NULL, _LIGHTYELLOW, _TAH_BLACK );

	// Two lines by accessories, but two accessories by line (reason why we don't multiply iCount by 2).
	pclSSheet->SetMaxRows( RD_SupplyValveAccessory_FirstAvailRow + iCount - 1 + ( iCount % 2 ) );

	// All cells are static by default and filled with _T("")
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	pclSSheet->FormatStaticText( -1, -1, -1, -1, _T("") );

	// Get sheet description of supply valve to retrieve width.
	CSheetDescription *pclSheetDescriptionSV = NULL;

	// HYS-1305 : Add insert with automatic flow limiter
	if( ( RVT_WithFlowLimitation != m_pclIndSelTRVParams->m_eValveType )
		&& ( false == m_pclIndSelTRVParams->m_bIsFLCVInsert ) )
	{
		pclSheetDescriptionSV = m_ViewDescription.GetFromSheetDescriptionID( SD_SupplyValve );
	}
	else
	{
		pclSheetDescriptionSV = m_ViewDescription.GetFromSheetDescriptionID( SD_SupplyFlowLimited );
	}

	// It's absolutely not normal to have this pointer NULL.
	ASSERT( NULL != pclSheetDescriptionSV );
	
	// Take SSheet size and client area size and determine the right width to take in count.
	CRect rectSize = pclSheetDescriptionSV->GetSSheetPointer()->GetSheetSizeInPixels( false );

	// Prepare first column width (to match supply valve sheet).
	long lFirstColumnWidth = pclSheetDescriptionSV->GetSSheetPointer()->GetColWidthInPixelsW( CD_SupplyValve_FirstColumn );
	
	// Do some correction in regards to some columns.
	long lTotalWidth = rectSize.Width() - lFirstColumnWidth;
	lTotalWidth -= pclSheetDescriptionSV->GetSSheetPointer()->GetColWidthInPixelsW( CD_SupplyValve_Pointer );

	// Try to create 2 columns in just the middle of supply valve sheet.
	long lLeftWidth = (long)( lTotalWidth / 2 );
	long lRightWidth = ( ( lTotalWidth % 2 ) > 0 ) ? lLeftWidth + 1 : lLeftWidth;

	// Set columns 
	// 'CSheetDescription::SD_ParameterMode_Multi' to set that this sheet can have more than one column with parameter.
	pclSheetDescriptionSVAccessory->Init( 1, pclSSheet->GetMaxRows(), 2, CSheetDescription::SD_ParameterMode_Multi );

	pclSheetDescriptionSVAccessory->AddColumnInPixels( CD_SupplyValveAccessory_FirstColumn, lFirstColumnWidth );
	pclSheetDescriptionSVAccessory->AddColumnInPixels( CD_SupplyValveAccessory_Left, lLeftWidth );
	pclSheetDescriptionSVAccessory->AddColumnInPixels( CD_SupplyValveAccessory_Right, lRightWidth );
	pclSheetDescriptionSVAccessory->AddColumn( CD_SupplyValveAccessory_LastColumn, 0 );
	
	// These two columns can contain parameter.
	pclSheetDescriptionSVAccessory->AddParameterColumn( CD_SupplyValveAccessory_Left );
	pclSheetDescriptionSVAccessory->AddParameterColumn( CD_SupplyValveAccessory_Right );
	
	// Set the focus column (don't set on Left of Right (on a check box) )
	pclSheetDescriptionSVAccessory->SetActiveColumn( CD_SupplyValveAccessory_FirstColumn );
	
	// Set selectable rows.
	pclSheetDescriptionSVAccessory->SetSelectableRangeRow( RD_SupplyValveAccessory_FirstAvailRow, pclSSheet->GetMaxRows() - 1 );

	// Increase row height.
	double RowHeight = 12.75;
	pclSSheet->SetRowHeight( RD_SupplyValveAccessory_FirstRow, RowHeight * 0.5 );
	pclSSheet->SetRowHeight( RD_SupplyValveAccessory_GroupName, RowHeight * 1.5 );

	// Set title.
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)GetTitleBackgroundColor( pclSheetDescriptionSVAccessory ) );

	pclSheetDescriptionSVAccessory->SetUserVariable( _SDUV_TITLEFORECOLOR, ( pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor ) ) );
	pclSheetDescriptionSVAccessory->SetUserVariable( _SDUV_TITLEBACKCOLOR, ( pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor ) ) );

	pclSSheet->AddCellSpanW( CD_SupplyValveAccessory_Left, RD_SupplyValve_GroupName, CD_SupplyValveAccessory_LastColumn - CD_SupplyValveAccessory_Left, 1 );
	pclSSheet->SetStaticText( CD_SupplyValveAccessory_Left, RD_SupplyValve_GroupName, IDS_SSHEETSSELTRV_SVACCGRP );

	// Fill accessories.
	long lRow = RD_SupplyValveAccessory_FirstAvailRow;
	
	CString str;
	LPARAM lparam;

	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_LEFT );

	// Left - Right.
	long lLeftOrRight = CD_SupplyValveAccessory_Left;
	BOOL bContinue = rList.GetFirst( str, lparam );
	
	while( TRUE == bContinue )
	{
		CDB_Product *pAccessory = (CDB_Product *)lparam;
		VERIFY( NULL != pAccessory );
		//VERIFY( 0 != pAccessory->IsClass( CLASS( CDB_Product ) ) );
		
		if( false == pAccessory->IsAnAccessory() )
		{
			continue;
		}

		// Create checkbox accessory.
		CString strName = _T("");

		if( true == pRuledTable->IsByPair( pAccessory->GetIDPtr().ID ) )
		{
			strName += _T("2x ");
		}

		strName += pAccessory->GetName();

		CCDBCheckboxAccessory *pCheckbox = CreateCheckboxAccessory( lLeftOrRight, lRow, false, true, strName, pAccessory, 
				pRuledTable, &m_vecSupplyValveAccessoryList, pclSheetDescriptionSVAccessory );

		if( NULL != pCheckbox )
		{
			pCheckbox->ApplyInternalChange();
		}
		
		// Description.
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleEllipses, (LPARAM)TRUE );
		pclSSheet->SetStaticText( lLeftOrRight, lRow + 1, pAccessory->GetComment() );
		bContinue = rList.GetNext( str, lparam );
		
		// Restart left part.
		if( TRUE == bContinue && lLeftOrRight == CD_SupplyValveAccessory_Right )
		{
			pclSSheet->SetCellBorder( CD_SupplyValveAccessory_Left, lRow + 1, CD_SupplyValveAccessory_LastColumn - 1, lRow + 1, true, 
					SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _GRAY );

			lLeftOrRight = CD_SupplyValveAccessory_Left;
			lRow += 2;
			ASSERT( lRow < pclSSheet->GetMaxRows() );
		}
		else
		{
			lLeftOrRight = CD_SupplyValveAccessory_Right;
		}
		
		if( FALSE == bContinue )
			lRow++;
	}

	VerifyCheckboxAccessories( NULL, false, &m_vecSupplyValveAccessoryList );
	pclSSheet->SetCellBorder( CD_SupplyValveAccessory_Left, lRow, CD_SupplyValveAccessory_LastColumn - 1, lRow, true, SS_BORDERTYPE_BOTTOM, 
			SS_BORDERSTYLE_SOLID, _BLACK );
	
	pclSSheet->SetBool( SSB_REDRAW, TRUE );
}

void CRViewSSelTrv::_FillSupplyValveActuatorRows( void )
{
	CDB_ControlValve *pclSelectedSupplyValve = GetCurrentSupplyValveSelected();

	if( NULL == pclSelectedSupplyValve )
	{
		return;
	}

	// Check if there is a least a group defined.
	CTable *pActuatorGroup = (CTable *)( pclSelectedSupplyValve->GetActuatorGroupIDPtr().MP );

	if( NULL == pActuatorGroup )
	{
		return;
	}

	// Call 'MultiSpreadBase' method to get a new SSheet.
	// Remark: 'm_uiSDActuatorID' was refreshed in 'Suggest' with current selected actuator type in 'DlgIndSelTrv'.
	CSheetDescription *pclSheetDescriptionActuator = CreateSSheet( m_uiSDActuatorID );
	
	if( NULL == pclSheetDescriptionActuator || NULL == pclSheetDescriptionActuator->GetSSheetPointer() )
	{
		return;
	}
	
	CSSheet *pclSSheet = pclSheetDescriptionActuator->GetSSheetPointer();

	pclSSheet->SetBool( SSB_REDRAW, FALSE );

	pclSSheet->Init();
	pclSSheet->SetBool( SSB_ALLOWDRAGDROP, FALSE );
	pclSSheet->SetBool( SSB_HORZSCROLLBAR, FALSE );
	pclSSheet->SetBool( SSB_VERTSCROLLBAR, FALSE );
	pclSSheet->SetBool( SSB_MOVEACTIVEONFOCUS, FALSE );
	pclSSheet->SetTextTip( SS_TT_STATUS_FLOATING, 250, NULL, _LIGHTYELLOW, _TAH_BLACK );

	// Set max rows.
	pclSSheet->SetMaxRows( RD_SVActuator_FirstAvailRow - 1 );

	// All cells are static by default and filled with _T("")
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	pclSSheet->FormatStaticText( -1, -1, -1, -1, _T("") );
	
	// Increase ROW height
	double dRowHeight = 12.75;
	
	pclSSheet->SetRowHeight( RD_SVActuator_FirstRow, dRowHeight * 0.5 );
	pclSSheet->SetRowHeight( RD_SVActuator_GroupName, dRowHeight * 1.5 );
	pclSSheet->SetRowHeight( RD_SVActuator_ColName, dRowHeight * 1.2 );
	pclSSheet->SetRowHeight( RD_SVActuator_Unit, dRowHeight * 1.2 );

	// Initialize.
	pclSheetDescriptionActuator->Init( 1, pclSSheet->GetMaxRows(), 1, CSheetDescription::SD_ParameterMode_Mono );

	if( SD_SVThermoActuator == m_uiSDActuatorID )
	{
		_FillSVThermostaticActuatorRows( pclSheetDescriptionActuator );
	}
	else
	{
		_FillSVElectroActuatorRows( pclSheetDescriptionActuator );
	}
	// Add possibilty to collapse / expand Actuator sheet
	CCDButtonExpandCollapseGroup **ppclGroupButton = NULL;
	ppclGroupButton = &m_pCDBExpandCollapseGroupActuator;
	if (NULL != ppclGroupButton)
	{
		*ppclGroupButton = CreateExpandCollapseGroupButton(CD_SupplyValveAccessory_FirstColumn, RD_SupplyValveAccessory_GroupName, true,
			CCDButtonExpandCollapseGroup::ButtonState::CollapseRow, pclSheetDescriptionActuator->GetFirstSelectableRow() -2,
			pclSheetDescriptionActuator->GetLastSelectableRow(false), pclSheetDescriptionActuator);

		// Show button.
		if (NULL != *ppclGroupButton)
		{
			(*ppclGroupButton)->SetShowStatus( true );
		}
	}
}

void CRViewSSelTrv::_FillSVThermostaticActuatorRows( CSheetDescription *pclSheetDescriptionActuator )
{
	if( NULL == m_pclIndSelTRVParams || NULL == m_pclIndSelTRVParams->m_pTADB )
	{
		ASSERT_RETURN;
	}

	if( NULL == pclSheetDescriptionActuator || NULL == pclSheetDescriptionActuator->GetSSheetPointer() )
	{
		return;
	}

	CSSheet *pclSSheet = pclSheetDescriptionActuator->GetSSheetPointer();
	
	CDB_ControlValve *pclSelectedThermostaticValve = GetCurrentSupplyValveSelected();

	if( NULL == pclSelectedThermostaticValve )
	{
		return;
	}

	CTable *pActuatorGroup = (CTable *)( pclSelectedThermostaticValve->GetActuatorGroupIDPtr().MP );
	
	if( NULL == pActuatorGroup )
	{
		return;
	}

	// Transfer actuator list in a CRank to have the possibility to sort them with the priority level.
	CRank ActuatorList;
	CString strThermoHeadTypeID = m_pclIndSelTRVParams->m_strComboActuatorTypeID;
	CString strThermoHeadFamilyID = m_pclIndSelTRVParams->m_strComboActuatorFamilyID;
	CString strThermoHeadVersionID = m_pclIndSelTRVParams->m_strComboActuatorVersionID;

	for( IDPTR ActrIDPtr = pActuatorGroup->GetFirst(); _T('\0') != *ActrIDPtr.ID; ActrIDPtr = pActuatorGroup->GetNext() )
	{
		CDB_ThermostaticActuator *pThermostaticActuator = dynamic_cast<CDB_ThermostaticActuator *>( (CData *)m_pclIndSelTRVParams->m_pTADB->Get( ActrIDPtr.ID ).MP );

		if( NULL == pThermostaticActuator )
		{
			continue;
		}

		if( false == pThermostaticActuator->IsSelectable( true ) )
		{
			continue;
		}

		if( false == strThermoHeadTypeID.IsEmpty() && 0 != strThermoHeadTypeID.Compare( pThermostaticActuator->GetTypeIDPtr().ID ) )
		{
			continue;
		}

		if( false == strThermoHeadFamilyID.IsEmpty() && 0 != strThermoHeadFamilyID.Compare( pThermostaticActuator->GetFamilyIDPtr().ID ) )
		{
			continue;
		}

		if( false == strThermoHeadVersionID.IsEmpty() && 0 != strThermoHeadVersionID.Compare( pThermostaticActuator->GetVersionIDPtr().ID ) )
		{
			continue;
		}

		ActuatorList.Add( pThermostaticActuator->GetIDPtr().ID, pThermostaticActuator->GetOrderKey(), ( LPARAM )pThermostaticActuator, false, false );
	}

	pclSheetDescriptionActuator->AddColumnInPixels( CD_SVThrmActuator_FirstColumn, m_mapSSheetColumnWidth[SD_SVThermoActuator][CD_SVThrmActuator_FirstColumn] );
	pclSheetDescriptionActuator->AddColumnInPixels( CD_SVThrmActuator_Box, m_mapSSheetColumnWidth[SD_SVThermoActuator][CD_SVThrmActuator_Box] );
	pclSheetDescriptionActuator->AddColumnInPixels( CD_SVThrmActuator_CheckBox, m_mapSSheetColumnWidth[SD_SVThermoActuator][CD_SVThrmActuator_CheckBox] );
	pclSheetDescriptionActuator->AddColumnInPixels( CD_SVThrmActuator_Name, m_mapSSheetColumnWidth[SD_SVThermoActuator][CD_SVThrmActuator_Name] );
	pclSheetDescriptionActuator->AddColumnInPixels( CD_SVThrmActuator_Description, m_mapSSheetColumnWidth[SD_SVThermoActuator][CD_SVThrmActuator_Description] );
	pclSheetDescriptionActuator->AddColumnInPixels( CD_SVThrmActuator_SettingRange, m_mapSSheetColumnWidth[SD_SVThermoActuator][CD_SVThrmActuator_SettingRange] );
	pclSheetDescriptionActuator->AddColumnInPixels( CD_SVThrmActuator_CapillaryLength, m_mapSSheetColumnWidth[SD_SVThermoActuator][CD_SVThrmActuator_CapillaryLength] );
	pclSheetDescriptionActuator->AddColumnInPixels( CD_SVThrmActuator_Hysteresis, m_mapSSheetColumnWidth[SD_SVThermoActuator][CD_SVThrmActuator_Hysteresis] );
	pclSheetDescriptionActuator->AddColumnInPixels( CD_SVThrmActuator_FrostProtection, m_mapSSheetColumnWidth[SD_SVThermoActuator][CD_SVThrmActuator_FrostProtection] );
	pclSheetDescriptionActuator->AddColumnInPixels( CD_SVThrmActuator_MaxTemp, m_mapSSheetColumnWidth[SD_SVThermoActuator][CD_SVThrmActuator_MaxTemp] );
	pclSheetDescriptionActuator->AddColumnInPixels( CD_SVThrmActuator_Pointer, m_mapSSheetColumnWidth[SD_SVThermoActuator][CD_SVThrmActuator_Pointer] );
	
	// Set in which column parameter must be saved.
	pclSheetDescriptionActuator->AddParameterColumn( CD_SVThrmActuator_Pointer );
	
	// Set the focus column.
	pclSheetDescriptionActuator->SetActiveColumn( CD_SVThrmActuator_Name );

	// Set selectable rows.
	pclSheetDescriptionActuator->SetSelectableRangeRow( RD_SVActuator_FirstAvailRow, pclSSheet->GetMaxRows() - 1 );
	
	// Set range for selection.
	pclSheetDescriptionActuator->SetFocusColumnRange( CD_SVThrmActuator_Name, CD_SVThrmActuator_MaxTemp );
	
	// Add the main title.
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)GetTitleBackgroundColor( pclSheetDescriptionActuator ) );
	pclSheetDescriptionActuator->SetUserVariable( _SDUV_TITLEFORECOLOR, ( pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor ) ) );
	pclSheetDescriptionActuator->SetUserVariable( _SDUV_TITLEBACKCOLOR, ( pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor ) ) );

	pclSSheet->AddCellSpanW( CD_SVThrmActuator_CheckBox, RD_SVActuator_GroupName, CD_SVThrmActuator_Pointer - CD_SVThrmActuator_CheckBox, 1 );
	pclSSheet->SetStaticText( CD_SVThrmActuator_CheckBox, RD_SVActuator_GroupName, IDS_SSHEETSSELTRV_SVACTGRP );
	
	// Columns title.
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

	pclSSheet->SetStaticText( CD_SVThrmActuator_Name, RD_SVActuator_ColName, IDS_SSHEETSSELTRV_ACTRNAME );
	pclSSheet->SetStaticText( CD_SVThrmActuator_Description, RD_SVActuator_ColName, IDS_SSHEETSSELTRV_ACTRDESC );
	pclSSheet->SetStaticText( CD_SVThrmActuator_SettingRange, RD_SVActuator_ColName, IDS_SSHEETSSELTRV_ACTRSETTRANGE );
	pclSSheet->SetStaticText( CD_SVThrmActuator_CapillaryLength, RD_SVActuator_ColName, IDS_SSHEETSSELTRV_ACTRCAPTUBELEN );
	pclSSheet->SetStaticText( CD_SVThrmActuator_Hysteresis, RD_SVActuator_ColName, IDS_SSHEETSSELTRV_ACTRHYSTERESIS );
	pclSSheet->SetStaticText( CD_SVThrmActuator_FrostProtection, RD_SVActuator_ColName, IDS_SSHEETSSELTRV_ACTRFROSPROT );
	pclSSheet->SetStaticText( CD_SVThrmActuator_MaxTemp, RD_SVActuator_ColName, IDS_SSHEETSSELCV_ACTRMAXTEMP );
	
	// Unit.
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();
	pclSSheet->SetStaticText( CD_SVThrmActuator_SettingRange, RD_SVActuator_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_TEMPERATURE ) ).c_str() );
	pclSSheet->SetStaticText( CD_SVThrmActuator_CapillaryLength, RD_SVActuator_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_LENGTH ) ).c_str() );
	pclSSheet->SetStaticText( CD_SVThrmActuator_Hysteresis, RD_SVActuator_Unit, _T("K") );
	pclSSheet->SetStaticText( CD_SVThrmActuator_FrostProtection, RD_SVActuator_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_TEMPERATURE ) ).c_str() );
	pclSSheet->SetStaticText( CD_SVThrmActuator_MaxTemp, RD_SVActuator_Unit, IDS_UNITDEGREECELSIUS );
	pclSSheet->SetCellBorder( CD_SVThrmActuator_CheckBox, RD_SVActuator_Unit, CD_SVThrmActuator_Pointer - 1, RD_SVActuator_Unit, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );
	
	// Fill actuator list.
	long lRow = RD_SVActuator_FirstAvailRow;
	long lActuatorCount = 0;
	pclSheetDescriptionActuator->SetUserVariable( _SDUV_TOTALACTUATOR_COUNT, 0 );
	bool bAtLeastOneActuatorPartOfASet = false;
	CString str;
	LPARAM lpParam;
	
	for( BOOL bContinue = ActuatorList.GetFirst( str, lpParam ); TRUE == bContinue; bContinue = ActuatorList.GetNext( str, lpParam ) )
	{
		CDB_ThermostaticActuator *pThermostaticActuator = (CDB_ThermostaticActuator*)lpParam;
		
		// Add this row.
		// 'true' to specify that this row can be selected.
		pclSheetDescriptionActuator->AddRows( 1, true );

		// Add checkbox.
		pclSSheet->SetCheckBox( CD_SVThrmActuator_CheckBox, lRow, _T(""), false, true );

		// If user wants a Trv-head set, we add an opened box in front of the line.
		if( true == pclSelectedThermostaticValve->IsPartOfaSet() )
		{
			CCDButtonBox *pCDButtonBoxActuator = CreateCellDescriptionBox( CD_SVThrmActuator_Box, lRow, true, CCDButtonBox::ButtonState::BoxOpened, 
					pclSheetDescriptionActuator );
			
			if( NULL != pCDButtonBoxActuator )
			{
				pCDButtonBoxActuator->ApplyInternalChange();
			}

			bAtLeastOneActuatorPartOfASet = true;
		}
		//HYS-726
		if( m_pclIndSelTRVParams->m_WC.GetTemp() > pThermostaticActuator->GetTmax() )
		{
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_RED );
			pclSSheet->SetStaticText( CD_SVThrmActuator_Name, lRow, pThermostaticActuator->GetName() );
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );
		}
		else
		{
			pclSSheet->SetStaticText( CD_SVThrmActuator_Name, lRow, pThermostaticActuator->GetName() );
		}

		pclSSheet->SetStaticText( CD_SVThrmActuator_Description, lRow, pThermostaticActuator->GetComment() );
		CString str;
		str.Format( _T("[%d-%d]"), pThermostaticActuator->GetMinSetting(), pThermostaticActuator->GetMaxSetting() );
		pclSSheet->SetStaticText( CD_SVThrmActuator_SettingRange, lRow, str );

		int iCapillaryLength = pThermostaticActuator->GetCapillaryLength();

		if( iCapillaryLength != -1 &&  iCapillaryLength > 0 )
		{
			str.Format( _T("%d"), iCapillaryLength );
		}
		else
		{
			str = _T("-");
		}

		pclSSheet->SetStaticText( CD_SVThrmActuator_CapillaryLength, lRow, str );

		double dHysteresis = pThermostaticActuator->GetHysteresis();
		
		if( dHysteresis != -1.0 &&  dHysteresis > 0.0 )
		{
			str = WriteDouble( dHysteresis, 3, 1, 1 );
		}
		else
		{
			str = _T("-");
		}

		pclSSheet->SetStaticText( CD_SVThrmActuator_Hysteresis, lRow, str );
		
		int iFrostProtection = pThermostaticActuator->GetFrostProtection();
		
		if( iFrostProtection != -1 &&  iFrostProtection > 0 )
		{
			str.Format( _T("%d"), iFrostProtection );
		}
		else
		{
			str = _T("-");
		}

		pclSSheet->SetStaticText( CD_SVThrmActuator_FrostProtection, lRow, str );
		//HYS-726
		CString sTmaxMedia = ( DBL_MAX == pThermostaticActuator->GetTmax() ) ? _T("-") : WriteDouble( pThermostaticActuator->GetTmax(), 3 );
		if( m_pclIndSelTRVParams->m_WC.GetTemp() > pThermostaticActuator->GetTmax() )
		{
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_RED );
			pclSSheet->SetStaticText( CD_SVThrmActuator_MaxTemp, lRow, sTmaxMedia );
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );
		}
		else
		{
			pclSSheet->SetStaticText( CD_SVThrmActuator_MaxTemp, lRow, sTmaxMedia );
		}

		pclSSheet->SetCellBorder( CD_SVThrmActuator_CheckBox, lRow, CD_SVThrmActuator_Pointer - 1, lRow, true, SS_BORDERTYPE_BOTTOM, 
				SS_BORDERSTYLE_SOLID, _GRAY );

		// Save parameter.
		CreateCellDescriptionProduct( pclSheetDescriptionActuator->GetFirstParameterColumn(), lRow, (LPARAM)pThermostaticActuator, 
				pclSheetDescriptionActuator );

		lRow++;
		lActuatorCount++;
	}

	long lLastDataRow = lRow - 1;

	// If there is no actuator that belongs to a set...
	if( false == bAtLeastOneActuatorPartOfASet )
	{
		pclSSheet->ShowCol( CD_SVThrmActuator_Box, FALSE );		
	}

	pclSSheet->SetCellBorder( CD_SVThrmActuator_CheckBox, lLastDataRow, CD_SVThrmActuator_MaxTemp, lLastDataRow, true, SS_BORDERTYPE_BOTTOM,
			SS_BORDERSTYLE_SOLID, _BLACK );

	// Save the total actuator number.
	pclSheetDescriptionActuator->SetUserVariable( _SDUV_TOTALACTUATOR_COUNT, lActuatorCount );

	// Set that there is no selection at now.
	SetCurrentSVActuatorSelected( NULL );

	// Add possibility to change column size.
	// Remark: place this code after any initialization (like adding, removing, resizing, hiding/showing columns).
	CResizingColumnInfo* pclResizingColumnInfo = pclSheetDescriptionActuator->GetResizingColumnInfo();
	pclResizingColumnInfo->ActivateFeature( true, false, RD_SVActuator_FirstAvailRow, lLastDataRow );
	pclResizingColumnInfo->AddRangeColumn( CD_SVThrmActuator_Name, CD_SVThrmActuator_MaxTemp, RD_SVActuator_ColName, RD_SVActuator_Unit );

	// Add possibility to click on title group to reset column width.
	CreateTitleGroupButtons( CD_SVThrmActuator_CheckBox, CD_SVThrmActuator_MaxTemp, RD_SVActuator_GroupName, pclSheetDescriptionActuator );
}

void CRViewSSelTrv::_FillSVElectroActuatorRows( CSheetDescription *pclSheetDescriptionActuator )
{
	if( NULL == m_pclIndSelTRVParams || NULL == m_pclIndSelTRVParams->m_pTADB )
	{
		ASSERT_RETURN;
	}

	if( NULL == pclSheetDescriptionActuator || NULL == pclSheetDescriptionActuator->GetSSheetPointer() )
	{
		return;
	}

	CSSheet *pclSSheet = pclSheetDescriptionActuator->GetSSheetPointer();
	
	CDB_ControlValve *pclSelectedThermostaticValve = GetCurrentSupplyValveSelected();
	
	if( NULL == pclSelectedThermostaticValve )
	{
		return;
	}

	CTable *pActuatorGroup = (CTable *)( pclSelectedThermostaticValve->GetActuatorGroupIDPtr().MP );

	if( NULL == pActuatorGroup )
	{
		return;
	}

	// Transfer actuator list in a CRank to have the possibility to sort them with the priority level.
	CRank ActuatorList;
	CString strElectroActuatorTypeID = m_pclIndSelTRVParams->m_strComboActuatorTypeID;
	CString strElectroActuatorFamilyID = m_pclIndSelTRVParams->m_strComboActuatorFamilyID;
	CString strElectroActuatorVersionID = m_pclIndSelTRVParams->m_strComboActuatorVersionID;
	
	for( IDPTR ActrIDPtr = pActuatorGroup->GetFirst(); _T('\0') != *ActrIDPtr.ID; ActrIDPtr = pActuatorGroup->GetNext() )
	{		
		CDB_ElectroActuator *pElectroActuator = dynamic_cast<CDB_ElectroActuator *>( (CData *)m_pclIndSelTRVParams->m_pTADB->Get( ActrIDPtr.ID ).MP );

		if( NULL == pElectroActuator )
		{
			continue;
		}

		if( false == pElectroActuator->IsSelectable( true ) )
		{
			continue;
		}

		if( 0 != CString( _T("ACTTYP_ELEC") ).Compare( pElectroActuator->GetTypeIDPtr().ID ) 
				&& 0 != CString( _T("ACTTYP_THRMELEC") ).Compare( pElectroActuator->GetTypeIDPtr().ID ) )
		{
			continue;
		}

		if( false == strElectroActuatorTypeID.IsEmpty() && 0 != strElectroActuatorTypeID.Compare( pElectroActuator->GetTypeIDPtr().ID ) )
		{
			continue;
		}

		if( false == strElectroActuatorFamilyID.IsEmpty() && 0 != strElectroActuatorFamilyID.Compare( pElectroActuator->GetFamilyIDPtr().ID ) )
		{
			continue;
		}

		if( false == strElectroActuatorVersionID.IsEmpty() && 0 != strElectroActuatorVersionID.Compare( pElectroActuator->GetVersionIDPtr().ID ) )
		{
			continue;
		}

		ActuatorList.Add( pElectroActuator->GetIDPtr().ID, pElectroActuator->GetOrderKey(), ( LPARAM )pElectroActuator, false, false );
	}

	CDB_CloseOffChar *pCloseOffChar = ( CDB_CloseOffChar * )( pclSelectedThermostaticValve->GetCloseOffCharIDPtr().MP );

	pclSheetDescriptionActuator->AddColumnInPixels( CD_SVElecActuator_FirstColumn, m_mapSSheetColumnWidth[SD_SVElectroActuator][CD_SVElecActuator_FirstColumn] );
	pclSheetDescriptionActuator->AddColumnInPixels( CD_SVElecActuator_CheckBox, m_mapSSheetColumnWidth[SD_SVElectroActuator][CD_SVElecActuator_CheckBox] );
	pclSheetDescriptionActuator->AddColumnInPixels( CD_SVElecActuator_Name, m_mapSSheetColumnWidth[SD_SVElectroActuator][CD_SVElecActuator_Name] );
	pclSheetDescriptionActuator->AddColumnInPixels( CD_SVElecActuator_CloseOffValue, m_mapSSheetColumnWidth[SD_SVElectroActuator][CD_SVElecActuator_CloseOffValue] );
	pclSheetDescriptionActuator->AddColumnInPixels( CD_SVElecActuator_ActuatingTime, m_mapSSheetColumnWidth[SD_SVElectroActuator][CD_SVElecActuator_ActuatingTime] );
	pclSheetDescriptionActuator->AddColumnInPixels( CD_SVElecActuator_IP, m_mapSSheetColumnWidth[SD_SVElectroActuator][CD_SVElecActuator_IP] );
	pclSheetDescriptionActuator->AddColumnInPixels( CD_SVElecActuator_PowSupply, m_mapSSheetColumnWidth[SD_SVElectroActuator][CD_SVElecActuator_PowSupply] );
	pclSheetDescriptionActuator->AddColumnInPixels( CD_SVElecActuator_InputSig, m_mapSSheetColumnWidth[SD_SVElectroActuator][CD_SVElecActuator_InputSig] );
	pclSheetDescriptionActuator->AddColumnInPixels( CD_SVElecActuator_OutputSig, m_mapSSheetColumnWidth[SD_SVElectroActuator][CD_SVElecActuator_OutputSig] );
	pclSheetDescriptionActuator->AddColumnInPixels( CD_SVElecActuator_MaxTemp, m_mapSSheetColumnWidth[SD_SVElectroActuator][CD_SVElecActuator_MaxTemp] );
	pclSheetDescriptionActuator->AddColumnInPixels( CD_SVElecActuator_Pointer, m_mapSSheetColumnWidth[SD_SVElectroActuator][CD_SVElecActuator_Pointer] );
	
	// Set in which column parameter must be saved.
	pclSheetDescriptionActuator->AddParameterColumn( CD_SVElecActuator_Pointer );
	
	// Set the focus column.
	pclSheetDescriptionActuator->SetActiveColumn( CD_SVElecActuator_Name );

	// Set selectable rows.
	pclSheetDescriptionActuator->SetSelectableRangeRow( RD_SVActuator_FirstAvailRow, pclSSheet->GetMaxRows() - 1 );
	
	// Set range for selection.
	pclSheetDescriptionActuator->SetFocusColumnRange( CD_SVElecActuator_Name, CD_SVElecActuator_MaxTemp );
	
	// Add the main title.
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)GetTitleBackgroundColor( pclSheetDescriptionActuator ) );
	pclSheetDescriptionActuator->SetUserVariable( _SDUV_TITLEFORECOLOR, ( pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor ) ) );
	pclSheetDescriptionActuator->SetUserVariable( _SDUV_TITLEBACKCOLOR, ( pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor ) ) );

	pclSSheet->AddCellSpanW( CD_SVElecActuator_CheckBox, RD_SVActuator_GroupName, CD_SVElecActuator_Pointer - CD_SVElecActuator_CheckBox, 1 );
	pclSSheet->SetStaticText( CD_SVElecActuator_CheckBox, RD_SVActuator_GroupName, IDS_SSHEETSSELTRV_SVACTGRP );
	
	// Columns title.
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

	pclSSheet->SetStaticText( CD_SVElecActuator_Name, RD_SVActuator_ColName, IDS_SSHEETSSELTRV_ACTRNAME );

	if( NULL != pCloseOffChar )
	{
		if( CDB_CloseOffChar::CloseOffDp == pCloseOffChar->GetLimitType() )
		{
			pclSSheet->SetStaticText( CD_SVElecActuator_CloseOffValue, RD_SVActuator_ColName, IDS_SSHEETSSELTRV_ACTRCLOSEOFFDP );
		}
		else
		{
			pclSSheet->SetStaticText( CD_SVElecActuator_CloseOffValue, RD_SVActuator_ColName, IDS_SSHEETSSELTRV_ACTRMAXINLETPRESSURE );
		}
	}
	else
	{
		pclSSheet->ShowCol( CD_SVElecActuator_CloseOffValue, FALSE );
	}

	pclSSheet->SetStaticText( CD_SVElecActuator_ActuatingTime, RD_SVActuator_ColName, IDS_SSHEETSSELTRV_ACTRACTUATINGTIME );
	
	// Unit.
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();

	if( NULL != pCloseOffChar )
	{
		if( CDB_CloseOffChar::CloseOffDp == pCloseOffChar->GetLimitType() )
		{
			pclSSheet->SetStaticText( CD_SVElecActuator_CloseOffValue, RD_SVActuator_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ) ).c_str() );
		}
		else
		{
			pclSSheet->SetStaticText( CD_SVElecActuator_CloseOffValue, RD_SVActuator_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_PRESSURE ) ).c_str() );
		}
	}

	pclSSheet->SetStaticText( CD_SVElecActuator_CloseOffValue, RD_SVActuator_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ) ).c_str() );
	pclSSheet->SetStaticText( CD_SVElecActuator_ActuatingTime, RD_SVActuator_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_TIME ) ).c_str() );
	pclSSheet->SetStaticText( CD_SVElecActuator_IP, RD_SVActuator_ColName, IDS_SSHEETSSELTRV_ACTRIP );
	pclSSheet->SetStaticText( CD_SVElecActuator_PowSupply, RD_SVActuator_ColName, IDS_SSHEETSSELTRV_ACTRPOWERSUP );
	pclSSheet->SetCellBorder( CD_SVElecActuator_CheckBox, RD_SVActuator_Unit, CD_SVElecActuator_Pointer - 1, RD_SVActuator_Unit, 
								true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );
	pclSSheet->SetStaticText( CD_SVElecActuator_MaxTemp, RD_SVActuator_ColName, IDS_SSHEETSSELCV_ACTRMAXTEMP );

	// Span cells Input/Output signal.
	pclSSheet->SetStaticText( CD_SVElecActuator_InputSig, RD_SVActuator_ColName, IDS_SSHEETSSELTRV_ACTRINPUTSIG );
	pclSSheet->SetStaticText( CD_SVElecActuator_OutputSig, RD_SVActuator_ColName, IDS_SSHEETSSELTRV_ACTROUTPUTSIG );


	
	// Fill actuator list.
	long lRow = RD_SVActuator_FirstAvailRow;
	long lActuatorCount = 0;
	pclSheetDescriptionActuator->SetUserVariable( _SDUV_TOTALACTUATOR_COUNT, 0 );
	
	bool bAtLeastOneActuatorPartOfASet = false;
	CString str;
	LPARAM lpParam;

	for( BOOL bContinue = ActuatorList.GetFirst( str, lpParam ); TRUE == bContinue; bContinue = ActuatorList.GetNext( str, lpParam ) )
	{
		CDB_ElectroActuator *pElectroActuator = (CDB_ElectroActuator*)lpParam;

		// Add this row.
		// 'true' to specify that this row can be selected.
		pclSheetDescriptionActuator->AddRows( 1, true );

		// If user wants a Trv-head set, we add an opened box in front of the line.
		if( true == pclSelectedThermostaticValve->IsPartOfaSet() )
		{
			CCDButtonBox *pCDButtonBoxActuator = CreateCellDescriptionBox( CD_SVElecActuator_Box, lRow, true, CCDButtonBox::ButtonState::BoxOpened,
					pclSheetDescriptionActuator );

			if( NULL != pCDButtonBoxActuator )
			{
				pCDButtonBoxActuator->ApplyInternalChange();
			}

			bAtLeastOneActuatorPartOfASet = true;
		}
		//HYS-726
		if( m_pclIndSelTRVParams->m_WC.GetTemp() > pElectroActuator->GetTmax() )
		{
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_RED );
			pclSSheet->SetStaticText( CD_SVElecActuator_Name, lRow, pElectroActuator->GetName() );
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );
		}
		else
		{
			pclSSheet->SetStaticText( CD_SVElecActuator_Name, lRow, pElectroActuator->GetName() );
		}

		// Add checkbox.
		pclSSheet->SetCheckBox( CD_SVElecActuator_CheckBox, lRow, _T(""), false, true );

		// Set the close off value if exist (Dp or inlet pressure depending of the current supply valve selected).
		if( NULL != pCloseOffChar )
		{
			CString str;

			if( CDB_CloseOffChar::CloseOffDp == pCloseOffChar->GetLimitType() )
			{
				double dCloseOffDp = pCloseOffChar->GetCloseOffDp( pElectroActuator->GetMaxForceTorque() );

				if( -1.0 == dCloseOffDp)
				{
					str = _T("-");
				}
				else
				{
					str = WriteCUDouble( _U_DIFFPRESS, dCloseOffDp );
				}
			}
			else
			{
				double dMaxInletPressure = pCloseOffChar->GetMaxInletPressure( pElectroActuator->GetMaxForceTorque() );

				if( -1.0 == dMaxInletPressure )
				{
					str = _T("-");
				}
				else
				{
					str = WriteCUDouble( _U_PRESSURE, dMaxInletPressure );
				}
			}
			
			pclSSheet->SetStaticText( CD_SVElecActuator_CloseOffValue, lRow, str );
		}
		
		// Compute Actuating time.
		// Remark: actuating Time is given in s/m or in s/deg; stroke is given in m or in deg.
		pclSSheet->SetStaticText( CD_SVElecActuator_ActuatingTime, lRow, pElectroActuator->GetActuatingTimesStr( pclSelectedThermostaticValve->GetStroke() ) );

		CString str = pElectroActuator->GetIPxxFull();
		pclSSheet->SetStaticText( CD_SVElecActuator_IP, lRow, str );
		pclSSheet->SetStaticText( CD_SVElecActuator_PowSupply, lRow, pElectroActuator->GetPowerSupplyStr() );
		str.Empty();
		pclSSheet->SetStaticText( CD_SVElecActuator_InputSig, lRow, pElectroActuator->GetInOutSignalsStr( true ) );
		pclSSheet->SetStaticText( CD_SVElecActuator_OutputSig, lRow, pElectroActuator->GetInOutSignalsStr( false ) );
		//HYS-726
		CString sTmaxMedia = ( DBL_MAX == pElectroActuator->GetTmax() ) ? _T("-") : WriteDouble( pElectroActuator->GetTmax(), 3 );
		if( m_pclIndSelTRVParams->m_WC.GetTemp() > pElectroActuator->GetTmax() )
		{
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_RED );
			pclSSheet->SetStaticText( CD_SVElecActuator_MaxTemp, lRow, sTmaxMedia );
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );
		}
		else
		{
			pclSSheet->SetStaticText( CD_SVElecActuator_MaxTemp, lRow, sTmaxMedia );
		}
		
		pclSSheet->SetCellBorder( CD_SVElecActuator_CheckBox, lRow, CD_SVElecActuator_Pointer - 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _GRAY );

		// Save parameter.
		CreateCellDescriptionProduct( pclSheetDescriptionActuator->GetFirstParameterColumn(), lRow, (LPARAM)pElectroActuator, pclSheetDescriptionActuator );

		lRow++;
		lActuatorCount++;
	}

	long lLastDataRow = lRow - 1;

	// If there is no actuator that belongs to a set...
	if( false == bAtLeastOneActuatorPartOfASet )
	{
		pclSSheet->ShowCol( CD_SVElecActuator_Box, FALSE );		
	}
	
	pclSSheet->SetCellBorder( CD_SVElecActuator_CheckBox, lLastDataRow, CD_SVElecActuator_Pointer - 1, lLastDataRow, true, SS_BORDERTYPE_BOTTOM, 
			SS_BORDERSTYLE_SOLID, _GRAY );

	// Save the total actuator number.
	pclSheetDescriptionActuator->SetUserVariable( _SDUV_TOTALACTUATOR_COUNT, lActuatorCount );
	
	// Set that there is no selection at now.
	SetCurrentSVActuatorSelected( NULL );

	// Add possibility to change column size.
	// Remark: place this code after any initialization (like adding, removing, resizing, hiding/showing columns).
	CResizingColumnInfo* pclResizingColumnInfo = pclSheetDescriptionActuator->GetResizingColumnInfo();
	pclResizingColumnInfo->ActivateFeature( true, false, RD_SVActuator_FirstAvailRow, lLastDataRow );
	pclResizingColumnInfo->AddRangeColumn( CD_SVElecActuator_Name, CD_SVElecActuator_MaxTemp, RD_SVActuator_ColName, RD_SVActuator_Unit );

	// Add possibility to click on title group to reset column width.
	CreateTitleGroupButtons( CD_SVElecActuator_CheckBox, CD_SVElecActuator_MaxTemp, RD_SVActuator_GroupName, pclSheetDescriptionActuator );
}

void CRViewSSelTrv::_UpdateSupplyValveSettingDp( double dReturnValveDp )
{
	if( NULL == m_pclIndSelTRVParams || NULL == m_pclIndSelTRVParams->m_pclSelectSupplyValveList )
	{
		ASSERT_RETURN;
	}
	
	CSheetDescription *pclSheetDescriptionSV = m_ViewDescription.GetFromSheetDescriptionID( SD_SupplyValve );
	
	if( NULL == pclSheetDescriptionSV || NULL == pclSheetDescriptionSV->GetSSheetPointer() )
	{
		return;
	}
	
	CSSheet *pclSSheet = pclSheetDescriptionSV->GetSSheetPointer();

	CSelectTrvList *pSelectedTrvList = m_pclIndSelTRVParams->m_pclSelectSupplyValveList;

	if( NULL == pSelectedTrvList )
	{
		return;
	}

	// Get some variables.
	CDS_TechnicalParameter *pTechParam = m_pclIndSelTRVParams->m_pTADS->GetpTechParams();
	double dTrvMinDpRatio = pTechParam->GetTrvMinDpRatio();
	double dTrvDefDpTot = pTechParam->GetTrvDefDpTot();
	double dRho = m_pclIndSelTRVParams->m_WC.GetDens();
	double dNu = m_pclIndSelTRVParams->m_WC.GetKinVisc();

	double dDpTotal = m_pclIndSelTRVParams->m_dDp;
	double dFlow = m_pclIndSelTRVParams->m_dFlow;

	// If thermostatic head is used, we take the Kv characteristic at 2DT Kelvin, otherwise we take KvS (iDeltaT = 0).
	int iDeltaT = ( true == m_pclIndSelTRVParams->m_bIsThermostaticHead ) ? 2 : 0;

	// If there is already a supply valve selection
	if( 0 != m_lSupplyValveSelectedRow )
	{
		// We remove current selection to reset it at the end of the update.
		// Remark: it's to force internal variables to maintain the right color under selection.
		pclSSheet->UnSelectMultipleRows();
	}

	// If we must compute new value depending on new return valve pressure drop...
	if( -1 != dReturnValveDp )
	{
		double dReqDpTrv = dDpTotal - dReturnValveDp;

		// By default, clean full opening remark if exist.
		pclSheetDescriptionSV->RemoveOneRemark( CSheetDescription::RemarkFlags::FullOpening );
		
		// Clean all others (keep internal variables).
		pclSheetDescriptionSV->ClearRemarks();

		// Loop all products.
		CCellDescription *pclCellDescription = pclSheetDescriptionSV->GetFirstCellDescription( RVSCellDescription::CD_Product );
		
		while( NULL != pclCellDescription )
		{
			CCellDescriptionProduct *pCDProduct = dynamic_cast<CCellDescriptionProduct*>( pclCellDescription );

			if( NULL == pCDProduct )
			{
				continue;
			}

			CDB_ThermostaticValve *pclSupplyValve = dynamic_cast<CDB_ThermostaticValve*>( (CData*)pCDProduct->GetProduct() );

			if( NULL == pclSupplyValve )
			{
				continue;
			}

			CSelectedValve *pclSelectedSupplyValve = GetSelectProduct<CSelectedValve>( pclSupplyValve, m_pclIndSelTRVParams->m_pclSelectSupplyValveList );

			if( NULL == pclSelectedSupplyValve )
			{
				continue;
			}

			// Get Kv characteristic of the valve. If it does not exist, get KvS characteristic of the valve.
			CDB_ThermoCharacteristic *pThermoCharacteristic = pclSupplyValve->GetThermoCharacteristic();

			if( NULL == pThermoCharacteristic )
			{
				continue;
			}

			double dDpFullyOpen;
			double dHMax = pThermoCharacteristic->GetOpeningMax();

			if( -1.0 == dHMax )
			{
				continue;
			}

			if( false == pThermoCharacteristic->GetValveDp( dFlow, &dDpFullyOpen, dHMax, dRho, dNu, iDeltaT ) )
			{
				continue;
			}

			// Determine if we must force full opening or not.
			// It's only the case when valves are not presettable.
			bool bForceFullOpening = false;

			if( 0 == CString( _T("TRVTYPE_NOPRSET") ).Compare( pclSupplyValve->GetTypeID() ) 
					|| 0 == CString( _T("TRVTYPE_INSERT_NOPRSET") ).Compare( pclSupplyValve->GetTypeID() ) )
			{
				bForceFullOpening = true;
			}
			
			double dH = -1.0;
			double dDp;
			bool bNotFound;
			
			// If supply valves are not presettable...
			if( true == bForceFullOpening )
			{
				// We can adjust to the required Dp but only get Dp at full opening.
				dH = pThermoCharacteristic->GetOpeningMax();
				dDp = dDpFullyOpen;
				// If user has asked a precise pressure drop on both supply and return valves and if Dp fully opened on supply valve is bigger than
				// Dp required on it, than we must set that we have not found.
				// Remark: 'dSelDp' is the required Dp on supply valve!
				bNotFound = ( dDp >= dReqDpTrv ) ? true : false;
			}
			else 
			{
				dDp = dReqDpTrv;
				
				// If required Dp on supply valve is bigger than the Dp when valve is fully opened, it means that we can perhaps reach the required Dp by
				// closing the valve...
				if( dDp > dDpFullyOpen )
				{
					// Try to find a valid opening to reach required Dp.
					// Remark: If valve opening is discrete, we use rounding case 4 (see 'CDB_ThermoCharacteristic::GetValveOpening()' comments in DataObj.h) otherwise
					//         we not apply rounding (case 1).
					bool bValid = pThermoCharacteristic->GetValveOpening( dFlow, dDp, &dH, dRho, dNu, 
							( eBool3::eb3True == pThermoCharacteristic->IsDiscrete() ) ? 4 : 1, iDeltaT );
			
					// If it's not possible to have opening to reach required Dp...
					if( false == bValid )
					{
						// Take pressure drop available at the minimum recommenced setting of the valve.
						dH = pThermoCharacteristic->GetMinRecSetting();
						
						if( dH <= 0.0 )
						{
							dH = pThermoCharacteristic->GetOpeningMin();
						}

						if( -1.0 != dH )
						{
							if( false == pThermoCharacteristic->GetValveDp( dFlow, &dDp, dH, dRho, dNu, iDeltaT ) )
							{
								continue;
							}
						}
					}
				}

				// If dH < 0, no solution was found...
				bNotFound = ( dH < 0 ) ? true : false;

				if( true == bNotFound )
				{
					// Than the only solution is to take Dp when valve is fully opened.
					dH = dHMax;
					dDp = dDpFullyOpen;
				}
			}

			// Update fields in the selected product.
			pclSelectedSupplyValve->SetH( dH );
			pclSelectedSupplyValve->SetDp( dDp );
			
			// Clear previous flag.
			bool bPNFlag = pclSelectedSupplyValve->GetFlag( CSelectedBase::Flags::ePN );
			bool bTempFlag = pclSelectedSupplyValve->GetFlag( CSelectedBase::Flags::eTemperature );
			pclSelectedSupplyValve->ClearAllFlags();

			pclSelectedSupplyValve->SetFlag( CSelectedBase::Flags::eValveSetting, dH <= 0.0 );
			pclSelectedSupplyValve->SetFlag( CSelectedBase::Flags::eValveMaxSetting, ( dH == dHMax ) ? true : false );

			// Set an error code to facilitate management with 'OnTextTipFetch'.
			if( true == bNotFound )
			{
				pclSelectedSupplyValve->SetFlag( CSelectedBase::Flags::eDp, true, CSelectTrvList::FlagError::DpNotFound );
			}
			else if( pclSupplyValve->GetDpmax() != 0.0 && dDp > pclSupplyValve->GetDpmax() )
			{
				pclSelectedSupplyValve->SetFlag( CSelectedBase::Flags::eDp, true, CSelectTrvList::FlagError::DpMaxReached );
			}
			else if( dDp < dTrvMinDpRatio * dDpTotal )
			{
				pclSelectedSupplyValve->SetFlag( CSelectedBase::Flags::eDp, true, CSelectTrvList::FlagError::DpAuthority );
			}

			pclSelectedSupplyValve->SetFlag( CSelectedBase::Flags::eValveFullODp, ( true == m_pclIndSelTRVParams->m_bDpEnabled && dDpFullyOpen > dReqDpTrv ) );

			pclSelectedSupplyValve->SetFlag( CSelectedBase::Flags::ePN, bPNFlag );	
			pclSelectedSupplyValve->SetFlag( CSelectedBase::Flags::eTemperature, bTempFlag );

			//////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// Refresh the content of the 'Setting' column.
			//////////////////////////////////////////////////////////////////////////////////////////////////////////////

			// By default, text color is black.
			pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

			CString str = pThermoCharacteristic->GetSettingString( dH );

			if( true == pclSelectedSupplyValve->IsFlagSet( CSelectedBase::Flags::eValveMaxSetting ) )
			{
				pclSheetDescriptionSV->WriteTextWithFlags( str, CD_SupplyValve_Setting, pCDProduct->GetCellPosition().y, 
						CSheetDescription::RemarkFlags::FullOpening, _T(""), ( dH < pThermoCharacteristic->GetMinRecSetting() ) ? _RED : _BLACK );
			}
			else
			{
				if( dH < pThermoCharacteristic->GetMinRecSetting() )
				{
					pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_ORANGE );
				}

				// Update setting.
				COLORREF backColor = pclSSheet->GetBackColor( CD_SupplyValve_Setting, pclCellDescription->GetCellPosition().y );
				pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)backColor );
				pclSSheet->SetStaticText( CD_SupplyValve_Setting, pCDProduct->GetCellPosition().y, str );
			}

			//////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// Refresh the content of the 'Dp' column.
			//////////////////////////////////////////////////////////////////////////////////////////////////////////////

			// By default, text color is black.
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

			if( true == pclSelectedSupplyValve->IsFlagSet( CSelectedBase::eDp ) || true == pclSelectedSupplyValve->IsFlagSet( CSelectedBase::eValveFullODp ) )
			{
				pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_ORANGE );
			}

			// Update dp.
			COLORREF backColor = pclSSheet->GetBackColor( CD_SupplyValve_Dp, pclCellDescription->GetCellPosition().y );
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)backColor );
			pclSSheet->SetStaticText( CD_SupplyValve_Dp, pCDProduct->GetCellPosition().y, WriteDouble( CDimValue::SItoCU( _U_DIFFPRESS, dDp ), 3, 0 ) );

			// Go to next product.
			pclCellDescription = pclSheetDescriptionSV->GetNextCellDescription( pclCellDescription, RVSCellDescription::CD_Product );
		}

		// Update (add or remove) remarks.
		LPARAM lParam;
		pclSheetDescriptionSV->GetUserVariable( _SDUV_REMARKROW, lParam );
		pclSheetDescriptionSV->WriteRemarks( (long)lParam, CD_SupplyValve_Name, CD_SupplyValve_Separator );
	}
	else
	{
		// We reset display in 'Setting' and 'Dp' columns.

		COLORREF backColor;
		double dReqDpTrv = dDpTotal - m_rSuggestParams.m_dMaxDpRV;

		// By default, clean full opening remark if exist.
		pclSheetDescriptionSV->RemoveOneRemark( CSheetDescription::RemarkFlags::FullOpening );
		
		// Clean all others (keep internal variables).
		pclSheetDescriptionSV->ClearRemarks();

		// Loop all products.
		CCellDescription *pclCellDescription = pclSheetDescriptionSV->GetFirstCellDescription( RVSCellDescription::CD_Product );
		
		while( NULL != pclCellDescription )
		{
			CCellDescriptionProduct *pCDProduct = dynamic_cast<CCellDescriptionProduct*>( pclCellDescription );

			if( NULL == pCDProduct )
			{
				continue;
			}

			CDB_ThermostaticValve *pclSupplyValve = dynamic_cast<CDB_ThermostaticValve*>( (CData*)pCDProduct->GetProduct() );

			if( NULL == pclSupplyValve )
			{
				continue;
			}

			CSelectedValve *pclSelectedSupplyValve = GetSelectProduct<CSelectedValve>( pclSupplyValve, m_pclIndSelTRVParams->m_pclSelectSupplyValveList );

			if( NULL == pclSelectedSupplyValve )
			{
				continue;
			}

			// Get Kv characteristic of the valve. If it does not exist, get KvS characteristic of the valve.
			CDB_ThermoCharacteristic *pThermoCharacteristic = pclSupplyValve->GetThermoCharacteristic();

			if( NULL == pThermoCharacteristic )
			{
				continue;
			}

			long lRow = pCDProduct->GetCellPosition().y;
			bool bAtLeastOneError = false;
			double dDpFullyOpen;
			double dHMax = pThermoCharacteristic->GetOpeningMax();
			
			if( -1.0 == dHMax )
			{
				continue;
			}
			
			if( false == pThermoCharacteristic->GetValveDp( dFlow, &dDpFullyOpen, dHMax, dRho, dNu, iDeltaT ) )
			{
				continue;
			}

			// Try to find the setting corresponding to ReqDpTrv
			bool bNotFound = ( dDpFullyOpen > dDpTotal ) ? true : false;

			// Update fields in the selected product.
			pclSelectedSupplyValve->SetH( dHMax );
			pclSelectedSupplyValve->SetDp( dDpFullyOpen );

			// Clear previous flag.
			bool bPNFlag = pclSelectedSupplyValve->GetFlag( CSelectedBase::Flags::ePN );
			bool bTempFlag = pclSelectedSupplyValve->GetFlag( CSelectedBase::Flags::eTemperature );
			pclSelectedSupplyValve->ClearAllFlags();

			pclSelectedSupplyValve->SetFlag( CSelectedBase::Flags::eValveSetting, false );
			pclSelectedSupplyValve->SetFlag( CSelectedBase::Flags::eValveMaxSetting, true );
			
			// Set an error code to facilitate management with 'OnTextTipFetch'.
			if( true == bNotFound )
			{
				pclSelectedSupplyValve->SetFlag( CSelectedBase::Flags::eDp, true, CSelectTrvList::FlagError::DpNotFound );
			}
			else if( pclSupplyValve->GetDpmax() != 0.0 && dDpFullyOpen > pclSupplyValve->GetDpmax() )
			{
				pclSelectedSupplyValve->SetFlag( CSelectedBase::Flags::eDp, true, CSelectTrvList::FlagError::DpMaxReached );
			}
			else if( dDpFullyOpen < dTrvMinDpRatio * dDpTotal )
			{
				pclSelectedSupplyValve->SetFlag( CSelectedBase::Flags::eDp, true, CSelectTrvList::FlagError::DpAuthority );
			}

			pclSelectedSupplyValve->SetFlag( CSelectedBase::Flags::eValveFullODp, dDpFullyOpen > dDpTotal );

			pclSelectedSupplyValve->SetFlag( CSelectedBase::Flags::ePN, bPNFlag );	
			pclSelectedSupplyValve->SetFlag( CSelectedBase::Flags::eTemperature, bTempFlag );

			//////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// Refresh the content of the 'Setting' column.
			//////////////////////////////////////////////////////////////////////////////////////////////////////////////

			// By default, text color is black.
			pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
			
			if( false == pclSelectedSupplyValve->IsFlagSet( CSelectedBase::eValveFullODp ) )
			{
				// Remark: we have return valves with different pressure drop. It it thus impossible at now to compute setting and Dp on
				//         the supply valves.
				//         why last condition? Because if pressure drop on supply valve when fully opened is already bigger than the Dp user wants, it is 
				//         impossible to do better. Than in this case we show setting and Dp.

				backColor = pclSSheet->GetBackColor( CD_SupplyValve_Setting, lRow );
				pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)backColor );
				pclSSheet->SetStaticText( CD_SupplyValve_Setting, lRow, _T("-") );

				backColor = pclSSheet->GetBackColor( CD_SupplyValve_Dp, lRow );
				pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)backColor );
				pclSSheet->SetStaticText( CD_SupplyValve_Dp, lRow, _T("-") );
			}
			else
			{
				CString str = pThermoCharacteristic->GetSettingString( dHMax );

				if( true == pclSelectedSupplyValve->IsFlagSet( CSelectedBase::Flags::eValveMaxSetting ) )
				{
					pclSheetDescriptionSV->WriteTextWithFlags( str, CD_SupplyValve_Setting, pCDProduct->GetCellPosition().y, 
							CSheetDescription::RemarkFlags::FullOpening, _T(""), ( dHMax < pThermoCharacteristic->GetMinRecSetting() ) ? _RED : _BLACK );
				}
				else
				{
					if( dHMax < pThermoCharacteristic->GetMinRecSetting() )
					{
						pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_RED );
						bAtLeastOneError = true;
					}

					// Update setting.
					backColor = pclSSheet->GetBackColor( CD_SupplyValve_Setting, lRow );
					pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)backColor );
					pclSSheet->SetStaticText( CD_SupplyValve_Setting, lRow, str );
				}

				//////////////////////////////////////////////////////////////////////////////////////////////////////////////
				// Refresh the content of the 'Dp' column.
				//////////////////////////////////////////////////////////////////////////////////////////////////////////////

				// By default, text color is black.
				pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

				if( true == pclSelectedSupplyValve->IsFlagSet( CSelectedBase::eDp ) || true == pclSelectedSupplyValve->IsFlagSet( CSelectedBase::eValveFullODp ) )
				{
					pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_RED );
					bAtLeastOneError = true;
				}

				// Update dp.
				backColor = pclSSheet->GetBackColor( CD_SupplyValve_Dp, lRow );
				pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)backColor );
				pclSSheet->SetStaticText( CD_SupplyValve_Dp, lRow, WriteDouble( CDimValue::SItoCU( _U_DIFFPRESS, dDpFullyOpen ), 3, 0 ) );
			}

			// If a least one error, we must set the name in red, otherwise we reset to black.
			pclSSheet->SetForeColor( CD_SupplyValve_Name, pCDProduct->GetCellPosition().y, ( true == bAtLeastOneError ) ? _RED : _BLACK );

			// Go to next product.
			pclCellDescription = pclSheetDescriptionSV->GetNextCellDescription( pclCellDescription, RVSCellDescription::CD_Product );
		}

		// Update (add or remove) remarks.
		LPARAM lParam;
		pclSheetDescriptionSV->GetUserVariable( _SDUV_REMARKROW, lParam );
		pclSheetDescriptionSV->WriteRemarks( (long)lParam, CD_SupplyValve_Name, CD_SupplyValve_Separator );
	}

	// If there is already a supply valve selection...
	if( m_lSupplyValveSelectedRow != 0 )
	{
		// Set the selection.
		// Remark: it's to force internal variables to maintain the right color under selection.
		pclSSheet->SelectOneRow( m_lSupplyValveSelectedRow, pclSheetDescriptionSV->GetSelectionFrom(), pclSheetDescriptionSV->GetSelectionTo() );
	}
}

void CRViewSSelTrv::_CreateOtherInsertGroup( CDS_SSelRadSet *pSelectedTrv )
{
	if( NULL == m_pclIndSelTRVParams )
	{
		ASSERT_RETURN;
	}

	// Call 'MultiSpreadBase' method to get a new SSheet.
	CSheetDescription *pclSheetDescriptionSVInsert = CreateSSheet( SD_SVInsert );

	if( NULL == pclSheetDescriptionSVInsert || NULL == pclSheetDescriptionSVInsert->GetSSheetPointer() )
	{
		return;
	}

	CSSheet *pclSSheet = pclSheetDescriptionSVInsert->GetSSheetPointer();

	pclSSheet->SetBool( SSB_REDRAW, FALSE );

	pclSSheet->Init();
	pclSSheet->SetBool( SSB_ALLOWDRAGDROP, FALSE );
	pclSSheet->SetBool( SSB_HORZSCROLLBAR, FALSE );
	pclSSheet->SetBool( SSB_VERTSCROLLBAR, FALSE );
	pclSSheet->SetBool( SSB_MOVEACTIVEONFOCUS, FALSE );
	pclSSheet->SetTextTip( SS_TT_STATUS_FLOATING, 250, NULL, _LIGHTYELLOW, _TAH_BLACK );

	pclSSheet->SetMaxRows( RD_SVInsert_DataRow );
	
	// All cells are static by default and filled with _T("").
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	pclSSheet->FormatStaticText( -1, -1, -1, -1, _T("") );

	// Increase ROW height
	double dRowHeight = 12.75;
	pclSSheet->SetRowHeight( RD_SVInsert_FirstRow, dRowHeight * 0.5 );
	pclSSheet->SetRowHeight( RD_SVInsert_GroupName, dRowHeight * 1.5 );
	pclSSheet->SetRowHeight( RD_SVInsert_DataRow, dRowHeight * 2 );

	// Set columns .
	pclSheetDescriptionSVInsert->Init( 1, pclSSheet->GetMaxRows(), 1, CSheetDescription::SD_ParameterMode_Mono );

	pclSheetDescriptionSVInsert->AddColumn( CD_SVInsert_FirstColumn, 4 );
	pclSheetDescriptionSVInsert->AddColumn( CD_SVInsert_Data, 118 );
	pclSheetDescriptionSVInsert->RemoveSelectableRow( RD_SVInsert_DataRow );

	// Add the main title.
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)GetTitleBackgroundColor( pclSheetDescriptionSVInsert ) );
	pclSheetDescriptionSVInsert->SetUserVariable( _SDUV_TITLEFORECOLOR, ( pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor ) ) );
	pclSheetDescriptionSVInsert->SetUserVariable( _SDUV_TITLEBACKCOLOR, ( pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor ) ) );

	pclSSheet->SetStaticText( CD_SVInsert_Data, RD_SVInsert_GroupName, IDS_SHEETHDR_OTHERINSERT );
	
	// Draw black line below column header.
	pclSSheet->SetCellBorder( CD_SVInsert_Data, RD_SVInsert_GroupName, CD_SVInsert_Data, RD_SVInsert_GroupName, true, SS_BORDERTYPE_BOTTOM );

	// Add Kvs (or CV depending of current unit).
	CString str;
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();

	if( 0 == pUnitDB->GetDefaultUnitIndex( _C_KVCVCOEFF ) )
	{
		str = TASApp.LoadLocalizedString( IDS_KVS );
	}
	else
	{
		str = TASApp.LoadLocalizedString( IDS_CV );
	}

	str += CString( _T(" = ") ) + WriteCUDouble( _C_KVCVCOEFF, m_pclIndSelTRVParams->m_dInsertKvValue, false, 3, 2 );
	
	// Add Dp on the insert.
	CString str2 = CString( _T("; ") ) + TASApp.LoadLocalizedString( IDS_SSHEETSSELTRV_DPONINSERT );
	str2 += CString( _T(" = ") );
	
	double dFlow = m_pclIndSelTRVParams->m_dFlow;
	double dKv = m_pclIndSelTRVParams->m_dInsertKvValue;

	m_dSVOtherInsertDp = 0.0;

	// Get current media density.
	if( dFlow > 0.0 && dKv > 0.0 )
	{
		m_dSVOtherInsertDp = CalcDp( dFlow, dKv, m_pclIndSelTRVParams->m_WC.GetDens() );
	}

	if( m_dSVOtherInsertDp > 0.0 )
	{
		str2 += WriteCUDouble( _U_DIFFPRESS, m_dSVOtherInsertDp, true, 3, 2 );
		str += str2;
	}

	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_LEFT );
	pclSSheet->SetStaticText( CD_SVInsert_Data, RD_SVInsert_DataRow, str );

	// Draw black line below data.
	pclSSheet->SetCellBorder( CD_SVInsert_Data, RD_SVInsert_DataRow, CD_SVInsert_Data, RD_SVInsert_DataRow, true, SS_BORDERTYPE_BOTTOM );
}

void CRViewSSelTrv::_ClickOnSupplyValveActuator( CSheetDescription *pclSheetDescriptionActuator, CDB_Actuator *pActuatorClicked, 
		CCellDescriptionProduct *pclCellDescriptionProduct, long lColumn, long lRow, bool bExternalCall )
{
	// Check supply valve selected.
	if( NULL == GetCurrentSupplyValveSelected() )
	{
		return;
	}

	CDB_ControlValve *pclCurrentSupplyValveSelected = GetCurrentSupplyValveSelected();

	if( NULL == pclCurrentSupplyValveSelected )
	{
		return;
	}

	CSSheet *pclSSheet = pclSheetDescriptionActuator->GetSSheetPointer();

	LPARAM lActuatorCount;
	pclSheetDescriptionActuator->GetUserVariable( _SDUV_TOTALACTUATOR_COUNT, lActuatorCount );

	// Memorize previous focus row.
	long lCurrentFocusedRow = GetCurrentFocusedRow();

	// To be sure to clean all previous data concerning focus.
	KillCurrentFocus();

	// Unselect row where current selection is set (yellow light).
	// Remark: 'KillCurrentFocus' unselect only rows where focus is on (blue light).
	pclSSheet->UnSelectMultipleRows();

	// Retrieve the current selected actuator if exist.
	CDB_Actuator *pclCurrentActuatorSelected = GetCurrentSVActuatorSelected();

	// If there is already one actuator selected and user click on the current one...
	// Remark: 'm_pCDBExpandCollapseRowsSVActuator' is not created if there is only one actuator. Thus we need to check first if there is only one actuator.
	//         Otherwise we have to check if user has clicked on the current one.
	if( NULL != pclCurrentActuatorSelected 
			&& ( ( 1 == lActuatorCount ) 
				|| ( NULL != m_pCDBExpandCollapseRowsSVActuator && lRow == m_pCDBExpandCollapseRowsSVActuator->GetCellPosition().y ) ) )
	{
		// Reset current product selected.
		SetCurrentSVActuatorSelected( NULL );

		// Uncheck checkbox.
		pclSSheet->SetCheckBox( SD_SVThermoActuator == m_uiSDActuatorID ? CD_SVThrmActuator_CheckBox : CD_SVElecActuator_CheckBox, lRow, _T(""), false, true );

		// Delete Expand/Collapse rows button if exist.
		// Remark: 'm_pCDBExpandCollapseRowsSVActuator' is set to NULL in method!
		DeleteExpandCollapseRowsButton( m_pCDBExpandCollapseRowsSVActuator, pclSheetDescriptionActuator );

		// Set focus on actuator currently selected.
		PrepareAndSetNewFocus( pclSheetDescriptionActuator, pclSheetDescriptionActuator->GetActiveColumn(), lRow, 0 );

		if( true == pclCurrentSupplyValveSelected->IsPartOfaSet() )
		{
			CSheetDescription *pclSheetDescriptionSupplyValve = m_ViewDescription.GetFromSheetDescriptionID( SD_SupplyValve );
			
			if( NULL != pclSheetDescriptionSupplyValve )
			{
				// Change box button to open state for the supply valve.
				CCDButtonBox *pCDButtonBox = GetCDButtonBox( CD_SupplyValve_Box, m_lSupplyValveSelectedRow, pclSheetDescriptionSupplyValve );

				if( NULL != pCDButtonBox )
				{
					pCDButtonBox->SetButtonState( CCDButtonBox::ButtonState::BoxOpened,  true );
				}
			}

			// Change box button to open state for actuator selected.
			int iBoxCol = ( SD_SVThermoActuator == m_uiSDActuatorID ) ? CD_SVThrmActuator_Box : CD_SVElecActuator_Box;
			CCDButtonBox *pCDButtonBox = GetCDButtonBox( iBoxCol, lRow, pclSheetDescriptionActuator );

			if( NULL != pCDButtonBox )
			{
				pCDButtonBox->SetButtonState( CCDButtonBox::ButtonState::BoxOpened,  true );
			}
		}

		m_lSVActuatorSelectedRow = 0;
	}
	else
	{
		// If an actuator is already selected...
		if( NULL != pclCurrentActuatorSelected )
		{
			// Change box button to open state for previous actuator.
			if( true == pclCurrentSupplyValveSelected->IsPartOfaSet() )
			{
				int iBoxCol = ( SD_SVThermoActuator == m_uiSDActuatorID ) ? CD_SVThrmActuator_Box : CD_SVElecActuator_Box;
				CCDButtonBox *pCDButtonBox = GetCDButtonBox( iBoxCol, m_lSVActuatorSelectedRow, pclSheetDescriptionActuator );

				if( NULL != pCDButtonBox )
				{
					pCDButtonBox->SetButtonState( CCDButtonBox::ButtonState::BoxOpened,  true );
				}
			}

			// Uncheck checkbox.
			pclSSheet->SetCheckBox( SD_SVThermoActuator == m_uiSDActuatorID ? CD_SVThrmActuator_CheckBox : CD_SVElecActuator_CheckBox, 
					m_lSVActuatorSelectedRow, _T(""), false, true );
		}

		// Save new actuator selection.
		SetCurrentSVActuatorSelected( pclCellDescriptionProduct );

		// Uncheck checkbox.
		pclSSheet->SetCheckBox( SD_SVThermoActuator == m_uiSDActuatorID ? CD_SVThrmActuator_CheckBox : CD_SVElecActuator_CheckBox, lRow, _T(""), true, true );

		// Delete Expand/Collapse rows button if exist.
		if( NULL != m_pCDBExpandCollapseRowsSVActuator )
		{
			// Remark: 'm_pCDBExpandCollapseRowsSVActuator' is set to NULL in method!
			DeleteExpandCollapseRowsButton( m_pCDBExpandCollapseRowsSVActuator, pclSheetDescriptionActuator );
		}

		// Create Expand/Collapse rows button if needed...
		if( lActuatorCount > 1 )
		{
			m_pCDBExpandCollapseRowsSVActuator = CreateExpandCollapseRowsButton( CD_SVThrmActuator_FirstColumn, lRow, true, 
					CCDButtonExpandCollapseRows::ButtonState::CollapseRow, pclSheetDescriptionActuator->GetFirstSelectableRow(), 
					pclSheetDescriptionActuator->GetLastSelectableRow( false ), pclSheetDescriptionActuator );

			// Show button.
			if( NULL != m_pCDBExpandCollapseRowsSVActuator )
			{
				m_pCDBExpandCollapseRowsSVActuator->SetShowStatus( true );
			}
		}
		// HYS-987: Fill supply valve actuator accessory row
		// Retrieve accessories belonging to the selected actuator.
		CRank rActuatorAccessoryList;
		CDB_RuledTableBase *pclRuledTable = NULL;
		GetCurrentSVActuatorAccessoryList( GetCurrentSVActuatorSelected(), &rActuatorAccessoryList, &pclRuledTable );

		if( rActuatorAccessoryList.GetCount() > 0 )
		{
			FillRowsAcc( false, &rActuatorAccessoryList, pclRuledTable );
		}

		// Select actuator (just highlight background).
		pclSSheet->SelectOneRow( lRow, pclSheetDescriptionActuator->GetSelectionFrom(), pclSheetDescriptionActuator->GetSelectionTo() );

		// Set focus on current selection.
		PrepareAndSetNewFocus( pclSheetDescriptionActuator, lColumn, lRow );

		m_lSVActuatorSelectedRow = lRow;

		// If user has selected Trv-head set and user has directly clicked on actuator (otherwise, it's a call 
		// coming from '_ClickOnSupplyValve' and this method will check itself if it must close or not the box button)...
		if( true == pclCurrentSupplyValveSelected->IsPartOfaSet() && true == bExternalCall )
		{
			CSheetDescription *pclSheetDescriptionSupplyValve = m_ViewDescription.GetFromSheetDescriptionID( SD_SupplyValve );
			
			if( NULL != pclSheetDescriptionSupplyValve )
			{
				// Change box button to open state for the supply valve.
				CCDButtonBox *pCDButtonBox = GetCDButtonBox( CD_SupplyValve_Box, m_lSupplyValveSelectedRow, pclSheetDescriptionSupplyValve );

				if( NULL != pCDButtonBox )
				{
					pCDButtonBox->SetButtonState( CCDButtonBox::ButtonState::BoxClosed,  true );
				}
			}

			// Change box button to open state for actuator selected.
			int iBoxCol = ( SD_SVThermoActuator == m_uiSDActuatorID ) ? CD_SVThrmActuator_Box : CD_SVElecActuator_Box;
			CCDButtonBox *pCDButtonBox = GetCDButtonBox( iBoxCol, lRow, pclSheetDescriptionActuator );

			if( NULL != pCDButtonBox )
			{
				pCDButtonBox->SetButtonState( CCDButtonBox::ButtonState::BoxClosed,  true );
			}
		}
	}
}

void CRViewSSelTrv::_ClickOnReturnValve( CSheetDescription *pclSheetDescriptionReturnValve, CDB_RegulatingValve *pReturnValveClicked, 
		CCellDescriptionProduct *pclCellDescriptionProduct, long lColumn, long lRow, bool bExternalCall )
{
	if( NULL == m_pclIndSelTRVParams || NULL == m_pclIndSelTRVParams->m_pclSelectSupplyValveList 
			|| NULL == m_pclIndSelTRVParams->m_pclSelectSupplyValveList->GetReturnValveList() )
	{
		ASSERT_RETURN;
	}

	// We don't check if user has already selected a supply valve like in another selection. It's absolutely possible to select a return valve without
	// supply valve.
	CSSheet *pclSSheet = pclSheetDescriptionReturnValve->GetSSheetPointer();

	// Memorize previous focus row.
	long lCurrentFocusedRow = GetCurrentFocusedRow();
	
	// To be sure to clean all previous data concerning focus.
	KillCurrentFocus();

	// Unselect row where current selection is set (yellow light).
	// Remark: 'KillCurrentFocus' unselect only rows where focus is on (blue light).
	pclSSheet->UnSelectMultipleRows();

	// Clean previous accessory list.
	m_vecReturnValveAccessoryList.clear();

	LPARAM lReturnValveCount;
	pclSheetDescriptionReturnValve->GetUserVariable( _SDUV_TOTALPRODUCT_COUNT, lReturnValveCount );

	// Retrieve the current selected return valve if exist.
	CDB_TAProduct *pclCurrentReturnValveSelected = GetCurrentReturnValveSelected();

	// If there is already one return valve selected and user click on the current one...
	// Remark: 'm_pCDBExpandCollapseRowsReturnValve' is not created if there is only one return valve. Thus we need to check first if there is only one return valve.
	//         Otherwise we have to check if user has clicked on the current one.
	if( NULL != pclCurrentReturnValveSelected 
			&& ( ( 1 == lReturnValveCount ) 
				|| ( NULL != m_pCDBExpandCollapseRowsReturnValve && lRow == m_pCDBExpandCollapseRowsReturnValve->GetCellPosition().y ) ) )
	{
		// Reset current product selected.
		SetCurrentReturnValveSelected( NULL );

		// Uncheck checkbox.
		pclSSheet->SetCheckBox( CD_ReturnValve_CheckBox, lRow, _T(""), false, true );

		// Delete Expand/Collapse rows button if exist.
		// Remark: 'm_pCDBExpandCollapseRowsReturnValve' is set to NULL in method!
		DeleteExpandCollapseRowsButton( m_pCDBExpandCollapseRowsReturnValve, pclSheetDescriptionReturnValve );

		// Show Show/Hide all priorities button if exist.
		if( NULL != m_pCDBShowAllPrioritiesReturnValve )
		{
			m_pCDBShowAllPrioritiesReturnValve->ApplyInternalChange();
		}

		// Set focus on return valve currently selected.
		PrepareAndSetNewFocus( pclSheetDescriptionReturnValve, pclSheetDescriptionReturnValve->GetActiveColumn(), lRow, 0 );

		// Remove all sheets linked to return valve.
		m_ViewDescription.RemoveOneSheetDescription( SD_ReturnValveAccessories );

		m_lReturnValveSelectedRow = 0;
		
		// Notify a change in the selection.
		SelectionHasChanged( IsSelectionReady() );

		// Clear 'Setting' and 'Dp' columns of all supply valves.
		if( true == m_bMustFillSVSettingDp )
		{
			_UpdateSupplyValveSettingDp( -1.0 );
		}
	}
	else
	{
		// If a return valve is already selected...
		if( NULL != pclCurrentReturnValveSelected )
		{
			// Remove all sheets linked to return valve.
			m_ViewDescription.RemoveOneSheetDescription( SD_ReturnValveAccessories );

			// uncheck checkbox
			pclSSheet->SetCheckBox(CD_ReturnValve_CheckBox, m_lReturnValveSelectedRow, _T(""), false, true);
		}

		// Save new return valve selection.
		SetCurrentReturnValveSelected( pclCellDescriptionProduct );

		// Check checkbox.
		pclSSheet->SetCheckBox( CD_ReturnValve_CheckBox, lRow, _T(""), true, true );

		// Delete Expand/Collapse rows button if exist.
		if( NULL != m_pCDBExpandCollapseRowsReturnValve )
		{
			// Remark: 'm_pCDBExpandCollapseRowsReturnValve' is set to NULL in method!
			DeleteExpandCollapseRowsButton( m_pCDBExpandCollapseRowsReturnValve, pclSheetDescriptionReturnValve );
		}

		// Create Expand/Collapse rows button if needed...
		if( lReturnValveCount > 1 )
		{
			m_pCDBExpandCollapseRowsReturnValve = CreateExpandCollapseRowsButton( CD_ReturnValve_FirstColumn, lRow, true, 
					CCDButtonExpandCollapseRows::ButtonState::CollapseRow, pclSheetDescriptionReturnValve->GetFirstSelectableRow(), 
					pclSheetDescriptionReturnValve->GetLastSelectableRow( false ), pclSheetDescriptionReturnValve );

			// Show button.
			if( NULL != m_pCDBExpandCollapseRowsReturnValve )
			{
				m_pCDBExpandCollapseRowsReturnValve->SetShowStatus( true );
			}
		}

		// Select return valve (just highlight background).
		pclSSheet->SelectOneRow( lRow, pclSheetDescriptionReturnValve->GetSelectionFrom(), pclSheetDescriptionReturnValve->GetSelectionTo() );

		// Fill accessories available for the current return valve.
		_FillReturnValveAccessoryRows();

		// Try to set the focus on the next table but do not lose the focus if no other table exist.
		CSheetDescription *pclNextSheetDescription = NULL;
		long lNewFocusedRow;
		bool fShiftPressed;
		
		if( DNFERRC_FocusOnNextGroupOK == DefineNextFocusedRow( pclSheetDescriptionReturnValve, CD_ReturnValve_FirstColumn, lRow, false, 
				lNewFocusedRow, pclNextSheetDescription, fShiftPressed ) )
		{
			long lNewFocusedCol = pclNextSheetDescription->GetFirstParameterColumn();
			PrepareAndSetNewFocus( pclNextSheetDescription, lNewFocusedCol, lNewFocusedRow );
		}
		else
		{
			// Because we have changed current selection background in yellow, if there is no other sheet following current one, we
			// must set focus on current selection.
			PrepareAndSetNewFocus( pclSheetDescriptionReturnValve, lColumn, lRow );
		}

		m_lReturnValveSelectedRow = lRow;

		// Notify a change in the selection.
		SelectionHasChanged( IsSelectionReady() );

		// Update 'Setting' and 'Dp' columns of all supply valves if needed...
		if( true == m_bMustFillSVSettingDp )
		{
			CDB_TAProduct *pclReturnValve = dynamic_cast<CDB_TAProduct*>( (CData*)pclCellDescriptionProduct->GetProduct() );

			if( NULL != pclReturnValve )
			{
				CSelectedValve *pclSelectedReturnValve = GetSelectProduct<CSelectedValve>( pclReturnValve, 
						m_pclIndSelTRVParams->m_pclSelectSupplyValveList->GetReturnValveList() );

				if( NULL != pclSelectedReturnValve )
				{
					_UpdateSupplyValveSettingDp( pclSelectedReturnValve->GetDp() );
				}
			}
		}
	}
}

long CRViewSSelTrv::_FillReturnValveRows( CDB_TAProduct *pEditedReturnValve )
{
	if( NULL == m_pclIndSelTRVParams || NULL == m_pclIndSelTRVParams->m_pclSelectSupplyValveList )
	{
		ASSERTA_RETURN( 0 );
	}

	// Exit if no return valve
	if( NULL == m_pclIndSelTRVParams->m_pclSelectSupplyValveList->GetReturnValveList() )
	{
		return 0;
	}

	// Retrieve return valve list.
	CSelectList *pSelectedReturnValveList = m_pclIndSelTRVParams->m_pclSelectSupplyValveList->GetReturnValveList();

	// If no return valve exist...
	if( 0 == pSelectedReturnValveList->GetCount() )
	{
		return 0;
	}

	// Call 'MultiSpreadBase' method to get a new SSheet.
	CSheetDescription *pclSheetDescriptionReturnValve = CreateSSheet( SD_ReturnValve );

	if( NULL == pclSheetDescriptionReturnValve || NULL == pclSheetDescriptionReturnValve->GetSSheetPointer() )
	{
		return 0;
	}
	
	CSSheet *pclSSheet = pclSheetDescriptionReturnValve->GetSSheetPointer();

	pclSSheet->SetBool( SSB_REDRAW, FALSE );

	pclSSheet->Init();
	pclSSheet->SetBool( SSB_ALLOWDRAGDROP, FALSE );
	pclSSheet->SetBool( SSB_HORZSCROLLBAR, FALSE );
	pclSSheet->SetBool( SSB_VERTSCROLLBAR, FALSE );
	pclSSheet->SetBool( SSB_MOVEACTIVEONFOCUS, FALSE );
	pclSSheet->SetTextTip( SS_TT_STATUS_FLOATING, 250, NULL, _LIGHTYELLOW, _TAH_BLACK );

	// Initialize return valve sheet and fill header.
	_InitAndFillReturnValveHeader( pclSheetDescriptionReturnValve, pclSSheet );

	long lRetRow = -1;
	long lRow = RD_ReturnValve_FirstAvailRow;

	long lReturnValveNumbers = 0;
	long lReturnValveNotPriorityNumbers = 0;
	bool bShowAllPrioritiesShown = false;
	long lShowAllPrioritiesButtonRow = 0;

	pclSheetDescriptionReturnValve->SetUserVariable( _SDUV_TOTALPRODUCT_COUNT, 0 );
	pclSheetDescriptionReturnValve->SetUserVariable( _SDUV_NOPRIORITYPRODUCT_COUNT, 0 );
	
	pclSheetDescriptionReturnValve->RestartRemarkGenerator();
	
	for( CSelectedValve *pclSelectedReturnValve = pSelectedReturnValveList->GetFirst<CSelectedValve>(); 
			NULL != pclSelectedReturnValve; pclSelectedReturnValve = pSelectedReturnValveList->GetNext<CSelectedValve>() )
	{
		CDB_TAProduct *pTAP = dynamic_cast<CDB_TAProduct *>( pclSelectedReturnValve->GetpData() );

		if( NULL == pTAP ) 
		{
			continue;
		}

		if( true == pclSelectedReturnValve->IsFlagSet(CSelectedBase::eNotPriority) )
		{
			lReturnValveNotPriorityNumbers++;

			if( false == bShowAllPrioritiesShown )
			{
				// Add one empty not selectable row.
				pclSheetDescriptionReturnValve->AddRows( 1 );
				pclSheetDescriptionReturnValve->RemoveSelectableRow( lRow );

				pclSSheet->SetCellBorder( CD_ReturnValve_CheckBox, lRow, CD_ReturnValve_Separator - 1, lRow, true, SS_BORDERTYPE_TOP | SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );
				pclSSheet->SetCellBorder( CD_ReturnValve_PipeSize, lRow, CD_ReturnValve_Pointer - 1, lRow, true, SS_BORDERTYPE_TOP | SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );

				lShowAllPrioritiesButtonRow = lRow++;
				bShowAllPrioritiesShown = true;
			}
		}

		long lRetVal = _FillOneReturnValveRow( pclSheetDescriptionReturnValve, pclSSheet, pclSelectedReturnValve, lRow, pEditedReturnValve );
		
		if( lRetVal > 0 )
		{
			lRetRow = lRetVal;
		}

		lRow++;
		lReturnValveNumbers++;
	}

	long lLastDataRow = lRow - 1;

	pclSheetDescriptionReturnValve->SetUserVariable( _SDUV_TOTALPRODUCT_COUNT, lReturnValveNumbers );
	pclSheetDescriptionReturnValve->SetUserVariable( _SDUV_NOPRIORITYPRODUCT_COUNT, lReturnValveNotPriorityNumbers );
	
	// Now we can create Show/Hide all priorities if needed.
	// Remarks: 2nd condition to avoid to have button if we have only no priority valves.
	if( true == bShowAllPrioritiesShown && lReturnValveNumbers > lReturnValveNotPriorityNumbers )
	{
		// Create Show/Hide all priorities button in regards to current state!
		CCDButtonShowAllPriorities::ButtonState eState = ( true == GetShowAllPrioritiesFlag( SD_ReturnValve ) ) ? 
				CCDButtonShowAllPriorities::ButtonState::HidePriorities : CCDButtonShowAllPriorities::ButtonState::ShowPriorities;
		
		CCDButtonShowAllPriorities* pclShowAllButton = CreateShowAllPrioritiesButton( CD_ReturnValve_FirstColumn, lShowAllPrioritiesButtonRow, 
				bShowAllPrioritiesShown, eState, lShowAllPrioritiesButtonRow + 1, pclSSheet->GetMaxRows(), pclSheetDescriptionReturnValve );

		// Here we must call 'ApplyInternalChange' to show or hide rows in regards to the current state!
		if( NULL != pclShowAllButton )
		{
			pclShowAllButton->ApplyInternalChange( );
		}
	}
	else
	{
		if( lShowAllPrioritiesButtonRow != 0 )
		{
			pclSSheet->ShowRow( lShowAllPrioritiesButtonRow, false );
		}
	}

	// Try to merge only if there is more than one return valve.
	if( lReturnValveNumbers > 2 || ( 2 == lReturnValveNumbers && lReturnValveNotPriorityNumbers != 1 ) )
	{
		vector<long> vecColumnList;
		vecColumnList.push_back( CD_ReturnValve_PipeSize );
		vecColumnList.push_back( CD_ReturnValve_PipeLinDp );
		vecColumnList.push_back( CD_ReturnValve_PipeV );
		pclSSheet->MergeRowsWithSameValues( RD_ReturnValve_FirstAvailRow, lLastDataRow, vecColumnList );
	}

	pclSSheet->SetCellBorder( CD_ReturnValve_CheckBox, lLastDataRow, CD_ReturnValve_Separator - 1, lLastDataRow, true, 
			SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );

	// Needed by pipe merging .
	pclSSheet->SetCellBorder( CD_ReturnValve_PipeSize, lLastDataRow, CD_ReturnValve_Pointer - 1, lLastDataRow, true, 
			SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );

	// Add Remarks.
	// Note: these lines must be placed after creating Show/Hide all priorities button to avoid to take in count these lines when defining
	//       range of row to be Show/Hide.
	pclSheetDescriptionReturnValve->SetUserVariable( _SDUV_REMARKROW, lRow );
	lRow = pclSheetDescriptionReturnValve->WriteRemarks( lRow, CD_ReturnValve_CheckBox, CD_ReturnValve_Separator );

	pclSSheet->SetBool( SSB_REDRAW, TRUE );

	// Set that there is no selection at now.
	SetCurrentReturnValveSelected( NULL );

	// Add possibility to change column size.
	// Remark: place this code after any initialization (like adding, removing, resizing, hiding/showing columns).
	CResizingColumnInfo* pclResizingColumnInfo = pclSheetDescriptionReturnValve->GetResizingColumnInfo();
	pclResizingColumnInfo->ActivateFeature( true, false, RD_ReturnValve_FirstAvailRow, lLastDataRow );
	pclResizingColumnInfo->AddRangeColumn( CD_ReturnValve_Name, CD_ReturnValve_TemperatureRange, RD_ReturnValve_ColName, RD_ReturnValve_Unit );
	pclResizingColumnInfo->AddRangeColumn( CD_ReturnValve_PipeSize, CD_ReturnValve_PipeV, RD_ReturnValve_ColName, RD_ReturnValve_Unit );

	// Add possibility to click on title group to reset column width.
	CreateTitleGroupButtons( CD_ReturnValve_CheckBox, CD_ReturnValve_PipeV, RD_ReturnValve_GroupName, pclSheetDescriptionReturnValve );

	return lRetRow;
}

void CRViewSSelTrv::_InitAndFillReturnValveHeader( CSheetDescription *pclSheetDescriptionReturnValve, CSSheet *pclSSheet )
{
	if( NULL == m_pclIndSelTRVParams || NULL == m_pclIndSelTRVParams->m_pclSelectSupplyValveList 
			|| NULL == m_pclIndSelTRVParams->m_pclSelectSupplyValveList->GetReturnValveList() 
			|| NULL == m_pclIndSelTRVParams->m_pPipeDB )
	{
		ASSERT_RETURN;
	}

	// Flags for column customizing.
	CSelectedBase *pclSelectedProduct = m_pclIndSelTRVParams->m_pclSelectSupplyValveList->GetReturnValveList()->GetFirst<CSelectedBase>();

	if( NULL == pclSelectedProduct )
	{
		return;
	}

	CDB_TAProduct *pTAP = dynamic_cast<CDB_TAProduct *>( pclSelectedProduct->GetpData() );
	
	if( NULL == pTAP ) 
	{
		return;
	}

	bool bIsUserPressureDrop = m_pclIndSelTRVParams->m_bDpEnabled;

	pclSSheet->SetMaxRows( RD_ReturnValve_FirstAvailRow - 1 );
	
	// All cells are static by default and filled with _T("")
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	pclSSheet->FormatStaticText( -1, -1, -1, -1, _T("") );

	// Increase ROW height
	double dRowHeight = 12.75;
	pclSSheet->SetRowHeight( RD_ReturnValve_FirstRow, dRowHeight * 0.5 );
	pclSSheet->SetRowHeight( RD_ReturnValve_GroupName, dRowHeight * 1.5 );
	pclSSheet->SetRowHeight( RD_ReturnValve_ColName, dRowHeight * 2 );
	pclSSheet->SetRowHeight( RD_ReturnValve_Unit, dRowHeight * 1.2 );

	// Set columns 
	pclSheetDescriptionReturnValve->Init( 1, pclSSheet->GetMaxRows(), 1, CSheetDescription::SD_ParameterMode_Mono );
	pclSheetDescriptionReturnValve->AddColumnInPixels( CD_ReturnValve_FirstColumn, m_mapSSheetColumnWidth[SD_ReturnValve][CD_ReturnValve_FirstColumn] );
	pclSheetDescriptionReturnValve->AddColumnInPixels( CD_ReturnValve_CheckBox, m_mapSSheetColumnWidth[SD_ReturnValve][CD_ReturnValve_CheckBox] );
	pclSheetDescriptionReturnValve->AddColumnInPixels( CD_ReturnValve_Name, m_mapSSheetColumnWidth[SD_ReturnValve][CD_ReturnValve_Name] );
	pclSheetDescriptionReturnValve->AddColumnInPixels( CD_ReturnValve_Connection, m_mapSSheetColumnWidth[SD_ReturnValve][CD_ReturnValve_Connection] );
	pclSheetDescriptionReturnValve->AddColumnInPixels( CD_ReturnValve_Version, m_mapSSheetColumnWidth[SD_ReturnValve][CD_ReturnValve_Version] );
	pclSheetDescriptionReturnValve->AddColumnInPixels( CD_ReturnValve_PN, m_mapSSheetColumnWidth[SD_ReturnValve][CD_ReturnValve_PN] );
	pclSheetDescriptionReturnValve->AddColumnInPixels( CD_ReturnValve_Size, m_mapSSheetColumnWidth[SD_ReturnValve][CD_ReturnValve_Size] );
	pclSheetDescriptionReturnValve->AddColumnInPixels( CD_ReturnValve_Preset, m_mapSSheetColumnWidth[SD_ReturnValve][CD_ReturnValve_Preset] );
	pclSheetDescriptionReturnValve->AddColumnInPixels( CD_ReturnValve_Dp, m_mapSSheetColumnWidth[SD_ReturnValve][CD_ReturnValve_Dp] );
	pclSheetDescriptionReturnValve->AddColumnInPixels( CD_ReturnValve_TemperatureRange, m_mapSSheetColumnWidth[SD_ReturnValve][CD_ReturnValve_TemperatureRange] );
	pclSheetDescriptionReturnValve->AddColumnInPixels( CD_ReturnValve_Separator, m_mapSSheetColumnWidth[SD_ReturnValve][CD_ReturnValve_Separator] );
	pclSheetDescriptionReturnValve->AddColumnInPixels( CD_ReturnValve_PipeSize, m_mapSSheetColumnWidth[SD_ReturnValve][CD_ReturnValve_PipeSize] );
	pclSheetDescriptionReturnValve->AddColumnInPixels( CD_ReturnValve_PipeLinDp, m_mapSSheetColumnWidth[SD_ReturnValve][CD_ReturnValve_PipeLinDp] );
	pclSheetDescriptionReturnValve->AddColumnInPixels( CD_ReturnValve_PipeV, m_mapSSheetColumnWidth[SD_ReturnValve][CD_ReturnValve_PipeV] );
	pclSheetDescriptionReturnValve->AddColumnInPixels( CD_ReturnValve_Pointer, m_mapSSheetColumnWidth[SD_ReturnValve][CD_ReturnValve_Pointer] );

	// Set in which column parameter must be saved.
	pclSheetDescriptionReturnValve->AddParameterColumn( CD_ReturnValve_Pointer );
	
	// Set the focus column.
	pclSheetDescriptionReturnValve->SetActiveColumn( CD_ReturnValve_Name );

	// Set range for selection.
	pclSheetDescriptionReturnValve->SetFocusColumnRange( CD_ReturnValve_CheckBox, CD_ReturnValve_TemperatureRange );

	// Hide columns corresponding to user selected combos.
	pclSSheet->ShowCol( CD_ReturnValve_Connection, ( true == m_pclIndSelTRVParams->m_strComboRVConnectID.IsEmpty() ) ? TRUE : FALSE );
	pclSSheet->ShowCol( CD_ReturnValve_Version, ( true == m_pclIndSelTRVParams->m_strComboRVVersionID.IsEmpty() ) ? TRUE : FALSE );

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Row name.
	
	// Add the main title.
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)GetTitleBackgroundColor( pclSheetDescriptionReturnValve ) );
	pclSheetDescriptionReturnValve->SetUserVariable( _SDUV_TITLEFORECOLOR, ( pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor ) ) );
	pclSheetDescriptionReturnValve->SetUserVariable( _SDUV_TITLEBACKCOLOR, ( pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor ) ) );

	pclSSheet->AddCellSpanW( CD_ReturnValve_CheckBox, RD_ReturnValve_GroupName, CD_ReturnValve_Separator - CD_ReturnValve_CheckBox, 1 );
	pclSSheet->SetStaticText( CD_ReturnValve_CheckBox, RD_ReturnValve_GroupName, IDS_SSHEETSSELTRV_RVGROUP );
	pclSSheet->AddCellSpanW( CD_ReturnValve_PipeSize, RD_ReturnValve_GroupName, CD_ReturnValve_Pointer - CD_ReturnValve_PipeSize, 1 );
	
	CTable *pclTable = dynamic_cast<CTable *>( m_pclIndSelTRVParams->m_pPipeDB->Get( m_pclIndSelTRVParams->m_strPipeSeriesID ).MP );
	ASSERT( NULL != pclTable );

	pclSSheet->SetStaticText( CD_ReturnValve_PipeSize, RD_ReturnValve_GroupName, pclTable->GetName() );
	
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

	pclSSheet->SetStaticText( CD_ReturnValve_Name, RD_ReturnValve_ColName, IDS_SSHEETSSEL_NAME );
	pclSSheet->SetStaticText( CD_ReturnValve_Connection, RD_ReturnValve_ColName, IDS_SSHEETSSEL_CONNECT );
	pclSSheet->SetStaticText( CD_ReturnValve_Version, RD_ReturnValve_ColName, IDS_SSHEETSSEL_VERSION );
	pclSSheet->SetStaticText( CD_ReturnValve_PN, RD_ReturnValve_ColName, IDS_SSHEETSSEL_PN );
	pclSSheet->SetStaticText( CD_ReturnValve_Size, RD_ReturnValve_ColName, IDS_SSHEETSSEL_SIZE );
	pclSSheet->SetStaticText( CD_ReturnValve_Preset, RD_ReturnValve_ColName, IDS_SSHEETSSEL_PRESET );
	pclSSheet->SetStaticText( CD_ReturnValve_Dp, RD_ReturnValve_ColName, IDS_SSHEETSSEL_DP );
	pclSSheet->SetStaticText( CD_ReturnValve_TemperatureRange, RD_ReturnValve_ColName, IDS_SSHEETSSEL_COLTEMPRANGE );

	pclSSheet->SetStaticText( CD_ReturnValve_Separator, RD_ReturnValve_ColName, _T("") );

	// Pipes.
	pclSSheet->SetStaticText( CD_ReturnValve_PipeSize, RD_ReturnValve_ColName, IDS_SSHEETSSEL_PIPESIZE );
	pclSSheet->SetStaticText( CD_ReturnValve_PipeLinDp, RD_ReturnValve_ColName, IDS_SSHEETSSEL_PIPELINDP );
	pclSSheet->SetStaticText( CD_ReturnValve_PipeV, RD_ReturnValve_ColName, IDS_SSHEETSSEL_PIPEV );
	//
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Row units.
	CUnitDatabase* pUnitDB = CDimValue::AccessUDB();
	pclSSheet->SetStaticText( CD_ReturnValve_Preset, RD_ReturnValve_Unit, IDS_SHEETHDR_TURNSPOS );
	pclSSheet->SetStaticText( CD_ReturnValve_Dp, RD_ReturnValve_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ) ).c_str() );
	pclSSheet->SetStaticText( CD_ReturnValve_TemperatureRange, RD_ReturnValve_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_TEMPERATURE ) ).c_str() );

	// Units.
	pclSSheet->SetStaticText( CD_ReturnValve_PipeLinDp, RD_ReturnValve_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_LINPRESSDROP ) ).c_str() );
	pclSSheet->SetStaticText( CD_ReturnValve_PipeV, RD_ReturnValve_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_VELOCITY ) ).c_str() );
	//
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Draw black line below column header.
	pclSSheet->SetCellBorder( CD_ReturnValve_CheckBox, RD_ReturnValve_Unit, CD_ReturnValve_Separator - 1, RD_ReturnValve_Unit, true, SS_BORDERTYPE_BOTTOM );
	pclSSheet->SetCellBorder( CD_ReturnValve_PipeSize, RD_ReturnValve_Unit, CD_ReturnValve_Pointer - 1, RD_ReturnValve_Unit, true, SS_BORDERTYPE_BOTTOM );
}

long CRViewSSelTrv::_FillOneReturnValveRow( CSheetDescription *pclSheetDescriptionReturnValve, CSSheet *pclSSheet, CSelectedValve *pSelectedTAP, 
		long lRow, CDB_TAProduct *pEditedReturnValve )
{
	if( NULL == m_pclIndSelTRVParams || NULL == m_pclIndSelTRVParams->m_pTADB || NULL == m_pclIndSelTRVParams->m_pclSelectSupplyValveList
			|| NULL == m_pclIndSelTRVParams->m_pclSelectSupplyValveList->GetSelectPipeList() )
	{
		ASSERTA_RETURN( -1 )
	}

	CDS_TechnicalParameter *pTechParam = m_pclIndSelTRVParams->m_pTADS->GetpTechParams();
	CDB_RegulatingValve *pReturnValve = dynamic_cast<CDB_RegulatingValve *>( pSelectedTAP->GetpData() );

	if( NULL == pReturnValve ) 
	{
		return -1;
	}
	
	CString str;
	long lRetRow = 0;
	bool bAtLeastOneError = false;
	bool bBest = pSelectedTAP->IsFlagSet(CSelectedBase::eBest );

	// Add this row.
	// 'true' to specify that this row can be selected.
	pclSheetDescriptionReturnValve->AddRows( 1, true );

	// First columns will be set at the end!
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

	// add checkbox
	pclSSheet->SetCheckBox(CD_ReturnValve_CheckBox, lRow, _T(""), false, true);

	// Set opening and Dp.
	
	// Remark: Dp total is Dp entered by user (Dp wanted on supply + return valve).
	double dDpTotal = m_pclIndSelTRVParams->m_dDp;
	bool bDpOrange = false;
	
	if( true == m_pclIndSelTRVParams->m_bDpEnabled && false == m_pclIndSelTRVParams->m_bIsTrvTypePreset 
		&& ( ( RadiatorReturnValveMode::RRVM_IMI == m_pclIndSelTRVParams->m_eReturnValveMode && true == m_rSuggestParams.m_bIsDifferentDpOnSVExist )
			|| RadiatorReturnValveMode::RRVM_Nothing == m_pclIndSelTRVParams->m_eReturnValveMode ) 
		&& pSelectedTAP->GetDpFullOpen() < dDpTotal )
	{
		pclSSheet->SetStaticText( CD_ReturnValve_Preset, lRow, _T("-") );
		pclSSheet->SetStaticText( CD_ReturnValve_Dp, lRow, _T("-") );
		m_bMustFillRVSettingDp = true;
	}
	else
	{
		// Opening.
		if( NULL != pReturnValve->GetValveCharacteristic() )
		{
			str = pReturnValve->GetValveCharacteristic()->GetSettingString( pSelectedTAP->GetH() );
		}
		else
		{
			str = WriteDouble( pSelectedTAP->GetH(), 2, 1 );
		}

		if( true == pSelectedTAP->IsFlagSet( CSelectedBase::eValveMaxSetting ) )
		{
			pclSheetDescriptionReturnValve->WriteTextWithFlags( str, CD_ReturnValve_Preset, lRow, CSheetDescription::RemarkFlags::FullOpening );
		}
		else
		{
			pclSSheet->SetStaticText( CD_ReturnValve_Preset, lRow, str );
		}
		
		// Dp.
		if( true == m_pclIndSelTRVParams->m_bDpEnabled && false == m_pclIndSelTRVParams->m_bIsTrvTypePreset )
		{
			// If user wants a Dp and non presettable supply valves has Dp already above this value, we force full opening when searching return valves.
			// In that case, Dp flag is not set for the return valve. But here we want to show in red because it is not correct.
				
			CSelectTrvList *pSelectTrvList = m_pclIndSelTRVParams->m_pclSelectSupplyValveList;
			
			if( NULL != pSelectTrvList )
			{
				// Retrieve current supply valve selected.
				CDB_ControlValve *pclCurrentSupplyValve = GetCurrentSupplyValveSelected();
				CSelectedValve *pSelectedSV = NULL;

				if( NULL != pclCurrentSupplyValve )
				{
					pSelectedSV = GetSelectProduct<CSelectedValve>( pclCurrentSupplyValve, m_pclIndSelTRVParams->m_pclSelectSupplyValveList );
				}
				else
				{
					// No supply valve selected. If pressure drop are all the same, we can take first supply valve.
					if( false == m_rSuggestParams.m_bIsDifferentDpOnSVExist )
					{
						pSelectedSV = pSelectTrvList->GetFirst<CSelectedValve>();
					}
				}
				
				if( NULL != pSelectedSV )
				{
					double dDpTotal = m_pclIndSelTRVParams->m_dDp;
					
					if( pSelectedSV->GetDp() > dDpTotal )
					{
						pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_ORANGE );
						bDpOrange = true;

						// Change flag to reflect this new status.
						pSelectedTAP->SetFlag( CSelectedBase::Flags::eDp, true, CSelectList::BvFlagError::DpToReachTooLow );
					}
				}
			}
		}

		// if no error on special condition just above...
		if( false == bDpOrange )
		{
			// We check flag.
			if( true == pSelectedTAP->GetFlag( CSelectedBase::Flags::eDp ) )
			{
				pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_ORANGE );
			}
		}

		str = WriteCUDouble( _U_DIFFPRESS, pSelectedTAP->GetDp() );
		pclSSheet->SetStaticText( CD_ReturnValve_Dp, lRow, str );
	}
	
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

	// Set the temperature range.
	if( true == pSelectedTAP->IsFlagSet( CSelectedBase::eTemperature ) )
	{
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_RED );
		bAtLeastOneError = true;
	}
	
	pclSSheet->SetStaticText( CD_ReturnValve_TemperatureRange, lRow, ((CDB_TAProduct *)pSelectedTAP->GetProductIDPtr().MP)->GetTempRange() );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

	// Now we can set first columns in regards to current status (error, best or normal).
	if( true == bAtLeastOneError )
	{
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_RED );
	}
	else if( true == pSelectedTAP->IsFlagSet( CSelectedBase::eBest ) )
	{
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_GREEN );
	}

	// In addition of current flag, we have possibility that valve has the flag 'Not available' or 'Deleted'.
	// In that case, we show valve name in red with "*" around it and symbol '!' or '!!' after.
	if( true == pReturnValve->IsDeleted() )
	{
		pclSheetDescriptionReturnValve->WriteTextWithFlags( CString( pReturnValve->GetName() ), CD_ReturnValve_Name, lRow, 
				CSheetDescription::RemarkFlags::Deleted, _T(""), _RED );
	}
	else if( false == pReturnValve->IsAvailable() )
	{
		pclSheetDescriptionReturnValve->WriteTextWithFlags( CString( pReturnValve->GetName() ), CD_ReturnValve_Name, lRow, 
				CSheetDescription::RemarkFlags::NotAvailable, _T(""), _RED );
	}
	else
	{
		pclSSheet->SetStaticText( CD_ReturnValve_Name, lRow, pReturnValve->GetName() );
	}

	if( true == bAtLeastOneError )
	{
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );
	}
	else if( true == pSelectedTAP->IsFlagSet( CSelectedBase::eBest ) )
	{
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_GREEN );
	}

	pclSSheet->SetStaticText( CD_ReturnValve_Connection, lRow, pReturnValve->GetConnect() );
	pclSSheet->SetStaticText( CD_ReturnValve_Version, lRow, pReturnValve->GetVersion() );
	pclSSheet->SetStaticText( CD_ReturnValve_PN, lRow, pReturnValve->GetPN().c_str() );
	pclSSheet->SetStaticText( CD_ReturnValve_Size, lRow, pReturnValve->GetSize() );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

	// Set pipe size.
	CSelectPipe selPipe( m_pclIndSelTRVParams );
	m_pclIndSelTRVParams->m_pclSelectSupplyValveList->GetSelectPipeList()->GetMatchingPipe( pReturnValve->GetSizeKey(), selPipe );
	pclSSheet->SetStaticText( CD_ReturnValve_PipeSize, lRow, selPipe.GetpPipe()->GetSize( m_pclIndSelTRVParams->m_pTADB ) );
		
	// Set the LinDp to orange if it is above or below the technical parameters limits.
	if( selPipe.GetLinDp() > pTechParam->GetPipeMaxDp() || selPipe.GetLinDp() < pTechParam->GetPipeMinDp())
	{
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_ORANGE );
	}
	
	pclSSheet->SetStaticText( CD_ReturnValve_PipeLinDp, lRow, WriteCUDouble( _U_LINPRESSDROP, selPipe.GetLinDp() ) );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

	// Set the Velocity to orange if it is above the technical parameter limit.
	// Orange if it is below the dMinVel.
	if( selPipe.GetU() > pTechParam->GetPipeMaxVel() || selPipe.GetU() < pTechParam->GetPipeMinVel() )
	{
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_ORANGE );
	}
	
	pclSSheet->SetStaticText( CD_ReturnValve_PipeV, lRow, WriteCUDouble( _U_VELOCITY, selPipe.GetU() ) );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

	// Save parameter.
	CreateCellDescriptionProduct( pclSheetDescriptionReturnValve->GetFirstParameterColumn(), lRow, (LPARAM)pReturnValve, pclSheetDescriptionReturnValve );
	
	if( NULL != pEditedReturnValve && pEditedReturnValve == pReturnValve )
	{
		lRetRow = lRow;
	}

	pclSSheet->SetCellBorder( CD_ReturnValve_CheckBox, lRow, CD_ReturnValve_Separator - 1, lRow, true, SS_BORDERTYPE_BOTTOM,SS_BORDERSTYLE_SOLID, _GRAY );
	pclSSheet->SetCellBorder( CD_ReturnValve_PipeSize, lRow, CD_ReturnValve_Pointer - 1, lRow, true, SS_BORDERTYPE_BOTTOM,SS_BORDERSTYLE_SOLID, _GRAY );

	return lRetRow;
}

void CRViewSSelTrv::_FillReturnValveAccessoryRows( )
{
	if( NULL == m_pclIndSelTRVParams || NULL == m_pclIndSelTRVParams->m_pTADB )
	{
		ASSERT_RETURN;
	}

	CDB_TAProduct *pclReturnValveSelected = GetCurrentReturnValveSelected();

	if( NULL == pclReturnValveSelected )
	{
		return;
	}

	CDB_RuledTable *pRuledTable = (CDB_RuledTable *)( pclReturnValveSelected->GetAccessoriesGroupIDPtr().MP );

	if( NULL == pRuledTable )
	{
		return;
	}

	CRank rList;
	int iCount = m_pclIndSelTRVParams->m_pTADB->GetAccessories( &rList, pRuledTable, m_pclIndSelTRVParams->m_eFilterSelection );
	
	if( 0 == iCount )
	{
		return;
	}

	// Call 'MultiSpreadBase' method to get a new SSheet.
	CSheetDescription *pclSheetDescriptionRVAccessory = CreateSSheet( SD_ReturnValveAccessories );

	if( NULL == pclSheetDescriptionRVAccessory || NULL == pclSheetDescriptionRVAccessory->GetSSheetPointer() )
	{
		return;
	}

	CSSheet *pclSSheet = pclSheetDescriptionRVAccessory->GetSSheetPointer();

	pclSSheet->SetBool( SSB_REDRAW, FALSE );

	pclSSheet->Init();
	pclSSheet->SetBool( SSB_ALLOWDRAGDROP, FALSE );
	pclSSheet->SetBool( SSB_HORZSCROLLBAR, FALSE );
	pclSSheet->SetBool( SSB_VERTSCROLLBAR, FALSE );
	pclSSheet->SetTextTip( SS_TT_STATUS_FLOATING, 250, NULL, _LIGHTYELLOW, _TAH_BLACK );

	// Two lines by accessories, but two accessories by line (reason why we don't multiply iCount by 2).
	pclSSheet->SetMaxRows( RD_ReturnValveAccessory_FirstAvailRow + iCount - 1 + ( iCount % 2 ) );

	// All cells are static by default and filled with _T("").
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	pclSSheet->FormatStaticText( -1, -1, -1, -1, _T("") );

	// Get sheet description of return Valve to retrieve width.
	CSheetDescription *pclSheetDescriptionRV = m_ViewDescription.GetFromSheetDescriptionID( SD_ReturnValve );
	ASSERT( NULL != pclSheetDescriptionRV );
	
	// Take SSheet size and client area size and determine the right width to take in count.
	CRect rectSize = pclSheetDescriptionRV->GetSSheetPointer()->GetSheetSizeInPixels( false );

	// Prepare first column width (to match balancing valve sheet).
	long lFirstColumnWidth = pclSheetDescriptionRV->GetSSheetPointer()->GetColWidthInPixelsW( CD_ReturnValve_FirstColumn );

	// Do some correction in regards to some columns.
	long lTotalWidth = rectSize.Width() - lFirstColumnWidth;
	lTotalWidth -= pclSheetDescriptionRV->GetSSheetPointer()->GetColWidthInPixelsW( CD_ReturnValve_Pointer );

	// Try to create 2 columns in just the middle of balancing valve sheet.
	long lLeftWidth = (long)( lTotalWidth / 2 );
	long lRightWidth = ( ( lTotalWidth % 2 ) > 0 ) ? lLeftWidth + 1 : lLeftWidth;

	// Set columns 
	// 'CSheetDescription::SD_ParameterMode_Multi' to set that this sheet can have more than one column with parameter.
	pclSheetDescriptionRVAccessory->Init( 1, pclSSheet->GetMaxRows(), 2, CSheetDescription::SD_ParameterMode_Multi );

	pclSheetDescriptionRVAccessory->AddColumnInPixels( CD_ReturnValveAccessory_FirstColumn, lFirstColumnWidth );
	pclSheetDescriptionRVAccessory->AddColumnInPixels( CD_ReturnValveAccessory_Left, lLeftWidth );
	pclSheetDescriptionRVAccessory->AddColumnInPixels( CD_ReturnValveAccessory_Right, lRightWidth );
	pclSheetDescriptionRVAccessory->AddColumn( CD_ReturnValveAccessory_LastColumn, 0 );
	
	// These two columns can contain parameter.
	pclSheetDescriptionRVAccessory->AddParameterColumn( CD_ReturnValveAccessory_Left );
	pclSheetDescriptionRVAccessory->AddParameterColumn( CD_ReturnValveAccessory_Right );
	
	// Set the focus column (don't set on Left of Right (on a check box) ).
	pclSheetDescriptionRVAccessory->SetActiveColumn( CD_ReturnValveAccessory_FirstColumn );
	
	// Set selectable rows.
	pclSheetDescriptionRVAccessory->SetSelectableRangeRow( RD_ReturnValveAccessory_FirstAvailRow, pclSSheet->GetMaxRows() - 1 );

	// Increase ROW height.
	double dRowHeight = 12.75;
	pclSSheet->SetRowHeight( RD_ReturnValveAccessory_FirstRow, dRowHeight * 0.5 );
	pclSSheet->SetRowHeight( RD_ReturnValveAccessory_GroupName, dRowHeight * 1.5 );

	// Set Title.
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)GetTitleBackgroundColor( pclSheetDescriptionRVAccessory ) );

	pclSheetDescriptionRVAccessory->SetUserVariable( _SDUV_TITLEFORECOLOR, ( pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor ) ) );
	pclSheetDescriptionRVAccessory->SetUserVariable( _SDUV_TITLEBACKCOLOR, ( pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor ) ) );

	pclSSheet->AddCellSpanW( CD_ReturnValveAccessory_Left, RD_ReturnValveAccessory_GroupName, CD_ReturnValveAccessory_LastColumn - CD_ReturnValveAccessory_Left, 1 );
	pclSSheet->SetStaticText( CD_ReturnValveAccessory_Left, RD_ReturnValveAccessory_GroupName, IDS_SSHEETSSELDPC_BVACCGROUP );

	// Fill Accessories.
	long lRow = RD_ReturnValveAccessory_FirstAvailRow;
	
	CString str;
	LPARAM lparam;

	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_LEFT );

	// Left - Right.
	long lLeftOrRight = CD_ReturnValveAccessory_Left;
	BOOL bContinue = rList.GetFirst( str, lparam );
	
	while( TRUE == bContinue )
	{
		CDB_Product *pAccessory = (CDB_Product *)lparam;
		VERIFY( NULL != pAccessory );
		//VERIFY( 0 != pAccessory->IsClass( CLASS( CDB_Product ) ) );
		if( false == pAccessory->IsAnAccessory() )
		{
			continue;
		}

		// Create checkbox accessory.
		CString strName = _T("");

		if( true == pRuledTable->IsByPair( pAccessory->GetIDPtr().ID ) )
		{
			strName += _T("2x ");
		}

		strName += pAccessory->GetName();

		CCDBCheckboxAccessory *pCheckbox = CreateCheckboxAccessory( lLeftOrRight, lRow, false, true, strName, pAccessory, 
				pRuledTable, &m_vecReturnValveAccessoryList, pclSheetDescriptionRVAccessory );

		if( NULL != pCheckbox )
		{
			pCheckbox->ApplyInternalChange();
		}
		
		// Description.
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleEllipses, (LPARAM)TRUE );
		pclSSheet->SetStaticText( lLeftOrRight, lRow + 1, pAccessory->GetComment() );
		bContinue = rList.GetNext( str, lparam );
		
		// Restart left part.
		if( TRUE == bContinue && CD_ReturnValveAccessory_Right == lLeftOrRight )
		{
			pclSSheet->SetCellBorder( CD_ReturnValveAccessory_Left, lRow + 1, CD_ReturnValveAccessory_LastColumn - 1, lRow + 1, true, 
					SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _GRAY );

			lLeftOrRight = CD_ReturnValveAccessory_Left;
			lRow += 2;
			ASSERT( lRow < pclSSheet->GetMaxRows() );
		}
		else
		{
			lLeftOrRight = CD_ReturnValveAccessory_Right;
		}
		
		if( FALSE == bContinue )
		{
			lRow++;
		}
	}

	VerifyCheckboxAccessories( NULL, false, &m_vecReturnValveAccessoryList );
	pclSSheet->SetCellBorder( CD_ReturnValveAccessory_Left, lRow, CD_ReturnValveAccessory_LastColumn - 1, lRow, true, SS_BORDERTYPE_BOTTOM, 
			SS_BORDERSTYLE_SOLID, _BLACK );

	pclSSheet->SetBool( SSB_REDRAW, TRUE );
}

void CRViewSSelTrv::_UpdateReturnValveSettingDp( double dSupplyValveDp )
{
	if( NULL == m_pclIndSelTRVParams || NULL == m_pclIndSelTRVParams->m_pclSelectSupplyValveList )
	{
		ASSERT_RETURN;
	}

	// Exit if no return valve.
	if( NULL == m_pclIndSelTRVParams->m_pclSelectSupplyValveList->GetReturnValveList() )
	{
		return;
	}

	CSheetDescription *pclSheetDescriptionRV = m_ViewDescription.GetFromSheetDescriptionID( SD_ReturnValve );

	if( NULL == pclSheetDescriptionRV || NULL == pclSheetDescriptionRV->GetSSheetPointer() )
	{
		return;
	}

	CSSheet *pclSSheet = pclSheetDescriptionRV->GetSSheetPointer();
	CSelectTrvList *pSelectedTrvList = m_pclIndSelTRVParams->m_pclSelectSupplyValveList;

	if( NULL == pSelectedTrvList )
	{
		return;
	}

	// Get some variables.
	CDS_TechnicalParameter *pTechParam = m_pclIndSelTRVParams->m_pTADS->GetpTechParams();
	double dTrvMinDpRatio = pTechParam->GetTrvMinDpRatio();
	double dTrvDefDpTot = pTechParam->GetTrvDefDpTot();
	double dRho = m_pclIndSelTRVParams->m_WC.GetDens();
	double dNu = m_pclIndSelTRVParams->m_WC.GetKinVisc();
	
	double dDpTotal = m_pclIndSelTRVParams->m_dDp;
	double dFlow = m_pclIndSelTRVParams->m_dFlow;

	// If there is already a return valve selection...
	if( 0 != m_lReturnValveSelectedRow )
	{
		// We remove current selection to reset it at the end of the update.
		// Remark: it's to force internal variables to maintain the right color under selection.
		pclSSheet->UnSelectMultipleRows();
	}

	// If we must compute new value depending on new supply valve pressure drop...
	if( -1.0 != dSupplyValveDp )
	{
		double dReqDpRv = dDpTotal - dSupplyValveDp;

		// By default, clean full opening remark if exist.
		pclSheetDescriptionRV->RemoveOneRemark( CSheetDescription::RemarkFlags::FullOpening );
		
		// Clean all others (keep internal variables).
		pclSheetDescriptionRV->ClearRemarks();

		// Loop all products.
		CCellDescription *pclCellDescription = pclSheetDescriptionRV->GetFirstCellDescription( RVSCellDescription::CD_Product );
		
		while( NULL != pclCellDescription )
		{
			CCellDescriptionProduct *pCDProduct = dynamic_cast<CCellDescriptionProduct*>( pclCellDescription );

			if( NULL == pCDProduct )
			{
				continue;
			}

			CDB_RegulatingValve *pclReturnValve = dynamic_cast<CDB_RegulatingValve*>( (CData*)pCDProduct->GetProduct() );

			if( NULL == pclReturnValve )
			{
				continue;
			}

			CSelectedValve *pclSelectedReturnValve = GetSelectProduct<CSelectedValve>( pclReturnValve, m_pclIndSelTRVParams->m_pclSelectSupplyValveList->GetReturnValveList() );

			if( NULL == pclSelectedReturnValve )
			{
				continue;
			}

			// Get characteristic of the valve.
			CDB_ValveCharacteristic *pValveCharacteristic = pclReturnValve->GetValveCharacteristic();

			if( NULL == pValveCharacteristic )
			{
				continue;
			}

			// Compute the pressure drop for the valve fully and half opened.
			double dDpFullyOpen, dDpHalfOpen, dDpQuarterOpen;
			double dHMax = pValveCharacteristic->GetOpeningMax();
			
			if( -1.0 == dHMax )
			{
				continue;
			}
			
			if( false == pValveCharacteristic->GetValveDp( dFlow, &dDpFullyOpen, dHMax, dRho, dNu ) )
			{
				continue;
			}
			
			double dHHalf = 0.5 * dHMax;

			if( false == pValveCharacteristic->GetValveDp( dFlow, &dDpHalfOpen, dHHalf, dRho, dNu ) )
			{
				continue;
			}
			
			double dHQuarter = 0.25 * dHMax;

			if( false == pValveCharacteristic->GetValveDp( dFlow, &dDpQuarterOpen, dHQuarter, dRho, dNu ) )
			{
				continue;
			}

			// Compute hand wheel setting.
			double dH = -1.0;
			double dDp = 0.0;
			bool bNotFound;

			// If required Dp on return valve is bigger than Dp on valve when fully opened...
			if( dReqDpRv >= dDpFullyOpen )
			{
				dDp = dReqDpRv;

				// Try to find a solution.
				bool bValid = pValveCharacteristic->GetValveOpening( dFlow, dDp, &dH, dRho, dNu, ( eBool3::eb3True == pValveCharacteristic->IsDiscrete() ) ? 3 : 1 );

				// If it's not possible to have opening to reach required Dp...
				if( false == bValid )
				{
					// Take pressure drop available at the minimum recommenced setting of the valve.
					dH = pValveCharacteristic->GetMinRecSetting();
					
					if( dH <= 0.0 )
					{
						dH = pValveCharacteristic->GetOpeningMin();
					}

					if( -1.0 != dH )
					{
						if( false == pValveCharacteristic->GetValveDp( dFlow, &dDp, dH, dRho, dNu ) )
						{
							continue;
						}
					}
				}
			}

			// If dh<0, no solution was found.
			bNotFound = dH < 0;

			if( true == bNotFound )
			{
				dH = dHMax;
				dDp = dDpFullyOpen;
			}
			else if( eBool3::eb3True == pValveCharacteristic->IsDiscrete() )
			{
				// Recalculate the Dp for the setting as determined above.
				if( false == pValveCharacteristic->GetValveDp( dFlow, &dDp, dH, dRho, dNu ) )
				{
					continue;
				}
			}

			pclSelectedReturnValve->SetDp( dDp );
			pclSelectedReturnValve->SetDpFullOpen( dDpFullyOpen );
			pclSelectedReturnValve->SetDpHalfOpen( dDpHalfOpen );
			pclSelectedReturnValve->SetH( dH );

			// Clear previous flags.
			bool bPNFlag = pclSelectedReturnValve->GetFlag( CSelectedBase::Flags::ePN );
			bool bTempFlag = pclSelectedReturnValve->GetFlag( CSelectedBase::Flags::eTemperature );
			pclSelectedReturnValve->ClearAllFlags();

			pclSelectedReturnValve->SetFlag( CSelectedBase::Flags::eValveSetting, dH < pValveCharacteristic->GetMinRecSetting() );
			pclSelectedReturnValve->SetFlag( CSelectedBase::Flags::eValveMaxSetting, dH == dHMax ? true : false );

			// Set flag error to facilitate management with 'OnTextTipFetch' in RViewSSelBv class.
			if( true == bNotFound )
			{
				pclSelectedReturnValve->SetFlag( CSelectedBase::Flags::eDp, true, CSelectList::BvFlagError::DpToReachTooLow );
			}
			else if( dDp < pTechParam->GetValvMinDp() )
			{
				pclSelectedReturnValve->SetFlag( CSelectedBase::Flags::eDp, true, CSelectList::BvFlagError::DpBelowMinDp );
			}
			else if( dDp > pTechParam->VerifyValvMaxDp(pclReturnValve) )
			{
				pclSelectedReturnValve->SetFlag( CSelectedBase::Flags::eDp, true, CSelectList::BvFlagError::DpAboveMaxDp );
			}

			pclSelectedReturnValve->SetFlag( CSelectedBase::Flags::ePN, bPNFlag );	
			pclSelectedReturnValve->SetFlag( CSelectedBase::Flags::eTemperature, bTempFlag );

			//////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// Refresh the content of the 'Setting' column.
			//////////////////////////////////////////////////////////////////////////////////////////////////////////////

			// By default, text color is black.
			pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
			
			CString str = pValveCharacteristic->GetSettingString( dH );

			if( true == pclSelectedReturnValve->IsFlagSet( CSelectedBase::Flags::eValveMaxSetting ) )
			{
				pclSheetDescriptionRV->WriteTextWithFlags( str, CD_ReturnValve_Preset, pCDProduct->GetCellPosition().y, 
						CSheetDescription::RemarkFlags::FullOpening, _T(""), ( dH < pValveCharacteristic->GetMinRecSetting() ) ? _RED : _BLACK );
			}
			else
			{
				if( dH < pValveCharacteristic->GetMinRecSetting() )
				{
					pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_ORANGE );
				}

				// Update setting.
				COLORREF backColor = pclSSheet->GetBackColor( CD_ReturnValve_Preset, pclCellDescription->GetCellPosition().y );
				pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)backColor );
				pclSSheet->SetStaticText( CD_ReturnValve_Preset, pCDProduct->GetCellPosition().y, str );
			}

			//////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// Refresh the content of the 'Dp' column.
			//////////////////////////////////////////////////////////////////////////////////////////////////////////////

			// By default, text color is black.
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

			if( true == pclSelectedReturnValve->IsFlagSet( CSelectedBase::eDp ) || true == pclSelectedReturnValve->IsFlagSet( CSelectedBase::eValveFullODp ) )
			{
				pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_ORANGE );
			}

			// Update dp.
			COLORREF backColor = pclSSheet->GetBackColor( CD_ReturnValve_Dp, pclCellDescription->GetCellPosition().y );
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)backColor );
			pclSSheet->SetStaticText( CD_ReturnValve_Dp, pCDProduct->GetCellPosition().y, WriteDouble( CDimValue::SItoCU( _U_DIFFPRESS, dDp ), 3, 0 ) );

			// Go to next product.
			pclCellDescription = pclSheetDescriptionRV->GetNextCellDescription( pclCellDescription, RVSCellDescription::CD_Product );
		}

		// Update (add or remove) remarks.
		LPARAM lParam;
		pclSheetDescriptionRV->GetUserVariable( _SDUV_REMARKROW, lParam );
		pclSheetDescriptionRV->WriteRemarks( (long)lParam, CD_ReturnValve_Name, CD_ReturnValve_Separator );
	}
	else
	{
		// We reset display in 'Setting' and 'Dp' columns.

		COLORREF backColor;
		double dReqDpTrv = dDpTotal - m_rSuggestParams.m_dMaxDpRV;

		// By default, clean full opening remark if exist.
		pclSheetDescriptionRV->RemoveOneRemark( CSheetDescription::RemarkFlags::FullOpening );
		
		// Clean all others (keep internal variables).
		pclSheetDescriptionRV->ClearRemarks();

		// Loop all products.
		CCellDescription *pclCellDescription = pclSheetDescriptionRV->GetFirstCellDescription( RVSCellDescription::CD_Product );
		
		while( NULL != pclCellDescription )
		{
			CCellDescriptionProduct *pCDProduct = dynamic_cast<CCellDescriptionProduct*>( pclCellDescription );

			if( NULL == pCDProduct )
			{
				continue;
			}

			CDB_RegulatingValve *pclReturnValve = dynamic_cast<CDB_RegulatingValve*>( (CData*)pCDProduct->GetProduct() );

			if( NULL == pclReturnValve )
			{
				continue;
			}

			CSelectedValve *pclSelectedReturnValve = GetSelectProduct<CSelectedValve>( pclReturnValve, 
					m_pclIndSelTRVParams->m_pclSelectSupplyValveList->GetReturnValveList() );

			if( NULL == pclSelectedReturnValve )
			{
				continue;
			}

			CDB_ValveCharacteristic *pValveCharacteristic = pclReturnValve->GetValveCharacteristic();

			if( NULL == pValveCharacteristic )
			{
				continue;
			}

			long lRow = pCDProduct->GetCellPosition().y;
			bool bAtLeastOneError = false;
			double dDpFullyOpen;
			
			double dHMax = pValveCharacteristic->GetOpeningMax();
			
			if( -1.0 == dHMax )
			{
				continue;
			}

			if( false == pValveCharacteristic->GetValveDp( dFlow, &dDpFullyOpen, dHMax, dRho, dNu ) )
			{
				continue;
			}

			// Try to find the setting corresponding to ReqDpTrv.
			bool bNotFound = ( dDpFullyOpen > dDpTotal ) ? true : false;

			// Clear previous flags.
			bool bPNFlag = pclSelectedReturnValve->GetFlag( CSelectedBase::Flags::ePN );
			bool bTempFlag = pclSelectedReturnValve->GetFlag( CSelectedBase::Flags::eTemperature );
			pclSelectedReturnValve->ClearAllFlags();

			// Update fields in the selected product.
			pclSelectedReturnValve->SetH( dHMax );
			pclSelectedReturnValve->SetDp( dDpFullyOpen );

			pclSelectedReturnValve->SetFlag( CSelectedBase::Flags::eValveSetting, false );
			pclSelectedReturnValve->SetFlag( CSelectedBase::Flags::eValveMaxSetting, true );

			// Set flag error to facilitate management with 'OnTextTipFetch' in RViewSSelBv class.
			if( true == bNotFound )
			{
				pclSelectedReturnValve->SetFlag( CSelectedBase::Flags::eDp, true, CSelectList::BvFlagError::DpFOTooHigh );
			}
			else if( dDpFullyOpen < pTechParam->GetValvMinDp() )
			{
				pclSelectedReturnValve->SetFlag( CSelectedBase::Flags::eDp, true, CSelectList::BvFlagError::DpBelowMinDp );
			}
			else if( dDpFullyOpen > pTechParam->VerifyValvMaxDp(pclReturnValve) )
			{
				pclSelectedReturnValve->SetFlag( CSelectedBase::Flags::eDp, true, CSelectList::BvFlagError::DpAboveMaxDp );
			}

			pclSelectedReturnValve->SetFlag( CSelectedBase::Flags::eValveFullODp, dDpFullyOpen > dDpTotal );

			pclSelectedReturnValve->SetFlag( CSelectedBase::Flags::ePN, bPNFlag );	
			pclSelectedReturnValve->SetFlag( CSelectedBase::Flags::eTemperature, bTempFlag );

			//////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// Refresh the content of the 'Setting' column.
			//////////////////////////////////////////////////////////////////////////////////////////////////////////////

			// By default, text color is black.
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );
			
			if( false == pclSelectedReturnValve->IsFlagSet( CSelectedBase::eValveFullODp ) )
			{
				// Remark: we have return valves with different pressure drop. It it thus impossible at now to compute setting and Dp on
				//         the supply valves.
				//         why last condition? Because if pressure drop on supply valve when fully opened is already bigger than the Dp user wants, it is 
				//         impossible to do better. Than in this case we show setting and Dp.
				backColor = pclSSheet->GetBackColor( CD_ReturnValve_Preset, lRow );
				pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)backColor );
				pclSSheet->SetStaticText( CD_ReturnValve_Preset, lRow, _T("-") );

				backColor = pclSSheet->GetBackColor( CD_ReturnValve_Dp, lRow );
				pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)backColor );
				pclSSheet->SetStaticText( CD_ReturnValve_Dp, lRow, _T("-") );
			}
			else
			{
				CString str = pValveCharacteristic->GetSettingString( dHMax );

				if( true == pclSelectedReturnValve->IsFlagSet( CSelectedBase::Flags::eValveMaxSetting ) )
				{
					pclSheetDescriptionRV->WriteTextWithFlags( str, CD_ReturnValve_Preset, pCDProduct->GetCellPosition().y, CSheetDescription::RemarkFlags::FullOpening, _T(""), ( dHMax < pValveCharacteristic->GetMinRecSetting() ) ? _RED : _BLACK );
				}
				else
				{
					if( dHMax < pValveCharacteristic->GetMinRecSetting() )
					{
						pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_RED );
						bAtLeastOneError = true;
					}

					// Update setting.
					backColor = pclSSheet->GetBackColor( CD_ReturnValve_Preset, lRow );
					pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)backColor );
					pclSSheet->SetStaticText( CD_ReturnValve_Preset, lRow, str );
				}

				//////////////////////////////////////////////////////////////////////////////////////////////////////////////
				// Refresh the content of the 'Dp' column.
				//////////////////////////////////////////////////////////////////////////////////////////////////////////////

				// By default, text color is black.
				pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

				if( true == pclSelectedReturnValve->IsFlagSet( CSelectedBase::eDp ) || true == pclSelectedReturnValve->IsFlagSet( CSelectedBase::eValveFullODp ) )
				{
					pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_RED );
					bAtLeastOneError = true;
				}

				// Update dp.
				backColor = pclSSheet->GetBackColor( CD_ReturnValve_Dp, lRow );
				pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)backColor );
				pclSSheet->SetStaticText( CD_ReturnValve_Dp, lRow, WriteDouble( CDimValue::SItoCU( _U_DIFFPRESS, dDpFullyOpen ), 3, 0 ) );
			}

			// If a least one error, we must set the name in red, otherwise we reset to black.
			pclSSheet->SetForeColor( CD_ReturnValve_Name, pCDProduct->GetCellPosition().y, ( true == bAtLeastOneError ) ? _RED : _BLACK );

			// Go to next product.
			pclCellDescription = pclSheetDescriptionRV->GetNextCellDescription( pclCellDescription, RVSCellDescription::CD_Product );
		}

		// Update (add or remove) remarks.
		LPARAM lParam;
		pclSheetDescriptionRV->GetUserVariable( _SDUV_REMARKROW, lParam );
		pclSheetDescriptionRV->WriteRemarks( (long)lParam, CD_ReturnValve_Name, CD_ReturnValve_Separator );
	}

	// If there is already a return valve selection...
	if( m_lReturnValveSelectedRow != 0 )
	{
		// Set the selection.
		// Remark: it's to force internal variables to maintain the right color under selection.
		pclSSheet->SelectOneRow( m_lReturnValveSelectedRow, pclSheetDescriptionRV->GetSelectionFrom(), pclSheetDescriptionRV->GetSelectionTo() );
	}
}

long CRViewSSelTrv::_GetRowOfEditedSVActuator( CSheetDescription *pclSheetDescriptionSVActuator, CDB_Actuator *pActuator )
{
	if( NULL == pclSheetDescriptionSVActuator || NULL == pActuator )
	{
		return -1;
	}

	// Retrieve list of all products in 'pclSheetDescriptionActuator'.
	CSheetDescription::vecCellDescription vecCellDescriptionList;
	pclSheetDescriptionSVActuator->GetCellDescriptionList( vecCellDescriptionList, RVSCellDescription::CD_Product );

	// Run all objects.
	long lReturnValue = -1;
	CSheetDescription::vecCellDescriptionIter vecIter = vecCellDescriptionList.begin();
	
	while( vecIter != vecCellDescriptionList.end() )
	{
		CCellDescriptionProduct *pCDProduct = dynamic_cast<CCellDescriptionProduct *>( *vecIter );
		
		if( NULL != pCDProduct && 0 != pCDProduct->GetProduct() && pActuator == (CDB_Actuator *)pCDProduct->GetProduct() )
		{
			lReturnValue = pCDProduct->GetCellPosition().y;
			break;
		}

		vecIter++;
	}

	return lReturnValue;
}

long CRViewSSelTrv::_GetRowOfEditedReturnValve( CSheetDescription *pclSheetDescriptionBv, CDB_TAProduct *pEditedTAP )
{
	if( NULL == pclSheetDescriptionBv || NULL == pEditedTAP )
	{
		return -1;
	}
	
	// Retrieve list of all products in 'pclSheetDescription'.
	CSheetDescription::vecCellDescription vecCellDescriptionList;
	pclSheetDescriptionBv->GetCellDescriptionList( vecCellDescriptionList, RVSCellDescription::CD_Product );
	
	// Run all objects.
	long lReturnValue = -1;
	CSheetDescription::vecCellDescriptionIter vecIter = vecCellDescriptionList.begin();
	
	while( vecIter != vecCellDescriptionList.end() )
	{
		CCellDescriptionProduct *pCDProduct = dynamic_cast<CCellDescriptionProduct *>( *vecIter );
		
		if( NULL != pCDProduct && 0 != pCDProduct->GetProduct() && pEditedTAP == (CDB_TAProduct *)pCDProduct->GetProduct() )
		{
			lReturnValue = pCDProduct->GetCellPosition().y;
			break;
		}
		
		vecIter++;
	}

	return lReturnValue;
}

void CRViewSSelTrv::_ReadAllColumnWidth( CDS_IndSelParameter *pclIndSelParameter )
{
	if( NULL == pclIndSelParameter )
	{
		ASSERT_RETURN;
	}

	// By default and before reading registry saved column width force reset column width for all sheets.
	for( UINT ui = SD_First; ui < SD_Last; ui++ )
	{
		ResetColumnWidth( ui );
	}
	
	std::map<UINT, short> mapSDIDVersion;
	mapSDIDVersion[CW_RVIEWSSELTRV_SHEETID_SUPPLYVALVE] = CW_RVIEWSSELTRV_SUPPLYVALVE_VERSION;
	mapSDIDVersion[CW_RVIEWSSELTRV_SHEETID_SUPPLYFLOWLIMITED] = CW_RVIEWSSELTRV_SUPPLYFLOWLIMITED_VERSION;
	mapSDIDVersion[CW_RVIEWSSELTRV_SHEETID_SVTHERMOACTUATOR] = CW_RVIEWSSELTRV_SVTHERMOACTUATOR_VERSION;
	mapSDIDVersion[CW_RVIEWSSELTRV_SHEETID_SVELECTROACTUATOR] = CW_RVIEWSSELTRV_SVELECTROACTUATOR_VERSION;
	mapSDIDVersion[CW_RVIEWSSELTRV_SHEETID_RETURNVALVE] = CW_RVIEWSSELTRV_RETURNVALVE_VERSION;

	// Container window sheet ID to sheetdescription of this rightview.
	std::map<UINT, short> mapCWtoRW;
	mapCWtoRW[CW_RVIEWSSELTRV_SHEETID_SUPPLYVALVE] = SD_SupplyValve;
	mapCWtoRW[CW_RVIEWSSELTRV_SHEETID_SUPPLYFLOWLIMITED] = SD_SupplyFlowLimited;
	mapCWtoRW[CW_RVIEWSSELTRV_SHEETID_SVTHERMOACTUATOR] = SD_SVThermoActuator;
	mapCWtoRW[CW_RVIEWSSELTRV_SHEETID_SVELECTROACTUATOR] = SD_SVElectroActuator;
	mapCWtoRW[CW_RVIEWSSELTRV_SHEETID_RETURNVALVE] = SD_ReturnValve;

	// Access to the 'RViewSSelTrv' group.
	CDS_IndSelParameter::CCWWindow *pclCWWindow = pclIndSelParameter->GetWindowColumnWidth( CW_WINDOWID_INDSELTRV, true );
	
	for( std::map<UINT, short>::iterator iter = mapSDIDVersion.begin(); iter != mapSDIDVersion.end(); iter++ )
	{
		CDS_IndSelParameter::CCWSheet *pclCWSheet = pclCWWindow->GetCWSheet( iter->first );

		if( NULL != pclCWSheet && m_mapSSheetColumnWidth[mapCWtoRW[iter->first]].size() == pclCWSheet->GetMap().size() )
		{
			short nVersion = pclCWSheet->GetVersion();

			if( nVersion == iter->second )
			{
				m_mapSSheetColumnWidth[mapCWtoRW[iter->first]] = pclCWSheet->GetMap();
			}
		}
	}
}

void CRViewSSelTrv::_WriteAllColumnWidth( CDS_IndSelParameter *pclIndSelParameter )
{
	if( NULL == pclIndSelParameter )
	{
		ASSERT_RETURN;
	}

	std::map<UINT, short> mapSDIDVersion;
	mapSDIDVersion[CW_RVIEWSSELTRV_SHEETID_SUPPLYVALVE] = CW_RVIEWSSELTRV_SUPPLYVALVE_VERSION;
	mapSDIDVersion[CW_RVIEWSSELTRV_SHEETID_SUPPLYFLOWLIMITED] = CW_RVIEWSSELTRV_SUPPLYFLOWLIMITED_VERSION;
	mapSDIDVersion[CW_RVIEWSSELTRV_SHEETID_SVTHERMOACTUATOR] = CW_RVIEWSSELTRV_SVTHERMOACTUATOR_VERSION;
	mapSDIDVersion[CW_RVIEWSSELTRV_SHEETID_SVELECTROACTUATOR] = CW_RVIEWSSELTRV_SVELECTROACTUATOR_VERSION;
	mapSDIDVersion[CW_RVIEWSSELTRV_SHEETID_RETURNVALVE] = CW_RVIEWSSELTRV_RETURNVALVE_VERSION;

	// Container window sheet ID to sheetdescription of this rightview.
	std::map<UINT, short> mapCWtoRW;
	mapCWtoRW[CW_RVIEWSSELTRV_SHEETID_SUPPLYVALVE] = SD_SupplyValve;
	mapCWtoRW[CW_RVIEWSSELTRV_SHEETID_SUPPLYFLOWLIMITED] = SD_SupplyFlowLimited;
	mapCWtoRW[CW_RVIEWSSELTRV_SHEETID_SVTHERMOACTUATOR] = SD_SVThermoActuator;
	mapCWtoRW[CW_RVIEWSSELTRV_SHEETID_SVELECTROACTUATOR] = SD_SVElectroActuator;
	mapCWtoRW[CW_RVIEWSSELTRV_SHEETID_RETURNVALVE] = SD_ReturnValve;

	// Access to the 'RViewSSelTrv' group.
	CDS_IndSelParameter::CCWWindow *pclCWWindow = pclIndSelParameter->GetWindowColumnWidth( CW_WINDOWID_INDSELTRV, true );

	for( std::map<UINT, short>::iterator iter = mapSDIDVersion.begin(); iter != mapSDIDVersion.end(); iter++ )
	{
		CDS_IndSelParameter::CCWSheet *pclCWSheet = pclCWWindow->GetCWSheet( iter->first, true );
		pclCWSheet->SetVersion( iter->second );
		pclCWSheet->GetMap() = m_mapSSheetColumnWidth[mapCWtoRW[iter->first]];
	}
}

void CRViewSSelTrv::GetCurrentSVActuatorAccessoryList( CDB_Actuator * pclSeletedActuator, CRank * pclActuatorAccessoryList, CDB_RuledTableBase ** ppclRuledTable )
{
	if( NULL == m_pclIndSelTRVParams || NULL == m_pclIndSelTRVParams->m_pTADB )
	{
		ASSERT_RETURN;
	}

	if( NULL == pclSeletedActuator || NULL == pclActuatorAccessoryList || NULL == ppclRuledTable )
	{
		return;
	}

	pclActuatorAccessoryList->PurgeAll();
	*ppclRuledTable = (CDB_RuledTable *)( pclSeletedActuator->GetAccessoriesGroupIDPtr().MP );

	if( NULL == *ppclRuledTable )
	{
		return;
	}

	m_pclIndSelTRVParams->m_pTADB->GetAccessories( pclActuatorAccessoryList, *ppclRuledTable, m_pclIndSelTRVParams->m_eFilterSelection );
}

void CRViewSSelTrv::FillRowsAcc( bool bForCtrl, CRank *pclList, CDB_RuledTableBase *pclRuledTable )
{
	if( NULL == m_pclIndSelTRVParams )
	{
		ASSERT_RETURN;
	}

	int iCount = pclList->GetCount();

	if( 0 == iCount )
	{
		return;
	}

	int iSSheetID;
	vecCDCAccessoryList *pvecAccessories = NULL;
	CCDButtonExpandCollapseGroup **ppclGroupButton = NULL;

	iSSheetID = SD_SVActuatorAccessories;
	pvecAccessories = &m_vecSVActuatorAccessoryList;
	ppclGroupButton = &m_pCDBExpandCollapseRowsSVActuatorAcc;


	// Call 'MultiSpreadBase' method to get a new SSheet.
	CSheetDescription *pclSheetDescription = NULL;
	pclSheetDescription = CreateSSheet( iSSheetID, m_uiSDActuatorID );

	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() )
	{
		return;
	}

	CSSheet *pclSSheet = pclSheetDescription->GetSSheetPointer();

	pclSSheet->SetBool( SSB_REDRAW, FALSE );

	pclSSheet->Init();
	pclSSheet->SetBool( SSB_ALLOWDRAGDROP, FALSE );
	pclSSheet->SetBool( SSB_HORZSCROLLBAR, FALSE );
	pclSSheet->SetBool( SSB_VERTSCROLLBAR, FALSE );
	pclSSheet->SetTextTip( SS_TT_STATUS_FLOATING, 250, NULL, _LIGHTYELLOW, _TAH_BLACK );

	// Two lines by accessories, but two accessories by line (reason why we don't multiply iCount by 2).
	pclSSheet->SetMaxRows( RD_Accessory_FirstAvailRow + iCount - 1 + ( iCount % 2 ) );

	// All cells are static by default and filled with _T("")
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	pclSSheet->FormatStaticText( -1, -1, -1, -1, _T( "" ) );

	// Get previous sheet description to retrieve width.
	CSheetDescription *pclPreviousSheetDescription = m_ViewDescription.GetPrevSheetDescription( pclSheetDescription );
	// It's absolutely not normal to have this pointer NULL.
	ASSERT( NULL != pclPreviousSheetDescription );

	// Take SSheet size and client area size and determine the right width to take in count.
	CRect rectSize = pclPreviousSheetDescription->GetSSheetPointer()->GetSheetSizeInPixels( false );

	// Prepare first column width (to match Dp controller sheet).
	long lFirstColumnWidth = 0;
	long lLastColumnWidth = 0;

	switch( pclPreviousSheetDescription->GetSheetDescriptionID() )
	{
		case SD_SVElectroActuator:
			lFirstColumnWidth = pclPreviousSheetDescription->GetSSheetPointer()->GetColWidthInPixelsW( CD_SVElecActuator_FirstColumn );
			lFirstColumnWidth += pclPreviousSheetDescription->GetSSheetPointer()->GetColWidthInPixelsW( CD_SVElecActuator_Box );
			lLastColumnWidth = pclPreviousSheetDescription->GetSSheetPointer()->GetColWidthInPixelsW( CD_SVElecActuator_Pointer );
			break;

		case SD_SVThermoActuator:
			lFirstColumnWidth = pclPreviousSheetDescription->GetSSheetPointer()->GetColWidthInPixelsW( CD_SVThrmActuator_FirstColumn );
			lFirstColumnWidth += pclPreviousSheetDescription->GetSSheetPointer()->GetColWidthInPixelsW( CD_SVThrmActuator_Box );
			lLastColumnWidth = pclPreviousSheetDescription->GetSSheetPointer()->GetColWidthInPixelsW( CD_SVThrmActuator_Pointer );
			break;
	}

	// Do some correction in regards to some columns.
	long lTotalWidth = rectSize.Width() - lFirstColumnWidth;
	lTotalWidth -= lLastColumnWidth;

	// Try to create 2 columns in just the middle of previous sheet.
	long lLeftWidth = (long)( lTotalWidth / 2 );
	long lRightWidth = ( ( lTotalWidth % 2 ) > 0 ) ? lLeftWidth + 1 : lLeftWidth;

	// Set columns
	// 'CSheetDescription::SD_ParameterMode_Multi' to set that this sheet can have more than one column with parameter.
	pclSheetDescription->Init( 1, pclSSheet->GetMaxRows(), 2, CSheetDescription::SD_ParameterMode_Multi );

	pclSheetDescription->AddColumnInPixels( CD_Accessory_FirstColumn, lFirstColumnWidth );
	pclSheetDescription->AddColumnInPixels( CD_Accessory_Left, lLeftWidth );
	pclSheetDescription->AddColumnInPixels( CD_Accessory_Right, lRightWidth );
	pclSheetDescription->AddColumnInPixels( CD_Accessory_LastColumn, 0 );

	// These two columns can contain parameter.
	pclSheetDescription->AddParameterColumn( CD_Accessory_Left );
	pclSheetDescription->AddParameterColumn( CD_Accessory_Right );

	// Set the focus column (don't set on Left of Right (on a check box) ).
	pclSheetDescription->SetActiveColumn( CD_Accessory_FirstColumn );

	// Set selectable rows.
	pclSheetDescription->SetSelectableRangeRow( RD_Accessory_FirstAvailRow, pclSSheet->GetMaxRows() - 1 );

	// Two lines by accessory, but two accessories by line
	//if( 0 != iTitleID )
	{
		// Increase row height.
		double dRowHeight = 12.75;
		pclSSheet->SetRowHeight( RD_Accessory_FirstRow, dRowHeight * 0.5 );
		pclSSheet->SetRowHeight( RD_Accessory_GroupName, dRowHeight * 1.5 );

		// Set title.
		pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)GetTitleBackgroundColor( pclSheetDescription ) );
		pclSheetDescription->SetUserVariable( _SDUV_TITLEFORECOLOR, ( pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor ) ) );
		pclSheetDescription->SetUserVariable( _SDUV_TITLEBACKCOLOR, ( pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor ) ) );

		pclSSheet->AddCellSpanW( CD_Accessory_Left, RD_Accessory_GroupName, CD_Accessory_LastColumn - CD_Accessory_Left, 1 );
		pclSSheet->SetStaticText( CD_Accessory_Left, RD_Accessory_GroupName, _T("Supply valve actuator accessories") );
	}

	// Fill accessories.
	long lRow = RD_Accessory_FirstAvailRow;

	CString str;
	LPARAM lparam;

	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_LEFT );

	// Left - Right.
	long lLeftOrRight = CD_Accessory_Left;
	BOOL bContinue = pclList->GetFirst( str, lparam );

	while( TRUE == bContinue )
	{
		CDB_Product *pAccessory = (CDB_Product *)lparam;
		VERIFY( NULL != pAccessory );
		if( false == pAccessory->IsAnAccessory() )
		{
			continue;
		}
		// Create checkbox accessory.

		// If accessory has its property 'IsAttached' to true and user is in selection by set, he can't select it.
		bool bEnabled = true;

		CCDBCheckboxAccessory *pCheckbox = CreateCheckboxAccessory( lLeftOrRight, lRow, false, bEnabled, pAccessory->GetName(), pAccessory, pclRuledTable,
			pvecAccessories, pclSheetDescription );

		if( NULL != pCheckbox )
		{
			pCheckbox->ApplyInternalChange();
		}

		// Description.
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleEllipses, (LPARAM)TRUE );
		pclSSheet->SetStaticText( lLeftOrRight, lRow + 1, pAccessory->GetComment() );
		bContinue = pclList->GetNext( str, lparam );

		// Restart left part.
		if( TRUE == bContinue && lLeftOrRight == CD_Accessory_Right )
		{
			pclSSheet->SetCellBorder( CD_Accessory_Left, lRow + 1, CD_Accessory_LastColumn - 1, lRow + 1, true, SS_BORDERTYPE_BOTTOM,
				SS_BORDERSTYLE_SOLID, _GRAY );

			lLeftOrRight = CD_Accessory_Left;
			lRow += 2;
			ASSERT( lRow < pclSSheet->GetMaxRows() );
		}
		else
		{
			lLeftOrRight = CD_Accessory_Right;
		}

		if( FALSE == bContinue )
		{
			lRow++;
		}
	}

	VerifyCheckboxAccessories( NULL, false, pvecAccessories );

	if( NULL != ppclGroupButton )
	{
		*ppclGroupButton = CreateExpandCollapseGroupButton( CD_Accessory_FirstColumn, RD_Accessory_GroupName, true,
			CCDButtonExpandCollapseGroup::ButtonState::CollapseRow, pclSheetDescription->GetFirstSelectableRow(),
			pclSheetDescription->GetLastSelectableRow( false ), pclSheetDescription );

		// Show button.
		if( NULL != *ppclGroupButton )
		{
			( *ppclGroupButton )->SetShowStatus( true );
		}
	}

	pclSSheet->SetCellBorder( CD_Accessory_Left, lRow, CD_Accessory_LastColumn - 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );

	pclSSheet->SetBool( SSB_REDRAW, TRUE );
}
