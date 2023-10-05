#include "stdafx.h"
#include "afxctl.h"

#include "TASelect.h"
#include "MainFrm.h"
#include "DataStruct.h"
#include "SelectPM.h"

#include "DlgLeftTabSelManager.h"
#include "ProductSelectionParameters.h"
#include "DlgPMGraphsOutput.h"
#include "DlgIndSelPMExcludedProducts.h"

#include "RViewSSelPM.h"

CRViewSSelPM *pRViewSSelPM = NULL;

CRViewSSelPM::CRViewSSelPM() : CRViewSSelSS( CMainFrame::RightViewList::eRVSSelPresureMaintenance, false )
{
	m_pclIndSelPMParams = NULL;

	for( int i = 0; i < SD_Last; i++ )
	{
		m_mapButtonExpandCollapseRows[( SheetDescription )i] = NULL;
		m_mapButtonExpandCollapseGroup[( SheetDescription )i] = NULL;
		m_mapButtonShowAllPriorities[( SheetDescription )i] = NULL;
		m_arlpSelectedUserParam[( SheetDescription )i] = 0;
	}

	vecCDCAccessoryList vecList;
	m_mapAccessoryList.insert( std::pair<int, vecCDCAccessoryList>( SD_ExpansionVessel, vecList ) );
	m_mapAccessoryList.insert( std::pair<int, vecCDCAccessoryList>( SD_TecBoxCompresso, vecList ) );
	m_mapAccessoryList.insert( std::pair<int, vecCDCAccessoryList>( SD_CompressoVessel, vecList ) );
	m_mapAccessoryList.insert( std::pair<int, vecCDCAccessoryList>( SD_TecBoxTransfero, vecList ) );
	m_mapAccessoryList.insert( std::pair<int, vecCDCAccessoryList>( SD_TecBoxTransferoBufferVesselAccessory, vecList ) );
	m_mapAccessoryList.insert( std::pair<int, vecCDCAccessoryList>( SD_TransferoVessel, vecList ) );
	m_mapAccessoryList.insert( std::pair<int, vecCDCAccessoryList>( SD_IntermediateVessel, vecList ) );
	m_mapAccessoryList.insert( std::pair<int, vecCDCAccessoryList>( SD_PlenoRefill, vecList ) );

	m_pclParent = NULL;
	m_bIsPMTypeAll = true;
	pRViewSSelPM = this;
}

CRViewSSelPM::~CRViewSSelPM()
{
	pRViewSSelPM = NULL;
}

void CRViewSSelPM::OnExpandCollapeGroupButtonClicked( CCDButtonExpandCollapseGroup *pCDBExpandCollapseGroup, CSheetDescription *pclSSheetDescription )
{
	UINT uiSheetDescriptionID = pclSSheetDescription->GetSheetDescriptionID();

	if( uiSheetDescriptionID != SD_ExpansionVessel && uiSheetDescriptionID != SD_ExpansionVesselMembrane && 
		uiSheetDescriptionID != SD_TecBoxCompresso && uiSheetDescriptionID != SD_TecBoxTransfero )
	{
		return;
	}

	CWnd::SetRedraw( FALSE );

	for( UINT uiLoopSheetID = SheetDescription::SD_First; uiLoopSheetID < SheetDescription::SD_Last; uiLoopSheetID++ )
	{
		if( uiLoopSheetID == uiSheetDescriptionID )
		{
			continue;
		}

		if( true == m_ViewDescription.IsSheetDescriptionExist( _ConvertSD2SDG( uiLoopSheetID, uiSheetDescriptionID ) ) )
		{
			UINT uiConvertedLoopSheetID = _ConvertSD2SDG( uiLoopSheetID, uiSheetDescriptionID );
			CSheetDescription *pclLoopSheetDescription = m_ViewDescription.GetFromSheetDescriptionID( uiConvertedLoopSheetID );
			UINT uiGroupID = pclLoopSheetDescription->GetGroupSheetDescriptionID();

			if( uiGroupID == uiSheetDescriptionID )
			{
				if( CCDButtonExpandCollapseGroup::ButtonState::ExpandRow == pCDBExpandCollapseGroup->GetButtonState() )
				{
					HideSSheet( uiConvertedLoopSheetID );
				}
				else
				{
					ShowSSheet( uiConvertedLoopSheetID );
				}
			}
		}
	}

	CWnd::SetRedraw( TRUE );
	Invalidate();
	UpdateWindow();
}

bool CRViewSSelPM::GetBufferVesselTecBoxSelected(LPARAM *pSecondlParam)
{
	CSheetDescription *pclSheetDescriptionPM = m_ViewDescription.GetFromSheetDescriptionID(_ConvertSD2SDG(SD_TecBoxTransfero, -1));

	if (NULL == pclSheetDescriptionPM)
	{
		return false;
	}

	// Retrieve the current selected product.
	CCellDescriptionProduct *pclCDCurrentProductSelected = NULL;
	LPARAM lpPointer;

	if (true == pclSheetDescriptionPM->GetUserVariable(_SDUV_SELECTEDPRODUCT, lpPointer))
	{
		pclCDCurrentProductSelected = (CCellDescriptionProduct *)lpPointer;
	}

	if (NULL != pclCDCurrentProductSelected)
	{
		
		if (NULL != pclCDCurrentProductSelected->GetSecondUserParam() && NULL != pSecondlParam)
		{
			*pSecondlParam = pclCDCurrentProductSelected->GetSecondUserParam();
		}
	}

	bool fReturn = false;

    if (NULL != pSecondlParam && 0 != *pSecondlParam)
	{
		fReturn = true;
	}

	return fReturn;
}

bool CRViewSSelPM::GetProductSelected( UINT uiSheetDescriptionID, UINT uiSDGroupID, CDB_Product **ppclProduct, long *plRow, LPARAM *plParam )
{
	if( false == m_ViewDescription.IsSheetDescriptionExist( _ConvertSD2SDG( uiSheetDescriptionID, uiSDGroupID ) ) || NULL == ppclProduct )
	{
		return false;
	}

	*ppclProduct = NULL;

	if( NULL != plRow )
	{
		*plRow = 0;
	}

	CSheetDescription *pclSheetDescriptionPM = m_ViewDescription.GetFromSheetDescriptionID( _ConvertSD2SDG( uiSheetDescriptionID, uiSDGroupID ) );

	if( NULL == pclSheetDescriptionPM )
	{
		return false;
	}

	// Retrieve the current selected product.
	CCellDescriptionProduct *pclCDCurrentProductSelected = NULL;
	LPARAM lpPointer;

	if( true == pclSheetDescriptionPM->GetUserVariable( _SDUV_SELECTEDPRODUCT, lpPointer ) )
	{
		pclCDCurrentProductSelected = ( CCellDescriptionProduct * )lpPointer;
	}

	if( NULL != pclCDCurrentProductSelected )
	{
		if( NULL != pclCDCurrentProductSelected->GetProduct() )
		{
			*ppclProduct = dynamic_cast<CDB_Product *>( ( CData * )pclCDCurrentProductSelected->GetProduct() );
		}

		if( NULL != pclCDCurrentProductSelected->GetUserParam() && NULL != plParam )
		{
			*plParam = pclCDCurrentProductSelected->GetUserParam();
		}

		if( NULL != plRow )
		{
			*plRow = pclCDCurrentProductSelected->GetCellPosition().y;
		}
	}

	bool fReturn = false;

	if( NULL != *ppclProduct )
	{
		fReturn = true;
	}
	else if( NULL != plParam && 0 != *plParam )
	{
		fReturn = true;
	}

	return fReturn;
}

bool CRViewSSelPM::GetVesselSelected( CDB_Vessel **ppclVessel, UINT *puiSheetDescriptionID, long *plRow, LPARAM *plParam )
{
	if( NULL == ppclVessel )
	{
		return false;
	}

	if( NULL != puiSheetDescriptionID )
	{
		*puiSheetDescriptionID = -1;
	}

	bool bReturn = true;

	if( true == GetExpansionVesselSelected( ppclVessel, plRow, plParam ) )
	{
		if( NULL != puiSheetDescriptionID )
		{
			*puiSheetDescriptionID = SD_ExpansionVessel;
		}
	}
	else if( true == GetExpansionVesselMembraneSelected( ppclVessel, plRow, plParam ) )
	{
		if( NULL != puiSheetDescriptionID )
		{
			*puiSheetDescriptionID = SD_ExpansionVesselMembrane;
		}
	}
	else if( true == GetCompressoVesselSelected( ppclVessel, plRow, plParam ) )
	{
		if( NULL != puiSheetDescriptionID )
		{
			*puiSheetDescriptionID = SD_CompressoVessel;
		}
	}
	else if( true == GetCompressoSecondaryVesselSelected( ppclVessel, plRow, plParam ) )
	{
		if( NULL != puiSheetDescriptionID )
		{
			*puiSheetDescriptionID = SD_CompressoSecondaryVessel;
		}
	}
	else if( true == GetTransferoVesselSelected( ppclVessel, plRow, plParam ) )
	{
		if( NULL != puiSheetDescriptionID )
		{
			*puiSheetDescriptionID = SD_TransferoVessel;
		}
	}
	else
	{
		bReturn = false;
	}

	return bReturn;
}

bool CRViewSSelPM::GetExpansionVesselSelected( CDB_Vessel **ppclVessel, long *plRow, LPARAM *plParam )
{
	return GetProductSelected( SD_ExpansionVessel, -1, ( CDB_Product ** )ppclVessel, plRow, plParam );
}

bool CRViewSSelPM::GetExpansionVesselMembraneSelected( CDB_Vessel **ppclVessel, long *plRow, LPARAM *plParam )
{
	return GetProductSelected( SD_ExpansionVesselMembrane, -1, ( CDB_Product ** )ppclVessel, plRow, plParam );
}

bool CRViewSSelPM::GetCompressoVesselSelected( CDB_Vessel **ppclVessel, long *plRow, LPARAM *plParam )
{
	if( true == GetProductSelected( SD_CompressoVessel, -1, ( CDB_Product ** )ppclVessel, plRow, plParam ) )
	{
		return true;
	}

	// We can have integrated vessel in the Compresso (Like Simply Compresso).
	CDB_TecBox *pclTechBox = NULL;

	if( false == GetTecBoxCompressoSelected( &pclTechBox, plRow, plParam ) )
	{
		return false;
	}

	if( NULL == dynamic_cast<CDB_Vessel*>( pclTechBox->GetIntegratedVesselIDPtr().MP ) )
	{
		return false;
	}
		
	// Integrated vessel exist.
	*ppclVessel = dynamic_cast<CDB_Vessel*>( pclTechBox->GetIntegratedVesselIDPtr().MP );
	return true;
}

bool CRViewSSelPM::GetCompressoSecondaryVesselSelected( CDB_Vessel **ppclVessel, long *plRow, LPARAM *plParam )
{
	return GetProductSelected( SD_CompressoSecondaryVessel, -1, ( CDB_Product ** )ppclVessel, plRow, plParam );
}

bool CRViewSSelPM::GetTransferoVesselSelected( CDB_Vessel **ppclVessel, long *plRow, LPARAM *plParam )
{
	if( true == GetProductSelected( SD_TransferoVessel, -1, (CDB_Product **)ppclVessel, plRow, plParam ) )
	{
		return true;
	}

	// We can have integrated vessel in the Transfero (Not yet!).
	CDB_TecBox *pclTechBox = NULL;

	if( false == GetTecBoxTransferoSelected( &pclTechBox, plRow, plParam ) )
	{
		return false;
	}

	if( NULL != dynamic_cast<CTable*>( pclTechBox->GetIntegratedVesselIDPtr().MP ) )
	{
		// Integrated vessel exist.
		return true;
	}

	return false;
}

bool CRViewSSelPM::GetTecBoxSelected( CDB_TecBox **ppclTechBox, UINT *puiSheetDescriptionID, long *plRow, LPARAM *plParam )
{
	if( NULL == ppclTechBox )
	{
		return false;
	}

	if( NULL != puiSheetDescriptionID )
	{
		*puiSheetDescriptionID = -1;
	}

	bool bReturn = true;

	if( true == GetTecBoxCompressoSelected( ppclTechBox, plRow, plParam ) )
	{
		if( NULL != puiSheetDescriptionID )
		{
			*puiSheetDescriptionID = SD_TecBoxCompresso;
		}
	}
	else if( true == GetTecBoxTransferoSelected( ppclTechBox, plRow, plParam ) )
	{
		if( NULL != puiSheetDescriptionID )
		{
			*puiSheetDescriptionID = SD_TecBoxTransfero;
		}
	}
	else
	{
		bReturn = false;
	}

	return bReturn;
}

bool CRViewSSelPM::GetTecBoxCompressoSelected( CDB_TecBox **ppclTechBox, long *plRow, LPARAM *plParam )
{
	return GetProductSelected( SD_TecBoxCompresso, -1, ( CDB_Product ** )ppclTechBox, plRow, plParam );
}
	
bool CRViewSSelPM::GetTecBoxTransferoSelected( CDB_TecBox **ppclTechBox, long *plRow, LPARAM *plParam )
{
	return GetProductSelected( SD_TecBoxTransfero, -1, ( CDB_Product ** )ppclTechBox, plRow, plParam );
}

bool CRViewSSelPM::GetIntermediateVesselSelected( CDB_Vessel **ppclIntermediateVessel, long *plRow, LPARAM *plParam )
{
	if( NULL == ppclIntermediateVessel || false == m_ViewDescription.IsSheetDescriptionExist( SD_IntermediateVessel ) )
	{
		return false;
	}

	*ppclIntermediateVessel = NULL;

	if( NULL != plRow )
	{
		*plRow = 0;
	}

	CSheetDescription *pclSDIntermdiateVessel = m_ViewDescription.GetFromSheetDescriptionID( SD_IntermediateVessel );

	if( NULL == pclSDIntermdiateVessel )
	{
		return false;
	}

	// Retrieve the current selected intermediate vessel.
	CCellDescriptionProduct *pclCDCurrentIntermediateVesselSelected = NULL;
	LPARAM lpPointer;

	if( true == pclSDIntermdiateVessel->GetUserVariable( _SDUV_SELECTEDPRODUCT, lpPointer ) )
	{
		pclCDCurrentIntermediateVesselSelected = ( CCellDescriptionProduct * )lpPointer;
	}

	if( NULL != pclCDCurrentIntermediateVesselSelected )
	{
		if( NULL != pclCDCurrentIntermediateVesselSelected->GetProduct() )
		{
			*ppclIntermediateVessel = dynamic_cast<CDB_Vessel *>( ( CData * )pclCDCurrentIntermediateVesselSelected->GetProduct() );
		}

		if( NULL != pclCDCurrentIntermediateVesselSelected->GetUserParam() && NULL != plParam )
		{
			*plParam = pclCDCurrentIntermediateVesselSelected->GetUserParam();
		}

		if( NULL != plRow )
		{
			*plRow = pclCDCurrentIntermediateVesselSelected->GetCellPosition().y;
		}
	}

	bool bReturn = false;

	if( NULL != *ppclIntermediateVessel )
	{
		bReturn = true;
	}
	else if( NULL != plParam && 0 != *plParam )
	{
		bReturn = true;
	}

	return bReturn;
}

bool CRViewSSelPM::GetTecBoxVentoSelected( CDB_TBPlenoVento **ppclVento, UINT uiSDGroupID, long *plRow, LPARAM *plParam )
{
	return GetProductSelected( SD_TecBoxVento, uiSDGroupID, ( CDB_Product ** )ppclVento, plRow, plParam );
}

bool CRViewSSelPM::GetTecBoxPlenoSelected( CDB_TBPlenoVento **ppclPleno, UINT uiSDGroupID, long *plRow, LPARAM *plParam )
{
	return GetProductSelected( SD_TecBoxPleno, uiSDGroupID, ( CDB_Product ** )ppclPleno, plRow, plParam );
}

bool CRViewSSelPM::GetTecBoxPlenoWMProtectionSelected( CDB_TBPlenoVento **ppclPleno, UINT uiSDGroupID, long *plRow, LPARAM *plParam )
{
	return GetProductSelected( SD_PlenoWaterMakeUpProtection, uiSDGroupID, ( CDB_Product ** )ppclPleno, plRow, plParam );
}

bool CRViewSSelPM::GetPlenoRefillSelected( CDB_PlenoRefill **ppclPlenoRefill, UINT uiSDGroupID, long *plRow, LPARAM *plParam )
{
	return GetProductSelected( SD_PlenoRefill, uiSDGroupID, ( CDB_Product ** )ppclPlenoRefill, plRow, plParam );
}

bool CRViewSSelPM::GetTransferoAccessoryList( CAccessoryList *pclAccessoryListToFill )
{
	if( NULL == pclAccessoryListToFill )
	{
		return false;
	}

	if( NULL == _GetvecCDAccessoryList( SD_TecBoxTransferoAccessory ) || 0 == _GetvecCDAccessoryList( SD_TecBoxTransferoAccessory )->size()  )
	{
		return false;
	}

	for( auto &iter : *_GetvecCDAccessoryList( SD_TecBoxTransferoAccessory ) )
	{
		if( NULL != iter && true == iter->GetCheckStatus() && NULL != iter->GetAccessoryPointer() )
		{
			pclAccessoryListToFill->Add( iter->GetAccessoryPointer()->GetIDPtr(), CAccessoryList::_AT_Accessory, iter->GetRuledTable() );
		}
	}

	return true;
}

bool CRViewSSelPM::GetTransferoVesselAccessoryList( CAccessoryList *pclAccessoryListToFill )
{
	if( NULL == pclAccessoryListToFill )
	{
		return false;
	}

	if( NULL == _GetvecCDAccessoryList( SD_TransferoVesselAccessory ) || 0 == _GetvecCDAccessoryList( SD_TransferoVesselAccessory )->size()  )
	{
		return false;
	}

	for( auto &iter : *_GetvecCDAccessoryList( SD_TransferoVesselAccessory ) )
	{
		if( NULL != iter && true == iter->GetCheckStatus() && NULL != iter->GetAccessoryPointer() )
		{
			pclAccessoryListToFill->Add( iter->GetAccessoryPointer()->GetIDPtr(), CAccessoryList::_AT_Accessory, iter->GetRuledTable() );
		}
	}

	return true;
}

void CRViewSSelPM::Reset()
{
	for( int i = 0; i < SD_Last; i++ )
	{
		if( 0 != m_mapButtonExpandCollapseRows.count( (SheetDescription)i ) )
		{
			m_mapButtonExpandCollapseRows[( SheetDescription )i] = NULL;
		}

		if( 0 != m_mapButtonExpandCollapseGroup.count( (SheetDescription)i ) )
		{
			m_mapButtonExpandCollapseGroup[( SheetDescription )i] = NULL;
		}

		if( 0 != m_mapButtonShowAllPriorities.count( (SheetDescription)i ) )
		{
			m_mapButtonShowAllPriorities[( SheetDescription )i] = NULL;
		}

		if( 0 != m_mapAccessoryList.count( (SheetDescription)i ) )
		{
			m_mapAccessoryList[( SheetDescription )i].clear();
		}

		m_arlpSelectedUserParam[( SheetDescription )i] = 0;
	}

	CDlgPMGraphsOutput::CPMInterface clOutputInterface;
	clOutputInterface.ClearOutput();

	if( TRUE == IsWindowVisible() )
	{
		_SetPMGraphsOutputContext( false );
	}

	CRViewSSelSS::Reset();
}

void CRViewSSelPM::Suggest( CProductSelelectionParameters *pclProductSelectionParameters, LPARAM lpParam )
{
	CRViewSSelSS::Suggest( pclProductSelectionParameters, lpParam );

	if( NULL == pclProductSelectionParameters || NULL == dynamic_cast<CIndSelPMParams*>( pclProductSelectionParameters ) )
	{
		return;
	}

	m_pclIndSelPMParams = dynamic_cast<CIndSelPMParams*>( pclProductSelectionParameters );

	if( NULL == m_pclIndSelPMParams || NULL == m_pclIndSelPMParams->m_pclSelectPMList 
			|| NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		return;
	}

	CDS_SSelPMaint *pclSelectedPM = NULL;

	// If we are in edition mode...
	if( NULL != m_pclIndSelPMParams->m_SelIDPtr.MP )
	{
		pclSelectedPM = reinterpret_cast<CDS_SSelPMaint*>( (CData*)( m_pclIndSelPMParams->m_SelIDPtr.MP ) );

		if( NULL == pclSelectedPM )
		{
			ASSERT( 0 );
		}
	}

	CPMInputUser *pclInputUser = m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser();

	m_bIsPMTypeAll = false;

	if( CPMInputUser::MT_All == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetPressureMaintenanceType() )
	{
		m_bIsPMTypeAll = true;
	}

	// To remove all current displayed sheets.
	Reset();

	BeginWaitCursor();
	CWnd::SetRedraw( FALSE );

	long lSelectedRow = -1;
	UINT uiSelectedProductSheetID = 0xFFFFFFFF;
	CDB_Product *pclProduct = NULL;
	CAccessoryList *pclAccessoryList = NULL;

	if( true == pclInputUser->IsPressurisationSystemExist() )
	{
		_InitAndFillComputedDataRows();

		if( true == TASApp.IsPMPriceIndexUsed() 
			&& ( true == m_bIsPMTypeAll 
					|| CPMInputUser::MT_WithCompressor == pclInputUser->GetPressureMaintenanceType()
					|| CPMInputUser::MT_WithPump == pclInputUser->GetPressureMaintenanceType() ) )
		{
			_InitPriceIndexGroup();
		}

		long lTemp;

		if( true == m_bIsPMTypeAll 
				|| CPMInputUser::MT_ExpansionVessel == pclInputUser->GetPressureMaintenanceType() )
		{
			// HYS-662 if for "All type" the safety pressure valve is lower than the PSV min don't show the sheet. 
			if( m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetSafetyValveResponsePressure() >=
					m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetMinimumRequiredPSVRounded( CPMInputUser::MT_ExpansionVessel ) )
			{
				lTemp = _FillExpansionVesselSheet( SD_ExpansionVessel, pclSelectedPM );

				if( lTemp > 0 )
				{
					lSelectedRow = lTemp;
					uiSelectedProductSheetID = SD_ExpansionVessel;
					pclProduct = dynamic_cast<CDB_Vessel *>( pclSelectedPM->GetVesselIDPtr().MP );
					pclAccessoryList = pclSelectedPM->GetVesselAccessoryList();
				}
			}
		}

		if( 0 != TASApp.GetTADBKey().CompareNoCase( _T("CH") ) &&
				( true == m_bIsPMTypeAll || CPMInputUser::MT_ExpansionVesselWithMembrane == pclInputUser->GetPressureMaintenanceType() ) )
		{
			// HYS-1577: the Squeeze and MN vessels are now phase out. We keep the code in case of.
			// HYS-1657: The MN vessels are released for now
			CTableOrdered *pTabO = (CTableOrdered *)( TASApp.GetpTADB()->Get( _T("PMT_EXPVSSLMBR_TAB") ).MP );

			if( NULL != pTabO )
			{
				// HYS-662 if for "All type" the safety pressure valve is lower than the PSV min don't show the sheet. 
				if( m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetSafetyValveResponsePressure() >=
						m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetMinimumRequiredPSVRounded( CPMInputUser::MT_ExpansionVessel ) )
				{
					lTemp = _FillExpansionVesselSheet( SD_ExpansionVesselMembrane, pclSelectedPM );

					if( lTemp > 0 )
					{
						lSelectedRow = lTemp;
						uiSelectedProductSheetID = SD_ExpansionVesselMembrane;
						pclProduct = dynamic_cast<CDB_Vessel *>( pclSelectedPM->GetVesselIDPtr().MP );
						pclAccessoryList = pclSelectedPM->GetVesselAccessoryList();
					}
				}
			}
		}

		if( Solar != pclInputUser->GetApplicationType() 
			&& ( true == m_bIsPMTypeAll
				|| CPMInputUser::MT_WithCompressor == pclInputUser->GetPressureMaintenanceType() ) )
		{
			// HYS-662 if for "All type" the safety pressure valve is lower than the PSV min don't show the sheet. 
			if( m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetSafetyValveResponsePressure() >=
					m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetMinimumRequiredPSVRounded( CPMInputUser::MT_WithCompressor ) )
			{
				lTemp = _FillTecBoxCompressoSheet( pclSelectedPM );

				if( lTemp > 0 )
				{
					lSelectedRow = lTemp;
					uiSelectedProductSheetID = SD_TecBoxCompresso;
					pclProduct = dynamic_cast<CDB_Product *>( pclSelectedPM->GetTecBoxCompTransfIDPtr().MP );
					pclAccessoryList = pclSelectedPM->GetTecBoxCompTransfAccessoryList();
				}
			}
		}

		if( Solar != pclInputUser->GetApplicationType()
			&& ( true == m_bIsPMTypeAll
				|| CPMInputUser::MT_WithPump == pclInputUser->GetPressureMaintenanceType() ) )
		{
			// HYS-662 if for "All type" the safety pressure valve is lower than the PSV min don't show the sheet.
			if( m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetSafetyValveResponsePressure() >=
					m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetMinimumRequiredPSVRounded(CPMInputUser::MT_WithPump) )
			{
				lTemp = _FillTecBoxTransferoSheet( pclSelectedPM );

				if( lTemp > 0 )
				{
					lSelectedRow = lTemp;
					uiSelectedProductSheetID = SD_TecBoxTransfero;
					pclProduct = dynamic_cast<CDB_Product *>( pclSelectedPM->GetTecBoxCompTransfIDPtr().MP );
					pclAccessoryList = pclSelectedPM->GetTecBoxCompTransfAccessoryList();
				}
			}
		}
	}
	else
	{
		if( NULL == pclSelectedPM )
		{
			_FillTecBoxVentoSheet();
			_FillTecBoxPlenoSheet();
		}
		else if( NULL != pclSelectedPM )
		{
			long lTemp = 0;

			if( NULL != pclSelectedPM->GetTecBoxVentoIDPtr().MP )
			{
				lTemp = _FillTecBoxVentoSheet( -1, NULL, pclSelectedPM, true );
				pclProduct = dynamic_cast<CDB_Product *>( pclSelectedPM->GetTecBoxVentoIDPtr().MP );
				uiSelectedProductSheetID = SD_TecBoxVento;
			}
			else if( BST_CHECKED == pclInputUser->GetDegassingChecked() )
			{
				// If user has done his selection with the 'Vacuum degassing' selected, we can show the Vento list.
				_FillTecBoxVentoSheet( -1, NULL, pclSelectedPM, true );
			}
			
			if( NULL != pclSelectedPM->GetTecBoxPlenoIDPtr().MP )
			{
				lTemp = _FillTecBoxPlenoSheet( -1, NULL, false, pclSelectedPM, true );
				pclProduct = dynamic_cast<CDB_Product *>( pclSelectedPM->GetTecBoxPlenoIDPtr().MP );
				uiSelectedProductSheetID = SD_TecBoxPleno;
			}

			if( lTemp > 0 )
			{
				lSelectedRow = lTemp;
			}
		}
	}

	if( NULL != pclSelectedPM && -1 != uiSelectedProductSheetID )
	{
		CSheetDescription *pclSheetDescription = m_ViewDescription.GetFromSheetDescriptionID( uiSelectedProductSheetID );

		if( NULL != pclSheetDescription && NULL != pclSheetDescription->GetSSheetPointer() )
		{
			CCellDescriptionProduct *pclCDProduct = NULL;

			if( true == pclInputUser->IsPressurisationSystemExist() )
			{
				CCellDescriptionProduct *pclCDProduct = FindCDProduct( lSelectedRow, (LPARAM)pclProduct, pclSheetDescription );

				if( NULL != pclCDProduct && NULL != pclCDProduct->GetProduct() )
				{
					// If we are in edition mode we simulate a click on the product.
					OnClickProduct( pclSheetDescription, pclCDProduct, pclSheetDescription->GetActiveColumn(), lSelectedRow );

					// Allow to check if we need to change the 'Show all priorities' button or not.
					CheckShowAllPrioritiesButtonState( pclSheetDescription, lSelectedRow );

					// Verify accessories.
					if( NULL != pclAccessoryList && pclAccessoryList->GetCount() > 0 )
					{
						// Remark: if product is a expansion vessel 'pclAccessoryList' points to 'CDS_SSelPMaint::m_clVesselAccessoryList'.
						//         If it's a Compresso or a Transfero, it points to 'CDS_SSelPMaint::m_clTechBoxAccessoryList'. In this case, primary and secondary
						//         vessel accessories will be filled just below.
						CAccessoryList::AccessoryItem rAccessoryItem = pclAccessoryList->GetFirst();

						while( NULL != rAccessoryItem.IDPtr.MP )
						{
							VerifyCheckboxAccessories( (CDB_Product *)rAccessoryItem.IDPtr.MP, true, _GetvecCDAccessoryList( uiSelectedProductSheetID ) );
							rAccessoryItem = pclAccessoryList->GetNext();
						}
					}

					// If it's a Compresso or a Transfero tech-box, we need to click on the selected Compresso or Transfero if exist !!!
					if( SD_TecBoxCompresso == uiSelectedProductSheetID || SD_TecBoxTransfero == uiSelectedProductSheetID )
					{
						// HYS-872: To remember check box buffer vessel accessories.
						if( true == m_ViewDescription.IsSheetDescriptionExist( SD_TecBoxTransferoBufferVesselAccessory ) )
						{
							CAccessoryList *pclTBBufferVesselAccessoryList = pclSelectedPM->GetTecBoxIntegratedVesselAccessoryList();
								
							if( NULL != pclTBBufferVesselAccessoryList && pclTBBufferVesselAccessoryList->GetCount() > 0 )
							{
								CAccessoryList::AccessoryItem rAccessoryItem = pclTBBufferVesselAccessoryList->GetFirst();

								while (NULL != rAccessoryItem.IDPtr.MP)
								{
									VerifyCheckboxAccessories( (CDB_Product *)rAccessoryItem.IDPtr.MP, true, _GetvecCDAccessoryList( SD_TecBoxTransferoBufferVesselAccessory ) );
									rAccessoryItem = pclTBBufferVesselAccessoryList->GetNext();
								}
							}
						}

						if( NULL != dynamic_cast<CDB_Vessel *>( pclSelectedPM->GetVesselIDPtr().MP ) )
						{
							// Remark: In case of Compresso or Transfero selection, the primary vessel can be obtained by calling 'CDS_SSelPMaint::GetVesselIDPtr()'.
							CDB_Vessel *pclPrimaryVessel = dynamic_cast<CDB_Vessel *>( pclSelectedPM->GetVesselIDPtr().MP );

							UINT uiSDTechBoxVesselID = ( SD_TecBoxCompresso == uiSelectedProductSheetID ) ? SD_CompressoVessel : SD_TransferoVessel;
							CSheetDescription *pclSDTechBoxVessel = m_ViewDescription.GetFromSheetDescriptionID( uiSDTechBoxVesselID );

							if( NULL != pclSDTechBoxVessel && NULL != pclSDTechBoxVessel->GetSSheetPointer() )
							{
								CCellDescriptionProduct *pclCDTBVessel = FindCDProduct( -1, (LPARAM)pclPrimaryVessel, pclSDTechBoxVessel );

								if( NULL != pclCDTBVessel && NULL != pclCDTBVessel->GetProduct() )
								{
									// If we are in edition mode we simulate a click on the product.
									OnClickProduct( pclSDTechBoxVessel, pclCDTBVessel, pclSDTechBoxVessel->GetActiveColumn(), pclCDTBVessel->GetCellPosition().y );

									// Allow to check if we need to change the 'Show all priorities' button or not.
									CheckShowAllPrioritiesButtonState( pclSDTechBoxVessel, pclCDTBVessel->GetCellPosition().y );

									// Check accessories.
									// Remark: In case of Compresso or Transfero selection, the vessel accessories can be obtained by calling 'CDS_SSelPMaint::GetVesselAccessoryList()'.
									CAccessoryList *pclTBAccessoryList = pclSelectedPM->GetVesselAccessoryList();

									if( NULL != pclTBAccessoryList && pclTBAccessoryList->GetCount() > 0 )
									{
										CAccessoryList::AccessoryItem rAccessoryItem = pclTBAccessoryList->GetFirst();

										while( NULL != rAccessoryItem.IDPtr.MP )
										{
											VerifyCheckboxAccessories( (CDB_Product * )rAccessoryItem.IDPtr.MP, true, _GetvecCDAccessoryList( uiSDTechBoxVesselID ) );
											rAccessoryItem = pclTBAccessoryList->GetNext();
										}
									}
								}
							}
						}
					}
				}
			}

			// If there is a Vento selected and the Vento sheet exists...
			CDB_Product *pclTecBoxVento = dynamic_cast<CDB_Product *>( pclSelectedPM->GetTecBoxVentoIDPtr().MP );
			CSheetDescription *pclSDTecBoxVento = m_ViewDescription.GetFromSheetDescriptionID( _ConvertSD2SDG( SD_TecBoxVento, uiSelectedProductSheetID ) );

			if( NULL != pclTecBoxVento && NULL != pclSDTecBoxVento && NULL != pclSDTecBoxVento->GetSSheetPointer() )
			{
				pclCDProduct = FindCDProduct( -1, (LPARAM)pclTecBoxVento, pclSDTecBoxVento );

				if( NULL != pclCDProduct && NULL != pclCDProduct->GetProduct() )
				{
					// If we are in edition mode we simulate a click on the product.
					OnClickProduct( pclSDTecBoxVento, pclCDProduct, pclSDTecBoxVento->GetActiveColumn(), pclCDProduct->GetCellPosition().y );

					// Allow to check if we need to change the 'Show all priorities' button or not.
					CheckShowAllPrioritiesButtonState( pclSDTecBoxVento, pclCDProduct->GetCellPosition().y );

					// Check accessories.
					CAccessoryList *pclTecBoxVentoAccessoryList = pclSelectedPM->GetTecBoxVentoAccessoryList();

					if( NULL != pclTecBoxVentoAccessoryList && pclTecBoxVentoAccessoryList->GetCount() > 0 )
					{
						CAccessoryList::AccessoryItem rAccessoryItem = pclTecBoxVentoAccessoryList->GetFirst();

						while( NULL != rAccessoryItem.IDPtr.MP )
						{
							VerifyCheckboxAccessories( (CDB_Product * )rAccessoryItem.IDPtr.MP, true, _GetvecCDAccessoryList( SD_TecBoxVento ) );
							rAccessoryItem = pclTecBoxVentoAccessoryList->GetNext();
						}
					}
				}
			}

			// If there is a Pleno selected and the Pleno sheet exists...
			CDB_Product *pclTecBoxPleno = dynamic_cast<CDB_Product *>( pclSelectedPM->GetTecBoxPlenoIDPtr().MP );
			CDB_Product *pclTecBoxPlenoWMProtection = dynamic_cast<CDB_Product *>( pclSelectedPM->GetTecBoxPlenoProtectionIDPtr().MP );
			CDB_Set *pclWTCombination = dynamic_cast<CDB_Set*>( pclSelectedPM->GetAdditionalTecBoxPlenoIDPtr().MP );
			CSheetDescription *pclSDTecBoxPleno = m_ViewDescription.GetFromSheetDescriptionID( _ConvertSD2SDG( SD_TecBoxPleno, uiSelectedProductSheetID ) );

			if( ( NULL != pclTecBoxPleno || NULL != pclWTCombination ) && NULL != pclSDTecBoxPleno && NULL != pclSDTecBoxPleno->GetSSheetPointer() )
			{
				if( NULL == pclWTCombination )
				{
					pclCDProduct = FindCDProduct( -1, (LPARAM)pclTecBoxPleno, pclSDTecBoxPleno );
				}
				else
				{
					pclCDProduct = FindCDProductByUserParam( -1, (LPARAM)pclWTCombination, pclSDTecBoxPleno );
				}

				if( NULL != pclCDProduct && ( NULL != pclCDProduct->GetProduct() || NULL != pclCDProduct->GetUserParam() ) )
				{
					// If we are in edition mode we simulate a click on the product.
					OnClickProduct( pclSDTecBoxPleno, pclCDProduct, pclSDTecBoxPleno->GetActiveColumn(), pclCDProduct->GetCellPosition().y );

					// Allow to check if we need to change the 'Show all priorities' button or not.
					CheckShowAllPrioritiesButtonState( pclSDTecBoxPleno, pclCDProduct->GetCellPosition().y );

					// Check accessories.
					CAccessoryList *pclTecBoxPlenoAccessoryList = pclSelectedPM->GetTecBoxPlenoAccessoryList();

					if( NULL != pclTecBoxPlenoAccessoryList && pclTecBoxPlenoAccessoryList->GetCount() > 0 )
					{
						CAccessoryList::AccessoryItem rAccessoryItem = pclTecBoxPlenoAccessoryList->GetFirst();

						while( NULL != rAccessoryItem.IDPtr.MP )
						{
							VerifyCheckboxAccessories( (CDB_Product * )rAccessoryItem.IDPtr.MP, true, _GetvecCDAccessoryList( SD_TecBoxPleno ) );
							rAccessoryItem = pclTecBoxPlenoAccessoryList->GetNext();
						}
					}
				}
				// HYS-1121: manage the water make-up protection when working on edition mode
				if( NULL != pclTecBoxPlenoWMProtection )
				{
					CSheetDescription *pclSDTecBoxPlenoWMProtection = m_ViewDescription.GetFromSheetDescriptionID( _ConvertSD2SDG( SD_PlenoWaterMakeUpProtection, uiSelectedProductSheetID ) );
					pclCDProduct = FindCDProduct( -1, (LPARAM)pclTecBoxPlenoWMProtection, pclSDTecBoxPlenoWMProtection );
					if( NULL != pclCDProduct && ( NULL != pclCDProduct->GetProduct() || NULL != pclCDProduct->GetUserParam() ) )
					{
						// If we are in edition mode we simulate a click on the product.
						OnClickProduct( pclSDTecBoxPlenoWMProtection, pclCDProduct, pclSDTecBoxPlenoWMProtection->GetActiveColumn(), pclCDProduct->GetCellPosition().y );

						// Allow to check if we need to change the 'Show all priorities' button or not.
						CheckShowAllPrioritiesButtonState( pclSDTecBoxPlenoWMProtection, pclCDProduct->GetCellPosition().y );

						// Check accessories.
						CAccessoryList *pclTecBoxPlenoAccessoryList = pclSelectedPM->GetTecBoxPlenoProtecAccessoryList();

						if( NULL != pclTecBoxPlenoAccessoryList && pclTecBoxPlenoAccessoryList->GetCount() > 0 )
						{
							CAccessoryList::AccessoryItem rAccessoryItem = pclTecBoxPlenoAccessoryList->GetFirst();

							while( NULL != rAccessoryItem.IDPtr.MP )
							{
								VerifyCheckboxAccessories( (CDB_Product *)rAccessoryItem.IDPtr.MP, true, _GetvecCDAccessoryList( SD_PlenoWaterMakeUpProtection ) );
								rAccessoryItem = pclTecBoxPlenoAccessoryList->GetNext();
							}
						}
					}
				}
			}

			// If there is a Pleno refill selected and the Pleno refill sheet exists...
			CDB_Product *pclPlenoRefill = dynamic_cast<CDB_Product *>( pclSelectedPM->GetPlenoRefillIDPtr().MP );
			CSheetDescription *pclSDPlenoRefill = m_ViewDescription.GetFromSheetDescriptionID( _ConvertSD2SDG( SD_PlenoRefill, uiSelectedProductSheetID ) );

			if( NULL != pclPlenoRefill && NULL != pclSDPlenoRefill && NULL != pclSDPlenoRefill->GetSSheetPointer() )
			{
				pclCDProduct = FindCDProduct( -1, (LPARAM)pclPlenoRefill, pclSDPlenoRefill );

				if( NULL != pclCDProduct && NULL != pclCDProduct->GetProduct() )
				{
					// If we are in edition mode we simulate a click on the product.
					OnClickProduct( pclSDPlenoRefill, pclCDProduct, pclSDPlenoRefill->GetActiveColumn(), pclCDProduct->GetCellPosition().y );

					// Allow to check if we need to change the 'Show all priorities' button or not.
					CheckShowAllPrioritiesButtonState( pclSDPlenoRefill, pclCDProduct->GetCellPosition().y );

					// Check accessories.
					CAccessoryList *pclPlenoRefillAccessoryList = pclSelectedPM->GetPlenoRefillAccessoryList();

					if( NULL != pclPlenoRefillAccessoryList && pclPlenoRefillAccessoryList->GetCount() > 0 )
					{
						CAccessoryList::AccessoryItem rAccessoryItem = pclPlenoRefillAccessoryList->GetFirst();

						while( NULL != rAccessoryItem.IDPtr.MP )
						{
							VerifyCheckboxAccessories( (CDB_Product * )rAccessoryItem.IDPtr.MP, true, _GetvecCDAccessoryList( SD_PlenoRefill ) );
							rAccessoryItem = pclPlenoRefillAccessoryList->GetNext();
						}
					}
				}
			}

			// If there is a intermediate vessel selected and the intermediate vessel sheet exists...
			CDB_Product *pclIntermediateVessel = dynamic_cast<CDB_Product *>( pclSelectedPM->GetIntermediateVesselIDPtr().MP );
			CSheetDescription *pclSDVesselIntermediate = m_ViewDescription.GetFromSheetDescriptionID( SD_IntermediateVessel );

			if( NULL != pclIntermediateVessel && NULL != pclSDVesselIntermediate && NULL != pclSDVesselIntermediate->GetSSheetPointer() )
			{
				pclCDProduct = FindCDProduct( -1, (LPARAM)pclIntermediateVessel, pclSDVesselIntermediate );

				if( NULL != pclCDProduct && NULL != pclCDProduct->GetProduct() )
				{
					// If we are in edition mode we simulate a click on the product.
					OnClickProduct( pclSDVesselIntermediate, pclCDProduct, pclSDVesselIntermediate->GetActiveColumn(), pclCDProduct->GetCellPosition().y );

					// Allow to check if we need to change the 'Show all priorities' button or not.
					CheckShowAllPrioritiesButtonState( pclSDVesselIntermediate, pclCDProduct->GetCellPosition().y );

					// Check accessories.
					CAccessoryList *pclIntermediateVesselAccessoryList = pclSelectedPM->GetIntermediateVesselAccessoryList();

					if( NULL != pclIntermediateVesselAccessoryList && pclIntermediateVesselAccessoryList->GetCount() > 0 )
					{
						CAccessoryList::AccessoryItem rAccessoryItem = pclIntermediateVesselAccessoryList->GetFirst();

						while( NULL != rAccessoryItem.IDPtr.MP )
						{
							VerifyCheckboxAccessories( (CDB_Product * )rAccessoryItem.IDPtr.MP, true, _GetvecCDAccessoryList( SD_IntermediateVessel ) );
							rAccessoryItem = pclIntermediateVesselAccessoryList->GetNext();
						}
					}
				}
			}
		}
	}
	else
	{
		// Define the first product row as the active cell and set a visual focus.
		CSheetDescription *pclSheetDescription = m_ViewDescription.GetFromSheetDescriptionID( SD_ExpansionVessel );
		PrepareAndSetNewFocus( pclSheetDescription, CD_ExpansionVessel_Name, RD_ExpansionVessel_FirstAvailRow, 0 );
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

void CRViewSSelPM::FillInSelected( CDS_SSelPMaint *pclSelectedPMFill )
{
	try
	{
		if( NULL == m_pclIndSelPMParams )
		{
			HYSELECT_THROW( _T("Internal error: 'm_pclIndSelPMParams' can't be NULL.") );
		}
		else if( NULL == m_pclIndSelPMParams->m_pclSelectPMList )
		{
			HYSELECT_THROW( _T("Internal error: 'm_pclIndSelPMParams->m_pclSelectPMList' can't be NULL.") );
		}
		else if( NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
		{
			HYSELECT_THROW( _T("Internal error: 'm_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()' can't be NULL.") );
		}
		else if( NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetPMWQSelectionPreferences() )
		{
			HYSELECT_THROW( _T("Internal error: 'm_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetPMWQSelectionPreferences()' can't be NULL.") );
		}

		// HYS-987: Copy the current selection before the reset to keep accessories information if we are in edit mode
		bool bIsEditionMode = false;
		CDS_SSelPMaint *pclSelectedPMCpy = NULL;
		IDPTR IDPtr;

		if( true == m_pclIndSelPMParams->m_bEditModeRunning )
		{
			m_pclIndSelPMParams->m_pTADS->CreateObject( IDPtr, CLASS( CDS_SSelPMaint ) );
			pclSelectedPMCpy = dynamic_cast<CDS_SSelPMaint*> ( IDPtr.MP );
			pclSelectedPMFill->Copy( pclSelectedPMCpy );
			bIsEditionMode = true;
		}

		CPMInputUser *pclInputUser = m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser();
		pclSelectedPMFill->Reset();

		bool bRedundancy = pclInputUser->GetPMWQSelectionPreferences()->IsChecked( CPMWQPrefs::GeneralRedundancyTecBox );
		int iTecBoxNumber = 1;

		// HYS-1537 : Check if the preference is enabled
		if( true == bRedundancy && false == pclInputUser->GetPMWQSelectionPreferences()->IsDisabled( CPMWQPrefs::GeneralRedundancyTecBox ) )
		{
			iTecBoxNumber = 2;
		}

		UINT uiVesselSheetDescriptionID = -1;
		UINT uiGroupSheetDescriptionID = -1;

		if( true == pclInputUser->IsPressurisationSystemExist() )
		{
			CDB_Vessel *pclVessel = NULL;
			long lRow = -1;

			// Check what is the expansion vessel selected.
			if( false == GetVesselSelected( &pclVessel, &uiVesselSheetDescriptionID, &lRow ) || NULL == pclVessel )
			{
				return;
			}

			// Save the selected expansion vessel.
			CAccessoryList *pclVesselAccessoryListToFill = NULL;
			CAccessoryList *pclTecBoxAccessoryListToFill = NULL;
			// HYS-872
			CAccessoryList *pclBufferVesselAccessoryListToFill = NULL;

			switch( uiVesselSheetDescriptionID )
			{
				case SD_ExpansionVessel:
				case SD_ExpansionVesselMembrane:
				{
					uiGroupSheetDescriptionID = uiVesselSheetDescriptionID;
					pclSelectedPMFill->SetSelectionType( CDS_SSelPMaint::Statico );
					pclSelectedPMFill->SetVesselIDPtr( pclVessel->GetIDPtr() );
					if( true == bIsEditionMode )
					{
						pclSelectedPMFill->SetVesselAccessoryList( pclSelectedPMCpy->GetVesselAccessoryList() );
					}

					pclVesselAccessoryListToFill = pclSelectedPMFill->GetVesselAccessoryList();

					CSelectPMList::ProductType eProductType = ( SD_ExpansionVessel == uiVesselSheetDescriptionID ) ? CSelectPMList::PT_ExpansionVessel 
							: CSelectPMList::PT_ExpansionVesselMembrane;

					// Search product in the CSelectList to get some variables to add for the expansion vessel.
					for( CSelectedPMBase *pclSelectedProduct = m_pclIndSelPMParams->m_pclSelectPMList->GetFirstProduct( eProductType );
							NULL != pclSelectedProduct; pclSelectedProduct = m_pclIndSelPMParams->m_pclSelectPMList->GetNextProduct( eProductType ) )
					{
						CSelectedVssl *pclSelectedVsl = dynamic_cast<CSelectedVssl *>( pclSelectedProduct );

						if( NULL == pclSelectedVsl )
						{
							continue;
						}

						if( pclSelectedVsl->GetpData() == pclVessel )
						{
							pclSelectedPMFill->SetVesselInitialPressure( pclSelectedVsl->GetInitialPressure() );
							pclSelectedPMFill->SetVesselWaterReserve( pclSelectedVsl->GetWaterReserve() );
							pclSelectedPMFill->SetVesselNumber( pclSelectedVsl->GetNbreOfVsslNeeded() );
							break;
						}
					}

					break;
				}

				case SD_CompressoVessel:
				{
					CDB_TecBox *pclTecBoxCompresso = NULL;

					if( false == GetTecBoxCompressoSelected( &pclTecBoxCompresso ) || NULL == pclTecBoxCompresso )
					{
						break;
					}

					uiGroupSheetDescriptionID = SD_TecBoxCompresso;
					pclSelectedPMFill->SetSelectionType( CDS_SSelPMaint::Compresso );

					// Save the TecBox selected.
					pclSelectedPMFill->SetTecBoxCompTransfIDPtr( pclTecBoxCompresso->GetIDPtr() );
					
					if( true == bIsEditionMode )
					{
						pclSelectedPMFill->SetTecBoxCompTransfAccessoryList( pclSelectedPMCpy->GetTecBoxCompTransfAccessoryList() );
					}
					
					pclTecBoxAccessoryListToFill = pclSelectedPMFill->GetTecBoxCompTransfAccessoryList();

					int iNbrOfDevices = 1;

					// HYS-599: Check if the Compresso is a CX model and in this case check how many devices we need.
					if( true == pclTecBoxCompresso->IsVariantExternalAir() )
					{
						// Search Compresso CX in CSelectList to set the correct opening.
						for( CSelectedPMBase *pclSelectedProduct = m_pclIndSelPMParams->m_pclSelectPMList->GetFirstProduct( CSelectPMList::PT_Compresso ); 
								NULL != pclSelectedProduct; pclSelectedProduct = m_pclIndSelPMParams->m_pclSelectPMList->GetNextProduct( CSelectPMList::PT_Compresso ) )
						{
							CSelectedCompresso *pclSelectedCompresso = dynamic_cast<CSelectedCompresso *>( pclSelectedProduct );

							if( NULL == pclSelectedCompresso )
							{
								continue;
							}

							if( pclSelectedCompresso->GetpData() == pclTecBoxCompresso )
							{
								iNbrOfDevices = pclSelectedCompresso->GetCompressoCXNumber();
							}
						}
					}
					// HYS-1537 : Check if the preference is enabled
					if( true == bRedundancy && false == pclInputUser->GetPMWQSelectionPreferences()->IsDisabled( CPMWQPrefs::GeneralRedundancyTecBox ) )
					{
						iNbrOfDevices *= 2;
					}
				
					pclSelectedPMFill->SetTecBoxCompTransfNumber( iNbrOfDevices );

					// If primary vessel is not integrated in the TecBox...
					if( false == pclVessel->IsPrimaryVesselIntegrated() )
					{
						// Save the Compresso vessel.
						pclSelectedPMFill->SetVesselIDPtr( pclVessel->GetIDPtr() );

						// Search product in the CSelectList to get some variables to add for the Compresso vessel.
						for( CSelectedPMBase *pclSelectedProduct = m_pclIndSelPMParams->m_pclSelectPMList->GetFirstProduct( CSelectPMList::PT_CompressoVessel );
								 NULL != pclSelectedProduct; pclSelectedProduct = m_pclIndSelPMParams->m_pclSelectPMList->GetNextProduct( CSelectPMList::PT_CompressoVessel ) )
						{
							CSelectedVssl *pclSelectedCompressoVessel = dynamic_cast<CSelectedVssl *>( pclSelectedProduct );

							if( NULL == pclSelectedCompressoVessel )
							{
								continue;
							}

							if( pclSelectedCompressoVessel->GetpData() == pclVessel )
							{
								// Remark: in case of Compresso and Transfero vessel, we have always one primary vessel and 0, 1 or n secondary vessels.
								pclSelectedPMFill->SetVesselInitialPressure( pclSelectedCompressoVessel->GetInitialPressure() );
								pclSelectedPMFill->SetVesselWaterReserve( pclSelectedCompressoVessel->GetWaterReserve() );
								pclSelectedPMFill->SetVesselNumber( 1 );

								// Secondary vessel ?
								if( pclSelectedCompressoVessel->GetNbreOfVsslNeeded() > 1 )
								{
									CDB_Vessel *pclSecondaryVessel = pclVessel->GetAssociatedSecondaryVessel();

									if( NULL != pclSecondaryVessel )
									{
										pclSelectedPMFill->SetSecondaryVesselIDPtr( pclSecondaryVessel->GetIDPtr() );
										pclSelectedPMFill->SetSecondaryVesselNumber( pclSelectedCompressoVessel->GetNbreOfVsslNeeded() - 1 );
									}
								}

								break;
							}
						}
					}
					else
					{
						// Save the primary integrated vessel.
						pclSelectedPMFill->SetTecBoxIntegratedVesselIDPtr( pclVessel->GetIDPtr() );
						pclSelectedPMFill->SetVesselNumber( 1 );

						// Save also the primary integrated vessel in this variable to allow us to not change too much the code in 'SelProPagePressMaint' for example.
						pclSelectedPMFill->SetVesselIDPtr( pclVessel->GetIDPtr() );

						// In this case, the 'SD_CompressoVessel' sheet doesn't exist. But hopefully the 'CSelectedVssl' has been created to allow
						// us to retrieve some data as the existence of secondary vessel or not.
						CSelectedVssl *pclSelectedVessel = dynamic_cast<CSelectedVssl *>( m_pclIndSelPMParams->m_pclSelectPMList->GetFirstProduct( CSelectPMList::PT_CompressoVessel ) );

						if( NULL != pclSelectedVessel && pclSelectedVessel->GetNbreOfVsslNeeded() > 1 )
						{
							// Secondary vessel ?
							CDB_Vessel *pclSecondaryVessel = pclVessel->GetAssociatedSecondaryVessel();

							if( NULL != pclSecondaryVessel )
							{
								pclSelectedPMFill->SetSecondaryVesselIDPtr( pclSecondaryVessel->GetIDPtr() );
								pclSelectedPMFill->SetSecondaryVesselNumber( pclSelectedVessel->GetNbreOfVsslNeeded() - 1 );
							}
						}
					}
					if( true == bIsEditionMode )
					{
						pclSelectedPMFill->SetVesselAccessoryList( pclSelectedPMCpy->GetVesselAccessoryList() );
					}

					pclVesselAccessoryListToFill = pclSelectedPMFill->GetVesselAccessoryList();
					break;
				}

				case SD_TransferoVessel:
				{
					CDB_TecBox *pclTecBoxTransfero = NULL;

					if( false == GetTecBoxTransferoSelected( &pclTecBoxTransfero, &lRow ) || NULL == pclTecBoxTransfero )
					{
						break;
					}

					uiGroupSheetDescriptionID = SD_TecBoxTransfero;
					pclSelectedPMFill->SetSelectionType( CDS_SSelPMaint::Transfero );

					// Save the TecBox selected.
					pclSelectedPMFill->SetTecBoxCompTransfIDPtr( pclTecBoxTransfero->GetIDPtr() );

					if( true == bIsEditionMode )
					{
						pclSelectedPMFill->SetTecBoxCompTransfAccessoryList( pclSelectedPMCpy->GetTecBoxCompTransfAccessoryList() );
					}

					pclTecBoxAccessoryListToFill = pclSelectedPMFill->GetTecBoxCompTransfAccessoryList();

					// Set the correct number in regards to the user choice about redundancy.
					pclSelectedPMFill->SetTecBoxCompTransfNumber( iTecBoxNumber );

					// Search TecBox Transfero in the CSelectList to get some variables to add for the Transfero vessel.
					for( CSelectedPMBase *pclSelectedProduct = m_pclIndSelPMParams->m_pclSelectPMList->GetFirstProduct( CSelectPMList::PT_Transfero );
							NULL != pclSelectedProduct; pclSelectedProduct = m_pclIndSelPMParams->m_pclSelectPMList->GetNextProduct( CSelectPMList::PT_Transfero ) )
					{
						CSelectedTransfero *pclSelectedTecBoxTransfero = dynamic_cast<CSelectedTransfero *>( pclSelectedProduct );

						if( NULL == pclSelectedTecBoxTransfero )
						{
							continue;
						}

						if( pclSelectedTecBoxTransfero->GetpData() == pclTecBoxTransfero && true == pclSelectedTecBoxTransfero->IsBufferVesselExist() )
						{
							// Save buffer vessel if exist.
							CSheetDescription *pclSheetDescription = m_ViewDescription.GetFromSheetDescriptionID( SD_TecBoxTransfero );

							if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() )
							{
								break;
							}

							CSSheet *pclSSheet = pclSheetDescription->GetSSheetPointer();
							CString strBufferVesselName;

							if( SS_TYPE_COMBOBOX == pclSSheet->GetCellTypeW( CD_TecBox_BufferVessel, lRow ) )
							{
								TCHAR tcBufferVesselName[256];
								int iCurrentSelection = pclSSheet->ComboBoxSendMessage( CD_TecBox_BufferVessel, lRow, SS_CBM_GETCURSEL, 0, 0 );
								pclSSheet->ComboBoxSendMessage( CD_TecBox_BufferVessel, lRow, SS_CBM_GETLBTEXT, iCurrentSelection, (LPARAM)tcBufferVesselName );
								strBufferVesselName = CString( tcBufferVesselName );
							}
							else
							{
								strBufferVesselName = pclSSheet->GetCellText( CD_TecBox_BufferVessel, lRow );
							}
						
							pclSelectedPMFill->SetTecBoxIntegratedVesselIDPtr( pclSelectedTecBoxTransfero->GetBufferVesselIDPtr( strBufferVesselName ) );

							// HYS-872: Get accessories needed for integrated vessel.
							if( true == bIsEditionMode )
							{
								pclSelectedPMFill->SetTecBoxIntegratedVesselAccessoryList( pclSelectedPMCpy->GetTecBoxIntegratedVesselAccessoryList() );
							}
							
							pclBufferVesselAccessoryListToFill = pclSelectedPMFill->GetTecBoxIntegratedVesselAccessoryList();
							break;
						}
					}

					// Save the Transfero vessel.
					pclSelectedPMFill->SetVesselIDPtr( pclVessel->GetIDPtr() );

					// Search Transfero vessel in the CSelectList to get some variables to add for the Transfero vessel.
					for( CSelectedPMBase *pclSelectedProduct = m_pclIndSelPMParams->m_pclSelectPMList->GetFirstProduct( CSelectPMList::PT_TransferoVessel );
							NULL != pclSelectedProduct; pclSelectedProduct = m_pclIndSelPMParams->m_pclSelectPMList->GetNextProduct( CSelectPMList::PT_TransferoVessel ) )
					{
						CSelectedVssl *pclSelectedTransferoVessel = dynamic_cast<CSelectedVssl *>( pclSelectedProduct );

						if( NULL == pclSelectedTransferoVessel )
						{
							continue;
						}

						if( pclSelectedTransferoVessel->GetpData() == pclVessel )
						{
							// Remark: in case of Compresso and Transfero vessel, we have always one primary vessel and 0, 1 or n secondary vessels.
							pclSelectedPMFill->SetVesselInitialPressure( pclSelectedTransferoVessel->GetInitialPressure() );
							pclSelectedPMFill->SetVesselWaterReserve( pclSelectedTransferoVessel->GetWaterReserve() );
							pclSelectedPMFill->SetVesselNumber( 1 );

							// Secondary vessel ?
							if( pclSelectedTransferoVessel->GetNbreOfVsslNeeded() > 1 )
							{
								CDB_Vessel *pclSecondaryVessel = pclVessel->GetAssociatedSecondaryVessel();

								if( NULL != pclSecondaryVessel )
								{
									pclSelectedPMFill->SetSecondaryVesselIDPtr( pclSecondaryVessel->GetIDPtr() );
									pclSelectedPMFill->SetSecondaryVesselNumber( pclSelectedTransferoVessel->GetNbreOfVsslNeeded() - 1 );
								}
							}

							break;
						}
					}
					
					if( true == bIsEditionMode )
					{
						pclSelectedPMFill->SetVesselAccessoryList( pclSelectedPMCpy->GetVesselAccessoryList() );
					}

					pclVesselAccessoryListToFill = pclSelectedPMFill->GetVesselAccessoryList();
					break;
				}
			}

			// Check vessel accessories.
			if( NULL != pclVesselAccessoryListToFill )
			{
				if( NULL != _GetvecCDAccessoryList( uiVesselSheetDescriptionID ) && _GetvecCDAccessoryList( uiVesselSheetDescriptionID )->size() > 0 )
				{
					if( false == bIsEditionMode )
					{
						// Retrieve selected accessory and add it.
						for( auto &iter : *_GetvecCDAccessoryList( uiVesselSheetDescriptionID ) )
						{
							if( NULL != iter && true == iter->GetCheckStatus() && NULL != iter->GetAccessoryPointer() )
							{
								pclVesselAccessoryListToFill->Add( iter->GetAccessoryPointer()->GetIDPtr(), CAccessoryList::_AT_Accessory, iter->GetRuledTable() );
							}
						}
					}
					else
					{
						UpdateAccessoryList( *_GetvecCDAccessoryList( uiVesselSheetDescriptionID ), pclVesselAccessoryListToFill, CAccessoryList::_AT_Accessory );
					}
				}
			}

			// Check TecBox Compresso/Transfero accessories.
			if( NULL != pclTecBoxAccessoryListToFill && NULL != _GetvecCDAccessoryList( uiGroupSheetDescriptionID ) 
					&& _GetvecCDAccessoryList( uiGroupSheetDescriptionID )->size() > 0 )
			{
				if( false == bIsEditionMode )
				{
					// Retrieve selected accessory and add it.
					for( auto &iter : *_GetvecCDAccessoryList( uiGroupSheetDescriptionID ) )
					{
						if( NULL != iter && true == iter->GetCheckStatus() && NULL != iter->GetAccessoryPointer() )
						{
							pclTecBoxAccessoryListToFill->Add( iter->GetAccessoryPointer()->GetIDPtr(), CAccessoryList::_AT_Accessory, iter->GetRuledTable() );
						}
					}
				}
				else
				{
					UpdateAccessoryList( *_GetvecCDAccessoryList( uiGroupSheetDescriptionID ), pclTecBoxAccessoryListToFill, CAccessoryList::_AT_Accessory );
				}
			}

			// Check Transfero buffer vessel accessories.
			if( NULL != pclBufferVesselAccessoryListToFill && NULL != _GetvecCDAccessoryList( SD_TecBoxTransferoBufferVesselAccessory ) 
					&& _GetvecCDAccessoryList( SD_TecBoxTransferoBufferVesselAccessory )->size() > 0 )
			{
				if( false == bIsEditionMode )
				{
					// Retrieve selected accessory and add it.
					for( auto &iter : *_GetvecCDAccessoryList( SD_TecBoxTransferoBufferVesselAccessory ) )
					{
						if( NULL != iter && true == iter->GetCheckStatus() && NULL != iter->GetAccessoryPointer() )
						{
							pclBufferVesselAccessoryListToFill->Add( iter->GetAccessoryPointer()->GetIDPtr(), CAccessoryList::_AT_Accessory, iter->GetRuledTable() );
						}
					}
				}
				else
				{
					UpdateAccessoryList( *_GetvecCDAccessoryList( SD_TecBoxTransferoBufferVesselAccessory ), pclBufferVesselAccessoryListToFill, CAccessoryList::_AT_Accessory );
				}
			}

			// Check if there is an intermediate vessel selected.
			CDB_Vessel *pclIntermediateVessel = NULL;

			if( true == GetIntermediateVesselSelected( &pclIntermediateVessel ) && NULL != pclIntermediateVessel )
			{
				pclSelectedPMFill->SetIntermediateVesselIDPtr( pclIntermediateVessel->GetIDPtr() );

				// Search product in the CSelectList to get some variables to add for the intermediate vessel.
				for( CSelectedPMBase *pclSelectedProduct = m_pclIndSelPMParams->m_pclSelectPMList->GetFirstProduct( CSelectPMList::PT_IntermediateVessel );
						NULL != pclSelectedProduct; pclSelectedProduct = m_pclIndSelPMParams->m_pclSelectPMList->GetNextProduct( CSelectPMList::PT_IntermediateVessel ) )
				{
					CSelectedVssl *pclSelectedVsl = dynamic_cast<CSelectedVssl *>( pclSelectedProduct );

					if( NULL == pclSelectedVsl )
					{
						continue;
					}

					if( pclSelectedVsl->GetpData() == pclIntermediateVessel )
					{
						pclSelectedPMFill->SetIntermediateVesselNumber( pclSelectedVsl->GetNbreOfVsslNeeded() );
						break;
					}
				}

				// Check if accessories to this selected intermediate vessel.
				if( true == bIsEditionMode )
				{
					pclSelectedPMFill->SetIntermediateVesselAccessoryList( pclSelectedPMCpy->GetIntermediateVesselAccessoryList() );
				}
				pclVesselAccessoryListToFill = pclSelectedPMFill->GetIntermediateVesselAccessoryList();
				CSheetDescription *pclSDIntermediateVessel = m_ViewDescription.GetFromSheetDescriptionID( SD_IntermediateVessel );

				if( NULL != pclVesselAccessoryListToFill && NULL != pclSDIntermediateVessel && NULL != _GetvecCDAccessoryList( SD_IntermediateVessel ) 
						&& _GetvecCDAccessoryList( SD_IntermediateVessel )->size() > 0 )
				{
					if( false == bIsEditionMode )
					{
						// Retrieve selected accessory and add it.
						for( auto &iter : *_GetvecCDAccessoryList( SD_IntermediateVessel ) )
						{
							if( NULL != iter && true == iter->GetCheckStatus() && NULL != iter->GetAccessoryPointer() )
							{
								pclVesselAccessoryListToFill->Add( iter->GetAccessoryPointer()->GetIDPtr(), CAccessoryList::_AT_Accessory, iter->GetRuledTable() );
							}
						}
					}
					else
					{
						UpdateAccessoryList( *_GetvecCDAccessoryList( SD_IntermediateVessel ), pclVesselAccessoryListToFill, CAccessoryList::_AT_Accessory );
					}
				}
			}
		}
		else
		{
			pclSelectedPMFill->SetSelectionType( CDS_SSelPMaint::NoPressurization );
		}

		// Add Vento if exist.
		CDB_TBPlenoVento *pclTecBoxVento = NULL;

		if( true == GetTecBoxVentoSelected( &pclTecBoxVento, uiGroupSheetDescriptionID ) && NULL != pclTecBoxVento )
		{
			// Save the TecBox selected.
			pclSelectedPMFill->SetTecBoxVentoIDPtr( pclTecBoxVento->GetIDPtr() );

			CSheetDescription *pclSDTecBoxVento = m_ViewDescription.GetFromSheetDescriptionID( _ConvertSD2SDG( SD_TecBoxVento, uiGroupSheetDescriptionID ) );
			CCellDescriptionProduct *pclCellDescriptionProduct = FindCDProduct( -1, (LPARAM)pclTecBoxVento, pclSDTecBoxVento );
			int iVentoNumber = 0;

			if( NULL != pclCellDescriptionProduct && NULL != pclCellDescriptionProduct->GetUserParam() )
			{
				CSelectedVento *pclSelectedVento = (CSelectedVento*)pclCellDescriptionProduct->GetUserParam();

				// Remark: 'iTecBoxNumber' is 2 if user asks for full redundancy.
				iVentoNumber = pclSelectedVento->GetNumberOfVento() * iTecBoxNumber;
			}

			// Set the correct number in regards to the user choice about redundancy.
			pclSelectedPMFill->SetTecBoxVentoNumber( iVentoNumber );

			CAccessoryList *pclTecBoxVentoAccessoryListToFill = NULL;
			if( true == bIsEditionMode )
			{
				pclSelectedPMFill->SetTecBoxVentoAccessoryList( pclSelectedPMCpy->GetTecBoxVentoAccessoryList() );
			}
			pclTecBoxVentoAccessoryListToFill = pclSelectedPMFill->GetTecBoxVentoAccessoryList();

			if( NULL != pclTecBoxVentoAccessoryListToFill && NULL != pclSDTecBoxVento && NULL != _GetvecCDAccessoryList( SD_TecBoxVento ) 
					&& _GetvecCDAccessoryList( SD_TecBoxVento )->size() > 0 )
			{
				if( false == bIsEditionMode )
				{
					// Retrieve selected accessory and add it.
					for( auto &iter : *_GetvecCDAccessoryList( SD_TecBoxVento ) )
					{
						if( NULL != iter && true == iter->GetCheckStatus() && NULL != iter->GetAccessoryPointer() )
						{
							pclTecBoxVentoAccessoryListToFill->Add( iter->GetAccessoryPointer()->GetIDPtr(), CAccessoryList::_AT_Accessory, iter->GetRuledTable() );
						}
					}
				}
				else
				{
					UpdateAccessoryList( *_GetvecCDAccessoryList( SD_TecBoxVento ), pclTecBoxVentoAccessoryListToFill, CAccessoryList::_AT_Accessory );
				}
			}
		}

		// Add Pleno if exist.
		CDB_TBPlenoVento *pclTecBoxPleno = NULL;
		LPARAM lParam = 0;

		if( true == GetTecBoxPlenoSelected( &pclTecBoxPleno, uiGroupSheetDescriptionID, NULL, &lParam ) )
		{
			// Save the TecBox selected.

			if( NULL == pclTecBoxPleno && 0 != lParam )
			{
				// We are in the case where there is a water treatment combination.
				pclSelectedPMFill->SetTecBoxPlenoIDPtr( _NULL_IDPTR );

				// We save the 'CDB_Set' that contains the water treatment combination.
				CSelectedWaterTreatmentCombination *pSelectedWTCombination = (CSelectedWaterTreatmentCombination *)lParam;
				CDB_Set *pWTCombination = pSelectedWTCombination->GetWTCombination();

				if( NULL != pWTCombination )
				{
					pclSelectedPMFill->SetAdditionalTecBoxPlenoIDPtr( pWTCombination->GetIDPtr() );

					// Set the correct number in regards to the user choice about redundancy.
					if( 0 == _tcscmp( pWTCombination->GetFirstIDPtr().ID, _T("NOTHING_ID") ) 
							&& 0 == _tcscmp( pWTCombination->GetSecondIDPtr().ID, _T("NOTHING_ID") ) )
					{
						pclSelectedPMFill->SetTecBoxPlenoNumber( 0 );
					}
					else
					{
						pclSelectedPMFill->SetTecBoxPlenoNumber( iTecBoxNumber );
					}
				}
			}
			else if( NULL != pclTecBoxPleno )
			{
				pclSelectedPMFill->SetTecBoxPlenoIDPtr( pclTecBoxPleno->GetIDPtr() );

				// Set the correct number in regards to the user choice about redundancy.
				pclSelectedPMFill->SetTecBoxPlenoNumber( iTecBoxNumber );
				
				// HYS-1121 : Look if the Pleno has a water make-up protection
				CDB_TBPlenoVento *pclTecBoxPlenoProtection = NULL;
				
				if( true == GetTecBoxPlenoWMProtectionSelected( &pclTecBoxPlenoProtection, uiGroupSheetDescriptionID, NULL, &lParam ) )
				{
					if( NULL != pclTecBoxPlenoProtection )
					{
						pclSelectedPMFill->SetTecBoxPlenoProtectionIDPtr( pclTecBoxPlenoProtection->GetIDPtr() );
						CAccessoryList *pclTBPlenoProtectionAccessoryListToFill = NULL;
					
						if( true == bIsEditionMode )
						{
							pclSelectedPMFill->SetTecBoxPlenoProtecAccessoryList( pclSelectedPMCpy->GetTecBoxPlenoProtecAccessoryList() );
						}
						
						pclTBPlenoProtectionAccessoryListToFill = pclSelectedPMFill->GetTecBoxPlenoProtecAccessoryList();

						CSheetDescription *pclSDTecBoxPleno = m_ViewDescription.GetFromSheetDescriptionID( _ConvertSD2SDG( SD_PlenoWaterMakeUpProtection, uiGroupSheetDescriptionID ) );

						if( NULL != pclTBPlenoProtectionAccessoryListToFill && NULL != pclSDTecBoxPleno && NULL != _GetvecCDAccessoryList( SD_PlenoWaterMakeUpProtection ) 
								&& _GetvecCDAccessoryList( SD_PlenoWaterMakeUpProtection )->size() > 0 )
						{
							if( false == bIsEditionMode )
							{
								// Retrieve selected accessory and add it.
								for( auto &iter : *_GetvecCDAccessoryList( SD_PlenoWaterMakeUpProtection ) )
								{
									if( NULL != iter && true == iter->GetCheckStatus() && NULL != iter->GetAccessoryPointer() )
									{
										pclTBPlenoProtectionAccessoryListToFill->Add( iter->GetAccessoryPointer()->GetIDPtr(), CAccessoryList::_AT_Accessory, iter->GetRuledTable() );
									}
								}
							}
							else
							{
								UpdateAccessoryList( *_GetvecCDAccessoryList( SD_PlenoWaterMakeUpProtection ), pclTBPlenoProtectionAccessoryListToFill, CAccessoryList::_AT_Accessory );
							}
						}
					}
				}
			}

			CAccessoryList *pclTecBoxPlenoAccessoryListToFill = NULL;
			if( true == bIsEditionMode )
			{
				pclSelectedPMFill->SetTecBoxPlenoAccessoryList( pclSelectedPMCpy->GetTecBoxPlenoAccessoryList() );
			}
			pclTecBoxPlenoAccessoryListToFill = pclSelectedPMFill->GetTecBoxPlenoAccessoryList();
		
			CSheetDescription *pclSDTecBoxPleno = m_ViewDescription.GetFromSheetDescriptionID( _ConvertSD2SDG( SD_TecBoxPleno, uiGroupSheetDescriptionID ) );

			if( NULL != pclTecBoxPlenoAccessoryListToFill && NULL != pclSDTecBoxPleno && NULL != _GetvecCDAccessoryList( SD_TecBoxPleno )
					&& _GetvecCDAccessoryList( SD_TecBoxPleno )->size() > 0 )
			{
				if( false == bIsEditionMode )
				{
					// Retrieve selected accessory and add it.
					for( auto &iter : *_GetvecCDAccessoryList( SD_TecBoxPleno ) )
					{
						if( NULL != iter && true == iter->GetCheckStatus() && NULL != iter->GetAccessoryPointer() )
						{
							pclTecBoxPlenoAccessoryListToFill->Add( iter->GetAccessoryPointer()->GetIDPtr(), CAccessoryList::_AT_Accessory, iter->GetRuledTable() );
						}
					}
				}
				else
				{
					UpdateAccessoryList( *_GetvecCDAccessoryList( SD_TecBoxPleno ), pclTecBoxPlenoAccessoryListToFill, CAccessoryList::_AT_Accessory );
				}
			}
		}

		// Add Pleno Refill if exist.
		CDB_PlenoRefill *pclPlenoRefill = NULL;

		if( true == GetPlenoRefillSelected( &pclPlenoRefill, uiGroupSheetDescriptionID ) && NULL != pclPlenoRefill )
		{
			// Save the Pleno Refill selected.
			pclSelectedPMFill->SetPlenoRefillIDPtr( pclPlenoRefill->GetIDPtr() );

			// Set the correct number in regards to the user choice about redundancy.
			pclSelectedPMFill->SetPlenoRefillNumber( iTecBoxNumber );

			CAccessoryList *pclPlenoRefillAccessoryListToFill = NULL; 
			
			if( true == bIsEditionMode )
			{
				pclSelectedPMFill->SetPlenoRefillAccessoryList( pclSelectedPMCpy->GetPlenoRefillAccessoryList() );
			}
			
			pclPlenoRefillAccessoryListToFill = pclSelectedPMFill->GetPlenoRefillAccessoryList();
			CSheetDescription *pclSDPlenoRefill = m_ViewDescription.GetFromSheetDescriptionID( _ConvertSD2SDG( SD_PlenoRefill, uiGroupSheetDescriptionID ) );

			if( NULL != pclPlenoRefillAccessoryListToFill && NULL != pclSDPlenoRefill && NULL != _GetvecCDAccessoryList( SD_PlenoRefill ) 
					&& _GetvecCDAccessoryList( SD_PlenoRefill )->size() > 0 )
			{
				if( false == bIsEditionMode )
				{
					// Retrieve selected accessory and add it.
					for( auto &iter : *_GetvecCDAccessoryList( SD_PlenoRefill ) )
					{
						if( NULL != iter && true == iter->GetCheckStatus() && NULL != iter->GetAccessoryPointer() )
						{
							pclPlenoRefillAccessoryListToFill->Add( iter->GetAccessoryPointer()->GetIDPtr(), CAccessoryList::_AT_Accessory, iter->GetRuledTable() );
						}
					}
				}
				else
				{
					UpdateAccessoryList( *_GetvecCDAccessoryList( SD_PlenoRefill ), pclPlenoRefillAccessoryListToFill, CAccessoryList::_AT_Accessory );
				}
			}
		}

		if( NULL != pclSelectedPMCpy )
		{
			m_pclIndSelPMParams->m_pTADS->DeleteObject( IDPtr );
		}
	}
	catch( CHySelectException &clHySelectException )
	{
		clHySelectException.AddMessage( _T("Error in 'CRViewSSelPM::FillInSelected'."), __LINE__, __FILE__ );
		throw;
	}
}

void CRViewSSelPM::OnNewDocument( CDS_IndSelParameter *pclIndSelParameter )
{
	_ReadAllColumnWidth( pclIndSelParameter );
}

void CRViewSSelPM::SaveSelectionParameters( CDS_IndSelParameter *pclIndSelParameter )
{
	_WriteAllColumnWidth( pclIndSelParameter );
}

void CRViewSSelPM::InitHeadersHelper( CSheetDescription *pclSheetDescription, CSelectPMList::ProductType eProductType )
{
	if( NULL == m_pclIndSelPMParams || NULL == m_pclIndSelPMParams->m_pclSelectPMList || NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERT_RETURN;
	}

	switch( eProductType )
	{
		case CSelectPMList::ProductType::PT_ExpansionVessel:
			_InitExpansionVesselHeaders( SD_ExpansionVessel, pclSheetDescription );
			break;

		case CSelectPMList::ProductType::PT_ExpansionVesselMembrane:
			_InitExpansionVesselHeaders( SD_ExpansionVesselMembrane, pclSheetDescription );
			break;

		case CSelectPMList::ProductType::PT_IntermediateVessel:
			_InitIntermediateVsslHeaders( pclSheetDescription );
			break;

		case CSelectPMList::ProductType::PT_Compresso:
			_InitTecBoxCompressoHeaders( pclSheetDescription );
			break;

		case CSelectPMList::ProductType::PT_CompressoVessel:
			_InitCompressoVsslHeaders( pclSheetDescription, m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser() );
			break;

		case CSelectPMList::ProductType::PT_Transfero:
			_InitTecBoxTransferoHeaders( pclSheetDescription );
			break;

		case CSelectPMList::ProductType::PT_TransferoVessel:
			_InitTransferoVsslHeaders( pclSheetDescription );
			break;

		case CSelectPMList::ProductType::PT_Vento:
			_InitTecBoxVentoHeaders( pclSheetDescription );
			break;

		case CSelectPMList::ProductType::PT_Pleno:
			_InitTecBoxPlenoHeaders( pclSheetDescription );
			break;

		case CSelectPMList::ProductType::PT_Pleno_Protec:
			_InitTecBoxPlenoHeaders( pclSheetDescription );
			break;

		case CSelectPMList::ProductType::PT_PlenoRefill:
			_InitPlenoRefillHeaders( pclSheetDescription );
			break;
	}
}

long CRViewSSelPM::FillRowHelper( CSheetDescription *pclSheetDescription, CSelectPMList::ProductType eProductType, long lRow, CSelectedPMBase *pclSelectedPM, 
		CPMInputUser *pclPMInputUser )
{
	long lReturn = -1;

	if( CSelectPMList::ProductType::PT_ExpansionVessel == eProductType )
	{
		CSelectedVssl *pclSelectedExpansionVessel = dynamic_cast<CSelectedVssl*>( pclSelectedPM );

		if( NULL != pclSelectedExpansionVessel )
		{
			lReturn = _FillExpansionVesselRow( pclSheetDescription, lRow, pclSelectedExpansionVessel, pclPMInputUser, NULL, NULL, true );
		}
	}
	else if( CSelectPMList::ProductType::PT_ExpansionVesselMembrane == eProductType )
	{
		CSelectedVssl *pclSelectedExpansionMembraneVessel = dynamic_cast<CSelectedVssl*>( pclSelectedPM );

		if( NULL != pclSelectedExpansionMembraneVessel )
		{
			lReturn = _FillExpansionVesselRow( pclSheetDescription, lRow, pclSelectedExpansionMembraneVessel, pclPMInputUser, NULL, NULL, true );
		}
	}
	else if( CSelectPMList::ProductType::PT_IntermediateVessel == eProductType )
	{
		CSelectedVssl *pclSelectedIntermVssl = dynamic_cast<CSelectedVssl*>( pclSelectedPM );

		if( NULL != pclSelectedIntermVssl )
		{
			lReturn = _FillIntermediateVsslRow( pclSheetDescription, lRow, pclSelectedIntermVssl, pclPMInputUser, true );
		}
	}
	else if( CSelectPMList::ProductType::PT_Compresso == eProductType )
	{
		CSelectedCompresso *pclSelectedTecBoxCompresso = dynamic_cast<CSelectedCompresso*>( pclSelectedPM );

		if( NULL != pclSelectedTecBoxCompresso )
		{
			lReturn = _FillTecBoxCompressoRow( pclSheetDescription, lRow, pclSelectedTecBoxCompresso, pclPMInputUser, false, NULL, NULL, true );
		}
	}
	else if( CSelectPMList::ProductType::PT_CompressoVessel == eProductType )
	{
		CSelectedVssl *pclSelCompressoVssl = dynamic_cast<CSelectedVssl*>( pclSelectedPM );

		if( NULL != pclSelCompressoVssl )
		{
			lReturn = _FillCompressoVsslRow( pclSheetDescription, lRow, pclSelCompressoVssl, pclPMInputUser, NULL, true );
		}
	}
	else if( CSelectPMList::ProductType::PT_Transfero == eProductType )
	{
		CSelectedTransfero *pclSelectedTecBoxTransfero = dynamic_cast<CSelectedTransfero*>( pclSelectedPM );

		if( NULL != pclSelectedTecBoxTransfero )
		{
			// HYS-868: don't show combo on excluded products list
			std::map< int, CSelectedTransfero::BufferVesselData > *p_mapTemp = pclSelectedTecBoxTransfero->GetBufferVesselList();
			if (p_mapTemp->size() > 1)
			{
				std::map< int, CSelectedTransfero::BufferVesselData >::iterator iter = p_mapTemp->begin();
				iter++;
				p_mapTemp->erase(iter, p_mapTemp->end());
			}
			lReturn = _FillTecBoxTransferoRow( pclSheetDescription, lRow, pclSelectedTecBoxTransfero, pclPMInputUser, NULL, NULL, true );
		}
	}
	else if( CSelectPMList::ProductType::PT_TransferoVessel == eProductType )
	{
		CSelectedVssl *pclSelTransferoVssl = dynamic_cast<CSelectedVssl*>( pclSelectedPM );

		if( NULL != pclSelTransferoVssl )
		{
			lReturn = _FillTransferoVsslRow( pclSheetDescription, lRow, pclSelTransferoVssl, pclPMInputUser, NULL, true );
		}
	}
	else if( CSelectPMList::ProductType::PT_Vento == eProductType )
	{
		CSelectedVento *pclSelectedTBVento = dynamic_cast<CSelectedVento*>( pclSelectedPM );

		if( NULL != pclSelectedTBVento )
		{
			lReturn = _FillTecBoxVentoRow( pclSheetDescription, lRow, pclSelectedTBVento, pclPMInputUser, NULL, NULL, true );
		}
	}
	else if( CSelectPMList::ProductType::PT_Pleno == eProductType )
	{
		lReturn = _FillTecBoxPlenoRow( pclSheetDescription, lRow, pclSelectedPM, pclPMInputUser, NULL, NULL, true );
	}
	else if( CSelectPMList::ProductType::PT_Pleno_Protec == eProductType )
	{
		lReturn = _FillTecBoxPlenoRow( pclSheetDescription, lRow, pclSelectedPM, pclPMInputUser, NULL, NULL, true );
	}
	else if( CSelectPMList::ProductType::PT_PlenoRefill == eProductType )
	{
		lReturn = _FillPlenoRefillRow( pclSheetDescription, lRow, pclSelectedPM, pclPMInputUser, true );
	}

	return lReturn;
}

// HYS-872: to save the combo selection
BEGIN_MESSAGE_MAP( CRViewSSelPM, CRViewSSelSS )

	ON_MESSAGE( SSM_COMBOSELCHANGE, OnComboSelChange )

END_MESSAGE_MAP()

LRESULT CRViewSSelPM::OnComboSelChange( WPARAM wParam, LPARAM lParam )
{
	CDB_Vessel *pSelectedBufferVessel = NULL;
	SS_CELLCOORD* pCellCoord = (SS_CELLCOORD*)lParam;
	CSelectedTransfero *pCurrentTransferoTecbox = NULL;

	CSheetDescription *pclSheetDescription = m_ViewDescription.GetFromSheetDescriptionID( _ConvertSD2SDG( SD_TecBoxTransfero, -1 ) );
	
	if( NULL != pclSheetDescription )
	{
		CSSheet *pclSSheet = pclSheetDescription->GetSSheetPointer();
		CCellDescriptionProduct *pclCDProduct = NULL;
		GetCDProduct( pCellCoord->Col, pCellCoord->Row, pclSheetDescription, &pclCDProduct );
		
		if( NULL != pclCDProduct )
		{
			pCurrentTransferoTecbox = (CSelectedTransfero *)pclCDProduct->GetUserParam();
			
			if( NULL != pCurrentTransferoTecbox )
			{
				// Get the buffer vessel list.
				std::map<int, CSelectedTransfero::BufferVesselData> *pMapBufferVesselList = pCurrentTransferoTecbox->GetBufferVesselList();
				ASSERT( NULL != pMapBufferVesselList );

				int index = pclSSheet->ComboBoxSendMessage( pCellCoord->Col, pCellCoord->Row, SS_CBM_GETCURSEL, 0, 0 );
				ASSERT( index < (int)pMapBufferVesselList->size() );

				pSelectedBufferVessel = pMapBufferVesselList->at( index ).m_pclVessel;
				ASSERT( NULL != pSelectedBufferVessel );
				
				// Save the combo selection.
				pclCDProduct->SetSecondUserParam( (LPARAM)pSelectedBufferVessel );
				UINT uiSDGroupID = pclSheetDescription->GetGroupSheetDescriptionID();
				UINT uiSheetParentID = -1;
				
				if( m_ViewDescription.IsSheetDescriptionExist( SD_TecBoxTransferoAccessory ) )
				{
					uiSheetParentID = SD_TecBoxTransferoAccessory;
				}
				else
				{
					uiSheetParentID = SD_TecBoxTransfero;
				}
				
				if( NULL != m_arlpSelectedUserParam[SD_TecBoxTransfero] )
				{
					// To update accessory sheet.
					EnableSSheetDrawing( FALSE );
					OnClickProduct( pclSheetDescription, pclCDProduct, pclSheetDescription->GetActiveColumn(), pclCDProduct->GetCellPosition().y );
					
					// To check again the selection.
					OnCellClicked( pclSheetDescription, pclSheetDescription->GetFirstParameterColumn(), pCellCoord->Row );
					EnableSSheetDrawing( TRUE );
				}
			}
		}
	}

	return 0;
}

bool CRViewSSelPM::OnTextTipFetch( CSheetDescription *pclSheetDescription, long lColumn, long lRow, WORD *pwMultiLine, SHORT *pnTipWidth, TCHAR *pstrTipText,
		BOOL *pbShowTip )
{
	if( NULL == m_pclIndSelPMParams || NULL == m_pclIndSelPMParams->m_pclSelectPMList || NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser() 
			|| false == m_bInitialised || NULL == pclSheetDescription )
	{
		ASSERTA_RETURN( false );
	}

	CPMInputUser *pclInputUser = m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser();

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

	CCellDescriptionProduct *pclCDProduct = NULL;
	CDB_Product *pclPMProduct = dynamic_cast<CDB_Product *>( ( CData * )GetCDProduct( lColumn, lRow, pclSheetDescription, &pclCDProduct ) );

	CSelectedPMBase *pclSelectedPM = NULL;

	if( NULL != pclCDProduct )
	{
		pclSelectedPM = ( CSelectedPMBase * )pclCDProduct->GetUserParam();
	}

	CSSheet *pclSSheet = pclSheetDescription->GetSSheetPointer();
	CDS_TechnicalParameter *pclTechParam = m_pclIndSelPMParams->m_pTADS->GetpTechParams();
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();

	UINT uiSheetDescriptionID = GETWITHOUTGROUPID( pclSheetDescription->GetSheetDescriptionID() );

	switch( uiSheetDescriptionID )
	{
		case SD_ComputedData:
			
			if( RD_ComputedData_MinimumPSV == lRow )
			{
				if( CD_ComputedData_Compresso == lColumn && _RED == pclSSheet->GetForeColor( lColumn, lRow ) )
				{
					// The safety valve response pressure input (%2) is too low for this selection (%1).
					CString strTemp1 = WriteCUDouble( _U_PRESSURE, pclInputUser->GetMinimumRequiredPSVRounded( CPMInputUser::MT_WithCompressor ), true );
					CString strTemp2 = WriteCUDouble( _U_PRESSURE, pclInputUser->GetSafetyValveResponsePressure(), true );
					FormatString( str, IDS_RVIEWSSELPM_TTPSVTOOLOW, strTemp1, strTemp2 );
				}
				
				if( CD_ComputedData_Transfero == lColumn && _RED == pclSSheet->GetForeColor( lColumn, lRow ) )
				{
					// The safety valve response pressure input (%2) is too low for this selection (%1).
					CString strTemp1 = WriteCUDouble( _U_PRESSURE, pclInputUser->GetMinimumRequiredPSVRounded( CPMInputUser::MT_WithPump ), true );
					CString strTemp2 = WriteCUDouble( _U_PRESSURE, pclInputUser->GetSafetyValveResponsePressure(), true );
					FormatString( str, IDS_RVIEWSSELPM_TTPSVTOOLOW, strTemp1, strTemp2 );
				}
				
				if ( (CD_ComputedData_ExpansionVessel == lColumn ) && (_RED == pclSSheet->GetForeColor(lColumn, lRow)) )
				{
					// The safety valve response pressure input (%2) is too low for this selection (%1).
					CString strTemp1 = WriteCUDouble(_U_PRESSURE, pclInputUser->GetMinimumRequiredPSVRounded( CPMInputUser::MT_ExpansionVessel ), true);
					CString strTemp2 = WriteCUDouble(_U_PRESSURE, pclInputUser->GetSafetyValveResponsePressure(), true);
					FormatString(str, IDS_RVIEWSSELPM_TTPSVTOOLOW, strTemp1, strTemp2);
				}
			}
			break;

		case SD_ExpansionVessel:
		case SD_ExpansionVesselMembrane:
			
			if( NULL != pclSelectedPM && NULL != dynamic_cast<CDB_Vessel *>( pclPMProduct ) )
			{
				if( true == pclSelectedPM->IsFlagSet( CSelectedPMBase::eVslFlags3000lbar ) )
				{
					// This solution does not respect the 'pressure.volume' limit rule fixed by the SWKI/SICC/SITC HE301-01 norm.\r\nMake sure you know what your are doing
					// if you decide to choose this vessel anyway.
					str = TASApp.LoadLocalizedString( IDS_RVIEWSSELPM_TTNOTRESPECTPRESSUREVOLUMELIMIT );
				}
				else 
				{
					if( CD_ExpansionVessel_NbreOfVssl == lColumn && true == pclSelectedPM->IsFlagSet( CSelectedPMBase::eVslFlagsNbreOfVssl ) &&
							NULL != dynamic_cast<CSelectedVssl *>( pclSelectedPM ) )
					{
						// Total number of additional vessels in parallel is bigger than the\r\nthe max. number of additional vessels in parallel set in the technical parameters (%1).
						CSelectedVssl *pclSelectedVessel = dynamic_cast<CSelectedVssl *>( pclSelectedPM );
						CString strHelp;
						strHelp.Format( _T("%i > %i"), pclSelectedVessel->GetNbreOfVsslNeeded() - 1, pclTechParam->GetMaxNumberOfAdditionalVesselsInParallel() );
						FormatString( str, IDS_RVIEWSSELPM_TTMAXADDITIONALVESSELSINPARALLEL, strHelp );
					}
					
					if( CD_ExpansionVessel_TempRange == lColumn && true == pclSelectedPM->IsFlagSet( CSelectedPMBase::eTemperature ) )
					{
						str = _OnTextTipFetchTemperatureHelper( pclSelectedPM, pclPMProduct, pclInputUser );
					}
				}
			}

			if( CD_ExpansionVessel_RequiredNominalVolumeEN12828InfoIcon == lColumn )
			{
				if( RD_ExpansionVessel_ColName == lRow )
				{
					str = TASApp.LoadLocalizedString( IDS_RVIEWSSELPM_TTREQUIREDNOMINALVOLUMEEN12828 );
				}
				else if( NULL != dynamic_cast<CSelectedVssl *>( pclSelectedPM ) )
				{
					CSelectedVssl *pclSelectedVessel = (CSelectedVssl *)( pclSelectedPM );
					CDB_Vessel *pclVessel = pclSelectedVessel->GetVessel();

					if( pclSelectedVessel->GetNbreOfVsslNeeded() > pclSelectedVessel->GetInitialNbreOfVsslNeeded() )
					{
						// In this case, we increase the number of vessel and we need to explain.

						// The max. available water reserve volume for this vessel is %1 (Initially the vessel number in parallel was %2).
						int iInitialTotalVessel = pclSelectedVessel->GetInitialNbreOfVsslNeeded();
						CString strInitialTotalVessel;
						strInitialTotalVessel.Format( _T("%i"), iInitialTotalVessel );

						CString strTemp;
						double dVwr = pclInputUser->GetWaterReserve( iInitialTotalVessel * pclVessel->GetNominalVolume() );
						FormatString( str, IDS_RVIEWSSELPM_TTREQUIREDNOMINALVOLUMEEN12828_TIP1, WriteCUDouble( _U_VOLUME, dVwr, true ), strInitialTotalVessel );
						str += _T("\r\n");

						// The pressure to push this volume is %1 and does not satisfy the EN12828 rule: pa >= p0 + 0.3.
						double dPamin = pclInputUser->ComputeInitialPressure( iInitialTotalVessel * pclVessel->GetNominalVolume(), dVwr );
						FormatString( strTemp, IDS_RVIEWSSELPM_TTREQUIREDNOMINALVOLUMEEN12828_TIP2, WriteCUDouble( _U_PRESSURE, dPamin, true ) );
						str += strTemp + _T("\r\n");

						// At the pressure of %1 (%2 + %3) the total water volume pushed becomes %4.
						CString strPaminEN = WriteCUDouble( _U_PRESSURE, pclInputUser->GetMinimumInitialPressure(), true );
						CString strP0 = WriteCUDouble( _U_PRESSURE, pclInputUser->GetMinimumPressure() );
						CString strInitialPressureMargin = WriteCUDouble( _U_PRESSURE, pclTechParam->GetInitialPressureMargin() );
						double dVwrPushed = pclInputUser->GetWaterReservePushed( pclSelectedVessel->GetNbreOfVsslNeeded() * pclVessel->GetNominalVolume(), pclInputUser->GetMinimumInitialPressure() );
						CString strVwrPushed = WriteCUDouble( _U_VOLUME, dVwrPushed, true );
						FormatString( strTemp, IDS_RVIEWSSELPM_TTREQUIREDNOMINALVOLUMEEN12828_TIP3, strPaminEN, strP0, strInitialPressureMargin, strVwrPushed );
						str += strTemp + _T("\r\n");

						// And the new required nominal volume becomes %1. This is the reason why we need to increase the number of vessels to %2.
						double dRequiredNominalVolumeEN12828 = pclInputUser->GetNominalVolumeForPaEN12828( iInitialTotalVessel * pclVessel->GetNominalVolume() );
						CString strTotalVessel;
						strTotalVessel.Format( _T("%i"), pclSelectedVessel->GetNbreOfVsslNeeded() );
						FormatString( strTemp, IDS_RVIEWSSELPM_TTREQUIREDNOMINALVOLUMEEN12828_TIP4, WriteCUDouble( _U_VOLUME, dRequiredNominalVolumeEN12828, true ), strTotalVessel );
						str += strTemp;
					}
				}
			}

			// These tooltips are shown even when mouse pass over the column title.
			if( CD_ExpansionVessel_Weight == lColumn )
			{
				str = TASApp.LoadLocalizedString( IDS_RVIEWSSELPM_TTWEIGHT );
			}
			else if( CD_ExpansionVessel_MaxWeight == lColumn )
			{
				str = TASApp.LoadLocalizedString( IDS_RVIEWSSELPM_TTMAXWEIGHT );
			}

			if( true == str.IsEmpty() && NULL != dynamic_cast<CDB_Vessel *>( pclPMProduct ) )
			{
				CDB_Vessel *pclVessel = dynamic_cast<CDB_Vessel *>( pclPMProduct );

				if( 0 != _tcslen( pclVessel->GetComment() ) )
				{
					str = pclVessel->GetComment();
				}
			}

			break;

		case SD_TecBoxCompresso:
			
			if( NULL != pclSelectedPM && NULL != dynamic_cast<CDB_TecBox *>( pclPMProduct ) )
			{
				CDB_TecBox *pclCompresso = dynamic_cast<CDB_TecBox *>( pclPMProduct );

				if( CD_TecBox_TempRange == lColumn && true == pclSelectedPM->IsFlagSet( CSelectedPMBase::eTemperature ) )
				{
					str = _OnTextTipFetchTemperatureHelper( pclSelectedPM, pclPMProduct, pclInputUser );
				}
				
				if( CD_TecBox_MaxWaterMakeUpTemp == lColumn && true == pclSelectedPM->IsFlagSet( CSelectedPMBase::ePlenoWMUpMaxTemp ) )
				{
					// Water make-up temperature is above the max. admissible temperature value for this device (%1).
					CString strTemp1 = WriteCUDouble( _U_TEMPERATURE, pclInputUser->GetWaterMakeUpWaterTemp(), true );
					CString strTemp2 = WriteCUDouble( _U_TEMPERATURE, pclCompresso->GetTmaxWaterMakeUp(), true );
					CString strHelp = strTemp1 + _T(" > ") + strTemp2;
					FormatString( str, IDS_RVIEWSSELPM_TTMAKEUPTEMPBIGGERTBWATERMAKEUP, strHelp );
				}

				if( true == str.IsEmpty() && NULL != pclCompresso )
				{
					if( 0 != _tcslen( pclCompresso->GetComment() ) )
					{
						str = pclCompresso->GetComment();
					}
				}
			}

			break;

		case SD_CompressoVessel:
			
			if( NULL != pclSelectedPM && NULL != dynamic_cast<CDB_Vessel *>( pclPMProduct ) )
			{
				if( CD_TecBoxVssl_Prim_TempRange == lColumn && true == pclSelectedPM->IsFlagSet( CSelectedPMBase::eTemperature ) )
				{
					str = _OnTextTipFetchTemperatureHelper( pclSelectedPM, pclPMProduct, pclInputUser );
				}
				
				if( CD_TecBoxVssl_Sec_NbreOfVssl == lColumn && true == pclSelectedPM->IsFlagSet( CSelectedPMBase::eVslFlagsNbreOfVssl ) &&
						 NULL != dynamic_cast<CSelectedVssl *>( pclSelectedPM ) )
				{
					// Total number of secondary vessels is bigger that the max. number of additional vessels\r\nin parallel set in the technical parameters (%1).
					CSelectedVssl *pclSelectedVessel = dynamic_cast<CSelectedVssl *>( pclSelectedPM );
					CString strHelp;
					strHelp.Format( _T("%i > %i"), pclSelectedVessel->GetNbreOfVsslNeeded() - 1, pclTechParam->GetMaxNumberOfAdditionalVesselsInParallel() );
					FormatString( str, IDS_RVIEWSSELPM_TTMAXSECONARYVESSELSINPARALLEL, strHelp );
				}
			}

			// These tooltips are shown even when mouse pass over the column title.
			if( CD_TecBoxVssl_Prim_Weight == lColumn || CD_TecBoxVssl_Sec_Weight == lColumn )
			{
				str = TASApp.LoadLocalizedString( IDS_RVIEWSSELPM_TTWEIGHT );
			}
			else if( CD_TecBoxVssl_Prim_MaxWeight == lColumn || CD_TecBoxVssl_Sec_MaxWeight == lColumn )
			{
				str = TASApp.LoadLocalizedString( IDS_RVIEWSSELPM_TTMAXWEIGHT );
			}

			if( true == str.IsEmpty() && NULL != dynamic_cast<CDB_Vessel *>( pclPMProduct ) )
			{
				CDB_Vessel *pclVessel = dynamic_cast<CDB_Vessel *>( pclPMProduct );

				if( 0 != _tcslen( pclVessel->GetComment() ) )
				{
					str = pclVessel->GetComment();
				}
			}

			break;

		case SD_TecBoxTransfero:
			
			if( NULL != pclSelectedPM && NULL != dynamic_cast<CDB_TecBox *>( pclPMProduct ) )
			{
				CDB_TecBox *pclTransfero = dynamic_cast<CDB_TecBox *>( pclPMProduct );

				if( CD_TecBox_Name == lColumn && _ORANGE == pclSSheet->GetForeColor( lColumn, lRow ) )
				{
					if( 0 == CString( pclTransfero->GetFamilyID() ).Compare( _T("TBXFAM_TRANSFERO_TV_C") ) &&
							true == pclTransfero->IsVariantHighFlowTemperature() )
					{
						CString strTemp;

						if( pclInputUser->GetMinTemperature() < pclTransfero->GetTmin() )
						{
							// When the minimum temperature (%1) is below the minimum admissible temperature (%2) the degassing option must be disabled!
							CString strTemp1 = WriteCUDouble( _U_TEMPERATURE, pclInputUser->GetMinTemperature(), true );
							CString strTemp2 = WriteCUDouble( _U_TEMPERATURE, pclTransfero->GetTmin(), true );
							FormatString( str, IDS_RVIEWSSELPM_TTMINTEMPBELOWLIMITFORDEGASSING, strTemp1, strTemp2 );
						}

						if( pclInputUser->GetReturnTemperature() > pclTransfero->GetTmax() )
						{
							// When the return temperature (%1) is above the maximum admissible\r\ntemperature (%2) the degassing option for this model must be disabled!
							CString strTemp1 = WriteCUDouble( _U_TEMPERATURE, pclInputUser->GetReturnTemperature(), true );
							CString strTemp2 = WriteCUDouble( _U_TEMPERATURE, pclTransfero->GetTmax(), true );

							FormatString( strTemp, IDS_RVIEWSSELPM_TTRETURNTEMPABOVELIMITFORDEGASSING, strTemp1, strTemp2 );

							if( true == str.IsEmpty() )
							{
								str = strTemp;
							}
							else
							{
								str += _T("\r\n") + strTemp;
							}
						}
					}
				}
				
				if( CD_TecBox_TempRange == lColumn && true == pclSelectedPM->IsFlagSet( CSelectedPMBase::eTemperature ) )
				{
					str = _OnTextTipFetchTemperatureHelper( pclSelectedPM, pclPMProduct, pclInputUser );
				}
				
				if( CD_TecBox_MaxWaterMakeUpTemp == lColumn && true == pclSelectedPM->IsFlagSet( CSelectedPMBase::ePlenoWMUpMaxTemp ) )
				{
					// Water make-up temperature is above the max. admissible temperature value for this device (%1).
					CString strTemp1 = WriteCUDouble( _U_TEMPERATURE, pclInputUser->GetWaterMakeUpWaterTemp(), true );
					CString strTemp2 = WriteCUDouble( _U_TEMPERATURE, pclTransfero->GetTmaxWaterMakeUp(), true );
					CString strHelp = strTemp1 + _T(" > ") + strTemp2;
					FormatString( str, IDS_RVIEWSSELPM_TTMAKEUPTEMPBIGGERTBWATERMAKEUP, strHelp );
				}
				
				if( ( CD_TecBox_BufferVessel == lColumn ) && ( dynamic_cast<CSelectedTransfero *>( pclSelectedPM )->GetBufferVesselList()->size() == 0) )
				{
					// HYS-1477: At this safety valve response pressure (%1) value, there is no buffer vessel available.
					CString strPSVS = WriteCUDouble( _U_PRESSURE, pclInputUser->GetSafetyValveResponsePressure(), true );
					FormatString( str, IDS_SELECTPMTBSE_XAT_TINTBUF_XPM_XVP_NOBUFVESSEL, strPSVS );
				}

				if( true == str.IsEmpty() && NULL != pclTransfero )
				{
					if( 0 != _tcslen( pclTransfero->GetComment() ) )
					{
						str = pclTransfero->GetComment();
					}
				}
			}

			break;

		case SD_TransferoVessel:
			if( NULL != pclSelectedPM && NULL != dynamic_cast<CDB_Vessel *>( pclPMProduct ) )
			{
				if( CD_TecBoxVssl_Prim_TempRange == lColumn && true == pclSelectedPM->IsFlagSet( CSelectedPMBase::eTemperature ) )
				{
					str = _OnTextTipFetchTemperatureHelper( pclSelectedPM, pclPMProduct, pclInputUser );
				}
				
				if( CD_TecBoxVssl_Sec_NbreOfVssl == lColumn && true == pclSelectedPM->IsFlagSet( CSelectedPMBase::eVslFlagsNbreOfVssl ) &&
						 NULL != dynamic_cast<CSelectedVssl *>( pclSelectedPM ) )
				{
					// Total number of secondary vessels is bigger that the max. number of additional vessels\r\nin parallel set in the technical parameters (%1).
					CSelectedVssl *pclSelectedVessel = dynamic_cast<CSelectedVssl *>( pclSelectedPM );
					CString strHelp;
					strHelp.Format( _T("%i > %i"), pclSelectedVessel->GetNbreOfVsslNeeded() - 1, pclTechParam->GetMaxNumberOfAdditionalVesselsInParallel() );
					FormatString( str, IDS_RVIEWSSELPM_TTMAXSECONARYVESSELSINPARALLEL, strHelp );
				}
			}

			// These tooltips are shown even when mouse pass over the column title.
			if( CD_TecBoxVssl_Prim_Weight == lColumn || CD_TecBoxVssl_Sec_Weight == lColumn )
			{
				str = TASApp.LoadLocalizedString( IDS_RVIEWSSELPM_TTWEIGHT );
			}
			else if( CD_TecBoxVssl_Prim_MaxWeight == lColumn || CD_TecBoxVssl_Sec_MaxWeight == lColumn )
			{
				str = TASApp.LoadLocalizedString( IDS_RVIEWSSELPM_TTMAXWEIGHT );
			}

			if( true == str.IsEmpty() && NULL != dynamic_cast<CDB_Vessel *>( pclPMProduct ) )
			{
				CDB_Vessel *pclVessel = dynamic_cast<CDB_Vessel *>( pclPMProduct );

				if( 0 != _tcslen( pclVessel->GetComment() ) )
				{
					str = pclVessel->GetComment();
				}
			}

			break;

		case SD_TecBoxVento:
			
			if( NULL != pclSelectedPM && NULL != dynamic_cast<CSelectedVento *>( pclSelectedPM ) && NULL != dynamic_cast<CDB_TBPlenoVento *>( pclPMProduct ) )
			{
				CSelectedVento *pclSelectedVento = (CSelectedVento*)pclSelectedPM;
				CDB_TBPlenoVento *pclVento = (CDB_TBPlenoVento*)pclPMProduct;

				if( CD_TecBoxVento_NbreOfDevice == lColumn )
				{
					if( true == pclSelectedPM->IsFlagSet( CSelectedPMBase::eVentoMaxParallel ) )
					{
						// Total number of Vento is bigger than max. number of Vento\r\nin parallel defined in the technical parameters (%1).
						CString strTemp1;
						strTemp1.Format( _T("%i > %i"), pclSelectedVento->GetNumberOfVento(), pclTechParam->GetMaxNumberOfVentoInParallel() );
						FormatString( str, IDS_RVIEWSSELPM_TTNBRVENTOBIGGERMAXINPARALLEL, strTemp1 );
					}
				}

				if( CD_TecBoxVento_TempRange == lColumn )
				{
					if( true == pclSelectedPM->IsFlagSet( CSelectedPMBase::eVentoMaxTempConnectPoint ) )
					{
						// Max. temperature at connection point of the degasser is above the max. temperature of this device (%1).
						CString strTemp1 = WriteCUDouble( _U_TEMPERATURE, pclInputUser->GetDegassingMaxTempConnectPoint(), true );
						CString strTemp2 = WriteCUDouble( _U_TEMPERATURE, pclVento->GetTmax(), true );
						CString strHelp = strTemp1 + _T(" > ") + strTemp2;
						FormatString( str, IDS_RVIEWSSELPM_TTDEGMAXTEMPCONNECTPTBIGGERTBMAXTEMP, strHelp );
					}
					else if( true == pclSelectedPM->IsFlagSet( CSelectedPMBase::eVentoMinTemp ) )
					{
						// Min. temperature that can be reached when the system is out of operation\r\nis below the min. admissible temperature of this device (%1).
						CString strTemp1 = WriteCUDouble( _U_TEMPERATURE, pclInputUser->GetMinTemperature(), true );
						CString strTemp2 = WriteCUDouble( _U_TEMPERATURE, pclPMProduct->GetTmin(), true );
						CString strHelp = strTemp1 + _T(" < ") + strTemp2;
						FormatString( str, IDS_RVIEWSSELPM_TTSYSMINTEMPLOWERTBMINTEMP, strHelp );
					}
				}
			}

			// These tooltips are shown even when mouse pass over the column title.
			if( CD_TecBoxVento_VNd == lColumn )
			{
				str = TASApp.LoadLocalizedString( IDS_RVIEWSSELPM_TTVND );
			}
			else if( CD_TecBoxVento_dpu == lColumn )
			{
				str = TASApp.LoadLocalizedString( IDS_RVIEWSSELPM_TTDPU );
			}
			else if( CD_TecBoxVento_qNwm == lColumn )
			{
				str = TASApp.LoadLocalizedString( IDS_RVIEWSSELPM_TTQNWM );
			}

			if( true == str.IsEmpty() && NULL != dynamic_cast<CDB_TBPlenoVento *>( pclPMProduct ) )
			{
				CDB_TBPlenoVento *pclVento = dynamic_cast<CDB_TBPlenoVento *>( pclPMProduct );

				if( 0 != _tcslen( pclVento->GetComment() ) )
				{
					str = pclVento->GetComment();
				}
			}

			break;
		case SD_PlenoWaterMakeUpProtection:
		case SD_TecBoxPleno:
			
			if( NULL != pclSelectedPM )
			{
				CSelectedWaterTreatmentCombination *pclSelectedWTCombination = dynamic_cast<CSelectedWaterTreatmentCombination*>( pclSelectedPM );
				CDB_TBPlenoVento *pclPleno = NULL;
				CSelectedPMBase *pclSelected = NULL;
				
				if( CD_TecBoxPleno_TempRange == lColumn )
				{
					if( NULL == pclSelectedWTCombination )
					{
						pclPleno = (CDB_TBPlenoVento*)pclPMProduct;
						pclSelected = pclSelectedPM;
					}
					else
					{
						if( NULL == pclSelectedWTCombination->GetWTCombination() )
						{
							break;
						}

						if( NULL != pclSelectedWTCombination->GetSelectedFirst() )
						{
							pclSelected = pclSelectedWTCombination->GetSelectedFirst();
							pclPleno = dynamic_cast<CDB_TBPlenoVento*>( (CData*)pclSelected->GetProductIDPtr().MP );
						}

						if( NULL != pclSelectedWTCombination->GetSelectedSecond() )
						{
							pclSelected = pclSelectedWTCombination->GetSelectedFirst();
							pclPleno = dynamic_cast<CDB_TBPlenoVento*>( (CData*)pclSelected->GetProductIDPtr().MP );
						}
					}

					if( NULL == pclPleno || NULL == pclSelected )
					{
						break;
					}

					if( true == pclSelected->IsFlagSet( CSelectedPMBase::ePlenoWMUpMinTemp ) )
					{
						// Water make-up temperature is below the min. admissible temperature value for this device (%1).
						CString strTemp1 = WriteCUDouble( _U_TEMPERATURE, pclInputUser->GetWaterMakeUpWaterTemp(), true );
						CString strTemp2 = WriteCUDouble( _U_TEMPERATURE, pclPleno->GetTmin(), true );
						CString strHelp = strTemp1 + _T(" < ") + strTemp2;
						FormatString( str, IDS_RVIEWSSELPM_TTMAKEUPTEMPLOWERTBWATERMAKEUP, strHelp );
					}
					else if( true == pclSelected->IsFlagSet( CSelectedPMBase::ePlenoWMUpMaxTemp ) )
					{
						// Water make-up temperature is above the max. admissible temperature value for this device (%1).
						CString strTemp1 = WriteCUDouble( _U_TEMPERATURE, pclInputUser->GetWaterMakeUpWaterTemp(), true );
						CString strTemp2 = WriteCUDouble( _U_TEMPERATURE, pclPleno->GetTmaxWaterMakeUp(), true );
						CString strHelp = strTemp1 + _T(" > ") + strTemp2;
						FormatString( str, IDS_RVIEWSSELPM_TTMAKEUPTEMPBIGGERTBWATERMAKEUP, strHelp );
					}
				}
			}

			// These tooltips are shown even when mouse pass over the column title.
			if( CD_TecBoxPleno_dpu == lColumn )
			{
				str = TASApp.LoadLocalizedString( IDS_RVIEWSSELPM_TTDPU );
			}
			else if( CD_TecBoxPleno_qNwm == lColumn )
			{
				str = TASApp.LoadLocalizedString( IDS_RVIEWSSELPM_TTQNWM );
			}

			if( true == str.IsEmpty() && NULL != dynamic_cast<CDB_TBPlenoVento *>( pclPMProduct ) )
			{
				CDB_TBPlenoVento *pclVento = dynamic_cast<CDB_TBPlenoVento *>( pclPMProduct );

				if( 0 != _tcslen( pclVento->GetComment() ) )
				{
					str = pclVento->GetComment();
				}
			}

			break;

		case SD_IntermediateVessel:
			
			if( NULL != pclSelectedPM && NULL != dynamic_cast<CDB_Vessel *>( pclPMProduct ) )
			{
				if( CD_ExpansionVessel_NbreOfVssl == lColumn && true == pclSelectedPM->IsFlagSet( CSelectedPMBase::eVslFlagsNbreOfVssl ) &&
						NULL != dynamic_cast<CSelectedVssl *>( pclSelectedPM ) )
				{
					// Total number of additional vessels in parallel is bigger than the\r\nmax. number of additional vessels in parallel set in the technical parameters (%1).
					CSelectedVssl *pclSelectedVessel = (CSelectedVssl*)pclSelectedPM;
					CString strHelp;
					strHelp.Format( _T("%i > %i"), pclSelectedVessel->GetNbreOfVsslNeeded() - 1, pclTechParam->GetMaxNumberOfAdditionalVesselsInParallel() );
					FormatString( str, IDS_RVIEWSSELPM_TTMAXADDITIONALVESSELSINPARALLEL, strHelp );
				}
				
				if( CD_IntermVssl_TempRange == lColumn && true == pclSelectedPM->IsFlagSet( CSelectedPMBase::eTemperature ) )
				{
					str = _OnTextTipFetchTemperatureHelper( pclSelectedPM, pclPMProduct, pclInputUser );
				}
			}

			// These tooltips are shown even when mouse pass over the column title.
			if( CD_IntermVssl_Weight == lColumn )
			{
				str = TASApp.LoadLocalizedString( IDS_RVIEWSSELPM_TTWEIGHT );
			}
			else if( CD_IntermVssl_MaxWeight == lColumn )
			{
				str = TASApp.LoadLocalizedString( IDS_RVIEWSSELPM_TTMAXWEIGHT );
			}

			if( true == str.IsEmpty() && NULL != dynamic_cast<CDB_Vessel *>( pclPMProduct ) )
			{
				CDB_Vessel *pclVessel = dynamic_cast<CDB_Vessel *>( pclPMProduct );

				if( 0 != _tcslen( pclVessel->GetComment() ) )
				{
					str = pclVessel->GetComment();
				}
			}

			break;

		case SD_ExpansionVesselAccessory:
		case SD_TecBoxCompressoAccessory:
		case SD_CompressoVesselAccessory:
		case SD_TecBoxTransferoAccessory:
		case SD_TecBoxTransferoBufferVesselAccessory:
		case SD_TransferoVesselAccessory:
		case SD_IntermediateVesselAcc:
			
			if( lColumn > CD_Accessory_FirstColumn && lColumn < CD_Accessory_LastColumn )
			{
				TextTipFetchEllipsesHelper( lColumn, lRow, pclSheetDescription, pnTipWidth, &str );
			}

			break;

		case SD_PlenoRefill:
			
			if( NULL != pclSelectedPM && NULL != dynamic_cast<CDB_PlenoRefill *>( pclPMProduct ) )
			{
				CDB_PlenoRefill *pclPlenoRefill = dynamic_cast<CDB_PlenoRefill *>( pclPMProduct );

				if( CD_PlenoRefill_TempRange == lColumn )
				{
					CString strTemp;

					if( true == pclSelectedPM->IsFlagSet( CSelectedPMBase::ePRefillWMUpMinTemp ) )
					{
						// Water make-up temperature is below the min. admissible temperature value for this device (%1).
						CString strTemp = WriteCUDouble( _U_TEMPERATURE, pclInputUser->GetWaterMakeUpWaterTemp(), true );
						strTemp += _T(" < ");
						strTemp += WriteCUDouble( _U_TEMPERATURE, pclPlenoRefill->GetTmin(), true );
						FormatString( str, IDS_RVIEWSSELPM_TTMAKEUPTEMPLOWERTBWATERMAKEUP, strTemp );
					}
					else if( true == pclSelectedPM->IsFlagSet( CSelectedPMBase::ePRefillWMUpMaxTemp ) )
					{
						// Water make-up temperature is above the max. admissible temperature value for this device (%1).
						CString strTemp = WriteCUDouble( _U_TEMPERATURE, pclInputUser->GetWaterMakeUpWaterTemp(), true );
						strTemp += _T(" > ");
						strTemp += WriteCUDouble( _U_TEMPERATURE, pclPlenoRefill->GetTmax(), true );
						FormatString( str, IDS_RVIEWSSELPM_TTMAKEUPTEMPBIGGERTBWATERMAKEUP, strTemp );
					}
				}
				else if( RD_TecBox_ColName == lRow )
				{
					if( CD_PlenoRefill_WidthWoConnect == lColumn )
					{
						str = TASApp.LoadLocalizedString( IDS_RVIEWSSELPM_TTWIDTHWOCONNECT );
					}
					else if( CD_PlenoRefill_WidthWConnect == lColumn )
					{
						str = TASApp.LoadLocalizedString( IDS_RVIEWSSELPM_TTWIDTHWCONNECT );
					}
				}

				if( true == str.IsEmpty() && NULL != pclPlenoRefill )
				{
					if( 0 != _tcslen( pclPlenoRefill->GetComment() ) )
					{
						str = pclPlenoRefill->GetComment();
					}
				}
			}
			break;
	}

	if( false == str.IsEmpty() )
	{
		*pnTipWidth = ( SHORT )pclSSheet->GetTipTextWidth( str );
		wcsncpy_s( pstrTipText, SS_TT_TEXTMAX, ( LPCTSTR )str, SS_TT_TEXTMAX );
		*pbShowTip = true;
		bReturnValue = true;
	}

	return bReturnValue;
}

CCellDescriptionProduct *CRViewSSelPM::FindCDProductByUserParam( long lRow, LPARAM lpUserParam, CSheetDescription *pclSheetDescription )
{
	if( 0 == lpUserParam || NULL == pclSheetDescription )
	{
		return NULL;
	}

	CDB_Set *pclSetToCompare = (CDB_Set *)lpUserParam;
	CCellDescriptionProduct *pclCDProduct = NULL;
	CCellDescription *pclLoopCellDescription = pclSheetDescription->GetFirstCellDescription( RVSCellDescription::CD_Product );

	while( NULL != pclLoopCellDescription )
	{
		// Try to retrieve cell description product.
		pclCDProduct = dynamic_cast<CCellDescriptionProduct *>( pclLoopCellDescription );
		
		if( NULL != pclCDProduct )
		{
			if( ( -1 == lRow || lRow == pclCDProduct->GetCellPosition().y ) && 0 != pclCDProduct->GetUserParam() )
			{
				CSelectedWaterTreatmentCombination *pSelectedWTCombination = (CSelectedWaterTreatmentCombination *)pclCDProduct->GetUserParam();
				CDB_Set *pclWTCombination = pSelectedWTCombination->GetWTCombination();

				if( NULL != pclWTCombination && 0 == _tcscmp( pclSetToCompare->GetIDPtr().ID, pclWTCombination->GetIDPtr().ID ) )
				{
					// Found!
					break;
				}
			}
		}

		pclLoopCellDescription = pclSheetDescription->GetNextCellDescription( pclLoopCellDescription, RVSCellDescription::CD_Product );
	}

	if( NULL == pclCDProduct )
	{
		pclCDProduct = CRViewSSelSS::FindCDProductByUserParam( lRow, lpUserParam, pclSheetDescription );
	}
	
	return pclCDProduct;
}

void CRViewSSelPM::OnCellDescriptionLinkClicked( CSheetDescription *pclSheetDescription, long lColumn, long lRow, LPARAM lpParam )
{
	if( NULL == m_pclIndSelPMParams || NULL == m_pclIndSelPMParams->m_pclSelectPMList
			|| NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() )
	{
		ASSERT( 0 );
		return;
	}

	CSelectPMList::ProductType eProductType = ( CSelectPMList::ProductType )lpParam;
	CDlgIndSelPMExcludedProducts dlg( m_pclIndSelPMParams->m_pclSelectPMList, eProductType, this );
	dlg.DoModal();
}

CDB_Product *CRViewSSelPM::RetrieveProductFromUserParam( LPARAM lpParam )
{
	if( NULL == lpParam )
	{
		ASSERTA_RETURN( NULL );
	}

	CDB_Product *pclProduct = NULL;
	CSelectedPMBase *pSelectedPM = (CSelectedPMBase *)lpParam;

	if( NULL == dynamic_cast<CSelectedWaterTreatmentCombination*>( pSelectedPM ) )
	{
		pclProduct = dynamic_cast<CDB_Product*>( (CData*)pSelectedPM->GetProductIDPtr().MP );
	}
	else
	{
		CSelectedWaterTreatmentCombination *pclWTCombination = dynamic_cast<CSelectedWaterTreatmentCombination*>( pSelectedPM );

		if( NULL != pclWTCombination->GetSelectedFirst() && _NULL_IDPTR != pclWTCombination->GetSelectedFirst()->GetProductIDPtr() )
		{
			pclProduct = dynamic_cast<CDB_Product*>( (CData*) pclWTCombination->GetSelectedFirst()->GetProductIDPtr().MP );
		}
		else if( NULL != pclWTCombination->GetSelectedSecond() && _NULL_IDPTR != pclWTCombination->GetSelectedSecond()->GetProductIDPtr() )
		{
			pclProduct = dynamic_cast<CDB_Product*>( (CData*) pclWTCombination->GetSelectedSecond()->GetProductIDPtr().MP );
		}
	}

	return pclProduct;
}

bool CRViewSSelPM::OnClickProduct( CSheetDescription *pclSheetDescriptionPM, CCellDescriptionProduct *pclCellDescriptionProduct, long lColumn, long lRow )
{
	if( NULL == m_pclIndSelPMParams || NULL == m_pclIndSelPMParams->m_pclSelectPMList || NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser() ||
			NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetPMWQSelectionPreferences() )
	{
		ASSERTA_RETURN( false );
	}

	if( NULL == pclSheetDescriptionPM || NULL == pclSheetDescriptionPM->GetSSheetPointer() || NULL == pclCellDescriptionProduct )
	{
		ASSERTA_RETURN( false );
	}

	LPARAM lpSelectedUserParamClicked = pclCellDescriptionProduct->GetUserParam();

	if( 0 == lpSelectedUserParamClicked )
	{
		return false;
	}

	// HYS-816: Disable the "OnClickProduct" on buffer vessel column when having combo
	if ((ColumnDescription_TecBox::CD_TecBox_BufferVessel == lColumn) &&
		(SS_TYPE_COMBOBOX == pclSheetDescriptionPM->GetSSheetPointer()->GetCellTypeW(lColumn, lRow)))
	{
		return false;
	}

	UINT uiSheetDescriptionID = GETWITHOUTGROUPID( pclSheetDescriptionPM->GetSheetDescriptionID() );
	UINT uiSDGroupID = pclSheetDescriptionPM->GetGroupSheetDescriptionID();
	LPARAM lpSelectedUserParamPrevious = 0;

	KillCurrentFocus();

	if( uiSDGroupID != uiSheetDescriptionID )
	{
 		// If it's the same group, we clear only from the current SheetDescription in which user has clicked 
 		// until the last sheet belonging to the same group.
		lpSelectedUserParamPrevious = m_arlpSelectedUserParam[uiSheetDescriptionID];
 		_ClearPreviousSelectedProduct( uiSheetDescriptionID, uiSDGroupID );
	}
	else
	{
		// If it's not the same group, we clear selection for all sheets belonging to the previous group.
		
		if( NULL != m_arlpSelectedUserParam[SD_ExpansionVessel] )
		{
			lpSelectedUserParamPrevious = m_arlpSelectedUserParam[SD_ExpansionVessel];
 			_ClearPreviousSelectedProduct( SD_ExpansionVessel, SD_ExpansionVessel );
		}
		else if( NULL != m_arlpSelectedUserParam[SD_ExpansionVesselMembrane] )
		{
			lpSelectedUserParamPrevious = m_arlpSelectedUserParam[SD_ExpansionVesselMembrane];
 			_ClearPreviousSelectedProduct( SD_ExpansionVesselMembrane, SD_ExpansionVesselMembrane );
		}
		else if( NULL != m_arlpSelectedUserParam[SD_TecBoxCompresso] )
		{
			lpSelectedUserParamPrevious = m_arlpSelectedUserParam[SD_TecBoxCompresso];
 			_ClearPreviousSelectedProduct( SD_TecBoxCompresso, SD_TecBoxCompresso );
		}
		else if( NULL != m_arlpSelectedUserParam[SD_TecBoxTransfero] )
		{
			lpSelectedUserParamPrevious = m_arlpSelectedUserParam[SD_TecBoxTransfero];
 			_ClearPreviousSelectedProduct( SD_TecBoxTransfero, SD_TecBoxTransfero );
		}
	}

	// Prepare some needed variables.
	CSSheet *pclSSheet = pclSheetDescriptionPM->GetSSheetPointer();
	int iFirstColumnID = CD_ExpansionVessel_FirstColumn;

	if( SD_ExpansionVessel == uiSheetDescriptionID )
	{
		iFirstColumnID = CD_ExpansionVessel_FirstColumn;
	}
	else if( SD_ExpansionVesselMembrane == uiSheetDescriptionID )
	{
		iFirstColumnID = CD_ExpansionVessel_FirstColumn;
	}
	else if( SD_TecBoxCompresso == uiSheetDescriptionID )
	{
		iFirstColumnID = CD_TecBox_FirstColumn;
	}
	else if( SD_CompressoVessel == uiSheetDescriptionID )
	{
		iFirstColumnID = CD_TecBoxVssl_FirstColumn;
	}
	else if( SD_CompressoSecondaryVessel == uiSheetDescriptionID )
	{
		iFirstColumnID = CD_TecBoxSecVssl_FirstColumn;
	}
	else if( SD_TecBoxTransfero == uiSheetDescriptionID )
	{
		iFirstColumnID = CD_TecBox_FirstColumn;
	}
	else if( SD_TransferoVessel == uiSheetDescriptionID )
	{
		iFirstColumnID = CD_TecBoxVssl_FirstColumn;
	}
	else if( SD_IntermediateVessel == uiSheetDescriptionID )
	{
		iFirstColumnID = CD_IntermVssl_FirstColumn;
	}
	else if( SD_TecBoxVento == uiSheetDescriptionID )
	{
		iFirstColumnID = CD_TecBoxVento_FirstColumn;
	}
	else if( SD_TecBoxPleno == uiSheetDescriptionID || SD_PlenoWaterMakeUpProtection == uiSheetDescriptionID )
	{
		iFirstColumnID = CD_TecBoxPleno_FirstColumn;
	}
	else if( SD_PlenoRefill == uiSheetDescriptionID )
	{
		iFirstColumnID = CD_PlenoRefill_FirstColumn;
	}

	LPARAM lProductTotalCount;
	pclSheetDescriptionPM->GetUserVariable( _SDUV_TOTALPRODUCT_COUNT, lProductTotalCount );

	// If there was one pressure maintenance product selected and user has clicked on the same one...
	if( 0 != lpSelectedUserParamPrevious && lpSelectedUserParamClicked == lpSelectedUserParamPrevious )
	{
		// Set focus on the pressure maintenance product currently selected.
		PrepareAndSetNewFocus( pclSheetDescriptionPM, pclSheetDescriptionPM->GetActiveColumn(), lRow, 0 );

		// Notify a change in the selection.
		SelectionHasChanged( IsSelectionReady() );

		CDlgPMGraphsOutput::CPMInterface clOutputInterface;
		clOutputInterface.ClearOutput();
		_SetPMGraphsOutputContext( false );

		if( SD_TecBoxVento == uiSheetDescriptionID )
		{
			pclSSheet->SetCheckBox( CD_TecBoxVento_CheckBox, lRow, _T( "" ), false, true );
			_VerifyPlenoSheet( NULL, uiSDGroupID );
		}
		else if( SD_ExpansionVessel == uiSheetDescriptionID || SD_ExpansionVesselMembrane == uiSheetDescriptionID )
		{
			pclSSheet->SetCheckBox( CD_ExpansionVessel_CheckBox, lRow, _T( "" ), false, true );
		}
		else if( SD_IntermediateVessel == uiSheetDescriptionID )
		{
			pclSSheet->SetCheckBox( CD_IntermVssl_CheckBox, lRow, _T( "" ), false, true );
		}
		else if( SD_TecBoxCompresso == uiSheetDescriptionID || SD_TecBoxTransfero == uiSheetDescriptionID )
		{
			pclSSheet->SetCheckBox( CD_TecBox_CheckBox, lRow, _T( "" ), false, true );
		}
		else if( SD_CompressoVessel == uiSheetDescriptionID || SD_TransferoVessel == uiSheetDescriptionID )
		{
			pclSSheet->SetCheckBox( CD_TecBoxVssl_CheckBox, lRow, _T( "" ), false, true );
		}
		else if( SD_CompressoSecondaryVessel == uiSheetDescriptionID )
		{
			pclSSheet->SetCheckBox( CD_TecBoxSecVssl_CheckBox, lRow, _T( "" ), false, true );
		}
		else if( SD_TecBoxPleno == uiSheetDescriptionID || SD_PlenoWaterMakeUpProtection == uiSheetDescriptionID )
		{
			pclSSheet->SetCheckBox( CD_TecBoxPleno_CheckBox, lRow, _T( "" ), false, true );
			if( SD_TecBoxPleno == uiSheetDescriptionID )
			{
				// First of all: clear Pleno sheet and its accessories.
				// -1 because in this case Pleno are not belonging to a group.
				_ClearPreviousSelectedProduct( SD_PlenoWaterMakeUpProtection, -1 );

				// '_ClearPreviousSelectedProduct' clear only sheets linked to Pleno (accessories or so on) but not the Pleno sheet itself.
				m_ViewDescription.RemoveOneSheetDescription( SD_PlenoWaterMakeUpProtection );
			}
			else
			{
				// HYS-1121 to check if the refill sheet has to be updated.
				_VerifyPlenoRefillSheet( uiSDGroupID );
			}
		}
		else if( SD_PlenoRefill == uiSheetDescriptionID )
		{
			pclSSheet->SetCheckBox( CD_PlenoRefill_CheckBox, lRow, _T( "" ), false, true );
		}
	}
	else
	{
		// Save the new pressure maintenance product selection.
		m_arlpSelectedUserParam[uiSheetDescriptionID] = pclCellDescriptionProduct->GetUserParam();
		SetCurrentPMSelected( uiSheetDescriptionID, uiSDGroupID, pclCellDescriptionProduct );

		// Create Expand/Collapse rows button if needed...
		if( lProductTotalCount > 1 )
		{
			m_mapButtonExpandCollapseRows[uiSheetDescriptionID] = CreateExpandCollapseRowsButton( iFirstColumnID, lRow, true,
					CCDButtonExpandCollapseRows::ButtonState::CollapseRow, pclSheetDescriptionPM->GetFirstSelectableRow(), 
					pclSheetDescriptionPM->GetLastSelectableRow( false ), pclSheetDescriptionPM );

			// Show button.
			if( NULL != m_mapButtonExpandCollapseRows[uiSheetDescriptionID] )
			{
				m_mapButtonExpandCollapseRows[uiSheetDescriptionID]->SetShowStatus( true );
			}
		}

		// Select pressure maintenance product (just highlight background).
		pclSSheet->SelectOneRow( lRow, pclSheetDescriptionPM->GetSelectionFrom(), pclSheetDescriptionPM->GetSelectionTo() );

		// Fill corresponding accessories and corresponding intermediate vessel if needed.
		CSheetDescription *pclAccessorySheetDescription = NULL;
		bool fCollapseOtherGroups = false;

		if( SD_ExpansionVessel == uiSheetDescriptionID )
		{
			CDB_Vessel *pclExpansionVesselSelected = NULL;

			if( true == GetExpansionVesselSelected( &pclExpansionVesselSelected ) )
			{
				// Call the 'OnCellClicked' in the base class to simulate a click on the Expand/Collapse group button 
				// to force the vessel sheet to collapse.
				if( lProductTotalCount > 1 )
				{
					OnCellClicked( m_ViewDescription.GetFromSheetDescriptionID( SD_ExpansionVessel ), CD_ExpansionVessel_FirstColumn, lRow );
				}

				_FillAccessoryRows( SD_ExpansionVesselAccessory, uiSDGroupID, SD_ExpansionVessel, IDS_SSHEETSSELPM_STACCGROUP, pclExpansionVesselSelected );
				pclAccessorySheetDescription = m_ViewDescription.GetFromSheetDescriptionID( SD_ExpansionVesselAccessory );

				CSelectedVssl *pclSelectedVessel = (CSelectedVssl *)pclCellDescriptionProduct->GetUserParam();
				_FillIntermediateVsslSheet( SD_IntermediateVessel, uiSDGroupID, SD_ExpansionVessel, pclSelectedVessel );

				// Fill Pleno and Vento if there are some.
				m_pclIndSelPMParams->m_pclSelectPMList->SelectVentoPleno( pclSelectedVessel );
				_FillTecBoxVentoSheet( uiSDGroupID, pclSelectedVessel );
				_FillTecBoxPlenoSheet( uiSDGroupID, pclSelectedVessel );

				// Update expansion pipe according selected vessel.
				CSheetDescription *pclSDComputedData = m_ViewDescription.GetFromSheetDescriptionID( SD_ComputedData );

				if( NULL != pclSDComputedData && NULL != pclSDComputedData->GetSSheetPointer() )
				{
					CSSheet *pclSSheetData = pclSDComputedData->GetSSheetPointer();
					CPMInputUser *pPMInputUser = m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser();

					IDPTR PipeDNIDPTR = pPMInputUser->GetExpansionPipeSizeIDPtr( pclExpansionVesselSelected, 10.0 );
					CDB_StringID *pPipeDN = dynamic_cast<CDB_StringID *>( PipeDNIDPTR.MP );

					if( NULL != pPipeDN )
					{
						pclSSheetData->SetStaticText( CD_ComputedData_ExpansionVessel, RD_ComputedData_ExpansionPipeBlw10, pPipeDN->GetString() );
					}

					PipeDNIDPTR = pPMInputUser->GetExpansionPipeSizeIDPtr( pclExpansionVesselSelected, 30.0 );
					pPipeDN = dynamic_cast<CDB_StringID *>( PipeDNIDPTR.MP );

					if( NULL != pPipeDN )
					{
						pclSSheetData->SetStaticText( CD_ComputedData_ExpansionVessel, RD_ComputedData_ExpansionPipeAbv10, pPipeDN->GetString() );
					}
				}

				// Fill 'CDlgPMGraphsOutput' with available graph for the expansion vessel.
				_FillExpansionVesselCurves( pclSelectedVessel );

				fCollapseOtherGroups = true;

				pclSSheet->SetCheckBox(CD_ExpansionVessel_CheckBox, lRow, _T(""), true, true);
			}
		}
		else if( SD_ExpansionVesselMembrane == uiSheetDescriptionID )
		{
			CDB_Vessel *pclExpansionVesselMembraneSelected = NULL;

			if( true == GetExpansionVesselMembraneSelected( &pclExpansionVesselMembraneSelected ) )
			{
				// Call the 'OnCellClicked' in the base class to simulate a click on the Expand/Collapse group button 
				// to force the vessel sheet to collapse.
				if( lProductTotalCount > 1 )
				{
					OnCellClicked( m_ViewDescription.GetFromSheetDescriptionID( SD_ExpansionVesselMembrane ), CD_ExpansionVessel_FirstColumn, lRow );
				}

				_FillAccessoryRows( SD_ExpansionVesselMembraneAccessory, uiSDGroupID, SD_ExpansionVesselMembrane, IDS_SSHEETSSELPM_STACCGROUP, 
						pclExpansionVesselMembraneSelected );

				pclAccessorySheetDescription = m_ViewDescription.GetFromSheetDescriptionID( SD_ExpansionVesselMembraneAccessory );

				CSelectedVssl *pclSelectedVessel = ( CSelectedVssl * )pclCellDescriptionProduct->GetUserParam();
				_FillIntermediateVsslSheet( SD_IntermediateVessel, uiSDGroupID, SD_ExpansionVesselMembrane, pclSelectedVessel );

				// Fill Pleno and Vento if there are some.
				m_pclIndSelPMParams->m_pclSelectPMList->SelectVentoPleno( pclSelectedVessel );
				_FillTecBoxVentoSheet( uiSDGroupID, pclSelectedVessel );
				_FillTecBoxPlenoSheet( uiSDGroupID, pclSelectedVessel );

				// Update expansion pipe according selected vessel.
				CSheetDescription *pclSDComputedData = m_ViewDescription.GetFromSheetDescriptionID( SD_ComputedData );

				if( NULL != pclSDComputedData && NULL != pclSDComputedData->GetSSheetPointer() )
				{
					CSSheet *pclSSheetData = pclSDComputedData->GetSSheetPointer();
					CPMInputUser *pPMInputUser = m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser();

					IDPTR PipeDNIDPTR = pPMInputUser->GetExpansionPipeSizeIDPtr( pclExpansionVesselMembraneSelected, 10.0 );
					CDB_StringID *pPipeDN = dynamic_cast<CDB_StringID *>( PipeDNIDPTR.MP );

					if( NULL != pPipeDN )
					{
						pclSSheetData->SetStaticText( CD_ComputedData_ExpansionVessel, RD_ComputedData_ExpansionPipeBlw10, pPipeDN->GetString() );
					}

					PipeDNIDPTR = pPMInputUser->GetExpansionPipeSizeIDPtr( pclExpansionVesselMembraneSelected, 30.0 );
					pPipeDN = dynamic_cast<CDB_StringID *>( PipeDNIDPTR.MP );

					if( NULL != pPipeDN )
					{
						pclSSheetData->SetStaticText( CD_ComputedData_ExpansionVessel, RD_ComputedData_ExpansionPipeAbv10, pPipeDN->GetString() );
					}
				}

				// Fill 'CDlgPMGraphsOutput' with available graph for the expansion vessel.
				_FillExpansionVesselCurves( pclSelectedVessel );

				fCollapseOtherGroups = true;

				pclSSheet->SetCheckBox(CD_ExpansionVessel_CheckBox, lRow, _T(""), true, true);
			}
		}
		else if( SD_TecBoxCompresso == uiSheetDescriptionID )
		{
			CDB_TecBox *pclCompressoTechBoxSelected = NULL;

			if( true == GetTecBoxCompressoSelected( &pclCompressoTechBoxSelected ) && NULL != pclCompressoTechBoxSelected )
			{
				// Call the 'OnCellClicked' in the base class to simulate a click on the Expand/Collapse group button 
				// to force the vessel sheet to collapse.
				if( lProductTotalCount > 1 )
				{
					OnCellClicked( m_ViewDescription.GetFromSheetDescriptionID( SD_TecBoxCompresso ), CD_TecBox_FirstColumn, lRow );
				}

				_FillAccessoryRows( SD_TecBoxCompressoAccessory, uiSDGroupID, SD_TecBoxCompresso, IDS_SSHEETSSELPM_CPACCGROUP, pclCompressoTechBoxSelected );
				pclAccessorySheetDescription = m_ViewDescription.GetFromSheetDescriptionID( SD_TecBoxCompressoAccessory );

				CSelectedCompresso *pclSelectedCompresso = dynamic_cast<CSelectedCompresso *>( (CSelectedPMBase *)( pclCellDescriptionProduct->GetUserParam() ) );
				ASSERT( NULL != pclSelectedCompresso );

				m_pclIndSelPMParams->m_pclSelectPMList->SelectCompressoVessel( pclSelectedCompresso );

				// 'Pre' because we need to determine if it's a Simply Compresso (with integrated primary vessel) or the other classic Compresso.
				_FillPreCompressoVesselSheet( pclSelectedCompresso, uiSDGroupID );

				// Fill Vento and Pleno if there are some.
				m_pclIndSelPMParams->m_pclSelectPMList->SelectVentoPleno( pclSelectedCompresso );
				_FillTecBoxVentoSheet( uiSDGroupID, pclSelectedCompresso );
				_FillTecBoxPlenoSheet( uiSDGroupID, pclSelectedCompresso );

				// Update expansion pipe according to the selected Compresso.
				CSheetDescription *pclSDComputedData = m_ViewDescription.GetFromSheetDescriptionID( SD_ComputedData );

				if( NULL != pclSDComputedData && NULL != pclSDComputedData->GetSSheetPointer() )
				{
					CSSheet *pclSSheetData = pclSDComputedData->GetSSheetPointer();
					CPMInputUser *pPMInputUser = m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser();

					IDPTR PipeDNIDPTR = pPMInputUser->GetExpansionPipeSizeIDPtr( pclCompressoTechBoxSelected, 10.0 );
					CDB_StringID *pPipeDN = dynamic_cast<CDB_StringID *>( PipeDNIDPTR.MP );

					if( NULL != pPipeDN )
					{
						pclSSheetData->SetStaticText( CD_ComputedData_Compresso, RD_ComputedData_ExpansionPipeBlw10, pPipeDN->GetString() );
					}

					PipeDNIDPTR = pPMInputUser->GetExpansionPipeSizeIDPtr( pclCompressoTechBoxSelected, 30.0 );
					pPipeDN = dynamic_cast<CDB_StringID *>( PipeDNIDPTR.MP );

					if( NULL != pPipeDN )
					{
						pclSSheetData->SetStaticText( CD_ComputedData_Compresso, RD_ComputedData_ExpansionPipeAbv10, pPipeDN->GetString() );
					}
				}

				// Fill 'CDlgPMGraphsOutput' with available graph for the Compresso.
				_FillCompressoCurves( pclCompressoTechBoxSelected, pclSelectedCompresso->GetCompressoCXNumber() );

				fCollapseOtherGroups = true;

				pclSSheet->SetCheckBox( CD_TecBox_CheckBox, lRow, _T(""), true, true );
			}
		}
		else if( SD_CompressoVessel == uiSheetDescriptionID )
		{
			CDB_Vessel *pclCompressoVsslSelected = NULL;
			UINT uiSheetDescriptionID = -1;

			if( true == GetCompressoVesselSelected( &pclCompressoVsslSelected ) && NULL != pclCompressoVsslSelected )
			{
				_FillAccessoryRows( SD_CompressoVesselAccessory, uiSDGroupID, SD_CompressoVessel, IDS_SSHEETSSELPM_CPVSSLACCGROUP, 
						pclCompressoVsslSelected );

				pclAccessorySheetDescription = m_ViewDescription.GetFromSheetDescriptionID( SD_CompressoVesselAccessory );

				CSelectedVssl *pclSelectedVessel = ( CSelectedVssl * )pclCellDescriptionProduct->GetUserParam();
				_FillIntermediateVsslSheet( SD_IntermediateVessel, uiSDGroupID, SD_CompressoVessel, pclSelectedVessel );

				fCollapseOtherGroups = true;

				pclSSheet->SetCheckBox(CD_TecBoxVssl_CheckBox, lRow, _T(""), true, true);
			}
		}
		else if( SD_CompressoSecondaryVessel == uiSheetDescriptionID )
		{
			CDB_Vessel *pclCompressoSecondaryVsslSelected = NULL;
			UINT uiSheetDescriptionID = -1;

			if( true == GetCompressoSecondaryVesselSelected( &pclCompressoSecondaryVsslSelected ) && NULL != pclCompressoSecondaryVsslSelected )
			{
				_FillAccessoryRows( SD_CompressoSecondaryVesselAccessory, uiSDGroupID, SD_CompressoSecondaryVessel, IDS_SSHEETSSELPM_CPVSSLACCGROUP, 
						pclCompressoSecondaryVsslSelected );

				pclAccessorySheetDescription = m_ViewDescription.GetFromSheetDescriptionID( SD_CompressoSecondaryVesselAccessory );

				CSelectedVssl *pclSelectedVessel = ( CSelectedVssl * )pclCellDescriptionProduct->GetUserParam();
				_FillIntermediateVsslSheet( SD_IntermediateVessel, uiSDGroupID, SD_CompressoSecondaryVessel, pclSelectedVessel );

				fCollapseOtherGroups = true;

				pclSSheet->SetCheckBox( CD_TecBoxSecVssl_CheckBox, lRow, _T(""), true, true );
			}
		}
		else if( SD_TecBoxTransfero == uiSheetDescriptionID )
		{
			CDB_TecBox *pclTransferoTechBoxSelected = NULL;
			CDB_Vessel *pclTechBoxBufferVesselSelected = NULL;

			UINT uiSheetDescriptionID = -1;
			UINT uiSheetParentID = -1;

			if( true == GetTecBoxTransferoSelected( &pclTransferoTechBoxSelected ) && NULL != pclTransferoTechBoxSelected )
			{
				// Call the 'OnCellClicked' in the base class to simulate a click on the Expand/Collapse group button 
				// to force the vessel sheet to collapse.
				if( lProductTotalCount > 1 )
				{
					OnCellClicked( m_ViewDescription.GetFromSheetDescriptionID( SD_TecBoxTransfero ), CD_TecBox_FirstColumn, lRow );
				}
				// HYS-872: For transfero tecbox display selected buffer vessel accessories
				CDB_Vessel *pclTechBoxBufferVesselSelected = NULL;
				GetBufferVesselTecBoxSelected((LPARAM*)(&pclTechBoxBufferVesselSelected));

				// Fill accessories linked to the Transfero.
				_FillAccessoryRows( SD_TecBoxTransferoAccessory, uiSDGroupID, SD_TecBoxTransfero, IDS_SSHEETSSELPM_TFACCGROUP, pclTransferoTechBoxSelected );

				// Fill accessories linked to the Transfero buffer vessel if exist.
				_FillAccessoryRows( SD_TecBoxTransferoBufferVesselAccessory, uiSDGroupID, SD_TecBoxTransfero, IDS_SSHEETSSELPM_TBTFBVSSLACCGROUP, pclTechBoxBufferVesselSelected );

				pclAccessorySheetDescription = m_ViewDescription.GetFromSheetDescriptionID( SD_TecBoxTransferoAccessory );

				CSelectedPMBase *pclSelectedTransfero = (CSelectedPMBase *)pclCellDescriptionProduct->GetUserParam();
				m_pclIndSelPMParams->m_pclSelectPMList->SelectTransferoVessel( pclSelectedTransfero );
				_FillTransferoVsslSheet( uiSDGroupID, NULL );

				// Fill Vento and Pleno if there are some.
				m_pclIndSelPMParams->m_pclSelectPMList->SelectVentoPleno( pclSelectedTransfero );
				_FillTecBoxVentoSheet( uiSDGroupID, pclSelectedTransfero );
				_FillTecBoxPlenoSheet( uiSDGroupID, pclSelectedTransfero );

				// Update Expansion pipe according selected Transfero
				CSheetDescription *pclSDComputedData = m_ViewDescription.GetFromSheetDescriptionID( SD_ComputedData );

				if( NULL != pclSDComputedData && NULL != pclSDComputedData->GetSSheetPointer() )
				{
					CSSheet *pclSSheetData = pclSDComputedData->GetSSheetPointer();
					CPMInputUser *pPMInputUser = m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser();

					IDPTR PipeDNIDPTR = pPMInputUser->GetExpansionPipeSizeIDPtr( pclTransferoTechBoxSelected, 10.0 );
					CDB_StringID *pPipeDN = dynamic_cast<CDB_StringID *>( PipeDNIDPTR.MP );

					if( NULL != pPipeDN )
					{
						pclSSheetData->SetStaticText( CD_ComputedData_Transfero, RD_ComputedData_ExpansionPipeBlw10, pPipeDN->GetString() );
					}

					PipeDNIDPTR = pPMInputUser->GetExpansionPipeSizeIDPtr( pclTransferoTechBoxSelected, 30.0 );
					pPipeDN = dynamic_cast<CDB_StringID *>( PipeDNIDPTR.MP );

					if( NULL != pPipeDN )
					{
						pclSSheetData->SetStaticText( CD_ComputedData_Transfero, RD_ComputedData_ExpansionPipeAbv10, pPipeDN->GetString() );
					}
				}

				// Fill 'CDlgPMGraphsOutput' with available graph for Transfero.
				_FillTransferoCurves( pclTransferoTechBoxSelected );

				fCollapseOtherGroups = true;

				pclSSheet->SetCheckBox(CD_TecBox_CheckBox, lRow, _T(""), true, true);
			}
		}
		else if( SD_TransferoVessel == uiSheetDescriptionID )
		{
			CDB_Vessel *pclTransferoVsslSelected = NULL;

			if( true == GetTransferoVesselSelected( &pclTransferoVsslSelected ) && NULL != pclTransferoVsslSelected )
			{
				_FillAccessoryRows( SD_TransferoVesselAccessory, uiSDGroupID, SD_TransferoVessel, IDS_SSHEETSSELPM_TRSFROVSSLACCGROUP, 
						pclTransferoVsslSelected );

				pclAccessorySheetDescription = m_ViewDescription.GetFromSheetDescriptionID( SD_TransferoVesselAccessory );

				CSelectedVssl *pclSelectedVessel = ( CSelectedVssl * )pclCellDescriptionProduct->GetUserParam();
				_FillIntermediateVsslSheet( SD_IntermediateVessel, uiSDGroupID, SD_TransferoVessel, pclSelectedVessel );
				fCollapseOtherGroups = true;

				pclSSheet->SetCheckBox(CD_TecBoxVssl_CheckBox, lRow, _T(""), true, true);
			}
		}
		else if( SD_IntermediateVessel == uiSheetDescriptionID )
		{
			CSheetDescription *pclSDIntermVssl = m_ViewDescription.GetFromSheetDescriptionID( SD_IntermediateVessel );

			if( NULL != pclSDIntermVssl )
			{
				CDB_Vessel *pclIntermVsslSelected = NULL;

				if( true == GetIntermediateVesselSelected( &pclIntermVsslSelected ) && NULL != pclIntermVsslSelected )
				{
					_FillAccessoryRows( SD_IntermediateVesselAcc, uiSDGroupID, SD_IntermediateVessel, IDS_SSHEETSSELPM_CPINTERMVSSLACCGROUP, 
							pclIntermVsslSelected );

					pclAccessorySheetDescription = m_ViewDescription.GetFromSheetDescriptionID( SD_IntermediateVesselAcc );

					pclSSheet->SetCheckBox( CD_IntermVssl_CheckBox, lRow, _T(""), true, true );
				}
			}
		}
		else if( SD_TecBoxVento == uiSheetDescriptionID )
		{
			CDB_TBPlenoVento *pclTecBoxVentoSelected = NULL;
			long lVentoRow;
			LPARAM lpParam;

			if( true == GetTecBoxVentoSelected( &pclTecBoxVentoSelected, uiSDGroupID, &lVentoRow, &lpParam ) )
			{
				_FillAccessoryRows( SD_TecBoxVentoAccessory, uiSDGroupID, SD_TecBoxVento, IDS_SSHEETSSELPM_VENTOACCGROUP, 
						pclTecBoxVentoSelected );

				fCollapseOtherGroups = true;
			}

			CSelectedVento *pclSelectedVento = (CSelectedVento *)lpParam;

			_VerifyPlenoSheet( pclSelectedVento, uiSDGroupID );

			pclSSheet->SetCheckBox(CD_TecBoxVento_CheckBox, lRow, _T(""), true, true);
		}
		else if( SD_TecBoxPleno == uiSheetDescriptionID || SD_PlenoWaterMakeUpProtection == uiSheetDescriptionID )
		{
			LPARAM lpParam = 0;
			CDB_Set *pclWTCombination = NULL;
			CDB_TBPlenoVento *pclTecBoxPlenoSelected = NULL;

			if( true == GetTecBoxPlenoSelected( &pclTecBoxPlenoSelected, uiSDGroupID, NULL, &lpParam )  )
			{
				if( SD_PlenoWaterMakeUpProtection == uiSheetDescriptionID )
				{
					pclTecBoxPlenoSelected = NULL;
					GetTecBoxPlenoWMProtectionSelected( &pclTecBoxPlenoSelected, uiSDGroupID, NULL, &lpParam );
				}
				
				if( NULL != pclTecBoxPlenoSelected )
				{
					// The classic way.
					_FillAccessoryRows( SD_TecBoxPlenoAccessory, uiSDGroupID, uiSheetDescriptionID, IDS_SSHEETSSELPM_PLENOACCGROUP,
							pclTecBoxPlenoSelected );
				}
				else if( 0 != lpParam )
				{
					// Here we have a water treatment combination for the Pleno (the case for TV connect with P BA4 P + P AB5 R).
					CSelectedWaterTreatmentCombination *pclSelectedWTCombination = (CSelectedWaterTreatmentCombination*)lpParam;
					pclWTCombination = pclSelectedWTCombination->GetWTCombination();

					if( NULL != pclWTCombination )
					{
						CRank rList;
						CDB_RuledTable *pclRuledTable = NULL;

						_PrepareTBPlenoAccessories( pclWTCombination, &rList, &pclRuledTable );

						_FillAccessoryRows( SD_TecBoxPlenoAccessory, uiSDGroupID, uiSheetDescriptionID, IDS_SSHEETSSELPM_PLENOACCGROUP, NULL,
								&rList, pclRuledTable );
					}
				}
				
				// HYS-1121 : Look for water make-up protection for Pleno
				if( SD_TecBoxPleno == uiSheetDescriptionID )
				{
					// First of all: clear Pleno sheet and its accessories.
					// -1 because in this case Pleno are not belonging to a group.
					_ClearPreviousSelectedProduct( SD_PlenoWaterMakeUpProtection, -1 );

					// '_ClearPreviousSelectedProduct' clear only sheets linked to Pleno (accessories or so on) but not the Pleno sheet itself.
					m_ViewDescription.RemoveOneSheetDescription( SD_PlenoWaterMakeUpProtection );
				}
				
				if( NULL != pclTecBoxPlenoSelected && true == pclTecBoxPlenoSelected->IsVariantWMProtectionConnection() )
				{
					// Display optional water make-up protection module
					CSelectedPMBase *pclSelectedPleno = (CSelectedPMBase *)pclCellDescriptionProduct->GetUserParam();
					// Fill Pleno and Vento if there are some.
					m_pclIndSelPMParams->m_pclSelectPMList->SelectPlenoProtec( pclSelectedPleno );
					// true for pleno water makeup protection
					_FillTecBoxPlenoSheet( uiSDGroupID, pclSelectedPleno, true );
				}
				
				if (true == m_ViewDescription.IsSheetDescriptionExist(SD_PlenoRefill) )
				{
					_ClearPreviousSelectedProduct( SD_PlenoRefill, -1 );
					m_ViewDescription.RemoveOneSheetDescription( SD_PlenoRefill );
				}
				
				bool bAtLeastOneDeviceSelectedHasVacuumDegassingFunction = _IsAtLeastOneSelectedDeviceHasVacuumDegassingFunction( uiSDGroupID );
				m_pclIndSelPMParams->m_pclSelectPMList->SelectPlenoRefill( pclTecBoxPlenoSelected, bAtLeastOneDeviceSelectedHasVacuumDegassingFunction, pclWTCombination );
				_FillPlenoRefillSheet( uiSDGroupID );
				fCollapseOtherGroups = true;

				pclSSheet->SetCheckBox(CD_TecBoxPleno_CheckBox, lRow, _T(""), true, true);
			}
		}
		else if( SD_PlenoRefill == uiSheetDescriptionID )
		{
			CDB_PlenoRefill *pclPlenoRefillSelected = NULL;

			if( true == GetPlenoRefillSelected( &pclPlenoRefillSelected, uiSDGroupID ) )
			{
				_FillAccessoryRows( SD_PlenoRefillAccessory, uiSDGroupID, SD_PlenoRefill, IDS_SSHEETSSELPM_PLENOREFILLACCGROUP, 
						pclPlenoRefillSelected );

				fCollapseOtherGroups = true;

				pclSSheet->SetCheckBox( CD_PlenoRefill_CheckBox, lRow, _T(""), true, true );

				// HYS-1445: need a special verification for Pleno Refill with P BAR 4 and Statico.
				// More details in the method.
				_VerifyPlenoRefillAccessories( uiSDGroupID );
			}
		}

		if( true == fCollapseOtherGroups )
		{
			_CollapseOtherGroups( uiSheetDescriptionID );
		}

		// Try to set the focus on the next table but do not lose the focus if no other table exist.
		CSheetDescription *pclNextSheetDescription = m_ViewDescription.GetNextSheetDescription( pclSheetDescriptionPM, -1, 
				pclSheetDescriptionPM->GetGroupSheetDescriptionID() );

		long lNewFocusedCol = lColumn;
		long lNewFocusedRow = lRow;

		if( NULL != pclNextSheetDescription )
		{
			// A next sheet exist in the same group. Verify now if there is at least one available product (It can happen that all products
			// are in the excluded list).
			long lProductTotalCount;
			long lProductNotPriorityCount;
			pclNextSheetDescription->GetUserVariable( _SDUV_TOTALPRODUCT_COUNT, lProductTotalCount );
			pclNextSheetDescription->GetUserVariable( _SDUV_NOPRIORITYPRODUCT_COUNT, lProductNotPriorityCount );

			if( 0 != lProductTotalCount || 0 != lProductNotPriorityCount )
			{
				lNewFocusedCol = pclNextSheetDescription->GetFirstParameterColumn();
				lNewFocusedRow = pclNextSheetDescription->GetFirstSelectableRow();
			}
			else
			{
				pclNextSheetDescription = pclSheetDescriptionPM;
			}
		}
		else
		{
			// Because we have changed current selection background in yellow, if there is no other sheet following current one, we
			// must set focus on current selection.
			pclNextSheetDescription = pclSheetDescriptionPM;
		}

		PrepareAndSetNewFocus( pclNextSheetDescription, lNewFocusedCol, lNewFocusedRow );

		// Notify a change in the selection.
		SelectionHasChanged( IsSelectionReady() );
	}

	_UpdateCurrentSelectionPriceIndex();

	return true;
}

bool CRViewSSelPM::ResetColumnWidth( short nSheetDescriptionID )
{
	TSpread clTSpread;

	if( FALSE == clTSpread.Create( WS_CHILD, CRect( 0, 0, 0, 0 ), this, 0 ) )
	{
		ASSERT( 0 );
		return false;
	}

	switch( nSheetDescriptionID )
	{
		case SD_ComputedData:
			m_mapSSheetColumnWidth[SD_ComputedData][CD_ComputedData_FirstColumn] = clTSpread.ColWidthToLogUnits( 4 );
			m_mapSSheetColumnWidth[SD_ComputedData][CD_ComputedData_Name] = clTSpread.ColWidthToLogUnits( 50 );
			m_mapSSheetColumnWidth[SD_ComputedData][CD_ComputedData_Value] = clTSpread.ColWidthToLogUnits( 12 );
			m_mapSSheetColumnWidth[SD_ComputedData][CD_ComputedData_ValueIndex] = clTSpread.ColWidthToLogUnits( 2 );
			m_mapSSheetColumnWidth[SD_ComputedData][CD_ComputedData_ExpansionVessel] = clTSpread.ColWidthToLogUnits( 14 );
			m_mapSSheetColumnWidth[SD_ComputedData][CD_ComputedData_ExpansionVesselIndex] = clTSpread.ColWidthToLogUnits( 2 );
			m_mapSSheetColumnWidth[SD_ComputedData][CD_ComputedData_Compresso] = clTSpread.ColWidthToLogUnits( 14 );
			m_mapSSheetColumnWidth[SD_ComputedData][CD_ComputedData_CompressoIndex] = clTSpread.ColWidthToLogUnits( 2 );
			m_mapSSheetColumnWidth[SD_ComputedData][CD_ComputedData_Transfero] = clTSpread.ColWidthToLogUnits( 14 );
			m_mapSSheetColumnWidth[SD_ComputedData][CD_ComputedData_TransferoIndex] = clTSpread.ColWidthToLogUnits( 2 );
			break;

		case SD_PriceIndex:
			m_mapSSheetColumnWidth[SD_PriceIndex][CD_PriceIndex_FirstColumn] = clTSpread.ColWidthToLogUnits( 4 );
			m_mapSSheetColumnWidth[SD_PriceIndex][CD_PriceIndex_Name] = clTSpread.ColWidthToLogUnits( 50 );
			m_mapSSheetColumnWidth[SD_PriceIndex][CD_PriceIndex_ExpansionVessel] = clTSpread.ColWidthToLogUnits( 14 );
			m_mapSSheetColumnWidth[SD_PriceIndex][CD_PriceIndex_ExpansionVesselMembrane] = clTSpread.ColWidthToLogUnits( 14 );
			m_mapSSheetColumnWidth[SD_PriceIndex][CD_PriceIndex_Compresso] = clTSpread.ColWidthToLogUnits( 14 );
			m_mapSSheetColumnWidth[SD_PriceIndex][CD_PriceIndex_Transfero] = clTSpread.ColWidthToLogUnits( 14 );
			break;

		case SD_ExpansionVessel:
			m_mapSSheetColumnWidth[SD_ExpansionVessel][CD_ExpansionVessel_FirstColumn] = clTSpread.ColWidthToLogUnits( 4 );
			m_mapSSheetColumnWidth[SD_ExpansionVessel][CD_ExpansionVessel_CheckBox] = clTSpread.ColWidthToLogUnits( 2 );
			m_mapSSheetColumnWidth[SD_ExpansionVessel][CD_ExpansionVessel_PriceIndexIcon] = clTSpread.ColWidthToLogUnits( 2 );
			m_mapSSheetColumnWidth[SD_ExpansionVessel][CD_ExpansionVessel_PriceIndexValue] = clTSpread.ColWidthToLogUnits( 3 );
			m_mapSSheetColumnWidth[SD_ExpansionVessel][CD_ExpansionVessel_PriceIndexBorder] = clTSpread.ColWidthToLogUnits( 2 );
			m_mapSSheetColumnWidth[SD_ExpansionVessel][CD_ExpansionVessel_Name] = clTSpread.ColWidthToLogUnits( 14 );
			m_mapSSheetColumnWidth[SD_ExpansionVessel][CD_ExpansionVessel_NbreOfVssl] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_ExpansionVessel][CD_ExpansionVessel_Volume] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_ExpansionVessel][CD_ExpansionVessel_MaxPressure] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_ExpansionVessel][CD_ExpansionVessel_TempRange] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_ExpansionVessel][CD_ExpansionVessel_FactoryPresetPressure] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_ExpansionVessel][CD_ExpansionVessel_Diameter] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_ExpansionVessel][CD_ExpansionVessel_Height] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_ExpansionVessel][CD_ExpansionVessel_Weight] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_ExpansionVessel][CD_ExpansionVessel_MaxWeight] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_ExpansionVessel][CD_ExpansionVessel_Standing] = clTSpread.ColWidthToLogUnits( 4 );
			m_mapSSheetColumnWidth[SD_ExpansionVessel][CD_ExpansionVessel_Connection] = clTSpread.ColWidthToLogUnits( 8 );
			m_mapSSheetColumnWidth[SD_ExpansionVessel][CD_ExpansionVessel_RequiredNominalVolumeEN12828] = clTSpread.ColWidthToLogUnits( 8 );
			m_mapSSheetColumnWidth[SD_ExpansionVessel][CD_ExpansionVessel_RequiredNominalVolumeEN12828InfoIcon] = clTSpread.ColWidthToLogUnits( 4 );
			m_mapSSheetColumnWidth[SD_ExpansionVessel][CD_ExpansionVessel_InitialPressure] = clTSpread.ColWidthToLogUnits( 12 );
			m_mapSSheetColumnWidth[SD_ExpansionVessel][CD_ExpansionVessel_WaterReserve] = clTSpread.ColWidthToLogUnits( 12 );
			m_mapSSheetColumnWidth[SD_ExpansionVessel][CD_ExpansionVessel_Pointer] = clTSpread.ColWidthToLogUnits( 1 );
			break;

		case SD_ExpansionVesselMembrane:
			m_mapSSheetColumnWidth[SD_ExpansionVesselMembrane][CD_ExpansionVessel_FirstColumn] = clTSpread.ColWidthToLogUnits( 4 );
			m_mapSSheetColumnWidth[SD_ExpansionVesselMembrane][CD_ExpansionVessel_CheckBox] = clTSpread.ColWidthToLogUnits( 2 );
			m_mapSSheetColumnWidth[SD_ExpansionVesselMembrane][CD_ExpansionVessel_PriceIndexIcon] = clTSpread.ColWidthToLogUnits( 2 );
			m_mapSSheetColumnWidth[SD_ExpansionVesselMembrane][CD_ExpansionVessel_PriceIndexValue] = clTSpread.ColWidthToLogUnits( 3 );
			m_mapSSheetColumnWidth[SD_ExpansionVesselMembrane][CD_ExpansionVessel_PriceIndexBorder] = clTSpread.ColWidthToLogUnits( 2 );
			m_mapSSheetColumnWidth[SD_ExpansionVesselMembrane][CD_ExpansionVessel_Name] = clTSpread.ColWidthToLogUnits( 14 );
			m_mapSSheetColumnWidth[SD_ExpansionVesselMembrane][CD_ExpansionVessel_NbreOfVssl] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_ExpansionVesselMembrane][CD_ExpansionVessel_Volume] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_ExpansionVesselMembrane][CD_ExpansionVessel_MaxPressure] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_ExpansionVesselMembrane][CD_ExpansionVessel_TempRange] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_ExpansionVesselMembrane][CD_ExpansionVessel_FactoryPresetPressure] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_ExpansionVesselMembrane][CD_ExpansionVessel_Diameter] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_ExpansionVesselMembrane][CD_ExpansionVessel_Height] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_ExpansionVesselMembrane][CD_ExpansionVessel_Weight] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_ExpansionVesselMembrane][CD_ExpansionVessel_MaxWeight] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_ExpansionVesselMembrane][CD_ExpansionVessel_Standing] = clTSpread.ColWidthToLogUnits( 4 );
			m_mapSSheetColumnWidth[SD_ExpansionVesselMembrane][CD_ExpansionVessel_Connection] = clTSpread.ColWidthToLogUnits( 8 );
			m_mapSSheetColumnWidth[SD_ExpansionVesselMembrane][CD_ExpansionVessel_RequiredNominalVolumeEN12828] = clTSpread.ColWidthToLogUnits( 8 );
			m_mapSSheetColumnWidth[SD_ExpansionVesselMembrane][CD_ExpansionVessel_RequiredNominalVolumeEN12828InfoIcon] = clTSpread.ColWidthToLogUnits( 4 );
			m_mapSSheetColumnWidth[SD_ExpansionVesselMembrane][CD_ExpansionVessel_InitialPressure] = clTSpread.ColWidthToLogUnits( 12 );
			m_mapSSheetColumnWidth[SD_ExpansionVesselMembrane][CD_ExpansionVessel_WaterReserve] = clTSpread.ColWidthToLogUnits( 12 );
			m_mapSSheetColumnWidth[SD_ExpansionVesselMembrane][CD_ExpansionVessel_Pointer] = clTSpread.ColWidthToLogUnits( 1 );
			break;

		case SD_TecBoxCompresso:
			m_mapSSheetColumnWidth[SD_TecBoxCompresso][CD_TecBox_FirstColumn] = clTSpread.ColWidthToLogUnits( 4 );
			m_mapSSheetColumnWidth[SD_TecBoxCompresso][CD_TecBox_CheckBox] = clTSpread.ColWidthToLogUnits( 2 );
			m_mapSSheetColumnWidth[SD_TecBoxCompresso][CD_TecBox_PriceIndexIcon] = clTSpread.ColWidthToLogUnits( 2 );
			m_mapSSheetColumnWidth[SD_TecBoxCompresso][CD_TecBox_PriceIndexValue] = clTSpread.ColWidthToLogUnits( 3 );
			m_mapSSheetColumnWidth[SD_TecBoxCompresso][CD_TecBox_PriceIndexBorder] = clTSpread.ColWidthToLogUnits( 2 );
			m_mapSSheetColumnWidth[SD_TecBoxCompresso][CD_TecBox_Name] = clTSpread.ColWidthToLogUnits( 25 );
			m_mapSSheetColumnWidth[SD_TecBoxCompresso][CD_TecBox_NbreOfDevice] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_TecBoxCompresso][CD_TecBox_PS] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_TecBoxCompresso][CD_TecBox_TempRange] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_TecBoxCompresso][CD_TecBox_MaxWaterMakeUpTemp] = clTSpread.ColWidthToLogUnits( 11 );
			m_mapSSheetColumnWidth[SD_TecBoxCompresso][CD_TecBox_B] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_TecBoxCompresso][CD_TecBox_H] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_TecBoxCompresso][CD_TecBox_T] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_TecBoxCompresso][CD_TecBox_Weight] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_TecBoxCompresso][CD_TecBox_Standing] = clTSpread.ColWidthToLogUnits( 4 );
			m_mapSSheetColumnWidth[SD_TecBoxCompresso][CD_TecBox_ElectricalPower] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_TecBoxCompresso][CD_TecBox_SupplyVoltage] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_TecBoxCompresso][CD_TecBox_Decibel] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_TecBoxCompresso][CD_TecBox_BufferVessel] = clTSpread.ColWidthToLogUnits( 14 );
			m_mapSSheetColumnWidth[SD_TecBoxCompresso][CD_TecBox_IntegratedVessel] = clTSpread.ColWidthToLogUnits( 14 );
			m_mapSSheetColumnWidth[SD_TecBoxCompresso][CD_TecBox_Pointer] = clTSpread.ColWidthToLogUnits( 1 );
			break;

		case SD_CompressoVessel:
			m_mapSSheetColumnWidth[SD_CompressoVessel][CD_TecBoxVssl_FirstColumn] = clTSpread.ColWidthToLogUnits( 4 );
			m_mapSSheetColumnWidth[SD_CompressoVessel][CD_TecBoxVssl_CheckBox] = clTSpread.ColWidthToLogUnits( 2 );
			m_mapSSheetColumnWidth[SD_CompressoVessel][CD_TecBoxVssl_PriceIndexRatioIcon] = clTSpread.ColWidthToLogUnits( 2 );
			m_mapSSheetColumnWidth[SD_CompressoVessel][CD_TecBoxVssl_PriceIndexRatioValue] = clTSpread.ColWidthToLogUnits( 3 );
			m_mapSSheetColumnWidth[SD_CompressoVessel][CD_TecBoxVssl_PriceIndexRatioBorder] = clTSpread.ColWidthToLogUnits( 2 );
			m_mapSSheetColumnWidth[SD_CompressoVessel][CD_TecBoxVssl_Prim_Name] = clTSpread.ColWidthToLogUnits( 14 );
			m_mapSSheetColumnWidth[SD_CompressoVessel][CD_TecBoxVssl_Prim_Volume] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_CompressoVessel][CD_TecBoxVssl_Prim_MaxPressure] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_CompressoVessel][CD_TecBoxVssl_Prim_TempRange] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_CompressoVessel][CD_TecBoxVssl_Prim_Diameter] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_CompressoVessel][CD_TecBoxVssl_Prim_Height] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_CompressoVessel][CD_TecBoxVssl_Prim_Weight] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_CompressoVessel][CD_TecBoxVssl_Prim_MaxWeight] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_CompressoVessel][CD_TecBoxVssl_Sec_Name] = clTSpread.ColWidthToLogUnits( 14 );
			m_mapSSheetColumnWidth[SD_CompressoVessel][CD_TecBoxVssl_Sec_NbreOfVssl] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_CompressoVessel][CD_TecBoxVssl_Sec_Diameter] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_CompressoVessel][CD_TecBoxVssl_Sec_Height] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_CompressoVessel][CD_TecBoxVssl_Sec_Weight] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_CompressoVessel][CD_TecBoxVssl_Sec_MaxWeight] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_CompressoVessel][CD_TecBoxVssl_Pointer] = clTSpread.ColWidthToLogUnits( 1 );
			break;

		case SD_CompressoSecondaryVessel:
			m_mapSSheetColumnWidth[SD_CompressoSecondaryVessel][CD_TecBoxSecVssl_FirstColumn] = clTSpread.ColWidthToLogUnits( 4 );
			m_mapSSheetColumnWidth[SD_CompressoSecondaryVessel][CD_TecBoxSecVssl_CheckBox] = clTSpread.ColWidthToLogUnits( 2 );
			m_mapSSheetColumnWidth[SD_CompressoSecondaryVessel][CD_TecBoxSecVssl_PriceIndexRatioIcon] = clTSpread.ColWidthToLogUnits( 2 );
			m_mapSSheetColumnWidth[SD_CompressoSecondaryVessel][CD_TecBoxSecVssl_PriceIndexRatioValue] = clTSpread.ColWidthToLogUnits( 3 );
			m_mapSSheetColumnWidth[SD_CompressoSecondaryVessel][CD_TecBoxSecVssl_PriceIndexRatioBorder] = clTSpread.ColWidthToLogUnits( 2 );
			m_mapSSheetColumnWidth[SD_CompressoSecondaryVessel][CD_TecBoxSecVssl_Name] = clTSpread.ColWidthToLogUnits( 14 );
			m_mapSSheetColumnWidth[SD_CompressoSecondaryVessel][CD_TecBoxSecVssl_Volume] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_CompressoSecondaryVessel][CD_TecBoxSecVssl_MaxPressure] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_CompressoSecondaryVessel][CD_TecBoxSecVssl_TempRange] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_CompressoSecondaryVessel][CD_TecBoxSecVssl_Diameter] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_CompressoSecondaryVessel][CD_TecBoxSecVssl_Height] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_CompressoSecondaryVessel][CD_TecBoxSecVssl_Weight] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_CompressoSecondaryVessel][CD_TecBoxSecVssl_MaxWeight] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_CompressoSecondaryVessel][CD_TecBoxSecVssl_Pointer] = clTSpread.ColWidthToLogUnits( 1 );
			break;

		case SD_TecBoxTransfero:
			m_mapSSheetColumnWidth[SD_TecBoxTransfero][CD_TecBox_FirstColumn] = clTSpread.ColWidthToLogUnits( 4 );
			m_mapSSheetColumnWidth[SD_TecBoxTransfero][CD_TecBox_CheckBox] = clTSpread.ColWidthToLogUnits( 2 );
			m_mapSSheetColumnWidth[SD_TecBoxTransfero][CD_TecBox_PriceIndexIcon] = clTSpread.ColWidthToLogUnits( 2 );
			m_mapSSheetColumnWidth[SD_TecBoxTransfero][CD_TecBox_PriceIndexValue] = clTSpread.ColWidthToLogUnits( 3 );
			m_mapSSheetColumnWidth[SD_TecBoxTransfero][CD_TecBox_PriceIndexBorder] = clTSpread.ColWidthToLogUnits( 2 );
			m_mapSSheetColumnWidth[SD_TecBoxTransfero][CD_TecBox_Name] = clTSpread.ColWidthToLogUnits( 25 );
			m_mapSSheetColumnWidth[SD_TecBoxTransfero][CD_TecBox_NbreOfDevice] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_TecBoxTransfero][CD_TecBox_PS] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_TecBoxTransfero][CD_TecBox_TempRange] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_TecBoxTransfero][CD_TecBox_MaxWaterMakeUpTemp] = clTSpread.ColWidthToLogUnits( 11 );
			m_mapSSheetColumnWidth[SD_TecBoxTransfero][CD_TecBox_B] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_TecBoxTransfero][CD_TecBox_H] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_TecBoxTransfero][CD_TecBox_T] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_TecBoxTransfero][CD_TecBox_Weight] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_TecBoxTransfero][CD_TecBox_Standing] = clTSpread.ColWidthToLogUnits( 4 );
			m_mapSSheetColumnWidth[SD_TecBoxTransfero][CD_TecBox_ElectricalPower] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_TecBoxTransfero][CD_TecBox_SupplyVoltage] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_TecBoxTransfero][CD_TecBox_Decibel] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_TecBoxTransfero][CD_TecBox_BufferVessel] = clTSpread.ColWidthToLogUnits( 14 );
			m_mapSSheetColumnWidth[SD_TecBoxTransfero][CD_TecBox_IntegratedVessel] = clTSpread.ColWidthToLogUnits( 14 );
			m_mapSSheetColumnWidth[SD_TecBoxTransfero][CD_TecBox_Pointer] = clTSpread.ColWidthToLogUnits( 1 );
			break;

		case SD_TransferoVessel:
			m_mapSSheetColumnWidth[SD_TransferoVessel][CD_TecBoxVssl_FirstColumn] = clTSpread.ColWidthToLogUnits( 4 );
			m_mapSSheetColumnWidth[SD_TransferoVessel][CD_TecBoxVssl_CheckBox] = clTSpread.ColWidthToLogUnits( 2 );
			m_mapSSheetColumnWidth[SD_TransferoVessel][CD_TecBoxVssl_PriceIndexRatioIcon] = clTSpread.ColWidthToLogUnits( 2 );
			m_mapSSheetColumnWidth[SD_TransferoVessel][CD_TecBoxVssl_PriceIndexRatioValue] = clTSpread.ColWidthToLogUnits( 3 );
			m_mapSSheetColumnWidth[SD_TransferoVessel][CD_TecBoxVssl_PriceIndexRatioBorder] = clTSpread.ColWidthToLogUnits( 2 );
			m_mapSSheetColumnWidth[SD_TransferoVessel][CD_TecBoxVssl_Prim_Name] = clTSpread.ColWidthToLogUnits( 14 );
			m_mapSSheetColumnWidth[SD_TransferoVessel][CD_TecBoxVssl_Prim_Volume] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_TransferoVessel][CD_TecBoxVssl_Prim_MaxPressure] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_TransferoVessel][CD_TecBoxVssl_Prim_TempRange] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_TransferoVessel][CD_TecBoxVssl_Prim_Diameter] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_TransferoVessel][CD_TecBoxVssl_Prim_Height] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_TransferoVessel][CD_TecBoxVssl_Prim_Weight] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_TransferoVessel][CD_TecBoxVssl_Prim_MaxWeight] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_TransferoVessel][CD_TecBoxVssl_Sec_Name] = clTSpread.ColWidthToLogUnits( 14 );
			m_mapSSheetColumnWidth[SD_TransferoVessel][CD_TecBoxVssl_Sec_NbreOfVssl] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_TransferoVessel][CD_TecBoxVssl_Sec_Diameter] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_TransferoVessel][CD_TecBoxVssl_Sec_Height] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_TransferoVessel][CD_TecBoxVssl_Sec_Weight] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_TransferoVessel][CD_TecBoxVssl_Sec_MaxWeight] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_TransferoVessel][CD_TecBoxVssl_Pointer] = clTSpread.ColWidthToLogUnits( 1 );
			break;

		case SD_IntermediateVessel:
			m_mapSSheetColumnWidth[SD_IntermediateVessel][CD_IntermVssl_FirstColumn] = clTSpread.ColWidthToLogUnits( 4 );
			m_mapSSheetColumnWidth[SD_IntermediateVessel][CD_IntermVssl_CheckBox] = clTSpread.ColWidthToLogUnits( 2 );
			m_mapSSheetColumnWidth[SD_IntermediateVessel][CD_IntermVssl_Type] = clTSpread.ColWidthToLogUnits( 14 );
			m_mapSSheetColumnWidth[SD_IntermediateVessel][CD_IntermVssl_NbreOfVssl] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_IntermediateVessel][CD_IntermVssl_Volume] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_IntermediateVessel][CD_IntermVssl_MaxPressure] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_IntermediateVessel][CD_IntermVssl_TempRange] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_IntermediateVessel][CD_IntermVssl_Diameter] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_IntermediateVessel][CD_IntermVssl_Height] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_IntermediateVessel][CD_IntermVssl_Weight] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_IntermediateVessel][CD_IntermVssl_MaxWeight] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_IntermediateVessel][CD_IntermVssl_Connection] = clTSpread.ColWidthToLogUnits( 8 );
			m_mapSSheetColumnWidth[SD_IntermediateVessel][CD_IntermVssl_Pointer] = clTSpread.ColWidthToLogUnits( 1 );
			break;

		case SD_TecBoxVento:
			m_mapSSheetColumnWidth[SD_TecBoxVento][CD_TecBoxVento_FirstColumn] = clTSpread.ColWidthToLogUnits( 4 );
			m_mapSSheetColumnWidth[SD_TecBoxVento][CD_TecBoxVento_CheckBox] = clTSpread.ColWidthToLogUnits( 2 );
			m_mapSSheetColumnWidth[SD_TecBoxVento][CD_TecBoxVento_PriceIndexIcon] = clTSpread.ColWidthToLogUnits( 2 );
			m_mapSSheetColumnWidth[SD_TecBoxVento][CD_TecBoxVento_PriceIndexValue] = clTSpread.ColWidthToLogUnits( 3 );
			m_mapSSheetColumnWidth[SD_TecBoxVento][CD_TecBoxVento_PriceIndexBorder] = clTSpread.ColWidthToLogUnits( 2 );
			m_mapSSheetColumnWidth[SD_TecBoxVento][CD_TecBoxVento_Name] = clTSpread.ColWidthToLogUnits( 14 );
			m_mapSSheetColumnWidth[SD_TecBoxVento][CD_TecBoxVento_NbreOfDevice] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_TecBoxVento][CD_TecBoxVento_MaxPressure] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_TecBoxVento][CD_TecBoxVento_TempRange] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_TecBoxVento][CD_TecBoxVento_MaxWaterMakeUpTemp] = clTSpread.ColWidthToLogUnits( 11 );
			m_mapSSheetColumnWidth[SD_TecBoxVento][CD_TecBoxVento_Width] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_TecBoxVento][CD_TecBoxVento_Height] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_TecBoxVento][CD_TecBoxVento_Depth] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_TecBoxVento][CD_TecBoxVento_Weight] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_TecBoxVento][CD_TecBoxVento_Standing] = clTSpread.ColWidthToLogUnits( 4 );
			m_mapSSheetColumnWidth[SD_TecBoxVento][CD_TecBoxVento_ElectricalPower] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_TecBoxVento][CD_TecBoxVento_SupplyVoltage] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_TecBoxVento][CD_TecBoxVento_VNd] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_TecBoxVento][CD_TecBoxVento_SPL] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_TecBoxVento][CD_TecBoxVento_dpu] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_TecBoxVento][CD_TecBoxVento_qNwm] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_TecBoxVento][CD_TecBoxVento_IP] = clTSpread.ColWidthToLogUnits( 4 );
			m_mapSSheetColumnWidth[SD_TecBoxVento][CD_TecBoxVento_Pointer] = clTSpread.ColWidthToLogUnits( 1 );
			break;

		case SD_PlenoWaterMakeUpProtection:
		case SD_TecBoxPleno:
			m_mapSSheetColumnWidth[SD_TecBoxPleno][CD_TecBoxPleno_FirstColumn] = clTSpread.ColWidthToLogUnits( 4 );
			m_mapSSheetColumnWidth[SD_TecBoxPleno][CD_TecBoxPleno_CheckBox] = clTSpread.ColWidthToLogUnits( 2 );
			m_mapSSheetColumnWidth[SD_TecBoxPleno][CD_TecBoxPleno_PriceIndexIcon] = clTSpread.ColWidthToLogUnits( 2 );
			m_mapSSheetColumnWidth[SD_TecBoxPleno][CD_TecBoxPleno_PriceIndexValue] = clTSpread.ColWidthToLogUnits( 3 );
			m_mapSSheetColumnWidth[SD_TecBoxPleno][CD_TecBoxPleno_PriceIndexBorder] = clTSpread.ColWidthToLogUnits( 2 );
			m_mapSSheetColumnWidth[SD_TecBoxPleno][CD_TecBoxPleno_Name] = clTSpread.ColWidthToLogUnits( 14 );
			m_mapSSheetColumnWidth[SD_TecBoxPleno][CD_TecBoxPleno_NbreOfDevice] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_TecBoxPleno][CD_TecBoxPleno_MaxPressure] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_TecBoxPleno][CD_TecBoxPleno_TempRange] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_TecBoxPleno][CD_TecBoxPleno_Width] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_TecBoxPleno][CD_TecBoxPleno_Height] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_TecBoxPleno][CD_TecBoxPleno_Depth] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_TecBoxPleno][CD_TecBoxPleno_Weight] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_TecBoxPleno][CD_TecBoxPleno_Standing] = clTSpread.ColWidthToLogUnits( 4 );
			m_mapSSheetColumnWidth[SD_TecBoxPleno][CD_TecBoxPleno_ElectricalPower] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_TecBoxPleno][CD_TecBoxPleno_SupplyVoltage] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_TecBoxPleno][CD_TecBoxPleno_Kvs] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_TecBoxPleno][CD_TecBoxPleno_SPL] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_TecBoxPleno][CD_TecBoxPleno_dpu] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_TecBoxPleno][CD_TecBoxPleno_qNwm] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_TecBoxPleno][CD_TecBoxPleno_IP] = clTSpread.ColWidthToLogUnits( 4 );
			m_mapSSheetColumnWidth[SD_TecBoxPleno][CD_TecBoxPleno_Pointer] = clTSpread.ColWidthToLogUnits( 1 );
			break;

		case SD_PlenoRefill:
			m_mapSSheetColumnWidth[SD_PlenoRefill][CD_PlenoRefill_FirstColumn] = clTSpread.ColWidthToLogUnits( 4 );
			m_mapSSheetColumnWidth[SD_PlenoRefill][CD_PlenoRefill_CheckBox] = clTSpread.ColWidthToLogUnits( 2 );
			m_mapSSheetColumnWidth[SD_PlenoRefill][CD_PlenoRefill_PriceIndexIcon] = clTSpread.ColWidthToLogUnits( 2 );
			m_mapSSheetColumnWidth[SD_PlenoRefill][CD_PlenoRefill_PriceIndexValue] = clTSpread.ColWidthToLogUnits( 3 );
			m_mapSSheetColumnWidth[SD_PlenoRefill][CD_PlenoRefill_PriceIndexBorder] = clTSpread.ColWidthToLogUnits( 2 );
			m_mapSSheetColumnWidth[SD_PlenoRefill][CD_PlenoRefill_Name] = clTSpread.ColWidthToLogUnits( 14 );
			m_mapSSheetColumnWidth[SD_PlenoRefill][CD_PlenoRefill_NbreOfDevice] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_PlenoRefill][CD_PlenoRefill_MaxPressure] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_PlenoRefill][CD_PlenoRefill_TempRange] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_PlenoRefill][CD_PlenoRefill_Functions] = clTSpread.ColWidthToLogUnits( 14 );
			m_mapSSheetColumnWidth[SD_PlenoRefill][CD_PlenoRefill_Capacity] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_PlenoRefill][CD_PlenoRefill_MaxVolume] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_PlenoRefill][CD_PlenoRefill_MaxFlow] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_PlenoRefill][CD_PlenoRefill_SuplyWaterPressureRange] = clTSpread.ColWidthToLogUnits( 10 );
			m_mapSSheetColumnWidth[SD_PlenoRefill][CD_PlenoRefill_InConnectAndSize] = clTSpread.ColWidthToLogUnits( 18 );
			m_mapSSheetColumnWidth[SD_PlenoRefill][CD_PlenoRefill_OutConnectAndSize] = clTSpread.ColWidthToLogUnits( 15 );
			m_mapSSheetColumnWidth[SD_PlenoRefill][CD_PlenoRefill_Height] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_PlenoRefill][CD_PlenoRefill_WidthWoConnect] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_PlenoRefill][CD_PlenoRefill_WidthWConnect] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_PlenoRefill][CD_PlenoRefill_Weight] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_PlenoRefill][CD_PlenoRefill_Pointer] = clTSpread.ColWidthToLogUnits( 1 );
	}

	if( clTSpread.GetSafeHwnd() != NULL )
	{
		clTSpread.DestroyWindow();
	}

	return true;
}

bool CRViewSSelPM::IsSelectionReady( void )
{
	if( NULL == m_pclIndSelPMParams || NULL == m_pclIndSelPMParams->m_pclSelectPMList || NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser() 
			|| NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetPMWQSelectionPreferences() )
	{
		ASSERTA_RETURN( false );
	}

	bool bReturn = false;

	if( true == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->IsPressurisationSystemExist() )
	{
		// Particular case for Simply Compresso: If VN <= 80l, the integrated vessel is enough and we don't need secondary vessel.
		// Thus in this case only the TecBox Simply Compresso is needed to enable the 'Select' button. If 80 < VN <= 160l, the Simply
		// Compresso needs a secondary vessel. In this case both Simply Compresso and secondary vessel must be selected to enable
		// the 'Select' button.

		CDB_TecBox *pclTecBox = NULL;
		long lRow = -1;
		LPARAM lParam = NULL;
		GetTecBoxCompressoSelected( &pclTecBox, &lRow, &lParam );

		if( NULL != pclTecBox && NULL != dynamic_cast<CDB_Vessel*>( pclTecBox->GetIntegratedVesselIDPtr().MP ) 
				&& m_pclIndSelPMParams->m_pclSelectPMList->IsProductExist( CSelectPMList::PT_CompressoVessel, CSelectPMList::WL_Selected ) )
		{
			// Here we have a TecBox with an integrated vessel (Like Simply Compresso).
			CSelectedVssl *pclSelectedVessel = (CSelectedVssl *)m_pclIndSelPMParams->m_pclSelectPMList->GetFirstProduct( CSelectPMList::PT_CompressoVessel, 
					CSelectPMList::WL_Selected );
			
			if( 1 == pclSelectedVessel->GetNbreOfVsslNeeded() )
			{
				// There is no need of a secondary vessel, the integrated vessel is enough.
				bReturn = true;
			}
			else
			{
				// Secondary vessel exist.
				CDB_Vessel *pclSecondaryVessel = NULL;
				GetCompressoSecondaryVesselSelected( &pclSecondaryVessel, &lRow, &lParam );

				if( NULL != pclSecondaryVessel )
				{
					// The secondary vessel exists and it is selected.
					bReturn = true;
				} 
			}
		}
		else
		{
			CDB_Vessel *pclVessel = NULL;

			if( true == GetVesselSelected( &pclVessel ) && NULL != pclVessel )
			{
				bReturn = ( false == pclVessel->IsDeleted() );
			}
		}
	}
	else
	{
		long lRow = -1;
		LPARAM lParam = NULL;
		CDB_TBPlenoVento *pclTBPlenoVento = NULL;

		if( true == GetTecBoxVentoSelected( &pclTBPlenoVento, -1 ) && NULL != pclTBPlenoVento )
		{
			bReturn = ( false == pclTBPlenoVento->IsDeleted() );
		}
		else if( true == GetTecBoxPlenoSelected( &pclTBPlenoVento, -1, &lRow, &lParam ) && 
				( NULL != pclTBPlenoVento || NULL != lParam ) )
		{
			if( NULL != pclTBPlenoVento )
			{
				bReturn = ( false == pclTBPlenoVento->IsDeleted() );
			}
			else if( NULL != dynamic_cast<CSelectedWaterTreatmentCombination*>( (CSelectedPMBase*)lParam ) )
			{
				CSelectedWaterTreatmentCombination *pWT = dynamic_cast<CSelectedWaterTreatmentCombination*>( (CSelectedPMBase*)lParam );
				
				if( NULL != pWT->GetSelectedFirst() && NULL != pWT->GetSelectedFirst()->GetProductIDPtr().MP )
				{
					bReturn = ( false == ( (CData*)pWT->GetSelectedFirst()->GetProductIDPtr().MP )->IsDeleted() );
				}
				else if( NULL != pWT->GetSelectedSecond() && NULL != pWT->GetSelectedSecond()->GetProductIDPtr().MP )
				{
					bReturn = ( false == ( (CData*)pWT->GetSelectedSecond()->GetProductIDPtr().MP )->IsDeleted() );
				}
			}
		}

	}

	return bReturn;
}

void CRViewSSelPM::SetCurrentPMSelected( UINT uiSheetDescriptionID, UINT uiSDGroupID, CCellDescriptionProduct *pclCDCurrentPMSelected )
{
	// Try to retrieve sheet description linked to separator.
	CSheetDescription *pclSheetDescriptionPM = m_ViewDescription.GetFromSheetDescriptionID( _ConvertSD2SDG( uiSheetDescriptionID, uiSDGroupID ) );

	if( NULL != pclSheetDescriptionPM )
	{
		pclSheetDescriptionPM->SetUserVariable( _SDUV_SELECTEDPRODUCT, (LPARAM)pclCDCurrentPMSelected );
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PRIVATE MEMBERS
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CRViewSSelPM::_InitAndFillComputedDataRows()
{
	if( NULL == m_pclIndSelPMParams || NULL == m_pclIndSelPMParams->m_pTADS || NULL == m_pclIndSelPMParams->m_pTADS->GetpTechParams() 
			|| NULL == m_pclIndSelPMParams->m_pclSelectPMList || NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERT_RETURN;
	}

	CDS_TechnicalParameter *pclTechnicalParameters = m_pclIndSelPMParams->m_pTADS->GetpTechParams();

	// Call 'MultiSpreadBase' method to get a new sheet.
	CSheetDescription *pclSDComputedData = CreateSSheet( SD_ComputedData );

	if( NULL == pclSDComputedData || NULL == pclSDComputedData->GetSSheetPointer() )
	{
		return;
	}

	CPMInputUser *pclPMInputUser = m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser();
	CSSheet *pclSSheet = pclSDComputedData->GetSSheetPointer();

	pclSSheet->SetBool( SSB_REDRAW, FALSE );

	pclSSheet->Init();
	pclSSheet->SetBool( SSB_ALLOWDRAGDROP, FALSE );
	pclSSheet->SetBool( SSB_HORZSCROLLBAR, FALSE );
	pclSSheet->SetBool( SSB_VERTSCROLLBAR, FALSE );
	pclSSheet->SetBool( SSB_MOVEACTIVEONFOCUS, FALSE );
	pclSSheet->SetTextTip( SS_TT_STATUS_FLOATING, 250, NULL, _LIGHTYELLOW, _TAH_BLACK );

	// Set max rows.
	pclSSheet->SetMaxRows( RD_ComputedData_FirstAvailRow - 1 );

	// All cells are static by default and filled with _T("").
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	pclSSheet->FormatStaticText( -1, -1, -1, -1, _T( "" ) );

	// Increase row height.
	double RowHeight = 12.75;
	pclSSheet->SetRowHeight( RD_ComputedData_FirstRow, RowHeight * 0.5 );
	pclSSheet->SetRowHeight( RD_ComputedData_GroupName, RowHeight * 1.5 );
	pclSSheet->SetRowHeight( RD_ComputedData_ColName, RowHeight * 1.5 );

	// Initialize.
	pclSDComputedData->Init( 1, pclSSheet->GetMaxRows(), 1, CSheetDescription::SD_ParameterMode_Mono );
	pclSDComputedData->AddColumnInPixels( CD_ComputedData_FirstColumn, m_mapSSheetColumnWidth[SD_ComputedData][CD_ComputedData_FirstColumn] );
	pclSDComputedData->AddColumnInPixels( CD_ComputedData_Name, m_mapSSheetColumnWidth[SD_ComputedData][CD_ComputedData_Name] );
	pclSDComputedData->AddColumnInPixels( CD_ComputedData_Value, m_mapSSheetColumnWidth[SD_ComputedData][CD_ComputedData_Value] );
	pclSDComputedData->AddColumnInPixels( CD_ComputedData_ValueIndex, m_mapSSheetColumnWidth[SD_ComputedData][CD_ComputedData_ValueIndex] );
	pclSDComputedData->AddColumnInPixels( CD_ComputedData_ExpansionVessel, m_mapSSheetColumnWidth[SD_ComputedData][CD_ComputedData_ExpansionVessel] );
	pclSDComputedData->AddColumnInPixels( CD_ComputedData_ExpansionVesselIndex, m_mapSSheetColumnWidth[SD_ComputedData][CD_ComputedData_ExpansionVesselIndex] );
	pclSDComputedData->AddColumnInPixels( CD_ComputedData_Compresso, m_mapSSheetColumnWidth[SD_ComputedData][CD_ComputedData_Compresso] );
	pclSDComputedData->AddColumnInPixels( CD_ComputedData_CompressoIndex, m_mapSSheetColumnWidth[SD_ComputedData][CD_ComputedData_CompressoIndex] );
	pclSDComputedData->AddColumnInPixels( CD_ComputedData_Transfero, m_mapSSheetColumnWidth[SD_ComputedData][CD_ComputedData_Transfero] );
	pclSDComputedData->AddColumnInPixels( CD_ComputedData_TransferoIndex, m_mapSSheetColumnWidth[SD_ComputedData][CD_ComputedData_TransferoIndex] );

	// Set the focus column.
	pclSDComputedData->SetActiveColumn( CD_ComputedData_Name );

	// Set range for selection.
	pclSDComputedData->SetFocusColumnRange( CD_ComputedData_Name, CD_ComputedData_LastColumn );


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Add the main title.
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );
	int iBackColor = ( true == m_pclIndSelPMParams->m_bEditModeRunning ) ? _TAH_TITLE_MAIN_REEDIT : _TAH_TITLE_MAIN;
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, iBackColor );
	pclSDComputedData->SetUserVariable( _SDUV_TITLEFORECOLOR, ( pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor ) ) );
	pclSDComputedData->SetUserVariable( _SDUV_TITLEBACKCOLOR, ( pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor ) ) );

	pclSSheet->AddCellSpanW( CD_ComputedData_FirstColumn + 1, RD_ComputedData_GroupName, CD_ComputedData_LastColumn - CD_ComputedData_FirstColumn, 1 );
	pclSSheet->SetStaticText( CD_ComputedData_FirstColumn + 1, RD_ComputedData_GroupName, IDS_SSHEETSSELPM_CDGROUP );
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Column name.
	if( false == pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn ) )
	{
		return;
	}

	if( false == pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE ) )
	{
		return;
	}

	pclSSheet->AddCellSpanW( CD_ComputedData_Value, RD_ComputedData_ColName, 2, 1 );

	pclSSheet->SetStaticText( CD_ComputedData_ExpansionVessel, RD_ComputedData_ColName, IDS_SSHEETSSELPM_CDEXPANSIONVESSEL );
	pclSSheet->AddCellSpanW( CD_ComputedData_ExpansionVessel, RD_ComputedData_ColName, 2, 1 );

	pclSSheet->SetStaticText( CD_ComputedData_Compresso, RD_ComputedData_ColName, IDS_SSHEETSSELPM_CDCOMPRESSO );
	pclSSheet->AddCellSpanW( CD_ComputedData_Compresso, RD_ComputedData_ColName, 2, 1 );

	pclSSheet->SetStaticText( CD_ComputedData_Transfero, RD_ComputedData_ColName, IDS_SSHEETSSELPM_CDTRANSFERO );
	pclSSheet->AddCellSpanW( CD_ComputedData_Transfero, RD_ComputedData_ColName, 2, 1 );
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	// Draw black line below column header.
	pclSSheet->SetCellBorder( CD_ComputedData_Name, RD_ComputedData_ColName, CD_ComputedData_LastColumn, RD_ComputedData_ColName, true, SS_BORDERTYPE_BOTTOM );

	pclSDComputedData->RestartRemarkGenerator();

	// Add needed rows.
	// 'false' to specify that these rows can't be selected.
	pclSDComputedData->AddRows( RD_ComputedData_LastRow - RD_ComputedData_FirstAvailRow, false );

	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();

	bool bIsForCooling = ( ProjectType::Cooling == pclPMInputUser->GetApplicationType() ) ? true : false;

	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_LEFT );
	pclSSheet->SetStaticText( CD_ComputedData_Name, RD_ComputedData_SystemExpansionCoeff, TASApp.LoadLocalizedString( IDS_SSHEETSSELPM_CDSYSTEMEXPCOEFF ) );
	
	// Factor [X].
	pclSSheet->SetStaticText( CD_ComputedData_Name, RD_ComputedData_XFactor, TASApp.LoadLocalizedString( IDS_SSHEETSSELPM_CDXFACTOR ) );
	
	// System expansion volume.
	int iStartIndex = 1;
	int iEndIndex = 1;
	CString str;

	if( true == pclPMInputUser->IsNorm( ProjectType::All, PressurisationNorm::PN_SWKIHE301_01 ) )
	{
		// System expansion volume [e.Vs.X]
		str = TASApp.LoadLocalizedString( IDS_SSHEETSSELPM_CDSYSTEMEXPVOLSWKI );
	}
	else
	{
		// System expansion volume [Ve]
		str = TASApp.LoadLocalizedString( IDS_SSHEETSSELPM_CDSYSTEMEXPVOL );
	}

	pclSSheet->SetStaticText( CD_ComputedData_Name, RD_ComputedData_SystemExpansionVolume, str );
	iEndIndex++;
	
	if( true == pclPMInputUser->IsNorm( ProjectType::All, PressurisationNorm::PN_SWKIHE301_01 ) 
			&& pclPMInputUser->GetStorageTankVolume() > 0.0 )
	{
		// HYS-1534: Storage expansion coefficient [esto]
		pclSSheet->SetStaticText( CD_ComputedData_Name, RD_ComputedData_StorageTankExpCoeff, TASApp.LoadLocalizedString( IDS_SSHEETSSELPM_CDSTORAGETANKEXPCOEFF ) );

		// HYS-1534: Storage expansion volume [Vsto.esto].
		pclSSheet->SetStaticText( CD_ComputedData_Name, RD_ComputedData_StorageTankExpansionVolume, TASApp.LoadLocalizedString( IDS_SSHEETSSELPM_CDSTORAGETANKEXPANSION ) );
		iEndIndex++;
	}
	
	// Total expansion volume [Ve,tot] right aligned and bold.
	// Ve,tot = Ve + storage tank expansion if exist (Only in SWKI HE301-01 norm).
	str = TASApp.LoadLocalizedString( IDS_SSHEETSSELPM_CDTOTALEXPANSION ) + _T(" ") + _GetSumOfIndexes( iStartIndex, iEndIndex, _T("=") );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );
	pclSSheet->SetStaticText( CD_ComputedData_Name, RD_ComputedData_TotalExpansionVolume, str );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

	iStartIndex = iEndIndex;
	iEndIndex++;

	if( ProjectType::Solar == pclPMInputUser->GetApplicationType() )
	{
		// Solar collector multiplier factor.
		pclSSheet->SetStaticText( CD_ComputedData_Name, RD_ComputedData_SolarCollectorMultiplierFactor, TASApp.LoadLocalizedString( IDS_SSHEETSSELPM_CDSOLARCOLLECTORMULTIPLIERFACTOR ) );

		// Solar collector security volume [VDK].
		pclSSheet->SetStaticText( CD_ComputedData_Name, RD_ComputedData_SolarCollectorSecurityVolume, TASApp.LoadLocalizedString( IDS_SSHEETSSELPM_CDSOLARCOLLECTORSECURITYVOLUME ) );
		iEndIndex++;
	}

	if( false == pclPMInputUser->IsNorm( ProjectType::All, PressurisationNorm::PN_SWKIHE301_01 ) )
	{
		// HYS-1022: 'Vwr' becomes 'Vwr,min'.
		// Remark: Only for other norm than SWKI HE301-01 norm. For SWKI HE301-01 the Vwr,min is included in Ve with the X factor.
		pclSSheet->SetStaticText( CD_ComputedData_Name, RD_ComputedData_MinWaterReserveToAdd, TASApp.LoadLocalizedString( IDS_SSHEETSSELPM_CDMINWATERRESERVE ) );
		iEndIndex++;
	}

	if( BST_CHECKED == pclPMInputUser->GetDegassingChecked() )
	{
		// HYS-1565: Add also the degassing water reserve.
		pclSSheet->SetStaticText( CD_ComputedData_Name, RD_ComputedData_DegassingWaterReserve, TASApp.LoadLocalizedString( IDS_SSHEETSSELPM_CDDEGASSINGWATERRESERVE ) );
		iEndIndex++;
	}
	
	// HYS-1565: Add also the vessel net volume [Vn] right aligned and bold.
	str = TASApp.LoadLocalizedString( IDS_SSHEETSSELPM_CDVESSELNETVOLUME ) + _T(" ") + _GetSumOfIndexes( iStartIndex, iEndIndex, _T("=") );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );
	pclSSheet->SetStaticText( CD_ComputedData_Name, RD_ComputedData_VesselNetVolume, str );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

	// Pressure factor.
	pclSSheet->SetStaticText( CD_ComputedData_Name, RD_ComputedData_PressureFactor, TASApp.LoadLocalizedString( IDS_SSHEETSSELPM_CDPRESSFACTOR ) );

	// Required nominal volume right aligned and bold.
	str = TASApp.LoadLocalizedString( IDS_SSHEETSSELPM_CDNOMVOL );
	str.Format( _T("%s (=%ix%i)"), str, iEndIndex, iEndIndex + 1 );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );
	pclSSheet->SetStaticText( CD_ComputedData_Name, RD_ComputedData_NominalVolume, str );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

	// HYS-1565: Add information of the minimal water reserve included in the expansion volume for the SWKI HE301-01 norm.
	pclSSheet->SetStaticText( CD_ComputedData_Name, RD_ComputedData_MinWaterReserveIncludedInExpansion, TASApp.LoadLocalizedString( IDS_SSHEETSSELPM_CDMINWATERRESERVE ) );

	pclSSheet->SetStaticText( CD_ComputedData_Name, RD_ComputedData_ContractedVolume, TASApp.LoadLocalizedString( IDS_SSHEETSSELPM_CDCONTVOL ) );

	// HYS-1343 : We show the contraction volume only for expansion vessel.
	if( false == bIsForCooling || ( CPMInputUser::MaintenanceType::MT_All != pclPMInputUser->GetPressureMaintenanceType()
			&& CPMInputUser::MaintenanceType::MT_ExpansionVessel != pclPMInputUser->GetPressureMaintenanceType()
			&& CPMInputUser::MaintenanceType::MT_ExpansionVesselWithMembrane != pclPMInputUser->GetPressureMaintenanceType() ) )
	{
		pclSSheet->ShowRow( RD_ComputedData_ContractedVolume , FALSE );
	}

	pclSSheet->SetStaticText( CD_ComputedData_Name, RD_ComputedData_VaporPressure, TASApp.LoadLocalizedString( IDS_SSHEETSSELPM_CDVAPOURPRESS ) );
	pclSSheet->SetStaticText( CD_ComputedData_Name, RD_ComputedData_Pz, TASApp.LoadLocalizedString( IDS_SSHEETSSELPM_CDPZ ) );
	pclSSheet->SetStaticText( CD_ComputedData_Name, RD_ComputedData_MinimumPressure, TASApp.LoadLocalizedString( IDS_SSHEETSSELPM_CDMINPRESS ) );
	
	// HYS-1022: 'pa' becomes 'pa,min. Thus this variable is now common to vessel, Compresso and Transfero.
	// HYS-1116: We show minimum initial pressure only when we are with the EN12828 norm.
	if( true == pclPMInputUser->IsNorm( ProjectType::Heating, PressurisationNorm::PN_EN12828 ) )
	{
		pclSSheet->SetStaticText( CD_ComputedData_Name, RD_ComputedData_MinimumInitialPressure, TASApp.LoadLocalizedString( IDS_SSHEETSSELPM_CDMININITIALPRESSURE ) );
		pclSSheet->ShowRow( RD_ComputedData_InitialPressure, FALSE );
	}
	else
	{
		pclSSheet->ShowRow( RD_ComputedData_MinimumInitialPressure, FALSE );
	}

	// HYS-1116: And in all cases we show initial pressure for Compresso/Transfero.
	pclSSheet->SetStaticText( CD_ComputedData_Name, RD_ComputedData_InitialPressure, TASApp.LoadLocalizedString( IDS_SSHEETSSELPM_CDINITIALPRESSURE ) );

	pclSSheet->SetStaticText( CD_ComputedData_Name, RD_ComputedData_MinimumPSV, TASApp.LoadLocalizedString( IDS_SSHEETSSELPM_CDMINPSV ) );
	pclSSheet->SetStaticText( CD_ComputedData_Name, RD_ComputedData_TargetPressure, TASApp.LoadLocalizedString( IDS_SSHEETSSELPM_CDTARGETPRESS ) );
	pclSSheet->SetStaticText( CD_ComputedData_Name, RD_ComputedData_FinalPressure, TASApp.LoadLocalizedString( IDS_SSHEETSSELPM_CDFINALPRESS ) );

	pclSSheet->SetStaticText( CD_ComputedData_Name, RD_ComputedData_NeededQRateEqVol, TASApp.LoadLocalizedString( IDS_SSHEETSSELPM_CDNEEDEDQRATEEQVOL ) );

	CString strlength = WriteCUDouble( _U_LENGTH, 10, true, 0 );
	FormatString( str, IDS_SSHEETSSELPM_CDEXPPIPE, strlength );
	pclSSheet->SetStaticText( CD_ComputedData_Name, RD_ComputedData_ExpansionPipeBlw10, str );

	strlength = WriteCUDouble( _U_LENGTH, 30, true, 0 );
	FormatString( str, IDS_SSHEETSSELPM_CDEXPPIPE, strlength );
	pclSSheet->SetStaticText( CD_ComputedData_Name, RD_ComputedData_ExpansionPipeAbv10, str );

	pclSSheet->SetCellBorder( CD_ComputedData_Name, RD_ComputedData_SystemExpansionCoeff, CD_ComputedData_LastColumn, RD_ComputedData_SystemExpansionCoeff, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );
	pclSSheet->SetCellBorder( CD_ComputedData_Name, RD_ComputedData_XFactor, CD_ComputedData_LastColumn, RD_ComputedData_XFactor, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );
	pclSSheet->SetCellBorder( CD_ComputedData_Name, RD_ComputedData_SystemExpansionVolume, CD_ComputedData_LastColumn, RD_ComputedData_SystemExpansionVolume, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );
	pclSSheet->SetCellBorder( CD_ComputedData_Name, RD_ComputedData_StorageTankExpCoeff, CD_ComputedData_LastColumn, RD_ComputedData_StorageTankExpCoeff, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );
	pclSSheet->SetCellBorder( CD_ComputedData_Name, RD_ComputedData_StorageTankExpansionVolume, CD_ComputedData_LastColumn, RD_ComputedData_StorageTankExpansionVolume, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );
	pclSSheet->SetCellBorder( CD_ComputedData_Name, RD_ComputedData_TotalExpansionVolume, CD_ComputedData_LastColumn, RD_ComputedData_TotalExpansionVolume, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );
	pclSSheet->SetCellBorder( CD_ComputedData_Name, RD_ComputedData_SolarCollectorMultiplierFactor, CD_ComputedData_LastColumn, RD_ComputedData_SolarCollectorMultiplierFactor, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );
	pclSSheet->SetCellBorder( CD_ComputedData_Name, RD_ComputedData_SolarCollectorSecurityVolume, CD_ComputedData_LastColumn, RD_ComputedData_SolarCollectorSecurityVolume, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );
	pclSSheet->SetCellBorder( CD_ComputedData_Name, RD_ComputedData_MinWaterReserveToAdd, CD_ComputedData_LastColumn, RD_ComputedData_MinWaterReserveToAdd, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );
	pclSSheet->SetCellBorder( CD_ComputedData_Name, RD_ComputedData_DegassingWaterReserve, CD_ComputedData_LastColumn, RD_ComputedData_DegassingWaterReserve, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );
	pclSSheet->SetCellBorder( CD_ComputedData_Name, RD_ComputedData_VesselNetVolume, CD_ComputedData_LastColumn, RD_ComputedData_VesselNetVolume, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );
	pclSSheet->SetCellBorder( CD_ComputedData_Name, RD_ComputedData_PressureFactor, CD_ComputedData_LastColumn, RD_ComputedData_PressureFactor, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );
	pclSSheet->SetCellBorder( CD_ComputedData_Name, RD_ComputedData_NominalVolume, CD_ComputedData_LastColumn, RD_ComputedData_NominalVolume, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );
	pclSSheet->SetCellBorder( CD_ComputedData_Name, RD_ComputedData_MinWaterReserveIncludedInExpansion, CD_ComputedData_LastColumn, RD_ComputedData_MinWaterReserveIncludedInExpansion, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );
	pclSSheet->SetCellBorder( CD_ComputedData_Name, RD_ComputedData_ContractedVolume, CD_ComputedData_LastColumn, RD_ComputedData_ContractedVolume, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );
	pclSSheet->SetCellBorder( CD_ComputedData_Name, RD_ComputedData_VaporPressure, CD_ComputedData_LastColumn, RD_ComputedData_VaporPressure, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );
	pclSSheet->SetCellBorder( CD_ComputedData_Name, RD_ComputedData_Pz, CD_ComputedData_LastColumn, RD_ComputedData_Pz, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );
	pclSSheet->SetCellBorder( CD_ComputedData_Name, RD_ComputedData_MinimumPressure, CD_ComputedData_LastColumn, RD_ComputedData_MinimumPressure, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );
	pclSSheet->SetCellBorder( CD_ComputedData_Name, RD_ComputedData_MinimumInitialPressure, CD_ComputedData_LastColumn, RD_ComputedData_MinimumInitialPressure, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );
	pclSSheet->SetCellBorder( CD_ComputedData_Name, RD_ComputedData_InitialPressure, CD_ComputedData_LastColumn, RD_ComputedData_InitialPressure, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );
	pclSSheet->SetCellBorder( CD_ComputedData_Name, RD_ComputedData_TargetPressure, CD_ComputedData_LastColumn, RD_ComputedData_TargetPressure, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );
	pclSSheet->SetCellBorder( CD_ComputedData_Name, RD_ComputedData_FinalPressure, CD_ComputedData_LastColumn, RD_ComputedData_FinalPressure, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );
	pclSSheet->SetCellBorder( CD_ComputedData_Name, RD_ComputedData_MinimumPSV, CD_ComputedData_LastColumn, RD_ComputedData_MinimumPSV, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );
	pclSSheet->SetCellBorder( CD_ComputedData_Name, RD_ComputedData_NeededQRateEqVol, CD_ComputedData_LastColumn, RD_ComputedData_NeededQRateEqVol, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );
	pclSSheet->SetCellBorder( CD_ComputedData_Name, RD_ComputedData_ExpansionPipeBlw10, CD_ComputedData_LastColumn, RD_ComputedData_ExpansionPipeBlw10, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );
	pclSSheet->SetCellBorder( CD_ComputedData_Name, RD_ComputedData_ExpansionPipeAbv10, CD_ComputedData_LastColumn, RD_ComputedData_ExpansionPipeAbv10, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );

	// Hide not needed rows.
	if( true == pclPMInputUser->IsNorm( ProjectType::All, PressurisationNorm::PN_SWKIHE301_01 ) )
	{
		// If user has not defined the storage volume, we don't show information.
		if( pclPMInputUser->GetStorageTankVolume() <= 0.0 )
		{
			pclSSheet->ShowRow( RD_ComputedData_StorageTankExpCoeff, FALSE );
			pclSSheet->ShowRow( RD_ComputedData_StorageTankExpansionVolume, FALSE );
		}
	}
	else
	{
		// In all other cases we don't show the X factor row.
		pclSSheet->ShowRow( RD_ComputedData_XFactor, FALSE );

		pclSSheet->ShowRow( RD_ComputedData_StorageTankExpCoeff, FALSE );
		pclSSheet->ShowRow( RD_ComputedData_StorageTankExpansionVolume, FALSE );
	}

	if( ProjectType::Solar != pclPMInputUser->GetApplicationType() )
	{
		pclSSheet->ShowRow( RD_ComputedData_SolarCollectorMultiplierFactor, FALSE );
		pclSSheet->ShowRow( RD_ComputedData_SolarCollectorSecurityVolume, FALSE );
	}

	// HYS-1565: don't keep minimal water reserve if we are in SWKI HE301-01 norm because this one is already included 
	// in Ve (= e.Vs.X) thanks to the X factor.
	if( true == pclPMInputUser->IsNorm( ProjectType::All, PressurisationNorm::PN_SWKIHE301_01 ) )
	{
		pclSSheet->ShowRow( RD_ComputedData_MinWaterReserveToAdd, FALSE );
	}

	// HYS-1565: If no degassing, we can hide this row.
	if( BST_UNCHECKED == pclPMInputUser->GetDegassingChecked() )
	{
		pclSSheet->ShowRow( RD_ComputedData_DegassingWaterReserve, FALSE );
	}

	// HYS-1565: Don't show minimal water reserve after the vessel net volume if we are not in the SWKI HE301-01 norm.
	// Because in this case, this value is already shown before the vessel net volume.
	if( false == pclPMInputUser->IsNorm( ProjectType::All, PressurisationNorm::PN_SWKIHE301_01 ) )
	{
		pclSSheet->ShowRow( RD_ComputedData_MinWaterReserveIncludedInExpansion, FALSE );
	}

	// System expansion coefficient.
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_RIGHT );
	pclSSheet->SetStaticText( CD_ComputedData_Value, RD_ComputedData_SystemExpansionCoeff, WriteDouble( pclPMInputUser->GetSystemExpansionCoefficient(), 4, 3 ) );

	if( true == pclPMInputUser->IsNorm( ProjectType::All, PressurisationNorm::PN_SWKIHE301_01 ) )
	{
		// X factor.
		pclSSheet->SetStaticText( CD_ComputedData_Value, RD_ComputedData_XFactor, WriteDouble( pclPMInputUser->GetXFactorSWKI(), 3, 1 ) );
	}

	iStartIndex = 1;
	iEndIndex = 1;

	// System expansion volume.
	pclSSheet->SetStaticText( CD_ComputedData_Value, RD_ComputedData_SystemExpansionVolume, WriteCUDouble( _U_VOLUME, pclPMInputUser->GetSystemExpansionVolume(), true ) );

	// Index.
	long lCurrentFontSize = pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontSize );
	long lIndexFontSize = lCurrentFontSize - 1;
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontSize, (LPARAM)lIndexFontSize );
	pclSSheet->SetStaticText( CD_ComputedData_ValueIndex, RD_ComputedData_SystemExpansionVolume, _GetSumOfIndexes( iEndIndex, iEndIndex ) );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontSize, (LPARAM)lCurrentFontSize );
	iEndIndex++;

	if( true == pclPMInputUser->IsNorm( ProjectType::All, PressurisationNorm::PN_SWKIHE301_01 ) && pclPMInputUser->GetStorageTankVolume() > 0.0 )
	{
		// Storage tank expansion coefficient.
		pclSSheet->SetStaticText( CD_ComputedData_Value, RD_ComputedData_StorageTankExpCoeff, WriteDouble( pclPMInputUser->GetStorageTankExpansionCoefficient(), 4, 3 ) );

		// Storage tank expansion volume.
		pclSSheet->SetStaticText( CD_ComputedData_Value, RD_ComputedData_StorageTankExpansionVolume, WriteCUDouble( _U_VOLUME, pclPMInputUser->GetStorageTankExpansionVolume(), true ) );

		// Index.
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontSize, (LPARAM)lIndexFontSize );
		pclSSheet->SetStaticText( CD_ComputedData_ValueIndex, RD_ComputedData_StorageTankExpansionVolume, _GetSumOfIndexes( iEndIndex, iEndIndex ) );
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontSize, (LPARAM)lCurrentFontSize );
		iEndIndex++;
	}

	// Total expansion volume in bold.
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );
	pclSSheet->SetStaticText( CD_ComputedData_Value, RD_ComputedData_TotalExpansionVolume, WriteCUDouble( _U_VOLUME, pclPMInputUser->GetTotalExpansionVolume(), true ) );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

	// Index.
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontSize, (LPARAM)lIndexFontSize );
	pclSSheet->SetStaticText( CD_ComputedData_ValueIndex, RD_ComputedData_TotalExpansionVolume, _GetSumOfIndexes( iEndIndex, iEndIndex ) );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontSize, (LPARAM)lCurrentFontSize );

	iStartIndex = iEndIndex;
	iEndIndex++;

	if( ProjectType::Solar == pclPMInputUser->GetApplicationType() )
	{
		// Solar collector multiplier factor.
		pclSSheet->SetStaticText( CD_ComputedData_Value, RD_ComputedData_SolarCollectorMultiplierFactor, WriteDouble( pclPMInputUser->GetSolarCollectorMultiplierFactor(), 2, 1 ) );

		// Solar collector expansion volume.
		pclSSheet->SetStaticText( CD_ComputedData_Value, RD_ComputedData_SolarCollectorSecurityVolume, WriteCUDouble( _U_VOLUME, pclPMInputUser->GetSolarCollectorSecurityVolume(), true ) );

		// Index.
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontSize, (LPARAM)lIndexFontSize );
		pclSSheet->SetStaticText( CD_ComputedData_ValueIndex, RD_ComputedData_SolarCollectorSecurityVolume, _GetSumOfIndexes( iEndIndex, iEndIndex ) );
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontSize, (LPARAM)lCurrentFontSize );
		iEndIndex++;
	}

	// When not in SWKI HE301-01 norm, we show 'Vwr,min' before 'Vessel net volume' because because it is not included in Ve as it's the case for the SWKI HE301-01 norm.
	if( false == pclPMInputUser->IsNorm( ProjectType::All, PressurisationNorm::PN_SWKIHE301_01 ) )
	{
		pclSSheet->SetStaticText( CD_ComputedData_Value, RD_ComputedData_MinWaterReserveToAdd, WriteCUDouble( _U_VOLUME, pclPMInputUser->GetMinimumWaterReserve(), true ) );

		// Index.
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontSize, (LPARAM)lIndexFontSize );
		pclSSheet->SetStaticText( CD_ComputedData_ValueIndex, RD_ComputedData_MinWaterReserveToAdd, _GetSumOfIndexes( iEndIndex, iEndIndex ) );
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontSize, (LPARAM)lCurrentFontSize );
		iEndIndex++;
	}

	// HYS-1565: if degassing is needed.
	if( BST_CHECKED == pclPMInputUser->GetDegassingChecked() )
	{
		pclSSheet->SetStaticText( CD_ComputedData_Value, RD_ComputedData_DegassingWaterReserve, WriteCUDouble( _U_VOLUME, pclPMInputUser->GetDegassingWaterReserve(), true ) );

		// Index.
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontSize, (LPARAM)lIndexFontSize );
		pclSSheet->SetStaticText( CD_ComputedData_ValueIndex, RD_ComputedData_DegassingWaterReserve, _GetSumOfIndexes( iEndIndex, iEndIndex ) );
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontSize, (LPARAM)lCurrentFontSize );
		iEndIndex++;
	}

	// HYS-1565: Show the vessel net volume in bold.
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );
	pclSSheet->SetStaticText( CD_ComputedData_Value, RD_ComputedData_VesselNetVolume, WriteCUDouble( _U_VOLUME, pclPMInputUser->GetVesselNetVolume(), true ) );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

	// Index.
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontSize, (LPARAM)lIndexFontSize );
	pclSSheet->SetStaticText( CD_ComputedData_ValueIndex, RD_ComputedData_VesselNetVolume, _GetSumOfIndexes( iEndIndex, iEndIndex ) );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontSize, (LPARAM)lCurrentFontSize );
	iEndIndex++;

	// Verify first validity of PSV for Compresso and Transfero.
	bool bBadPSVCompresso = false;
	double dMinPSVCompresso = pclPMInputUser->GetMinimumRequiredPSVRounded(CPMInputUser::MT_WithCompressor);
	
	if( pclPMInputUser->GetSafetyValveResponsePressure() < dMinPSVCompresso )
	{
		bBadPSVCompresso = true;
	}

	bool bBadPSVTransfero = false;
	double dMinPSVTransfero = pclPMInputUser->GetMinimumRequiredPSVRounded(CPMInputUser::MT_WithPump);
	
	if( pclPMInputUser->GetSafetyValveResponsePressure() < dMinPSVTransfero )
	{
		bBadPSVTransfero = true;
	}

	// Verify also if vessel is OK with the safety valve response pressure.
	bool bBadPSVVessel = false;
	double dMinPSVVessel = pclPMInputUser->GetMinimumRequiredPSVRounded( CPMInputUser::MT_ExpansionVessel );

	if( pclPMInputUser->GetSafetyValveResponsePressure() < dMinPSVVessel )
	{
		bBadPSVVessel = true;
	}

	// Pressure factor for vessel.
	str = _T("");

	if( false == bBadPSVVessel )
	{
		str = WriteDouble( pclPMInputUser->GetPressureFactor(), 3, 2 );
	}

	pclSSheet->SetStaticText( CD_ComputedData_ExpansionVessel, RD_ComputedData_PressureFactor, str );

	// Index.
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontSize, (LPARAM)lIndexFontSize );
	pclSSheet->SetStaticText( CD_ComputedData_ExpansionVesselIndex, RD_ComputedData_PressureFactor, _GetSumOfIndexes( iEndIndex, iEndIndex ) );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontSize, (LPARAM)lCurrentFontSize );

	// Pressure factor for Compresso.
	str = _T("");

	if( false == bBadPSVCompresso )
	{
		str = WriteDouble( pclPMInputUser->GetPressureFactor( true ), 3, 2 );
	}

	pclSSheet->SetStaticText( CD_ComputedData_Compresso, RD_ComputedData_PressureFactor, str );

	// Index.
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontSize, (LPARAM)lIndexFontSize );
	pclSSheet->SetStaticText( CD_ComputedData_CompressoIndex, RD_ComputedData_PressureFactor, _GetSumOfIndexes( iEndIndex, iEndIndex ) );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontSize, (LPARAM)lCurrentFontSize );

	// Pressure factor for Transfero.
	str = _T("");
	
	if( false == bBadPSVTransfero )
	{
		str = WriteDouble( pclPMInputUser->GetPressureFactor( true ), 3, 2 );
	}
	
	pclSSheet->SetStaticText( CD_ComputedData_Transfero, RD_ComputedData_PressureFactor, str );

	// Index.
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontSize, (LPARAM)lIndexFontSize );
	pclSSheet->SetStaticText( CD_ComputedData_TransferoIndex, RD_ComputedData_PressureFactor, _GetSumOfIndexes( iEndIndex, iEndIndex ) );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontSize, (LPARAM)lCurrentFontSize );

	// Nominal volume in bold.
	str = ( false == bBadPSVVessel ) ? WriteCUDouble( _U_VOLUME, pclPMInputUser->GetVesselNominalVolume(), true ) : _T("");
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );
	pclSSheet->SetStaticText( CD_ComputedData_ExpansionVessel, RD_ComputedData_NominalVolume, str );

	str = ( false == bBadPSVCompresso ) ? WriteCUDouble( _U_VOLUME, pclPMInputUser->GetVesselNominalVolume( true ), true ) : _T("");
	pclSSheet->SetStaticText( CD_ComputedData_Compresso, RD_ComputedData_NominalVolume, str );
	
	str = ( false == bBadPSVTransfero ) ? WriteCUDouble( _U_VOLUME, pclPMInputUser->GetVesselNominalVolume( true ), true ) : _T("");
	pclSSheet->SetStaticText( CD_ComputedData_Transfero, RD_ComputedData_NominalVolume, str );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

	// For the SWKI HE301-01 norm, we show 'Vwr,min' after 'Total expansion volume' because it is already included in 'Ve,tot' because the X-Factor.
	if( true == pclPMInputUser->IsNorm( ProjectType::All, PressurisationNorm::PN_SWKIHE301_01 ) )
	{
		pclSSheet->SetStaticText( CD_ComputedData_Value, RD_ComputedData_MinWaterReserveIncludedInExpansion, WriteCUDouble( _U_VOLUME, pclPMInputUser->GetMinimumWaterReserve(), true ) );
	}

	if( true == bIsForCooling )
	{
		// HYS-1343 : We show the contraction volume only for expansion vessel.
		if( CPMInputUser::MaintenanceType::MT_All == pclPMInputUser->GetPressureMaintenanceType()
				|| CPMInputUser::MaintenanceType::MT_ExpansionVessel == pclPMInputUser->GetPressureMaintenanceType()
				|| CPMInputUser::MaintenanceType::MT_ExpansionVesselWithMembrane == pclPMInputUser->GetPressureMaintenanceType() )
		{
			pclSSheet->SetStaticText( CD_ComputedData_Value, RD_ComputedData_ContractedVolume, WriteCUDouble( _U_VOLUME, pclPMInputUser->ComputeContractionVolume(), true ) );
		}
	}

	pclSSheet->SetStaticText( CD_ComputedData_Value, RD_ComputedData_VaporPressure, WriteCUDouble( _U_PRESSURE, pclPMInputUser->GetVaporPressure(), true ) );
	
	if( BST_CHECKED == pclPMInputUser->GetPzChecked() )
	{
		pclSSheet->SetStaticText( CD_ComputedData_Value, RD_ComputedData_Pz, WriteCUDouble( _U_PRESSURE, pclPMInputUser->GetPz(), true ) );
	}
	else
	{
		pclSSheet->SetStaticText( CD_ComputedData_Value, RD_ComputedData_Pz, _T("-") );
	}

	pclSSheet->SetStaticText( CD_ComputedData_Value, RD_ComputedData_MinimumPressure, WriteCUDouble( _U_PRESSURE, pclPMInputUser->GetMinimumPressure(), true ) );

	// Minimum initial pressure.
	if( true == pclPMInputUser->IsNorm( ProjectType::All, PressurisationNorm::PN_EN12828 ) )
	{
		// HYS-1116: When using EN12828 norm, we display the minimum initial pressure for all pressure maintenance types (Statico, Compresso and Transfero).
		pclSSheet->SetStaticText( CD_ComputedData_Value, RD_ComputedData_MinimumInitialPressure, WriteCUDouble( _U_PRESSURE, pclPMInputUser->GetMinimumInitialPressure(), true ) );
	}
	else
	{
		// HYS-1116: In all cases we show the initial pressure for Compresso and Transfero.
		// Remark: For Compresso and Transfero we have the initial pressure set exactly in the same way as the minimum initial pressure for EN12828 -> p0 + 0.3 bar.
		//         This is why we call here the 'GetMinimumInitialPressure' method.
		str = ( false == bBadPSVCompresso ) ? WriteCUDouble( _U_PRESSURE, pclPMInputUser->GetMinimumInitialPressure(), true ) : _T("");
		pclSSheet->SetStaticText( CD_ComputedData_Compresso, RD_ComputedData_InitialPressure, str );

		str = ( false == bBadPSVTransfero ) ? WriteCUDouble( _U_PRESSURE, pclPMInputUser->GetMinimumInitialPressure(), true ) : _T("");
		pclSSheet->SetStaticText( CD_ComputedData_Transfero, RD_ComputedData_InitialPressure, str );
	}

	// Target pressure.
	str = ( false == bBadPSVCompresso ) ? WriteCUDouble( _U_PRESSURE, pclPMInputUser->GetTargetPressureForTecBox( CDB_TecBox::etbtCompresso ), true ) : _T("");
	pclSSheet->SetStaticText( CD_ComputedData_Compresso, RD_ComputedData_TargetPressure, str );

	str = ( false == bBadPSVTransfero ) ? WriteCUDouble( _U_PRESSURE, pclPMInputUser->GetTargetPressureForTecBox( CDB_TecBox::etbtTransfero ), true ) : _T("");
	pclSSheet->SetStaticText( CD_ComputedData_Transfero, RD_ComputedData_TargetPressure, str );

	// Final pressure.
	str = ( false == bBadPSVVessel ) ? WriteCUDouble( _U_PRESSURE, pclPMInputUser->GetFinalPressure(), true ) : _T("");
	pclSSheet->SetStaticText( CD_ComputedData_ExpansionVessel, RD_ComputedData_FinalPressure, str );

	str = ( false == bBadPSVCompresso ) ? WriteCUDouble( _U_PRESSURE, pclPMInputUser->GetFinalPressure( CDB_TecBox::etbtCompresso ), true ) : _T("");
	pclSSheet->SetStaticText( CD_ComputedData_Compresso, RD_ComputedData_FinalPressure, str );
	
	str = ( false == bBadPSVTransfero ) ? WriteCUDouble( _U_PRESSURE, pclPMInputUser->GetFinalPressure( CDB_TecBox::etbtTransfero ), true ) : _T("");
	pclSSheet->SetStaticText( CD_ComputedData_Transfero, RD_ComputedData_FinalPressure, str );

	// Minimum required PSV.
	if( true == bBadPSVVessel )
	{
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_RED );
	}

	pclSSheet->SetStaticText( CD_ComputedData_ExpansionVessel, RD_ComputedData_MinimumPSV, WriteCUDouble( _U_PRESSURE, dMinPSVVessel, true ) );
	pclSSheet->SetTextPatternProperty(CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK);

	if( true == bBadPSVCompresso )
	{
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_RED );
	}

	pclSSheet->SetStaticText( CD_ComputedData_Compresso, RD_ComputedData_MinimumPSV, WriteCUDouble( _U_PRESSURE, dMinPSVCompresso, true ) );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

	if( true == bBadPSVTransfero )
	{
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_RED );
	}
	
	pclSSheet->SetStaticText( CD_ComputedData_Transfero, RD_ComputedData_MinimumPSV, WriteCUDouble( _U_PRESSURE, dMinPSVTransfero, true ) );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

	// VD.
	str = ( false == bBadPSVCompresso ) ? WriteCUDouble( _U_FLOW, pclPMInputUser->GetVD(), true ) : _T("");
	pclSSheet->SetStaticText( CD_ComputedData_Compresso, RD_ComputedData_NeededQRateEqVol, str );

	str = ( false == bBadPSVTransfero ) ? WriteCUDouble( _U_FLOW, pclPMInputUser->GetVD(), true ) : _T("");
	pclSSheet->SetStaticText( CD_ComputedData_Transfero, RD_ComputedData_NeededQRateEqVol, str );
	pclSSheet->AddCellSpan( CD_ComputedData_Compresso, RD_ComputedData_NeededQRateEqVol, 2, 1 );


	pclSSheet->SetCellBorder( CD_ComputedData_Name, RD_ComputedData_LastRow, CD_ComputedData_LastColumn, RD_ComputedData_LastRow, true, 
			SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );

	// Add Remarks.
	// Note: these lines must be placed after creating Show/Hide all priorities button to avoid to take in count these lines when defining
	//       range of row to be Show/Hide.
	pclSDComputedData->WriteRemarks( RD_ComputedData_LastRow, CD_ComputedData_Name, CD_ComputedData_LastColumn );

	// Add possibility to change column size.
	// Remark: place this code after any initialization (like adding, removing, resizing, hiding/showing columns).
	CResizingColumnInfo *pclResizingColumnInfo = pclSDComputedData->GetResizingColumnInfo();
	pclResizingColumnInfo->ActivateFeature( true, false, RD_ComputedData_FirstAvailRow, RD_ComputedData_LastRow - 1 );
	pclResizingColumnInfo->AddRangeColumn( CD_ComputedData_Name, CD_ComputedData_LastColumn, RD_ComputedData_ColName, RD_ComputedData_ColName );

	// Add possibility to click on title group to reset column width.
	CreateTitleGroupButtons( CD_ComputedData_Name, CD_ComputedData_LastColumn, RD_ComputedData_GroupName, pclSDComputedData );

	// Create the button that allow to Expand/Collapse all the group.
	// Because we collapse the group, we must call 'AddRangeColumn' just before. In this method, we compute the 'm_lRowTopPixel' and 'm_lRowBottomPixel' of
	// the y-area used for resizing column. If we collapse the group, it will not be correct.
	m_mapButtonExpandCollapseGroup[SD_ComputedData] = CreateExpandCollapseGroupButton( CD_ComputedData_FirstColumn, RD_ComputedData_GroupName, 
			true, CCDButtonExpandCollapseGroup::ButtonState::CollapseRow, RD_ComputedData_ColName, RD_ComputedData_LastRow, pclSDComputedData );

	if( NULL != m_mapButtonExpandCollapseGroup[SD_ComputedData] )
	{
		m_mapButtonExpandCollapseGroup[SD_ComputedData]->SetShowStatus( true );
		m_mapButtonExpandCollapseGroup[SD_ComputedData]->ToggleButtonState( true );
	}

	// Very now if we need to expand this group if there is an error with PSV.
	if( CPMInputUser::MT_All == pclPMInputUser->GetPressureMaintenanceType() )
	{
		double dSafetyValve = pclPMInputUser->GetSafetyValveResponsePressure();

		if( pclPMInputUser->GetMinimumRequiredPSVRounded( CPMInputUser::MaintenanceType::MT_ExpansionVessel ) > dSafetyValve
				|| pclPMInputUser->GetMinimumRequiredPSVRounded( CPMInputUser::MaintenanceType::MT_WithCompressor ) > dSafetyValve
				|| pclPMInputUser->GetMinimumRequiredPSVRounded( CPMInputUser::MaintenanceType::MT_WithPump ) > dSafetyValve )
		{
			m_mapButtonExpandCollapseGroup[SD_ComputedData]->SetButtonState( CCDButtonExpandCollapseRows::CollapseRow, true );
			m_mapButtonExpandCollapseGroup[SD_ComputedData]->ApplyInternalChange();
		}
	}

	pclSSheet->SetBool( SSB_REDRAW, TRUE );
}

void CRViewSSelPM::_InitPriceIndexGroup( void )
{
	if( NULL == m_pclIndSelPMParams || NULL == m_pclIndSelPMParams->m_pclSelectPMList )
	{
		ASSERT_RETURN;
	}

	// Call 'MultiSpreadBase' method to get a new sheet.
	CSheetDescription *pclSDPriceIndex = CreateSSheet( SD_PriceIndex );

	if( NULL == pclSDPriceIndex || NULL == pclSDPriceIndex->GetSSheetPointer() )
	{
		return;
	}

	CSSheet *pclSSheet = pclSDPriceIndex->GetSSheetPointer();

	pclSSheet->SetBool( SSB_REDRAW, FALSE );

	pclSSheet->Init();
	pclSSheet->SetBool( SSB_ALLOWDRAGDROP, FALSE );
	pclSSheet->SetBool( SSB_HORZSCROLLBAR, FALSE );
	pclSSheet->SetBool( SSB_VERTSCROLLBAR, FALSE );
	pclSSheet->SetBool( SSB_MOVEACTIVEONFOCUS, FALSE );
	pclSSheet->SetTextTip( SS_TT_STATUS_FLOATING, 250, NULL, _LIGHTYELLOW, _TAH_BLACK );

	// Set max rows.
	// pclSSheet->SetMaxRows( RD_PriceIndex_FirstAvailRow - 1 );
	pclSSheet->SetMaxRows( 5 );

	// All cells are static by default and filled with _T("").
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	pclSSheet->FormatStaticText( -1, -1, -1, -1, _T( "" ) );

	// Increase row height.
	double RowHeight = 12.75;
	pclSSheet->SetRowHeight( RD_PriceIndex_FirstRow, RowHeight * 0.5 );
	pclSSheet->SetRowHeight( RD_PriceIndex_GroupName, RowHeight * 1.5 );
	pclSSheet->SetRowHeight( RD_PriceIndex_ColName, RowHeight * 1.5 );

	// Initialize.
	pclSDPriceIndex->Init( 1, pclSSheet->GetMaxRows(), 1, CSheetDescription::SD_ParameterMode_Mono );
	pclSDPriceIndex->AddColumnInPixels( CD_PriceIndex_FirstColumn, m_mapSSheetColumnWidth[SD_PriceIndex][CD_PriceIndex_FirstColumn] );
	pclSDPriceIndex->AddColumnInPixels( CD_PriceIndex_Name, m_mapSSheetColumnWidth[SD_PriceIndex][CD_PriceIndex_Name] );
	pclSDPriceIndex->AddColumnInPixels( CD_PriceIndex_ExpansionVessel, m_mapSSheetColumnWidth[SD_PriceIndex][CD_PriceIndex_ExpansionVessel] );
	pclSDPriceIndex->AddColumnInPixels( CD_PriceIndex_ExpansionVesselMembrane, m_mapSSheetColumnWidth[SD_PriceIndex][CD_PriceIndex_ExpansionVesselMembrane] );
	pclSDPriceIndex->AddColumnInPixels( CD_PriceIndex_Compresso, m_mapSSheetColumnWidth[SD_PriceIndex][CD_PriceIndex_Compresso] );
	pclSDPriceIndex->AddColumnInPixels( CD_PriceIndex_Transfero, m_mapSSheetColumnWidth[SD_PriceIndex][CD_PriceIndex_Transfero] );

	if( false == m_pclIndSelPMParams->m_bIsVesselMembraneTypeExistInPMTypeCombo )
	{
		int iNewWidth = m_mapSSheetColumnWidth[SD_PriceIndex][CD_PriceIndex_Name] - m_mapSSheetColumnWidth[SD_PriceIndex][CD_PriceIndex_ExpansionVesselMembrane];
		pclSSheet->SetColWidthInPixels( CD_PriceIndex_Name, iNewWidth );
		pclSSheet->ShowCol( CD_PriceIndex_ExpansionVesselMembrane, FALSE );
	}

	// Set the focus column.
	pclSDPriceIndex->SetActiveColumn( CD_PriceIndex_Name );

	// Set range for selection.
	pclSDPriceIndex->SetFocusColumnRange( CD_PriceIndex_Name, CD_PriceIndex_LastColumn );


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Add the main title.
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );
	int iBackColor = ( true == m_pclIndSelPMParams->m_bEditModeRunning ) ? _TAH_TITLE_MAIN_REEDIT : _TAH_TITLE_MAIN;
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, iBackColor );
	pclSDPriceIndex->SetUserVariable( _SDUV_TITLEFORECOLOR, ( pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor ) ) );
	pclSDPriceIndex->SetUserVariable( _SDUV_TITLEBACKCOLOR, ( pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor ) ) );

	pclSSheet->AddCellSpanW( CD_PriceIndex_FirstColumn + 1, RD_PriceIndex_GroupName, CD_PriceIndex_LastColumn - CD_PriceIndex_FirstColumn, 1 );
	pclSSheet->SetStaticText( CD_PriceIndex_FirstColumn + 1, RD_PriceIndex_GroupName, IDS_SSHEETSSELPM_PIGROUP );
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Column name.
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap, (LPARAM)TRUE );

	pclSSheet->SetStaticText( CD_PriceIndex_ExpansionVessel, RD_PriceIndex_ColName, IDS_SSHEETSSELPM_CDEXPANSIONVESSEL );

	if( true == m_pclIndSelPMParams->m_bIsVesselMembraneTypeExistInPMTypeCombo )
	{
		pclSSheet->SetStaticText( CD_PriceIndex_ExpansionVesselMembrane, RD_PriceIndex_ColName, IDS_SSHEETSSELPM_CDEXPANSIONVESSELMEMBRANE );
	}

	pclSSheet->SetStaticText( CD_PriceIndex_Compresso, RD_PriceIndex_ColName, IDS_SSHEETSSELPM_CDCOMPRESSO );
	pclSSheet->SetStaticText( CD_PriceIndex_Transfero, RD_PriceIndex_ColName, IDS_SSHEETSSELPM_CDTRANSFERO );
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	// Draw black line below column header.
	pclSSheet->SetCellBorder( CD_PriceIndex_Name, RD_PriceIndex_ColName, CD_PriceIndex_LastColumn, RD_PriceIndex_ColName, true, SS_BORDERTYPE_BOTTOM );

	// Add needed rows.
	// 'false' to specify that these rows can't be selected.
	// pclSDPriceIndex->AddRows( RD_PriceIndex_LastRow - RD_PriceIndex_FirstAvailRow, false );

	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();

	bool fIsForCooling = ( ProjectType::Cooling == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetApplicationType() ) ? true : false;

	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)TRUE );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_LEFT );
	pclSSheet->SetStaticText( CD_PriceIndex_Name, RD_PriceIndex_BestPriceIndex, TASApp.LoadLocalizedString( IDS_SSHEETSSELPM_PIBESTPRICEINDEX ) );
	pclSSheet->SetStaticText( CD_PriceIndex_Name, RD_PriceIndex_CurrentSelection, TASApp.LoadLocalizedString( IDS_SSHEETSSELPM_PICURRENTSELECTION ) );

	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_CENTER );

	// Best price index for expansion vessel.
	CString strBestPriceIndex = _T("-");

	if( m_pclIndSelPMParams->m_pclSelectPMList->GetBestPriceIndex( CSelectPMList::BPIS_ExpansionVessel ) > 0.0 )
	{
		strBestPriceIndex.Format( _T("%i"), (int)m_pclIndSelPMParams->m_pclSelectPMList->GetBestPriceIndex( CSelectPMList::BPIS_ExpansionVessel ) );
	}

	pclSSheet->SetStaticText( CD_PriceIndex_ExpansionVessel, RD_PriceIndex_BestPriceIndex, strBestPriceIndex );

	if( true == m_pclIndSelPMParams->m_bIsVesselMembraneTypeExistInPMTypeCombo )
	{
		// Best price index for expansion vessel with membrane.
		strBestPriceIndex = _T("-");

		if( m_pclIndSelPMParams->m_pclSelectPMList->GetBestPriceIndex( CSelectPMList::BPIS_ExpansionVesselMembrane ) > 0.0 )
		{
			strBestPriceIndex.Format( _T("%i"), (int)m_pclIndSelPMParams->m_pclSelectPMList->GetBestPriceIndex( CSelectPMList::BPIS_ExpansionVesselMembrane ) );
		}

		pclSSheet->SetStaticText( CD_PriceIndex_ExpansionVesselMembrane, RD_PriceIndex_BestPriceIndex, strBestPriceIndex );
	}

	// Best price index for Compresso.
	strBestPriceIndex = _T("-");

	if( m_pclIndSelPMParams->m_pclSelectPMList->GetBestPriceIndex( CSelectPMList::BPIS_Compresso ) > 0.0 )
	{
		strBestPriceIndex.Format( _T("%i"), (int)m_pclIndSelPMParams->m_pclSelectPMList->GetBestPriceIndex( CSelectPMList::BPIS_Compresso ) );
	}

	pclSSheet->SetStaticText( CD_PriceIndex_Compresso, RD_PriceIndex_BestPriceIndex, strBestPriceIndex );
	
	// Best price index for Transfero.
	strBestPriceIndex = _T("-");

	if( m_pclIndSelPMParams->m_pclSelectPMList->GetBestPriceIndex( CSelectPMList::BPIS_Transfero ) > 0.0 )
	{
		strBestPriceIndex.Format( _T("%i"), (int)m_pclIndSelPMParams->m_pclSelectPMList->GetBestPriceIndex( CSelectPMList::BPIS_Transfero ) );
	}

	pclSSheet->SetStaticText( CD_PriceIndex_Transfero, RD_PriceIndex_BestPriceIndex, strBestPriceIndex );

	// Current selection.
	pclSSheet->SetStaticText( CD_PriceIndex_ExpansionVessel, RD_PriceIndex_CurrentSelection, _T("-") );

	if( true == m_pclIndSelPMParams->m_bIsVesselMembraneTypeExistInPMTypeCombo )
	{
		pclSSheet->SetStaticText( CD_PriceIndex_ExpansionVesselMembrane, RD_PriceIndex_CurrentSelection, _T("-") );
	}

	pclSSheet->SetStaticText( CD_PriceIndex_Compresso, RD_PriceIndex_CurrentSelection, _T("-") );
	pclSSheet->SetStaticText( CD_PriceIndex_Transfero, RD_PriceIndex_CurrentSelection, _T("-") );
	
	pclSSheet->SetCellBorder( CD_PriceIndex_Name, RD_PriceIndex_LastRow, CD_PriceIndex_LastColumn, RD_PriceIndex_LastRow, true, SS_BORDERTYPE_BOTTOM,
			SS_BORDERSTYLE_SOLID, _BLACK );


	// Add possibility to change column size.
	// Remark: place this code after any initialization (like adding, removing, resizing, hiding/showing columns).
	CResizingColumnInfo *pclResizingColumnInfo = pclSDPriceIndex->GetResizingColumnInfo();
	pclResizingColumnInfo->ActivateFeature( true, false, RD_PriceIndex_FirstAvailRow, RD_PriceIndex_LastRow - 1 );
	pclResizingColumnInfo->AddRangeColumn( CD_PriceIndex_Name, CD_PriceIndex_LastColumn, RD_PriceIndex_ColName, RD_PriceIndex_ColName );

	// Add possibility to click on title group to reset column width.
	CreateTitleGroupButtons( CD_PriceIndex_Name, CD_PriceIndex_LastColumn, RD_PriceIndex_GroupName, pclSDPriceIndex );

	// Create the button that allow to Expand/Collapse all the group.
	// Because we collapse the group, we must call 'AddRangeColumn' just before. In this method, we compute the 'm_lRowTopPixel' and 'm_lRowBottomPixel' of
	// the y-area used for resizing column. If we collapse the group, it will not be correct.
	m_mapButtonExpandCollapseGroup[SD_PriceIndex] = CreateExpandCollapseGroupButton( CD_PriceIndex_FirstColumn, RD_PriceIndex_GroupName, true,
			CCDButtonExpandCollapseGroup::ButtonState::CollapseRow, RD_PriceIndex_ColName, RD_PriceIndex_LastRow, pclSDPriceIndex );

	if( NULL != m_mapButtonExpandCollapseGroup[SD_PriceIndex] )
	{
		m_mapButtonExpandCollapseGroup[SD_PriceIndex]->SetShowStatus( true );
	}

	pclSSheet->SetBool( SSB_REDRAW, TRUE );
}

long CRViewSSelPM::_FillExpansionVesselSheet( UINT uiSheetID, CDS_SSelPMaint *pclEditedSSelPM )
{
	if( NULL == m_pclIndSelPMParams || NULL == m_pclIndSelPMParams->m_pclSelectPMList || NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERTA_RETURN( -1 );
	}

	// Call 'MultiSpreadBase' method to get a new sheet.
	CSheetDescription *pclSDExpansionVessel = CreateSSheet( uiSheetID );

	if( NULL == pclSDExpansionVessel || NULL == pclSDExpansionVessel->GetSSheetPointer() )
	{
		ASSERTA_RETURN( -1 );
	}

	CSelectPMList::ProductType eProductType = ( SD_ExpansionVessel == uiSheetID ) ? CSelectPMList::PT_ExpansionVessel 
			: CSelectPMList::PT_ExpansionVesselMembrane;

	CSelectPMList::BestPriceIndexSolution eBestPriceIndexSolution = ( SD_ExpansionVessel == uiSheetID ) ? CSelectPMList::BPIS_ExpansionVessel 
			: CSelectPMList::BPIS_ExpansionVesselMembrane;

	CSSheet *pclSSheet = pclSDExpansionVessel->GetSSheetPointer();
	pclSDExpansionVessel->SetGroupSheetDescriptionID( uiSheetID );

	pclSSheet->SetBool( SSB_REDRAW, FALSE );

	_InitExpansionVesselHeaders( uiSheetID, pclSDExpansionVessel );

	m_lSelectedRow = -1;
	m_lProductTotalCount = 0;
	m_lProductNotPriorityCount = 0;
	m_bShowAllPrioritiesShown = false;
	m_lShowAllPrioritiesButtonRow = 0;
	long lRow = RD_ExpansionVessel_FirstAvailRow;
	pclSDExpansionVessel->RestartRemarkGenerator();

	CSelectedPMBase *pclSelectedBestPriceProduct = NULL;

	if( true == TASApp.IsPMPriceIndexUsed() )
	{
		pclSelectedBestPriceProduct = m_pclIndSelPMParams->m_pclSelectPMList->GetBestPriceIndexProduct( eBestPriceIndexSolution );
	}

	// Priority/NotaPriority loop.
	// Remark: Now we show all when user selects a specific pressure maintenance system.
	int iEndLoop = ( true == m_bIsPMTypeAll ) ? 2 : 1;

	for( int iLoop = 0; iLoop < iEndLoop; iLoop++ )
	{
		for( CSelectedPMBase *pclSelectedProduct = m_pclIndSelPMParams->m_pclSelectPMList->GetFirstProduct( eProductType );
				NULL != pclSelectedProduct; pclSelectedProduct = m_pclIndSelPMParams->m_pclSelectPMList->GetNextProduct( eProductType ) )
		{
			// Skip not Priority when iLoop == 0
			if( true == m_bIsPMTypeAll && ( iLoop == 0 ) == pclSelectedProduct->IsFlagSet( CSelectedPMBase::eNotPriority ) )
			{
				continue;
			}

			CSelectedVssl *pclSelectedExpansionVessel = dynamic_cast<CSelectedVssl *>( pclSelectedProduct );

			if( NULL == pclSelectedExpansionVessel )
			{
				continue;
			}

			long lReturn = _FillExpansionVesselRow( pclSDExpansionVessel, lRow, pclSelectedExpansionVessel, 
					m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser() , pclEditedSSelPM, pclSelectedBestPriceProduct );

			if( -1 == lReturn )
			{
				continue;
			}
			else
			{
				lRow = lReturn;
			}

			lRow++;
			m_lProductTotalCount++;
		}
	}

	long lLastDataRow = lRow - 1;
	pclSDExpansionVessel->SetUserVariable( _SDUV_TOTALPRODUCT_COUNT, m_lProductTotalCount );
	pclSDExpansionVessel->SetUserVariable( _SDUV_NOPRIORITYPRODUCT_COUNT, m_lProductNotPriorityCount );

	// Now we can create Show/Hide all priorities if needed.
	if( true == m_bIsPMTypeAll )
	{
		// Remarks: 2nd condition to avoid to have button if we have only no priority valves.
		if( true == m_bShowAllPrioritiesShown && m_lProductTotalCount > m_lProductNotPriorityCount )
		{
			// Create Show/Hide all priorities button in regards to current state!
			// HYS-1539 : The button state is the same for all sheet
			CCDButtonShowAllPriorities::ButtonState eState = ( true == GetShowAllPrioritiesFlag( (int)uiSheetID ) ) ? 
					CCDButtonShowAllPriorities::ButtonState::HidePriorities : CCDButtonShowAllPriorities::ButtonState::ShowPriorities;

			CCDButtonShowAllPriorities *pclShowAllButton = CreateShowAllPrioritiesButton( CD_ExpansionVessel_FirstColumn,  m_lShowAllPrioritiesButtonRow, 
					m_bShowAllPrioritiesShown, eState, m_lShowAllPrioritiesButtonRow + 1, pclSSheet->GetMaxRows(), pclSDExpansionVessel );

			// Here we must call 'ApplyInternalChange' to show or hide rows in regards to the current state!
			if( NULL != pclShowAllButton )
			{
				pclShowAllButton->ApplyInternalChange();
			}
		}
		else
		{
			if( m_lShowAllPrioritiesButtonRow != 0 )
			{
				pclSSheet->ShowRow( m_lShowAllPrioritiesButtonRow, false );
			}
		}
	}

	pclSSheet->SetCellBorder( CD_ExpansionVessel_FirstColumn + 1, lLastDataRow, CD_ExpansionVessel_Pointer - 1, lLastDataRow, true, 
			SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );

	// Add first the link to the excluded products if exist.
	lRow = _AddExcludedProductsLink( pclSDExpansionVessel, lRow, eProductType );

	long lFirstVisibleCol = ( true == TASApp.IsPMPriceIndexUsed() ) ? CD_ExpansionVessel_PriceIndexIcon : CD_ExpansionVessel_Name;

	// Add Remarks.
	// Note: these lines must be placed after creating Show/Hide all priorities button to avoid to take in count these lines when defining
	//       range of row to be Show/Hide.
	lRow = pclSDExpansionVessel->WriteRemarks( lRow, lFirstVisibleCol, CD_ExpansionVessel_Pointer );

	// Add possibility to change column size.
	// Remark: place this code after any initialization (like adding, removing, resizing, hiding/showing columns).
	CResizingColumnInfo *pclResizingColumnInfo = pclSDExpansionVessel->GetResizingColumnInfo();
	pclResizingColumnInfo->ActivateFeature( true, false, RD_ExpansionVessel_FirstAvailRow, lLastDataRow );
	long lFirstResizableCol = ( true == TASApp.IsPMPriceIndexUsed() ) ? CD_ExpansionVessel_PriceIndexBorder : CD_ExpansionVessel_Name;
	pclResizingColumnInfo->AddRangeColumn( lFirstResizableCol, CD_ExpansionVessel_Pointer - 1, RD_ExpansionVessel_ColName, RD_ExpansionVessel_ColUnit );

	// Add possibility to click on title group to reset column width.
	CreateTitleGroupButtons( lFirstVisibleCol, CD_ExpansionVessel_WaterReserve, RD_ExpansionVessel_GroupName, pclSDExpansionVessel );

	// Create the button to allow to expand/collapse all the group.
	m_mapButtonExpandCollapseGroup[uiSheetID] = CreateExpandCollapseGroupButton( CD_ExpansionVessel_FirstColumn, RD_ExpansionVessel_GroupName, true,
			CCDButtonExpandCollapseGroup::ButtonState::CollapseRow, RD_ExpansionVessel_ColName, pclSSheet->GetMaxRows(), pclSDExpansionVessel );

	if( NULL != m_mapButtonExpandCollapseGroup[uiSheetID] )
	{
		m_mapButtonExpandCollapseGroup[uiSheetID]->SetShowStatus( true );
	}

	pclSSheet->SetBool( SSB_REDRAW, TRUE );
	return m_lSelectedRow;
}

void CRViewSSelPM::_InitExpansionVesselHeaders( UINT uiSheetID, CSheetDescription *pclSDExpansionVessel )
{
	if( NULL == pclSDExpansionVessel || NULL == pclSDExpansionVessel->GetSSheetPointer() )
	{
		ASSERT_RETURN;
	}

	CSSheet *pclSSheet = pclSDExpansionVessel->GetSSheetPointer();

	pclSSheet->Init();
	pclSSheet->SetBool( SSB_ALLOWDRAGDROP, FALSE );
	pclSSheet->SetBool( SSB_HORZSCROLLBAR, FALSE );
	pclSSheet->SetBool( SSB_VERTSCROLLBAR, FALSE );
	pclSSheet->SetBool( SSB_MOVEACTIVEONFOCUS, FALSE );
	pclSSheet->SetTextTip( SS_TT_STATUS_FLOATING, 250, NULL, _LIGHTYELLOW, _TAH_BLACK );

	// Set max rows.
	pclSSheet->SetMaxRows( RD_ExpansionVessel_FirstAvailRow - 1 );

	// All cells are static by default and filled with _T("").
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	pclSSheet->FormatStaticText( -1, -1, -1, -1, _T( "" ) );

	// Increase row height.
	double dRowHeight = 12.75;
	pclSSheet->SetRowHeight( RD_ExpansionVessel_FirstRow, dRowHeight * 0.5 );
	pclSSheet->SetRowHeight( RD_ExpansionVessel_GroupName, dRowHeight * 1.5 );
	pclSSheet->SetRowHeight( RD_ExpansionVessel_ColName, dRowHeight * 2.5 );
	pclSSheet->SetRowHeight( RD_ExpansionVessel_ColUnit, dRowHeight * 1.2 );

	// Initialize.
	pclSDExpansionVessel->Init( 1, pclSSheet->GetMaxRows(), 1, CSheetDescription::SD_ParameterMode_Mono );
	pclSDExpansionVessel->AddColumnInPixels( CD_ExpansionVessel_FirstColumn, m_mapSSheetColumnWidth[uiSheetID][CD_ExpansionVessel_FirstColumn] );

	if( true == TASApp.IsPMPriceIndexUsed() )
	{
		pclSDExpansionVessel->AddColumnInPixels( CD_ExpansionVessel_PriceIndexIcon, m_mapSSheetColumnWidth[uiSheetID][CD_ExpansionVessel_PriceIndexIcon] );
		pclSDExpansionVessel->AddColumnInPixels( CD_ExpansionVessel_PriceIndexValue, m_mapSSheetColumnWidth[uiSheetID][CD_ExpansionVessel_PriceIndexValue] );
		pclSDExpansionVessel->AddColumnInPixels( CD_ExpansionVessel_PriceIndexBorder, m_mapSSheetColumnWidth[uiSheetID][CD_ExpansionVessel_PriceIndexBorder] );
	}

	pclSDExpansionVessel->AddColumnInPixels( CD_ExpansionVessel_CheckBox, m_mapSSheetColumnWidth[uiSheetID][CD_ExpansionVessel_CheckBox] );
	pclSDExpansionVessel->AddColumnInPixels( CD_ExpansionVessel_Name, m_mapSSheetColumnWidth[uiSheetID][CD_ExpansionVessel_Name] );
	pclSDExpansionVessel->AddColumnInPixels( CD_ExpansionVessel_NbreOfVssl, m_mapSSheetColumnWidth[uiSheetID][CD_ExpansionVessel_NbreOfVssl] );
	pclSDExpansionVessel->AddColumnInPixels( CD_ExpansionVessel_Volume, m_mapSSheetColumnWidth[uiSheetID][CD_ExpansionVessel_Volume] );
	pclSDExpansionVessel->AddColumnInPixels( CD_ExpansionVessel_MaxPressure, m_mapSSheetColumnWidth[uiSheetID][CD_ExpansionVessel_MaxPressure] );
	pclSDExpansionVessel->AddColumnInPixels( CD_ExpansionVessel_TempRange, m_mapSSheetColumnWidth[uiSheetID][CD_ExpansionVessel_TempRange] );
	pclSDExpansionVessel->AddColumnInPixels( CD_ExpansionVessel_FactoryPresetPressure, m_mapSSheetColumnWidth[uiSheetID][CD_ExpansionVessel_FactoryPresetPressure] );
	pclSDExpansionVessel->AddColumnInPixels( CD_ExpansionVessel_Diameter, m_mapSSheetColumnWidth[uiSheetID][CD_ExpansionVessel_Diameter] );
	pclSDExpansionVessel->AddColumnInPixels( CD_ExpansionVessel_Height, m_mapSSheetColumnWidth[uiSheetID][CD_ExpansionVessel_Height] );
	pclSDExpansionVessel->AddColumnInPixels( CD_ExpansionVessel_Weight, m_mapSSheetColumnWidth[uiSheetID][CD_ExpansionVessel_Weight] );
	pclSDExpansionVessel->AddColumnInPixels( CD_ExpansionVessel_MaxWeight, m_mapSSheetColumnWidth[uiSheetID][CD_ExpansionVessel_MaxWeight] );
	pclSDExpansionVessel->AddColumnInPixels( CD_ExpansionVessel_Standing, m_mapSSheetColumnWidth[uiSheetID][CD_ExpansionVessel_Standing] );
	pclSDExpansionVessel->AddColumnInPixels( CD_ExpansionVessel_Connection, m_mapSSheetColumnWidth[uiSheetID][CD_ExpansionVessel_Connection] );
	pclSDExpansionVessel->AddColumnInPixels( CD_ExpansionVessel_RequiredNominalVolumeEN12828, m_mapSSheetColumnWidth[uiSheetID][CD_ExpansionVessel_RequiredNominalVolumeEN12828] );
	pclSDExpansionVessel->AddColumnInPixels( CD_ExpansionVessel_RequiredNominalVolumeEN12828InfoIcon, m_mapSSheetColumnWidth[uiSheetID][CD_ExpansionVessel_RequiredNominalVolumeEN12828InfoIcon] );
	pclSDExpansionVessel->AddColumnInPixels( CD_ExpansionVessel_InitialPressure, m_mapSSheetColumnWidth[uiSheetID][CD_ExpansionVessel_InitialPressure] );
	pclSDExpansionVessel->AddColumnInPixels( CD_ExpansionVessel_WaterReserve, m_mapSSheetColumnWidth[uiSheetID][CD_ExpansionVessel_WaterReserve] );
	pclSDExpansionVessel->AddColumnInPixels( CD_ExpansionVessel_Pointer, m_mapSSheetColumnWidth[uiSheetID][CD_ExpansionVessel_Pointer] );

	// Must be placed after any call to 'AddColumnInPixels'. Because this method will add needed columns and TSpread will show them by default.
	// When calling the first time 'AddColumnInPixels' for the 'CD_ExpansionVessel_Name' column, this method will call TSpread::SetMaxCols() to adapt the
	// number of columns. All the column before 'CD_ExpansionVessel_Name' are set to a default width and are visible.
	if( false == TASApp.IsPMPriceIndexUsed() )
	{
		pclSSheet->ShowCol( CD_ExpansionVessel_PriceIndexIcon, FALSE );
		pclSSheet->ShowCol( CD_ExpansionVessel_PriceIndexValue, FALSE );
		pclSSheet->ShowCol( CD_ExpansionVessel_PriceIndexBorder, FALSE );
	}

	if( false == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->IsNorm( ProjectType::Heating, PressurisationNorm::PN_EN12828 ) )
	{
		pclSSheet->ShowCol( CD_ExpansionVessel_RequiredNominalVolumeEN12828, FALSE );
		pclSSheet->ShowCol( CD_ExpansionVessel_RequiredNominalVolumeEN12828InfoIcon, FALSE );
	}
	
	// Set in which column parameter must be saved.
	pclSDExpansionVessel->AddParameterColumn( CD_ExpansionVessel_Pointer );

	// Set the focus column.
	pclSDExpansionVessel->SetActiveColumn( CD_ExpansionVessel_FirstColumn + 1 );

	// Set range for selection.
	pclSDExpansionVessel->SetFocusColumnRange( CD_ExpansionVessel_FirstColumn + 1, CD_ExpansionVessel_WaterReserve );


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Add the main title.
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, _IMI_TITLE_GROUP1 );
	pclSDExpansionVessel->SetUserVariable( _SDUV_TITLEFORECOLOR, ( pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor ) ) );
	pclSDExpansionVessel->SetUserVariable( _SDUV_TITLEBACKCOLOR, ( pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor ) ) );

	int iTitleID = ( SD_ExpansionVessel == uiSheetID ) ? IDS_SSHEETSSELPM_EXPANSIONVESSELGROUP : IDS_SSHEETSSELPM_EXPANSIONVESSELMBRGROUP;

 	pclSSheet->AddCellSpanW( CD_ExpansionVessel_FirstColumn + 1, RD_ExpansionVessel_GroupName, CD_ExpansionVessel_Pointer - ( CD_ExpansionVessel_FirstColumn + 1 ), 1 );
 	pclSSheet->SetStaticText( CD_ExpansionVessel_FirstColumn + 1, RD_ExpansionVessel_GroupName, iTitleID );
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Column name.
	if( false == pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn ) )
	{
		return;
	}

	if( false == pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE ) )
	{
		return;
	}

	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap, (LPARAM)TRUE );
	pclSSheet->SetStaticText( CD_ExpansionVessel_Name, RD_ExpansionVessel_ColName, IDS_SSHEETSSELPM_VESSELNAME );
	pclSSheet->SetStaticText( CD_ExpansionVessel_NbreOfVssl, RD_ExpansionVessel_ColName, IDS_SSHEETSSELPM_VESSELQTY );
	pclSSheet->SetStaticText( CD_ExpansionVessel_Volume, RD_ExpansionVessel_ColName, IDS_SSHEETSSELPM_VESSELVOL );
	
	if( true == TASApp.IsPMPriceIndexUsed() )
	{
		pclSSheet->AddCellSpan( CD_ExpansionVessel_PriceIndexIcon, RD_ExpansionVessel_ColName, 3, 1 );
		pclSSheet->SetStaticText( CD_ExpansionVessel_PriceIndexIcon, RD_ExpansionVessel_ColName, IDS_SSHEETSSELPM_PRICEINDEX );
	}

	pclSSheet->SetStaticText( CD_ExpansionVessel_MaxPressure, RD_ExpansionVessel_ColName, IDS_SSHEETSSELPM_VESSELMAXPRESS );
	pclSSheet->SetStaticText( CD_ExpansionVessel_TempRange, RD_ExpansionVessel_ColName, IDS_SSHEETSSELPM_VESSELTEMPRANGE );
	pclSSheet->SetStaticText( CD_ExpansionVessel_FactoryPresetPressure, RD_ExpansionVessel_ColName, IDS_SSHEETSSELPM_STATICOFACTPRESETP );
	pclSSheet->SetStaticText( CD_ExpansionVessel_Diameter, RD_ExpansionVessel_ColName, IDS_SSHEETSSELPM_VESSELDIAMETER );
	pclSSheet->SetStaticText( CD_ExpansionVessel_Height, RD_ExpansionVessel_ColName, IDS_SSHEETSSELPM_VESSELHEIGHT );
	pclSSheet->SetStaticText( CD_ExpansionVessel_Weight, RD_ExpansionVessel_ColName, IDS_SSHEETSSELPM_VESSELMASS );
	pclSSheet->SetStaticText( CD_ExpansionVessel_MaxWeight, RD_ExpansionVessel_ColName, IDS_SSHEETSSELPM_VESSELMAXWEIGHT );
	pclSSheet->SetStaticText( CD_ExpansionVessel_Connection, RD_ExpansionVessel_ColName, IDS_SSHEETSSELPM_VESSELCONNECT );

	// HYS-1352: Add icon information when working with the EN12828 norm.
	pclSSheet->AddCellSpanW( CD_ExpansionVessel_RequiredNominalVolumeEN12828, RD_ExpansionVessel_ColName, 2, 1 );
	pclSSheet->SetStaticText( CD_ExpansionVessel_RequiredNominalVolumeEN12828, RD_ExpansionVessel_ColName, IDS_RVIEWSSELPM_REQUIREDNOMINALVOLUMEEN12828 );

	pclSSheet->SetStaticText( CD_ExpansionVessel_InitialPressure, RD_ExpansionVessel_ColName, IDS_SSHEETSSELPM_STATICOINIP );
	pclSSheet->SetStaticText( CD_ExpansionVessel_WaterReserve, RD_ExpansionVessel_ColName, IDS_SSHEETSSELPM_STATICOWATR );
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Column units.
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap, (LPARAM)FALSE );

	if( true == TASApp.IsPMPriceIndexUsed() )
	{
		pclSSheet->SetStaticText( CD_ExpansionVessel_PriceIndexValue, RD_ExpansionVessel_ColUnit, _T("%") );
	}

	pclSSheet->SetStaticText( CD_ExpansionVessel_Volume, RD_ExpansionVessel_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_VOLUME ) ).c_str() );
	pclSSheet->SetStaticText( CD_ExpansionVessel_MaxPressure, RD_ExpansionVessel_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_PRESSURE ) ).c_str() );
	pclSSheet->SetStaticText( CD_ExpansionVessel_TempRange, RD_ExpansionVessel_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_TEMPERATURE ) ).c_str() );
	pclSSheet->SetStaticText( CD_ExpansionVessel_FactoryPresetPressure, RD_ExpansionVessel_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_PRESSURE ) ).c_str() );
	pclSSheet->SetStaticText( CD_ExpansionVessel_Diameter, RD_ExpansionVessel_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_LENGTH ) ).c_str() );
	pclSSheet->SetStaticText( CD_ExpansionVessel_Height, RD_ExpansionVessel_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_LENGTH ) ).c_str() );
	pclSSheet->SetStaticText( CD_ExpansionVessel_Weight, RD_ExpansionVessel_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_MASS ) ).c_str() );
	pclSSheet->SetStaticText( CD_ExpansionVessel_MaxWeight, RD_ExpansionVessel_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_MASS ) ).c_str() );

	// HYS-1352: Add icon information when working with the EN12828 norm.
	pclSSheet->AddCellSpanW( CD_ExpansionVessel_RequiredNominalVolumeEN12828, RD_ExpansionVessel_ColUnit, 2, 1 );
	pclSSheet->SetStaticText( CD_ExpansionVessel_RequiredNominalVolumeEN12828, RD_ExpansionVessel_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_VOLUME ) ).c_str() );

	pclSSheet->SetStaticText( CD_ExpansionVessel_InitialPressure, RD_ExpansionVessel_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_PRESSURE ) ).c_str() );
	pclSSheet->SetStaticText( CD_ExpansionVessel_WaterReserve, RD_ExpansionVessel_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_VOLUME ) ).c_str() );
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Draw black line below column header.
	pclSSheet->SetCellBorder( CD_ExpansionVessel_FirstColumn + 1, RD_ExpansionVessel_ColUnit, CD_ExpansionVessel_Pointer - 1, RD_ExpansionVessel_ColUnit, true, SS_BORDERTYPE_BOTTOM );
}

long CRViewSSelPM::_FillExpansionVesselRow( CSheetDescription *pclSDExpansionVessel, long lRow, CSelectedVssl *pclSelectedExpansionVessel, 
		CPMInputUser *pclPMInputUser, CDS_SSelPMaint *pclEditedSSelPM, CSelectedPMBase* pclSelectedBestPriceProduct, bool bForRejectedProduct )
{
	if( NULL == pclSDExpansionVessel || NULL == pclSDExpansionVessel->GetSSheetPointer() || NULL == pclSelectedExpansionVessel || NULL == pclPMInputUser )
	{
		ASSERTA_RETURN( -1 );
	}

	CDB_Vessel *pclExpansionVessel = dynamic_cast<CDB_Vessel *>( pclSelectedExpansionVessel->GetpData() );

	if( NULL == pclExpansionVessel )
	{
		return -1;
	}

	CSSheet *pclSSheet = pclSDExpansionVessel->GetSSheetPointer();

	if( true == m_bIsPMTypeAll && true == pclSelectedExpansionVessel->IsFlagSet( CSelectedPMBase::eNotPriority ) )
	{
		m_lProductNotPriorityCount++;

		if( false == m_bShowAllPrioritiesShown )
		{
			// Add one empty not selectable row.
			pclSDExpansionVessel->AddRows( 1 );

			pclSSheet->SetCellBorder( CD_ExpansionVessel_FirstColumn + 1, lRow, CD_ExpansionVessel_Pointer - 1, lRow, true, 
					SS_BORDERTYPE_TOP | SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );

			m_lShowAllPrioritiesButtonRow = lRow++;
			m_bShowAllPrioritiesShown = true;
		}
	}

	// Add this row.
	// 'true' to specify that this row can be selected.
	pclSDExpansionVessel->AddRows( 1, true );

	// Add checkbox.
	pclSSheet->SetCheckBox( CD_ExpansionVessel_CheckBox, lRow, _T(""), false, true );

	// Set the price index if used.
	if( true == TASApp.IsPMPriceIndexUsed() )
	{
		if( NULL != pclSelectedBestPriceProduct 
				&&  0 == _tcscmp( pclSelectedBestPriceProduct->GetProductIDPtr().ID, pclSelectedExpansionVessel->GetProductIDPtr().ID ) )
		{
			pclSSheet->SetPictureCellWithID( IDI_BESTPRICE, CD_ExpansionVessel_PriceIndexIcon, lRow, CSSheet::PictureCellType::Icon );
		}

		CString str = _T("-");

		if( pclSelectedExpansionVessel->GetPriceIndex() > 0.0 )
		{
			str.Format( _T("%i"), (int)pclSelectedExpansionVessel->GetPriceIndex() );
		}

		pclSSheet->SetStaticText( CD_ExpansionVessel_PriceIndexValue, lRow, str );
	}

	bool bBest = pclSelectedExpansionVessel->IsFlagSet( CSelectedPMBase::eBest );

	if( true == bBest )
	{
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_GREEN );
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, true );
	}
	else if( false == bForRejectedProduct && true == pclSelectedExpansionVessel->IsFlagSet( CSelectedPMBase::eVslFlags3000lbar ) )
	{
		// HYS-867: show warning for this vessel that derogates to the 'pressure.volume' limit rule.
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_ORANGE );
	}

	// Set name.
	pclSSheet->SetStaticText( CD_ExpansionVessel_Name, lRow, pclExpansionVessel->GetName() );

	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, false );

	// Set quantity.
	if( true == pclSelectedExpansionVessel->IsFlagSet( CSelectedPMBase::eVslFlagsNbreOfVssl ) )
	{
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_ORANGE );
	}
	else if( false == bForRejectedProduct && true == pclSelectedExpansionVessel->IsFlagSet( CSelectedPMBase::eVslFlags3000lbar ) )
	{
		// HYS-867: show warning for this vessel that derogates to the 'pressure.volume' limit rule.
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_ORANGE );
	}

	pclSSheet->SetStaticText( CD_ExpansionVessel_NbreOfVssl, lRow, WriteDouble( pclSelectedExpansionVessel->GetNbreOfVsslNeeded(), 1, 0, 1 ) );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );
	
	// Set volume nominal.
	if( false == bForRejectedProduct && true == pclSelectedExpansionVessel->IsFlagSet( CSelectedPMBase::eVslFlags3000lbar ) )
	{
		// HYS-867: show warning for this vessel that derogates to the 'pressure.volume' limit rule.
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_ORANGE );
	}

	double dVesselVolume = pclExpansionVessel->GetNominalVolume();
	pclSSheet->SetStaticText( CD_ExpansionVessel_Volume, lRow, WriteCUDouble( _U_VOLUME, dVesselVolume ) );

	// Set max pressure.
	pclSSheet->SetStaticText( CD_ExpansionVessel_MaxPressure, lRow, WriteCUDouble( _U_PRESSURE, pclExpansionVessel->GetPmaxmax() ) );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

	// Set temperature range.
	if( true == pclSelectedExpansionVessel->IsFlagSet( CSelectedPMBase::eTemperature ) )
	{
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_ORANGE );
	}
	else if( false == bForRejectedProduct && true == pclSelectedExpansionVessel->IsFlagSet( CSelectedPMBase::eVslFlags3000lbar ) )
	{
		// HYS-867: show warning for this vessel that derogates to the 'pressure.volume' limit rule.
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_ORANGE );
	}

	pclSSheet->SetStaticText( CD_ExpansionVessel_TempRange, lRow, pclExpansionVessel->GetTempRange() );

	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

	// Set factory preset pressure.
	if( false == bForRejectedProduct && true == pclSelectedExpansionVessel->IsFlagSet( CSelectedPMBase::eVslFlags3000lbar ) )
	{
		// HYS-867: show warning for this vessel that derogates to the 'pressure.volume' limit rule.
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_ORANGE );
	}

	pclSSheet->SetStaticText( CD_ExpansionVessel_FactoryPresetPressure, lRow, WriteCUDouble( _U_PRESSURE, pclExpansionVessel->GetFactoryP0() ) );

	// Set diameter.
	pclSSheet->SetStaticText( CD_ExpansionVessel_Diameter, lRow, WriteCUDouble( _U_LENGTH, pclExpansionVessel->GetDiameter() ) );

	// Set height.
	pclSSheet->SetStaticText( CD_ExpansionVessel_Height, lRow, WriteCUDouble( _U_LENGTH, pclExpansionVessel->GetHeight() ) );

	// Set weight.
	pclSSheet->SetStaticText( CD_ExpansionVessel_Weight, lRow, WriteCUDouble( _U_MASS, pclExpansionVessel->GetWeight() ) );

	// Set max. weight.
	CString strMaxWeight = _T("-");
	double dMaxWeight = pclPMInputUser->GetMaxWeight( pclExpansionVessel->GetWeight(), dVesselVolume, pclPMInputUser->GetMinTemperature() );

	if( -1.0 != dMaxWeight )
	{
		strMaxWeight = WriteCUDouble( _U_MASS, dMaxWeight );
	}

	pclSSheet->SetStaticText( CD_ExpansionVessel_MaxWeight, lRow, strMaxWeight );

	// Set standing
	pclSSheet->SetPictureCellWithID( pclExpansionVessel->GetInstallationLayoutPictureID(), CD_ExpansionVessel_Standing, lRow, CSSheet::PictureCellType::Icon );

	// Set connection.
	pclSSheet->SetStaticText( CD_ExpansionVessel_Connection, lRow, pclExpansionVessel->GetConnect() );

	long lPrevAlignement = pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_RIGHT );

	if( true == pclPMInputUser->IsNorm( ProjectType::Heating, PressurisationNorm::PN_EN12828 ) )
	{
		if( pclSelectedExpansionVessel->GetNbreOfVsslNeeded() > pclSelectedExpansionVessel->GetInitialNbreOfVsslNeeded() )
		{
			double dRequiredNominalVolumeEN12828 = pclPMInputUser->GetNominalVolumeForPaEN12828( dVesselVolume * pclSelectedExpansionVessel->GetInitialNbreOfVsslNeeded() );
			pclSSheet->SetStaticText( CD_ExpansionVessel_RequiredNominalVolumeEN12828, lRow, WriteCUDouble( _U_VOLUME, dRequiredNominalVolumeEN12828 ) );
	
			// HYS-1352: Add icon information.
			pclSSheet->SetPictureCellWithID( IDI_PMEN12828_ICONINFO, CD_ExpansionVessel_RequiredNominalVolumeEN12828InfoIcon, lRow, CSSheet::PictureCellType::Icon );
			SS_CELLTYPE rCellType;
			pclSSheet->GetCellType( CD_ExpansionVessel_RequiredNominalVolumeEN12828InfoIcon, lRow, &rCellType );
			rCellType.Style &= ~VPS_CENTER;
			rCellType.Style |= ( SSS_ALIGN_VCENTER | SSS_ALIGN_LEFT );
			pclSSheet->SetCellType( CD_ExpansionVessel_RequiredNominalVolumeEN12828InfoIcon, lRow, &rCellType );
		}
		else
		{
			pclSSheet->SetStaticText( CD_ExpansionVessel_RequiredNominalVolumeEN12828, lRow, WriteCUDouble( _U_VOLUME, pclPMInputUser->GetVesselNominalVolume() ) );
		}
	}

	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)lPrevAlignement );

	// HYS-1022: 'pa,pot' becomes 'pa'. 
	// Set initial pressure.
	CString strInitialPressure = _T("-");

	if( false == pclSelectedExpansionVessel->GetRejected() && pclSelectedExpansionVessel->GetInitialPressure() > 0.0 )
	{
		strInitialPressure = WriteCUDouble( _U_PRESSURE, pclSelectedExpansionVessel->GetInitialPressure() );
	}

	pclSSheet->SetStaticText( CD_ExpansionVessel_InitialPressure, lRow, strInitialPressure );

	// HYS-1022: 'Vwr,opt' becomes 'Vwr'.
	// Set water reserve.
	pclSSheet->SetStaticText( CD_ExpansionVessel_WaterReserve, lRow, WriteCUDouble( _U_VOLUME, pclSelectedExpansionVessel->GetWaterReserve() ) );

	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

	// Save parameter.
	CCellDescriptionProduct *pclCDProduct = CreateCellDescriptionProduct( pclSDExpansionVessel->GetFirstParameterColumn(), lRow, (LPARAM)pclExpansionVessel, pclSDExpansionVessel );

	if( NULL != pclCDProduct )
	{
		pclCDProduct->SetUserParam( (LPARAM)pclSelectedExpansionVessel );
	}

	CDB_Vessel *pclEditedExpansionVessel = NULL;

	if( NULL != pclEditedSSelPM )
	{
		pclEditedExpansionVessel = dynamic_cast<CDB_Vessel *>( pclEditedSSelPM->GetVesselIDPtr().MP );
	}

	if( NULL != pclEditedExpansionVessel && pclEditedExpansionVessel == pclExpansionVessel )
	{
		m_lSelectedRow = lRow;
	}

	pclSSheet->SetCellBorder( CD_ExpansionVessel_FirstColumn + 1, lRow, CD_ExpansionVessel_Pointer - 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _GRAY );

	return lRow;
}

bool CRViewSSelPM::_FillExpansionVesselCurves( CSelectedVssl *pclExpansionVesselSelected )
{
	if( NULL == m_pclIndSelPMParams || NULL == m_pclIndSelPMParams->m_pclSelectPMList || NULL == pclExpansionVesselSelected )
	{
		ASSERTA_RETURN( false );
	}

	CDlgPMGraphsOutput::CPMInterface clOutputInterface;
	clOutputInterface.ClearOutput();

	if( NULL == m_pclIndSelPMParams->m_pclSelectPMList || NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		return false;
	}

	CDialogEx *pclPressureGraph = m_pclIndSelPMParams->m_pclSelectPMList->GetExpansionVesselCurves( pclExpansionVesselSelected, clOutputInterface.GetCWndPMGraphsOutput() );

	if( NULL == pclPressureGraph )
	{
		return false;
	}

	clOutputInterface.AddTChartDialog( pclPressureGraph );

	double dp0 = m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetMinimumPressure();
	double dpa = pclExpansionVesselSelected->GetInitialPressure();
	double dpe = m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetFinalPressure();
	double dPSV = m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetSafetyValveResponsePressure();
	HBITMAP hPressureGauge = m_pclIndSelPMParams->m_pclSelectPMList->GetPressureGauge( dp0, dpa, dpe, dPSV, clOutputInterface.GetCWndPMGraphsOutput() );

	if( NULL == hPressureGauge )
	{
		return false;
	}

	clOutputInterface.AddBitmap( hPressureGauge );

	_SetPMGraphsOutputContext( false );

	return true;
}

long CRViewSSelPM::_FillTecBoxCompressoSheet( CDS_SSelPMaint *pclEditedSSelPM )
{
	if( NULL == m_pclIndSelPMParams || NULL == m_pclIndSelPMParams->m_pclSelectPMList || NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser() 
			|| NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetPMWQSelectionPreferences() )
	{
		ASSERTA_RETURN( -1 );
	}

	// Call 'MultiSpreadBase' method to get a new sheet.
	CSheetDescription *pclSDTecBoxCompresso = CreateSSheet( SD_TecBoxCompresso );

	if( NULL == pclSDTecBoxCompresso || NULL == pclSDTecBoxCompresso->GetSSheetPointer() )
	{
		return -1;
	}

	CSSheet *pclSSheet = pclSDTecBoxCompresso->GetSSheetPointer();
	pclSDTecBoxCompresso->SetGroupSheetDescriptionID( SD_TecBoxCompresso );

	pclSSheet->SetBool( SSB_REDRAW, FALSE );

	_InitTecBoxCompressoHeaders( pclSDTecBoxCompresso );

	m_lSelectedRow = -1;
	m_lProductTotalCount = 0;
	m_lProductNotPriorityCount = 0;
	m_bShowAllPrioritiesShown = false;
	m_lShowAllPrioritiesButtonRow = 0;
	long lRow = RD_TecBox_FirstAvailRow;
	pclSDTecBoxCompresso->RestartRemarkGenerator();

	CSelectedPMBase *pclSelectedBestPriceProduct = NULL;

	if( true == TASApp.IsPMPriceIndexUsed() )
	{
		pclSelectedBestPriceProduct = m_pclIndSelPMParams->m_pclSelectPMList->GetBestPriceIndexProduct( CSelectPMList::BPIS_Compresso );
	}

	bool bNormSwiss = m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->IsNorm( ProjectType::All, PN_SWKIHE301_01 );
	bool bCheckPriority = true;

	// Priority/NotaPriority loop.
	// Remark: Now we show all when user selects a specific pressure maintenance system.
	int iEndLoop = ( true == bCheckPriority ) ? 2 : 1;

	for( int iLoop = 0; iLoop < iEndLoop; iLoop++ )
	{
		for( CSelectedPMBase *pclSelectedProduct = m_pclIndSelPMParams->m_pclSelectPMList->GetFirstProduct( CSelectPMList::PT_Compresso );
				NULL != pclSelectedProduct; pclSelectedProduct = m_pclIndSelPMParams->m_pclSelectPMList->GetNextProduct( CSelectPMList::PT_Compresso ) )
		{
			// Skip not Priority when iLoop == 0
			if( true == bCheckPriority && ( ( 0 == iLoop ) == pclSelectedProduct->IsFlagSet( CSelectedPMBase::eNotPriority ) ) )
			{
				continue;
			}

			CSelectedCompresso *pclSelectedTecBoxCompresso = dynamic_cast<CSelectedCompresso *>( pclSelectedProduct );

			if( NULL == pclSelectedTecBoxCompresso )
			{
				continue;
			}

			long lReturn = _FillTecBoxCompressoRow( pclSDTecBoxCompresso, lRow, pclSelectedTecBoxCompresso, m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser(),
					bCheckPriority, pclEditedSSelPM, pclSelectedBestPriceProduct );

			if( -1 == lReturn )
			{
				continue;
			}
			else
			{
				lRow = lReturn;
			}

			lRow++;
			m_lProductTotalCount++;
		}
	}

	long lLastDataRow = lRow - 1;
	pclSDTecBoxCompresso->SetUserVariable( _SDUV_TOTALPRODUCT_COUNT, m_lProductTotalCount );
	pclSDTecBoxCompresso->SetUserVariable( _SDUV_NOPRIORITYPRODUCT_COUNT, m_lProductNotPriorityCount );

	// Now we can create Show/Hide all priorities if needed.
	if( true == bCheckPriority )
	{
		// Remarks: 2nd condition to avoid to have button if we have only no priority valves.
		if( true == m_bShowAllPrioritiesShown && m_lProductTotalCount > m_lProductNotPriorityCount )
		{
			// Create Show/Hide all priorities button in regards to current state!
			// HYS-1539 : The button state is the same for all sheet
			CCDButtonShowAllPriorities::ButtonState eState = ( true == GetShowAllPrioritiesFlag( SD_TecBoxCompresso ) ) ? 
					CCDButtonShowAllPriorities::ButtonState::HidePriorities : CCDButtonShowAllPriorities::ButtonState::ShowPriorities;

			CCDButtonShowAllPriorities *pclShowAllButton = CreateShowAllPrioritiesButton( CD_TecBox_FirstColumn, m_lShowAllPrioritiesButtonRow, 
					m_bShowAllPrioritiesShown, eState, m_lShowAllPrioritiesButtonRow + 1, pclSSheet->GetMaxRows(), pclSDTecBoxCompresso );

			// Here we must call 'ApplyInternalChange' to show or hide rows in regards to the current state!
			if( NULL != pclShowAllButton )
			{
				pclShowAllButton->ApplyInternalChange();
			}
		}
		else
		{
			if( m_lShowAllPrioritiesButtonRow != 0 )
			{
				pclSSheet->ShowRow( m_lShowAllPrioritiesButtonRow, false );
			}
		}
	}

	pclSSheet->SetCellBorder( CD_TecBox_FirstColumn + 1, lLastDataRow, CD_TecBox_Pointer - 1, lLastDataRow, true, SS_BORDERTYPE_BOTTOM, 
			SS_BORDERSTYLE_SOLID, _BLACK );

	// Add link to the excluded products if exist.
	lRow = _AddExcludedProductsLink( pclSDTecBoxCompresso, lRow, CSelectPMList::PT_Compresso );

	// Add Remarks.
	// Note: these lines must be placed after creating Show/Hide all priorities button to avoid to take in count these lines when defining
	//       range of row to be Show/Hide.
	lRow = pclSDTecBoxCompresso->WriteRemarks( lRow, CD_TecBox_FirstColumn + 1, CD_TecBox_Pointer );

	// Add possibility to change column size.
	// Remark: place this code after any initialization (like adding, removing, resizing, hiding/showing columns).
	CResizingColumnInfo *pclResizingColumnInfo = pclSDTecBoxCompresso->GetResizingColumnInfo();
	pclResizingColumnInfo->ActivateFeature( true, false, RD_TecBox_FirstAvailRow, lLastDataRow );
	pclResizingColumnInfo->AddRangeColumn( CD_TecBox_PriceIndexBorder, CD_TecBox_Pointer - 1, RD_TecBox_ColName, RD_TecBox_ColUnit );

	// Add possibility to click on title group to reset column width.
	CreateTitleGroupButtons( CD_TecBox_FirstColumn + 1, CD_TecBox_Pointer - 1, RD_TecBox_GroupName, pclSDTecBoxCompresso );

	// Create the button to allow to expand/collapse all the group.
	m_mapButtonExpandCollapseGroup[SD_TecBoxCompresso] = CreateExpandCollapseGroupButton( CD_TecBox_FirstColumn, RD_TecBox_GroupName, true,
			CCDButtonExpandCollapseGroup::ButtonState::CollapseRow, RD_TecBox_ColName, pclSSheet->GetMaxRows(), pclSDTecBoxCompresso );

	if( NULL != m_mapButtonExpandCollapseGroup[SD_TecBoxCompresso] )
	{
		m_mapButtonExpandCollapseGroup[SD_TecBoxCompresso]->SetShowStatus( true );
	}

	pclSSheet->SetBool( SSB_REDRAW, TRUE );
	return m_lSelectedRow;
}

void CRViewSSelPM::_InitTecBoxCompressoHeaders( CSheetDescription *pclSDTecBoxCompresso )
{
	if( NULL == pclSDTecBoxCompresso || NULL == pclSDTecBoxCompresso->GetSSheetPointer() )
	{
		ASSERT_RETURN;
	}

	CSSheet *pclSSheet = pclSDTecBoxCompresso->GetSSheetPointer();

	pclSSheet->Init();
	pclSSheet->SetBool( SSB_ALLOWDRAGDROP, FALSE );
	pclSSheet->SetBool( SSB_HORZSCROLLBAR, FALSE );
	pclSSheet->SetBool( SSB_VERTSCROLLBAR, FALSE );
	pclSSheet->SetBool( SSB_MOVEACTIVEONFOCUS, FALSE );
	pclSSheet->SetTextTip( SS_TT_STATUS_FLOATING, 250, NULL, _LIGHTYELLOW, _TAH_BLACK );

	// Set max rows.
	pclSSheet->SetMaxRows( RD_TecBox_FirstAvailRow - 1 );

	// All cells are static by default and filled with _T("").
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	pclSSheet->FormatStaticText( -1, -1, -1, -1, _T( "" ) );

	// Increase row height.
	double dRowHeight = 12.75;
	pclSSheet->SetRowHeight( RD_TecBox_FirstRow, dRowHeight * 0.5 );
	pclSSheet->SetRowHeight( RD_TecBox_GroupName, dRowHeight * 1.5 );
	pclSSheet->SetRowHeight( RD_TecBox_ColName, dRowHeight * 2 );
	pclSSheet->SetRowHeight( RD_TecBox_ColUnit, dRowHeight * 1.2 );

	// Initialize.
	pclSDTecBoxCompresso->Init( 1, pclSSheet->GetMaxRows(), 1, CSheetDescription::SD_ParameterMode_Mono );
	pclSDTecBoxCompresso->AddColumnInPixels( CD_TecBox_FirstColumn, m_mapSSheetColumnWidth[SD_TecBoxCompresso][CD_TecBox_FirstColumn] );
	pclSDTecBoxCompresso->AddColumnInPixels( CD_TecBox_CheckBox, m_mapSSheetColumnWidth[SD_TecBoxCompresso][CD_TecBox_CheckBox] );
	
	if( true == TASApp.IsPMPriceIndexUsed() )
	{
		pclSDTecBoxCompresso->AddColumnInPixels( CD_TecBox_PriceIndexIcon, m_mapSSheetColumnWidth[SD_TecBoxCompresso][CD_TecBox_PriceIndexIcon] );
		pclSDTecBoxCompresso->AddColumnInPixels( CD_TecBox_PriceIndexValue, m_mapSSheetColumnWidth[SD_TecBoxCompresso][CD_TecBox_PriceIndexValue] );
		pclSDTecBoxCompresso->AddColumnInPixels( CD_TecBox_PriceIndexBorder, m_mapSSheetColumnWidth[SD_TecBoxCompresso][CD_TecBox_PriceIndexBorder] );
	}

	pclSDTecBoxCompresso->AddColumnInPixels( CD_TecBox_Name, m_mapSSheetColumnWidth[SD_TecBoxCompresso][CD_TecBox_Name] );
	pclSDTecBoxCompresso->AddColumnInPixels( CD_TecBox_NbreOfDevice, m_mapSSheetColumnWidth[SD_TecBoxCompresso][CD_TecBox_NbreOfDevice] );
	pclSDTecBoxCompresso->AddColumnInPixels( CD_TecBox_PS, m_mapSSheetColumnWidth[SD_TecBoxCompresso][CD_TecBox_PS] );
	pclSDTecBoxCompresso->AddColumnInPixels( CD_TecBox_TempRange, m_mapSSheetColumnWidth[SD_TecBoxCompresso][CD_TecBox_TempRange] );
	pclSDTecBoxCompresso->AddColumnInPixels( CD_TecBox_MaxWaterMakeUpTemp, m_mapSSheetColumnWidth[SD_TecBoxCompresso][CD_TecBox_MaxWaterMakeUpTemp] );
	pclSDTecBoxCompresso->AddColumnInPixels( CD_TecBox_B, m_mapSSheetColumnWidth[SD_TecBoxCompresso][CD_TecBox_B] );
	pclSDTecBoxCompresso->AddColumnInPixels( CD_TecBox_H, m_mapSSheetColumnWidth[SD_TecBoxCompresso][CD_TecBox_H] );
	pclSDTecBoxCompresso->AddColumnInPixels( CD_TecBox_T, m_mapSSheetColumnWidth[SD_TecBoxCompresso][CD_TecBox_T] );
	pclSDTecBoxCompresso->AddColumnInPixels( CD_TecBox_Weight, m_mapSSheetColumnWidth[SD_TecBoxCompresso][CD_TecBox_Weight] );
	pclSDTecBoxCompresso->AddColumnInPixels( CD_TecBox_Standing, m_mapSSheetColumnWidth[SD_TecBoxCompresso][CD_TecBox_Standing] );
	pclSDTecBoxCompresso->AddColumnInPixels( CD_TecBox_ElectricalPower, m_mapSSheetColumnWidth[SD_TecBoxCompresso][CD_TecBox_ElectricalPower] );
	pclSDTecBoxCompresso->AddColumnInPixels( CD_TecBox_SupplyVoltage, m_mapSSheetColumnWidth[SD_TecBoxCompresso][CD_TecBox_SupplyVoltage] );
	pclSDTecBoxCompresso->AddColumnInPixels( CD_TecBox_Decibel, m_mapSSheetColumnWidth[SD_TecBoxCompresso][CD_TecBox_Decibel] );
	pclSDTecBoxCompresso->AddColumnInPixels( CD_TecBox_BufferVessel, m_mapSSheetColumnWidth[SD_TecBoxCompresso][CD_TecBox_BufferVessel] );
	pclSDTecBoxCompresso->AddColumnInPixels( CD_TecBox_IntegratedVessel, m_mapSSheetColumnWidth[SD_TecBoxCompresso][CD_TecBox_IntegratedVessel] );
	pclSDTecBoxCompresso->AddColumnInPixels( CD_TecBox_Pointer, m_mapSSheetColumnWidth[SD_TecBoxCompresso][CD_TecBox_Pointer] );

	// Must be placed after any call to 'AddColumnInPixels'. Because this method will add needed columns and TSpread will show them by default.
	// When calling the first time 'AddColumnInPixels' for the 'CD_TecBox_Name' column, this method will call TSpread::SetMaxCols() to adapt the
	// number of columns. All the column before 'CD_TecBox_Name' are set to a default width and are visible.
	if( false == TASApp.IsPMPriceIndexUsed() )
	{
		pclSSheet->ShowCol( CD_TecBox_PriceIndexIcon, FALSE );
		pclSSheet->ShowCol( CD_TecBox_PriceIndexValue, FALSE );
		pclSSheet->ShowCol( CD_TecBox_PriceIndexBorder, FALSE );
	}

	pclSSheet->ShowCol( CD_TecBox_BufferVessel, FALSE );

	// Set in which column parameter must be saved.
	pclSDTecBoxCompresso->AddParameterColumn( CD_TecBox_Pointer );

	// Set the focus column.
	pclSDTecBoxCompresso->SetActiveColumn( CD_TecBox_FirstColumn + 1 );

	// Set range for selection.
	pclSDTecBoxCompresso->SetFocusColumnRange( CD_TecBox_FirstColumn + 1, CD_TecBox_Pointer - 1 );

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Add the main title.
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, _IMI_TITLE_GROUP1 );
	pclSDTecBoxCompresso->SetUserVariable( _SDUV_TITLEFORECOLOR, ( pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor ) ) );
	pclSDTecBoxCompresso->SetUserVariable( _SDUV_TITLEBACKCOLOR, ( pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor ) ) );

	pclSSheet->AddCellSpanW( CD_TecBox_FirstColumn + 1, RD_TecBox_GroupName, CD_TecBox_Pointer - ( CD_TecBox_FirstColumn + 1 ), 1 );
	pclSSheet->SetStaticText( CD_TecBox_FirstColumn + 1, RD_TecBox_GroupName, IDS_SSHEETSSELPM_COMPGROUP );
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Column name.
	if( false == pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn ) )
	{
		return;
	}

	if( false == pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE ) )
	{
		return;
	}

	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap, (LPARAM)TRUE );

	if( true == TASApp.IsPMPriceIndexUsed() )
	{
		pclSSheet->AddCellSpan( CD_TecBox_PriceIndexIcon, RD_TecBox_ColName, 3, 1 );
		pclSSheet->SetStaticText( CD_TecBox_PriceIndexIcon, RD_TecBox_ColName, IDS_SSHEETSSELPM_PRICEINDEX );
	}

	pclSSheet->SetStaticText( CD_TecBox_Name, RD_TecBox_ColName, IDS_SSHEETSSELPM_CPNAME );
	pclSSheet->SetStaticText( CD_TecBox_NbreOfDevice, RD_TecBox_ColName, IDS_SSHEETSSELPM_TBNUMBEROFDEVICES );
	pclSSheet->SetStaticText( CD_TecBox_PS, RD_TecBox_ColName, IDS_SSHEETSSELPM_CPPS );
	pclSSheet->SetStaticText( CD_TecBox_TempRange, RD_TecBox_ColName, IDS_SSHEETSSELPM_VESSELTEMPRANGE );
	pclSSheet->SetStaticText( CD_TecBox_MaxWaterMakeUpTemp, RD_TecBox_ColName, IDS_SSHEETSSELPM_MAXWATERMAKEUPTEMP );
	pclSSheet->SetStaticText( CD_TecBox_B, RD_TecBox_ColName, IDS_SSHEETSSELPM_CPWIDTH );
	pclSSheet->SetStaticText( CD_TecBox_H, RD_TecBox_ColName, IDS_SSHEETSSELPM_CPHEIGHT );
	pclSSheet->SetStaticText( CD_TecBox_T, RD_TecBox_ColName, IDS_SSHEETSSELPM_CPDEPTH );
	pclSSheet->SetStaticText( CD_TecBox_Weight, RD_TecBox_ColName, IDS_SSHEETSSELPM_CPMASS );
	pclSSheet->SetStaticText( CD_TecBox_ElectricalPower, RD_TecBox_ColName, IDS_SSHEETSSELPM_CPELECPOWER );
	pclSSheet->SetStaticText( CD_TecBox_SupplyVoltage, RD_TecBox_ColName, IDS_SSHEETSSELPM_CPELECVOLTAGE );
	pclSSheet->SetStaticText( CD_TecBox_Decibel, RD_TecBox_ColName, IDS_SSHEETSSELPM_TBSOUNDPRESSLEVEL );
	pclSSheet->SetStaticText( CD_TecBox_IntegratedVessel, RD_TecBox_ColName, IDS_SSHEETSSELPM_INTEGRATEDVESSEL );
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Column units.
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap, (LPARAM)FALSE );

	if( true == TASApp.IsPMPriceIndexUsed() )
	{
		pclSSheet->SetStaticText( CD_TecBox_PriceIndexValue, RD_TecBox_ColUnit, _T("%") );
	}

	pclSSheet->SetStaticText( CD_TecBox_PS, RD_TecBox_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_PRESSURE ) ).c_str() );
	pclSSheet->SetStaticText( CD_TecBox_TempRange, RD_TecBox_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_TEMPERATURE ) ).c_str() );
	pclSSheet->SetStaticText( CD_TecBox_MaxWaterMakeUpTemp, RD_TecBox_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_TEMPERATURE ) ).c_str() );
	pclSSheet->SetStaticText( CD_TecBox_B, RD_TecBox_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_LENGTH ) ).c_str() );
	pclSSheet->SetStaticText( CD_TecBox_H, RD_TecBox_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_LENGTH ) ).c_str() );
	pclSSheet->SetStaticText( CD_TecBox_T, RD_TecBox_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_LENGTH ) ).c_str() );
	pclSSheet->SetStaticText( CD_TecBox_Weight, RD_TecBox_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_MASS ) ).c_str() );
	pclSSheet->SetStaticText( CD_TecBox_ElectricalPower, RD_TecBox_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_ELEC_POWER ) ).c_str() );
	pclSSheet->SetStaticText( CD_TecBox_Decibel, RD_TecBox_ColUnit, IDS_SSHEETSSELPM_TBDECIBELUNIT );
	pclSSheet->SetStaticText( CD_TecBox_SupplyVoltage, RD_TecBox_ColUnit, IDS_SSHEETSSELPM_TBVOLTUNIT );
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// By default we don't show these columns.
	pclSSheet->ShowCol( CD_TecBox_MaxWaterMakeUpTemp, FALSE );
	pclSSheet->ShowCol( CD_TecBox_IntegratedVessel, FALSE );

	// Draw black line below column header.
	pclSSheet->SetCellBorder( CD_TecBox_FirstColumn + 1, RD_TecBox_ColUnit, CD_TecBox_Pointer - 1, RD_TecBox_ColUnit, true, SS_BORDERTYPE_BOTTOM );
}

long CRViewSSelPM::_FillTecBoxCompressoRow( CSheetDescription *pclSDTecBoxCompresso, long lRow, CSelectedCompresso *pclSelectedTecBoxCompresso,
		CPMInputUser *pclPMInputUser, bool bCheckPriority, CDS_SSelPMaint *pclEditedSSelPM, CSelectedPMBase *pclSelectedBestPriceProduct, bool bForRejectedProduct )
{
	if( NULL == pclSDTecBoxCompresso || NULL == pclSDTecBoxCompresso->GetSSheetPointer() || NULL == pclSelectedTecBoxCompresso || NULL == pclPMInputUser )
	{
		return -1;
	}

	CDB_TecBox *pclTecBoxCompresso = dynamic_cast<CDB_TecBox *>( pclSelectedTecBoxCompresso->GetpData() );

	if( NULL == pclTecBoxCompresso )
	{
		return -1;
	}

	CSSheet *pclSSheet = pclSDTecBoxCompresso->GetSSheetPointer();

	bool bRedundancy = pclPMInputUser->GetPMWQSelectionPreferences()->IsChecked( CPMWQPrefs::GeneralRedundancyTecBox );

	if( true == bCheckPriority && true == pclSelectedTecBoxCompresso->IsFlagSet( CSelectedPMBase::eNotPriority ) )
	{
		m_lProductNotPriorityCount++;

		if( false == m_bShowAllPrioritiesShown )
		{
			// Add one empty not selectable row.
			pclSDTecBoxCompresso->AddRows( 1 );

			pclSSheet->SetCellBorder( CD_TecBox_FirstColumn + 1, lRow, CD_TecBox_Pointer - 1, lRow, true, SS_BORDERTYPE_TOP | SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );

			m_lShowAllPrioritiesButtonRow = lRow++;
			m_bShowAllPrioritiesShown = true;
		}
	}

	// Add this row.
	// 'true' to specify that this row can be selected.
	pclSDTecBoxCompresso->AddRows( 1, true );

	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

	// Add checkbox.
	pclSSheet->SetCheckBox(CD_TecBox_CheckBox, lRow, _T(""), false, true);

	// Set the price index if used.
	if( true == TASApp.IsPMPriceIndexUsed() )
	{
		if( NULL != pclSelectedBestPriceProduct && 
			0 == _tcscmp( pclSelectedBestPriceProduct->GetProductIDPtr().ID, pclSelectedTecBoxCompresso->GetProductIDPtr().ID ) )
		{
			pclSSheet->SetPictureCellWithID( IDI_BESTPRICE, CD_TecBox_PriceIndexIcon, lRow, CSSheet::PictureCellType::Icon );
		}

		CString str = _T("-");

		if( pclSelectedTecBoxCompresso->GetPriceIndex() > 0.0 )
		{
			str.Format( _T("%i"), (int)pclSelectedTecBoxCompresso->GetPriceIndex() );
		}

		pclSSheet->SetStaticText( CD_TecBox_PriceIndexValue, lRow, str );
	}

	bool bBest = pclSelectedTecBoxCompresso->IsFlagSet( CSelectedPMBase::eBest );

	if( true == bBest )
	{
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_GREEN );
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, true );
	}

	// Set name.
	pclSSheet->SetStaticText( CD_TecBox_Name, lRow, pclTecBoxCompresso->GetName() );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, false );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

	int iNbrOfDevices = 1;

	// HYS-599: Check if the Compresso is a CX model and in this case check how many devices we need.
	if( true == pclTecBoxCompresso->IsVariantExternalAir() )
	{
		iNbrOfDevices = pclSelectedTecBoxCompresso->GetCompressoCXNumber();
	}
	// HYS-1537 : Check if the preference is enabled
	if( true == bRedundancy && false == pclPMInputUser->GetPMWQSelectionPreferences()->IsChecked( CPMWQPrefs::GeneralRedundancyTecBox ) )
	{
		iNbrOfDevices *= 2;
	}

	CString str;
	str.Format( _T("%i"), iNbrOfDevices );
	pclSSheet->SetStaticText( CD_TecBox_NbreOfDevice, lRow, str );

	pclSSheet->SetStaticText( CD_TecBox_PS, lRow, WriteCUDouble( _U_PRESSURE, pclTecBoxCompresso->GetPmaxmax() ) );

	if( true == pclSelectedTecBoxCompresso->IsFlagSet( CSelectedPMBase::eTemperature ) )
	{
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_ORANGE );
	}

	pclSSheet->SetStaticText( CD_TecBox_TempRange, lRow, pclTecBoxCompresso->GetTempRange() );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

	if( true == pclTecBoxCompresso->IsFctWaterMakeUp() )
	{
		pclSSheet->ShowCol( CD_TecBox_MaxWaterMakeUpTemp, TRUE );

		// Remark: for Compresso we don't have the min. admissible temperature for the integrated water make-up part.
		if( true == pclSelectedTecBoxCompresso->IsFlagSet( CSelectedPMBase::ePlenoWMUpMaxTemp ) )
		{
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_RED );
		}

		pclSSheet->SetStaticText( CD_TecBox_MaxWaterMakeUpTemp, lRow, WriteCUDouble( _U_TEMPERATURE, pclTecBoxCompresso->GetTmaxWaterMakeUp() ) );
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );
	}

	pclSSheet->SetStaticText( CD_TecBox_B, lRow, WriteCUDouble( _U_LENGTH, pclTecBoxCompresso->GetWidth() ) );
	pclSSheet->SetStaticText( CD_TecBox_H, lRow, WriteCUDouble( _U_LENGTH, pclTecBoxCompresso->GetHeight() ) );
	pclSSheet->SetStaticText( CD_TecBox_T, lRow, WriteCUDouble( _U_LENGTH, pclTecBoxCompresso->GetDepth() ) );
	pclSSheet->SetStaticText( CD_TecBox_Weight, lRow, WriteCUDouble( _U_MASS, pclTecBoxCompresso->GetWeight() ) );
	pclSSheet->SetPictureCellWithID( pclTecBoxCompresso->GetInstallationLayoutPictureID(), CD_TecBox_Standing, lRow, CSSheet::PictureCellType::Icon );
	pclSSheet->SetStaticText( CD_TecBox_ElectricalPower, lRow, WriteCUDouble( _U_ELEC_POWER, pclTecBoxCompresso->GetPower() ) );
	pclSSheet->SetStaticText( CD_TecBox_SupplyVoltage, lRow, pclTecBoxCompresso->GetPowerSupplyStr() );

	pclSSheet->SetStaticText( CD_TecBox_Decibel, lRow, WriteDouble( pclTecBoxCompresso->GetSoundPressureLevel(), 1 ) );

	// Check if there is an integrated vessel.
	CDB_Vessel *pclIntegratedVessel = dynamic_cast<CDB_Vessel *>( pclTecBoxCompresso->GetIntegratedVesselIDPtr().MP );

	if( NULL != pclIntegratedVessel )
	{
		pclSSheet->ShowCol( CD_TecBox_IntegratedVessel, TRUE );
		pclSSheet->SetStaticText( CD_TecBox_IntegratedVessel, lRow, pclIntegratedVessel->GetName() );
	}

	// Save parameter.
	CCellDescriptionProduct *pclCDProduct = CreateCellDescriptionProduct( pclSDTecBoxCompresso->GetFirstParameterColumn(), lRow, 
			(LPARAM)pclTecBoxCompresso, pclSDTecBoxCompresso );

	if( NULL != pclCDProduct )
	{
		pclCDProduct->SetUserParam( (LPARAM)pclSelectedTecBoxCompresso );
	}

	CDB_TecBox *pclEditedTecBoxCompresso = NULL;

	if( NULL != pclEditedSSelPM )
	{
		pclEditedTecBoxCompresso = dynamic_cast<CDB_TecBox *>( pclEditedSSelPM->GetTecBoxCompTransfIDPtr().MP );
	}

	if( NULL != pclEditedTecBoxCompresso && pclEditedTecBoxCompresso == pclTecBoxCompresso )
	{
		m_lSelectedRow = lRow;
	}

	pclSSheet->SetCellBorder( CD_TecBox_FirstColumn + 1, lRow, CD_TecBox_Pointer - 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _GRAY );

	return lRow;
}

bool CRViewSSelPM::_FillCompressoCurves( CDB_TecBox *pclCompressoSelected, int iNbrOfDevices )
{
	if( NULL == m_pclIndSelPMParams || NULL == m_pclIndSelPMParams->m_pclSelectPMList || NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERTA_RETURN( false );
	}

	if( NULL == pclCompressoSelected || false == pclCompressoSelected->IsFctCompressor() )
	{
		return false;
	}

	CDlgPMGraphsOutput::CPMInterface clOutputInterface;
	clOutputInterface.ClearOutput();

	double dPman = m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetTargetPressureForTecBox( pclCompressoSelected->GetTecBoxType() );
	double dVD = m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetVD();

	CDlgPMTChartHelper *pclCompressoCurve = m_pclIndSelPMParams->m_pclSelectPMList->GetCompressoCurves( pclCompressoSelected, dPman, dVD,
			clOutputInterface.GetCWndPMGraphsOutput(), iNbrOfDevices );

	if( NULL == pclCompressoCurve )
	{
		return false;
	}

	clOutputInterface.AddTChartDialog( pclCompressoCurve );

	double dp0 = m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetMinimumPressure();
	double dpa = m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetMinimumInitialPressure();
	double dpe = m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetFinalPressure( CDB_TecBox::etbtCompresso );
	double dPSV = m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetSafetyValveResponsePressure();
	HBITMAP hPressureGauge = m_pclIndSelPMParams->m_pclSelectPMList->GetPressureGauge( dp0, dpa, dpe, dPSV, clOutputInterface.GetCWndPMGraphsOutput() );

	if( NULL == hPressureGauge )
	{
		return false;
	}

	clOutputInterface.AddBitmap( hPressureGauge );

	_SetPMGraphsOutputContext( false );

	return true;
}

void CRViewSSelPM::_FillPreCompressoVesselSheet( CSelectedPMBase *pclSelectedCompresso, UINT uiSDGroupID )
{
	if( NULL == m_pclIndSelPMParams || NULL == m_pclIndSelPMParams->m_pclSelectPMList )
	{
		ASSERT_RETURN;
	}

	if( NULL == pclSelectedCompresso || NULL == dynamic_cast<CDB_TecBox*>( pclSelectedCompresso->GetProductIDPtr().MP ) )
	{
		return;
	}

	CDB_TecBox *pclCompresso = dynamic_cast<CDB_TecBox*>( pclSelectedCompresso->GetProductIDPtr().MP );

	if( true == pclCompresso->IsVariantIntegratedPrimaryVessel() )
	{
		// Simply Compresso is in particular case. We have the primary vessel inside the TecBox, and here we can show the only one secondary
		// vessel available if needed ( 80l < VN <= 160l ).
		_FillCompressoSecondaryVsslSheet( uiSDGroupID );

		// Check also here if we need to add intermediate vessel. Effectively, if there is no secondary vessel sheet (and not the primary vessel sheet because
		// integrated to the Compresso), we need to automatically show the intermediate vessels.
		CDB_Vessel *pclSecondaryVessel = NULL;
		GetCompressoSecondaryVesselSelected( &pclSecondaryVessel );

		if( NULL == pclSecondaryVessel )
		{
			CSelectedVssl *pclSelCompressoVssl = dynamic_cast<CSelectedVssl *>( m_pclIndSelPMParams->m_pclSelectPMList->GetFirstProduct( CSelectPMList::PT_CompressoVessel ) );
			_FillIntermediateVsslSheet( SD_IntermediateVessel, uiSDGroupID, SD_TecBoxCompresso, pclSelCompressoVssl );
		}
	}
	else
	{
		_FillCompressoVsslSheet( uiSDGroupID );
	}
}

void CRViewSSelPM::_FillCompressoVsslSheet( UINT uiSDGroupID )
{
	if( NULL == m_pclIndSelPMParams || NULL == m_pclIndSelPMParams->m_pclSelectPMList || NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERT_RETURN;
	}

	// Get 'Parent' sheet identification.
	UINT uiAfterSheetDescriptionID = _GetAfterSheetDescriptionID( SD_CompressoVessel, uiSDGroupID );

	// Call 'MultiSpreadBase' method to get a new sheet.
	CSheetDescription *pclSDCmprssVssl = CreateSSheet( SD_CompressoVessel, uiAfterSheetDescriptionID );

	if( NULL == pclSDCmprssVssl || NULL == pclSDCmprssVssl->GetSSheetPointer() )
	{
		return;
	}

	pclSDCmprssVssl->SetGroupSheetDescriptionID( uiSDGroupID );
	pclSDCmprssVssl->SetParentSheetDescriptionID( SD_CompressoVessel );
	CSSheet *pclSSheet = pclSDCmprssVssl->GetSSheetPointer();

	pclSSheet->SetBool( SSB_REDRAW, FALSE );

	_InitCompressoVsslHeaders( pclSDCmprssVssl, m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser() );

	m_lProductTotalCount = 0;
	m_lProductNotPriorityCount = 0;
	m_bShowAllPrioritiesShown = false;
	m_lShowAllPrioritiesButtonRow = 0;
	long lRow = RD_TecBoxVssl_FirstAvailRow;
	pclSDCmprssVssl->RestartRemarkGenerator();

	CSelectedPMBase* pclSelectedBestPriceProduct = NULL;

	if( true == TASApp.IsPMPriceIndexUsed() )
	{
		pclSelectedBestPriceProduct = m_pclIndSelPMParams->m_pclSelectPMList->GetLowestPriceObject( CSelectPMList::ProductType::PT_CompressoVessel );
	}

	// Priority/NotaPriority loop.
	for( int iLoop = 1; iLoop >= 0; --iLoop )
	{
		for( CSelectedPMBase *pclSelectedProduct = m_pclIndSelPMParams->m_pclSelectPMList->GetLastProduct( CSelectPMList::PT_CompressoVessel );
				NULL != pclSelectedProduct; pclSelectedProduct = m_pclIndSelPMParams->m_pclSelectPMList->GetPrevProduct( CSelectPMList::PT_CompressoVessel ) )
		{
			// Skip not Priority when iLoop == 1
			if( ( iLoop == 1 ) == pclSelectedProduct->IsFlagSet( CSelectedPMBase::eNotPriority ) )
			{
				continue;
			}

			CSelectedVssl *pclSelCompressoVssl = dynamic_cast<CSelectedVssl *>( pclSelectedProduct );

			if( NULL == pclSelCompressoVssl )
			{
				continue;
			}

			long lReturn = _FillCompressoVsslRow( pclSDCmprssVssl, lRow, pclSelCompressoVssl, m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser(),
					pclSelectedBestPriceProduct );

			if( -1 == lReturn )
			{
				continue;
			}
			else
			{
				lRow = lReturn;
			}

			lRow++;
			m_lProductTotalCount++;
		}
	}

	long lLastDataRow = lRow - 1;
	pclSDCmprssVssl->SetUserVariable( _SDUV_TOTALPRODUCT_COUNT, m_lProductTotalCount );
	pclSDCmprssVssl->SetUserVariable( _SDUV_NOPRIORITYPRODUCT_COUNT, m_lProductNotPriorityCount );

	// Now we can create Show/Hide all priorities if needed.
	// Remarks: 2nd condition to avoid to have button if we have only no priority valves.
	if( true == m_bShowAllPrioritiesShown && m_lProductTotalCount > m_lProductNotPriorityCount )
	{
		// Create Show/Hide all priorities button in regards to current state!
		// HYS-1539 : The button state is the same for all sheet
		CCDButtonShowAllPriorities::ButtonState eState = ( true == GetShowAllPrioritiesFlag( SD_CompressoVessel ) ) ? 
				CCDButtonShowAllPriorities::ButtonState::HidePriorities : CCDButtonShowAllPriorities::ButtonState::ShowPriorities;
		
		CCDButtonShowAllPriorities *pclShowAllButton = CreateShowAllPrioritiesButton( CD_TecBoxVssl_FirstColumn, m_lShowAllPrioritiesButtonRow,
				m_bShowAllPrioritiesShown, eState, m_lShowAllPrioritiesButtonRow + 1, pclSSheet->GetMaxRows(), pclSDCmprssVssl );

		// Here we must call 'ApplyInternalChange' to show or hide rows in regards to the current state!
		if( NULL != pclShowAllButton )
		{
			pclShowAllButton->ApplyInternalChange();
		}
	}
	else
	{
		if( m_lShowAllPrioritiesButtonRow != 0 )
		{
			pclSSheet->ShowRow( m_lShowAllPrioritiesButtonRow, false );
		}
	}

	pclSSheet->SetCellBorder( CD_TecBoxVssl_FirstColumn + 1, lLastDataRow, CD_TecBoxVssl_Pointer - 1, lLastDataRow, true, SS_BORDERTYPE_BOTTOM, 
			SS_BORDERSTYLE_SOLID, _BLACK );

	// Add link to the excluded products if exist.
	lRow = _AddExcludedProductsLink( pclSDCmprssVssl, lRow, CSelectPMList::PT_CompressoVessel );

	// Add Remarks.
	// Note: these lines must be placed after creating Show/Hide all priorities button to avoid to take in count these lines when defining
	//       range of row to be Show/Hide.
	lRow = pclSDCmprssVssl->WriteRemarks( lRow, CD_TecBoxVssl_Prim_Name, CD_TecBoxVssl_Pointer );

	// Add possibility to change column size.
	// Remark: place this code after any initialization (like adding, removing, resizing, hiding/showing columns).
	CResizingColumnInfo *pclResizingColumnInfo = pclSDCmprssVssl->GetResizingColumnInfo();
	pclResizingColumnInfo->ActivateFeature( true, false, RD_TecBoxVssl_FirstAvailRow, lLastDataRow );
	pclResizingColumnInfo->AddRangeColumn( CD_TecBoxVssl_PriceIndexRatioBorder, CD_TecBoxVssl_Pointer - 1, RD_TecBoxVssl_ColName, RD_TecBoxVssl_ColUnit );

	// Add possibility to click on title group to reset column width.
	CreateTitleGroupButtons( CD_TecBoxVssl_FirstColumn + 1, CD_TecBoxVssl_Pointer - 1, RD_TecBoxVssl_GroupName, pclSDCmprssVssl );

	// Create the button to allow to expand/collapse all the group.
	// 	m_mapButtonExpandCollapseGroup[SD_CompressoPrimSecVssl] = CreateExpandCollapseGroupButton( CD_Compresso_FirstColumn, RD_TecBoxVssl_GroupName, true, CCDButtonExpandCollapseGroup::ButtonState::CollapseRow,
	// 		RD_TecBoxVssl_ColName, pclSSheet->GetMaxRows(), pclSDCmprssVssl );
	// 	if( NULL != m_mapButtonExpandCollapseGroup[SD_CompressoPrimSecVssl] )
	// 		m_mapButtonExpandCollapseGroup[SD_CompressoPrimSecVssl]->SetShowStatus( true );

	pclSSheet->SetBool( SSB_REDRAW, TRUE );
}

void CRViewSSelPM::_InitCompressoVsslHeaders( CSheetDescription *pclSDCompressoVssl, CPMInputUser *pclPMInputUser )
{
	if( NULL == pclSDCompressoVssl || NULL == pclSDCompressoVssl->GetSSheetPointer() )
	{
		return;
	}

	CSSheet *pclSSheet = pclSDCompressoVssl->GetSSheetPointer();

	pclSSheet->Init();
	pclSSheet->SetBool( SSB_ALLOWDRAGDROP, FALSE );
	pclSSheet->SetBool( SSB_HORZSCROLLBAR, FALSE );
	pclSSheet->SetBool( SSB_VERTSCROLLBAR, FALSE );
	pclSSheet->SetBool( SSB_MOVEACTIVEONFOCUS, FALSE );
	pclSSheet->SetTextTip( SS_TT_STATUS_FLOATING, 250, NULL, _LIGHTYELLOW, _TAH_BLACK );

	// Set max rows.
	pclSSheet->SetMaxRows( RD_TecBoxVssl_FirstAvailRow - 1 );

	// All cells are static by default and filled with _T("").
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	pclSSheet->FormatStaticText( -1, -1, -1, -1, _T( "" ) );

	// Increase row height.
	double dRowHeight = 12.75;
	pclSSheet->SetRowHeight( RD_TecBoxVssl_FirstRow, dRowHeight * 0.5 );
	pclSSheet->SetRowHeight( RD_TecBoxVssl_GroupName, dRowHeight * 1.5 );
	pclSSheet->SetRowHeight( RD_TecBoxVssl_ColName, dRowHeight * 2 );
	pclSSheet->SetRowHeight( RD_TecBoxVssl_ColUnit, dRowHeight * 1.2 );

	// Initialize.
	pclSDCompressoVssl->Init( 1, pclSSheet->GetMaxRows(), 1, CSheetDescription::SD_ParameterMode_Mono );
	pclSDCompressoVssl->AddColumnInPixels( CD_TecBoxVssl_FirstColumn, m_mapSSheetColumnWidth[SD_CompressoVessel][CD_TecBoxVssl_FirstColumn] );
	pclSDCompressoVssl->AddColumnInPixels( CD_TecBoxVssl_CheckBox, m_mapSSheetColumnWidth[SD_CompressoVessel][CD_TecBoxVssl_CheckBox] );

	if( true == TASApp.IsPMPriceIndexUsed() )
	{
		pclSDCompressoVssl->AddColumnInPixels( CD_TecBoxVssl_PriceIndexRatioIcon, m_mapSSheetColumnWidth[SD_CompressoVessel][CD_TecBoxVssl_PriceIndexRatioIcon] );
		pclSDCompressoVssl->AddColumnInPixels( CD_TecBoxVssl_PriceIndexRatioValue, m_mapSSheetColumnWidth[SD_CompressoVessel][CD_TecBoxVssl_PriceIndexRatioValue] );
		pclSDCompressoVssl->AddColumnInPixels( CD_TecBoxVssl_PriceIndexRatioBorder, m_mapSSheetColumnWidth[SD_CompressoVessel][CD_TecBoxVssl_PriceIndexRatioBorder] );
	}

	pclSDCompressoVssl->AddColumnInPixels( CD_TecBoxVssl_Prim_Name, m_mapSSheetColumnWidth[SD_CompressoVessel][CD_TecBoxVssl_Prim_Name] );
	pclSDCompressoVssl->AddColumnInPixels( CD_TecBoxVssl_Prim_Volume, m_mapSSheetColumnWidth[SD_CompressoVessel][CD_TecBoxVssl_Prim_Volume] );
	pclSDCompressoVssl->AddColumnInPixels( CD_TecBoxVssl_Prim_MaxPressure, m_mapSSheetColumnWidth[SD_CompressoVessel][CD_TecBoxVssl_Prim_MaxPressure] );
	pclSDCompressoVssl->AddColumnInPixels( CD_TecBoxVssl_Prim_TempRange, m_mapSSheetColumnWidth[SD_CompressoVessel][CD_TecBoxVssl_Prim_TempRange] );
	pclSDCompressoVssl->AddColumnInPixels( CD_TecBoxVssl_Prim_Diameter, m_mapSSheetColumnWidth[SD_CompressoVessel][CD_TecBoxVssl_Prim_Diameter] );
	pclSDCompressoVssl->AddColumnInPixels( CD_TecBoxVssl_Prim_Height, m_mapSSheetColumnWidth[SD_CompressoVessel][CD_TecBoxVssl_Prim_Height] );
	pclSDCompressoVssl->AddColumnInPixels( CD_TecBoxVssl_Prim_Weight, m_mapSSheetColumnWidth[SD_CompressoVessel][CD_TecBoxVssl_Prim_Weight] );
	pclSDCompressoVssl->AddColumnInPixels( CD_TecBoxVssl_Prim_MaxWeight, m_mapSSheetColumnWidth[SD_CompressoVessel][CD_TecBoxVssl_Prim_MaxWeight] );
	pclSDCompressoVssl->AddColumnInPixels( CD_TecBoxVssl_Sec_Name, m_mapSSheetColumnWidth[SD_CompressoVessel][CD_TecBoxVssl_Sec_Name] );
	pclSDCompressoVssl->AddColumnInPixels( CD_TecBoxVssl_Sec_NbreOfVssl, m_mapSSheetColumnWidth[SD_CompressoVessel][CD_TecBoxVssl_Sec_NbreOfVssl] );
	pclSDCompressoVssl->AddColumnInPixels( CD_TecBoxVssl_Sec_Diameter, m_mapSSheetColumnWidth[SD_CompressoVessel][CD_TecBoxVssl_Sec_Diameter] );
	pclSDCompressoVssl->AddColumnInPixels( CD_TecBoxVssl_Sec_Height, m_mapSSheetColumnWidth[SD_CompressoVessel][CD_TecBoxVssl_Sec_Height] );
	pclSDCompressoVssl->AddColumnInPixels( CD_TecBoxVssl_Sec_Weight, m_mapSSheetColumnWidth[SD_CompressoVessel][CD_TecBoxVssl_Sec_Weight] );
	pclSDCompressoVssl->AddColumnInPixels( CD_TecBoxVssl_Sec_MaxWeight, m_mapSSheetColumnWidth[SD_CompressoVessel][CD_TecBoxVssl_Sec_MaxWeight] );
	pclSDCompressoVssl->AddColumnInPixels( CD_TecBoxVssl_Pointer, m_mapSSheetColumnWidth[SD_CompressoVessel][CD_TecBoxVssl_Pointer] );

	// Must be placed after any call to 'AddColumnInPixels'. Because this method will add needed columns and TSpread will show them by default.
	// When calling the first time 'AddColumnInPixels' for the 'CD_TecBoxVssl_Prim_Name' column, this method will call TSpread::SetMaxCols() to adapt the
	// number of columns. All the column before 'CD_TecBoxVssl_Prim_Name' are set to a default width and are visible.
	if( false == TASApp.IsPMPriceIndexUsed() )
	{
		pclSSheet->ShowCol( CD_TecBoxVssl_PriceIndexRatioIcon, FALSE );
		pclSSheet->ShowCol( CD_TecBoxVssl_PriceIndexRatioValue, FALSE );
		pclSSheet->ShowCol( CD_TecBoxVssl_PriceIndexRatioBorder, FALSE );
	}

	// Set in which column parameter must be saved.
	pclSDCompressoVssl->AddParameterColumn( CD_TecBoxVssl_Pointer );

	// Set the focus column.
	pclSDCompressoVssl->SetActiveColumn( CD_TecBoxVssl_FirstColumn + 1 );

	// Set range for selection.
	pclSDCompressoVssl->SetFocusColumnRange( CD_TecBoxVssl_FirstColumn + 1, CD_TecBoxVssl_Pointer - 1 );

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Add the main title.
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, _IMI_TITLE_GROUP3 );
	pclSDCompressoVssl->SetUserVariable( _SDUV_TITLEFORECOLOR, ( pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor ) ) );
	pclSDCompressoVssl->SetUserVariable( _SDUV_TITLEBACKCOLOR, ( pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor ) ) );

	pclSSheet->AddCellSpanW( CD_TecBoxVssl_FirstColumn + 1, RD_TecBoxVssl_GroupName, CD_TecBoxVssl_Pointer - ( CD_TecBoxVssl_FirstColumn + 1 ), 1 );
	pclSSheet->SetStaticText( CD_TecBoxVssl_FirstColumn + 1, RD_TecBoxVssl_GroupName, IDS_SSHEETSSELPM_CMPRSSVSSLGROUP );
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Column name.
	if( false == pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn ) )
	{
		return;
	}

	if( false == pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE ) )
	{
		return;
	}

	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap, (LPARAM)TRUE );

	if( true == TASApp.IsPMPriceIndexUsed() )
	{
		pclSSheet->AddCellSpan( CD_TecBoxVssl_PriceIndexRatioIcon, RD_TecBoxVssl_ColName, 3, 1 );
		pclSSheet->SetStaticText( CD_TecBoxVssl_PriceIndexRatioIcon, RD_TecBoxVssl_ColName, IDS_SSHEETSSELPM_PRICEINDEXRATIO );
	}

	pclSSheet->SetStaticText( CD_TecBoxVssl_Prim_Name, RD_TecBoxVssl_ColName, IDS_SSHEETSSELPM_VESSELNAME );
	pclSSheet->SetStaticText( CD_TecBoxVssl_Prim_Volume, RD_TecBoxVssl_ColName, IDS_SSHEETSSELPM_VESSELVOL );

	int iID = ( 0 == TASApp.GetTADBKey().CompareNoCase( _T("CH") ) ) ? IDS_SSHEETSSELPM_VESSELMAXPRESSCH : IDS_SSHEETSSELPM_VESSELMAXPRESS;
	pclSSheet->SetStaticText( CD_TecBoxVssl_Prim_MaxPressure, RD_TecBoxVssl_ColName, iID );

	pclSSheet->SetStaticText( CD_TecBoxVssl_Prim_TempRange, RD_TecBoxVssl_ColName, IDS_SSHEETSSELPM_VESSELTEMPRANGE );
	pclSSheet->SetStaticText( CD_TecBoxVssl_Prim_Diameter, RD_TecBoxVssl_ColName, IDS_SSHEETSSELPM_VESSELDIAMETER );
	pclSSheet->SetStaticText( CD_TecBoxVssl_Prim_Height, RD_TecBoxVssl_ColName, IDS_SSHEETSSELPM_VESSELHEIGHT );
	pclSSheet->SetStaticText( CD_TecBoxVssl_Prim_Weight, RD_TecBoxVssl_ColName, IDS_SSHEETSSELPM_VESSELMASS );
	pclSSheet->SetStaticText( CD_TecBoxVssl_Prim_MaxWeight, RD_TecBoxVssl_ColName, IDS_SSHEETSSELPM_VESSELMAXWEIGHT );
	pclSSheet->SetStaticText( CD_TecBoxVssl_Sec_Name, RD_TecBoxVssl_ColName, IDS_SSHEETSSELPM_VESSELNAME );
	pclSSheet->SetStaticText( CD_TecBoxVssl_Sec_NbreOfVssl, RD_TecBoxVssl_ColName, IDS_SSHEETSSELPM_VESSELSECQTY );
	pclSSheet->SetStaticText( CD_TecBoxVssl_Sec_Diameter, RD_TecBoxVssl_ColName, IDS_SSHEETSSELPM_VESSELDIAMETER );
	pclSSheet->SetStaticText( CD_TecBoxVssl_Sec_Height, RD_TecBoxVssl_ColName, IDS_SSHEETSSELPM_VESSELHEIGHT );
	pclSSheet->SetStaticText( CD_TecBoxVssl_Sec_Weight, RD_TecBoxVssl_ColName, IDS_SSHEETSSELPM_VESSELMASS );
	pclSSheet->SetStaticText( CD_TecBoxVssl_Sec_MaxWeight, RD_TecBoxVssl_ColName, IDS_SSHEETSSELPM_VESSELMAXWEIGHT );
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Column units.
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();

	if( true == TASApp.IsPMPriceIndexUsed() )
	{
		pclSSheet->SetStaticText( CD_TecBoxVssl_PriceIndexRatioValue, RD_TecBoxVssl_ColUnit, _T("%") );
	}

	pclSSheet->SetStaticText( CD_TecBoxVssl_Prim_Volume, RD_TecBoxVssl_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_VOLUME ) ).c_str() );
	pclSSheet->SetStaticText( CD_TecBoxVssl_Prim_MaxPressure, RD_TecBoxVssl_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_PRESSURE ) ).c_str() );
	pclSSheet->SetStaticText( CD_TecBoxVssl_Prim_TempRange, RD_TecBoxVssl_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_TEMPERATURE ) ).c_str() );
	pclSSheet->SetStaticText( CD_TecBoxVssl_Prim_Diameter, RD_TecBoxVssl_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_LENGTH ) ).c_str() );
	pclSSheet->SetStaticText( CD_TecBoxVssl_Prim_Height, RD_TecBoxVssl_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_LENGTH ) ).c_str() );
	pclSSheet->SetStaticText( CD_TecBoxVssl_Prim_Weight, RD_TecBoxVssl_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_MASS ) ).c_str() );
	pclSSheet->SetStaticText( CD_TecBoxVssl_Prim_MaxWeight, RD_TecBoxVssl_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_MASS ) ).c_str() );
	pclSSheet->SetStaticText( CD_TecBoxVssl_Sec_Diameter, RD_TecBoxVssl_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_LENGTH ) ).c_str() );
	pclSSheet->SetStaticText( CD_TecBoxVssl_Sec_Height, RD_TecBoxVssl_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_LENGTH ) ).c_str() );
	pclSSheet->SetStaticText( CD_TecBoxVssl_Sec_Weight, RD_TecBoxVssl_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_MASS ) ).c_str() );
	pclSSheet->SetStaticText( CD_TecBoxVssl_Sec_MaxWeight, RD_TecBoxVssl_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_MASS ) ).c_str() );
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Draw black line below column header.
	pclSSheet->SetCellBorder( CD_TecBoxVssl_FirstColumn + 1, RD_TecBoxVssl_ColUnit, CD_TecBox_Pointer - 1, RD_TecBoxVssl_ColUnit, true, SS_BORDERTYPE_BOTTOM );
}

long CRViewSSelPM::_FillCompressoVsslRow( CSheetDescription *pclSDCompressoVssl, long lRow, CSelectedVssl *pclSelCompressoVssl, 
		CPMInputUser *pclPMInputUser, CSelectedPMBase *pclSelectedBestPriceProduct, bool bForRejectedProduct )
{
	if( NULL == pclSDCompressoVssl || NULL == pclSDCompressoVssl->GetSSheetPointer() || NULL == pclSelCompressoVssl || NULL == pclPMInputUser )
	{
		return -1;
	}

	CDB_Vessel *pcldbVessel = dynamic_cast<CDB_Vessel *>( pclSelCompressoVssl->GetpData() );

	if( NULL == pcldbVessel )
	{
		return -1;
	}

	CSSheet *pclSSheet = pclSDCompressoVssl->GetSSheetPointer();

	if( true == pclSelCompressoVssl->IsFlagSet( CSelectedPMBase::eNotPriority ) )
	{
		m_lProductNotPriorityCount++;

		if( false == m_bShowAllPrioritiesShown )
		{
			// Add one empty not selectable row.
			pclSDCompressoVssl->AddRows( 1 );

			pclSSheet->SetCellBorder( CD_TecBoxVssl_FirstColumn + 1, lRow, CD_TecBoxVssl_Pointer - 1, lRow, true, SS_BORDERTYPE_TOP | SS_BORDERTYPE_BOTTOM, 
					SS_BORDERSTYLE_SOLID, _BLACK );

			m_lShowAllPrioritiesButtonRow = lRow++;
			m_bShowAllPrioritiesShown = true;
		}
	}

	// Add this row.
	// 'true' to specify that this row can be selected.
	pclSDCompressoVssl->AddRows( 1, true );

	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

	// Add checkbox.
	pclSSheet->SetCheckBox(CD_TecBoxVssl_CheckBox, lRow, _T(""), false, true);

	// Set price index if used and defined.
	if( true == TASApp.IsPMPriceIndexUsed() )
	{
		if( NULL != pclSelectedBestPriceProduct && 
				0 == _tcscmp( pclSelectedBestPriceProduct->GetProductIDPtr().ID, pclSelCompressoVssl->GetProductIDPtr().ID ) )
		{
			pclSSheet->SetPictureCellWithID( IDI_BESTPRICE, CD_TecBoxVssl_PriceIndexRatioIcon, lRow, CSSheet::PictureCellType::Icon );
		}

		CString strPriceIndex = _T("-");

		if( pclSelCompressoVssl->GetPriceIndex() > 0.0 )
		{
			double dPriceIndex = pclSelCompressoVssl->GetPriceIndex();
			dPriceIndex += pclSelCompressoVssl->GetSecVesselPriceIndex();
			strPriceIndex.Format( _T("%i"), (int)dPriceIndex );
		}

		pclSSheet->SetStaticText( CD_TecBoxVssl_PriceIndexRatioValue, lRow, strPriceIndex );
	}

	bool bBest = pclSelCompressoVssl->IsFlagSet( CSelectedPMBase::eBest );

	if( true == bBest )
	{
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_GREEN );
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, true );
	}

	// Primary vessel.
	// Set the name.
	pclSSheet->SetStaticText( CD_TecBoxVssl_Prim_Name, lRow, pcldbVessel->GetName() );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, false );

	// Set volume nominal.
	double dVesselVolume = pcldbVessel->GetNominalVolume();
	pclSSheet->SetStaticText( CD_TecBoxVssl_Prim_Volume, lRow, WriteCUDouble( _U_VOLUME, dVesselVolume ) );

	// Set max pressure.
	
	double dPS = ( 0 == TASApp.GetTADBKey().CompareNoCase( _T("CH") ) ) ? pcldbVessel->GetPSch() : pcldbVessel->GetPmaxmax();
	pclSSheet->SetStaticText( CD_TecBoxVssl_Prim_MaxPressure, lRow, WriteCUDouble( _U_PRESSURE, dPS ) );

	// Set temperature range.
	if( true == pclSelCompressoVssl->IsFlagSet( CSelectedPMBase::eTemperature ) )
	{
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_ORANGE );
	}

	pclSSheet->SetStaticText( CD_TecBoxVssl_Prim_TempRange, lRow, pcldbVessel->GetTempRange() );

	// Restore normal fonts
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

	// Set diameter.
	pclSSheet->SetStaticText( CD_TecBoxVssl_Prim_Diameter, lRow, WriteCUDouble( _U_LENGTH, pcldbVessel->GetDiameter() ) );

	// Set height.
	pclSSheet->SetStaticText( CD_TecBoxVssl_Prim_Height, lRow, WriteCUDouble( _U_LENGTH, pcldbVessel->GetHeight() ) );

	// Set weight.
	pclSSheet->SetStaticText( CD_TecBoxVssl_Prim_Weight, lRow, WriteCUDouble( _U_MASS, pcldbVessel->GetWeight() ) );

	// Set max. weight.
	CString strMaxWeight = _T("-");
	double dMaxWeight = pclPMInputUser->GetMaxWeight( pcldbVessel->GetWeight(), dVesselVolume, pclPMInputUser->GetMinTemperature() );

	if( -1.0 != dMaxWeight )
	{
		strMaxWeight = WriteCUDouble( _U_MASS, dMaxWeight );
	}

	pclSSheet->SetStaticText( CD_TecBoxVssl_Prim_MaxWeight, lRow, strMaxWeight );

	if( pclSelCompressoVssl->GetNbreOfVsslNeeded() > 1 )
	{
		// Set name.
		if( true == bBest )
		{
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_GREEN );
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, true );
		}

		CDB_Vessel *pcldbSecVessel = pcldbVessel->GetAssociatedSecondaryVessel();

		if( NULL != pcldbSecVessel )
		{
			pclSSheet->SetStaticText( CD_TecBoxVssl_Sec_Name, lRow, pcldbSecVessel->GetName() );
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, false );

			// Set quantity.
			if( true == pclSelCompressoVssl->IsFlagSet( CSelectedPMBase::eVslFlagsNbreOfVssl ) )
			{
				pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_ORANGE );
			}

			pclSSheet->SetStaticText( CD_TecBoxVssl_Sec_NbreOfVssl, lRow, WriteDouble( pclSelCompressoVssl->GetNbreOfVsslNeeded() - 1, 1, 0, 1 ) );
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

			// Set diameter.
			pclSSheet->SetStaticText( CD_TecBoxVssl_Sec_Diameter, lRow, WriteCUDouble( _U_LENGTH, pcldbSecVessel->GetDiameter() ) );

			// Set height.
			pclSSheet->SetStaticText( CD_TecBoxVssl_Sec_Height, lRow, WriteCUDouble( _U_LENGTH, pcldbSecVessel->GetHeight() ) );

			// Set weight.
			pclSSheet->SetStaticText( CD_TecBoxVssl_Sec_Weight, lRow, WriteCUDouble( _U_MASS, pcldbSecVessel->GetWeight() ) );

			// Set max. weight.
			strMaxWeight = _T("-");
			dMaxWeight = pclPMInputUser->GetMaxWeight( pcldbSecVessel->GetWeight(), dVesselVolume, pclPMInputUser->GetMinTemperature() );

			if( -1.0 != dMaxWeight )
			{
				strMaxWeight = WriteCUDouble( _U_MASS, dMaxWeight );
			}

			pclSSheet->SetStaticText( CD_TecBoxVssl_Sec_MaxWeight, lRow, strMaxWeight );
		}
	}
	else
	{
		pclSSheet->SetStaticText( CD_TecBoxVssl_Sec_Name, lRow, _T("-") );
		pclSSheet->SetStaticText( CD_TecBoxVssl_Sec_NbreOfVssl, lRow, _T("-") );
		pclSSheet->SetStaticText( CD_TecBoxVssl_Sec_Diameter, lRow, _T("-") );
		pclSSheet->SetStaticText( CD_TecBoxVssl_Sec_Height, lRow, _T("-") );
		pclSSheet->SetStaticText( CD_TecBoxVssl_Sec_Weight, lRow, _T("-") );
		pclSSheet->SetStaticText( CD_TecBoxVssl_Sec_MaxWeight, lRow, _T("-") );
	}

	// Save parameter.
	CCellDescriptionProduct *pclCDProduct = CreateCellDescriptionProduct( pclSDCompressoVssl->GetFirstParameterColumn(), lRow, (LPARAM)pcldbVessel,
			pclSDCompressoVssl );

	if( NULL != pclCDProduct )
	{
		pclCDProduct->SetUserParam( (LPARAM)pclSelCompressoVssl );
	}

	pclSSheet->SetCellBorder( CD_TecBoxVssl_FirstColumn + 1, lRow, CD_TecBoxVssl_Pointer - 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _GRAY );

	return lRow;
}

void CRViewSSelPM::_FillCompressoSecondaryVsslSheet( UINT uiSDGroupID )
{
	if( NULL == m_pclIndSelPMParams || NULL == m_pclIndSelPMParams->m_pclSelectPMList || NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERT_RETURN;
	}

	CSelectedVssl *pclSelCompressoVssl = dynamic_cast<CSelectedVssl *>( m_pclIndSelPMParams->m_pclSelectPMList->GetFirstProduct( CSelectPMList::PT_CompressoVessel ) );

	// If no need of secondary vessel, we don't show this sheet.
	if( NULL == pclSelCompressoVssl || 1 == pclSelCompressoVssl->GetNbreOfVsslNeeded() )
	{
		return;
	}

	CDB_Vessel *pclIntegratedVessel = dynamic_cast<CDB_Vessel *>( pclSelCompressoVssl->GetpData() );

	if( NULL == pclIntegratedVessel )
	{
		return;
	}

	CDB_Vessel *pclSecondaryVessel = dynamic_cast<CDB_Vessel *>( pclIntegratedVessel->GetSecVsslIDPtr().MP );

	if( NULL == pclSecondaryVessel )
	{
		return;
	}

	// Get 'Parent' sheet identification.
	UINT uiAfterSheetDescriptionID = _GetAfterSheetDescriptionID( SD_CompressoSecondaryVessel, uiSDGroupID );

	// Call 'MultiSpreadBase' method to get a new sheet.
	CSheetDescription *pclSDCompressoSecondaryVessel = CreateSSheet( SD_CompressoSecondaryVessel, uiAfterSheetDescriptionID );

	if( NULL == pclSDCompressoSecondaryVessel || NULL == pclSDCompressoSecondaryVessel->GetSSheetPointer() )
	{
		return;
	}

	pclSDCompressoSecondaryVessel->SetGroupSheetDescriptionID( uiSDGroupID );
	pclSDCompressoSecondaryVessel->SetParentSheetDescriptionID( SD_CompressoSecondaryVessel );
	CSSheet *pclSSheet = pclSDCompressoSecondaryVessel->GetSSheetPointer();

	pclSSheet->SetBool( SSB_REDRAW, FALSE );

	_InitCompressoSecondaryVsslHeaders( pclSDCompressoSecondaryVessel );

	m_lProductTotalCount = 0;
	m_lProductNotPriorityCount = 0;
	m_bShowAllPrioritiesShown = false;
	long lRow = RD_TecBoxVssl_FirstAvailRow;
	pclSDCompressoSecondaryVessel->RestartRemarkGenerator();

	CSelectedPMBase *pclSelectedBestPriceProduct = NULL;

	if( true == TASApp.IsPMPriceIndexUsed() )
	{
		pclSelectedBestPriceProduct = m_pclIndSelPMParams->m_pclSelectPMList->GetLowestPriceObject( CSelectPMList::ProductType::PT_CompressoVessel );
	}

	lRow = _FillCompressoSecondaryVsslRow( pclSDCompressoSecondaryVessel, lRow, pclSelCompressoVssl, m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser(),
			pclSelectedBestPriceProduct );
	
	lRow++;

	long lLastDataRow = lRow - 1;
	pclSDCompressoSecondaryVessel->SetUserVariable( _SDUV_TOTALPRODUCT_COUNT, 1 );
	pclSDCompressoSecondaryVessel->SetUserVariable( _SDUV_NOPRIORITYPRODUCT_COUNT, 0 );

	pclSSheet->SetCellBorder( CD_TecBoxSecVssl_FirstColumn + 1, lLastDataRow, CD_TecBoxSecVssl_Pointer - 1, lLastDataRow, true, SS_BORDERTYPE_BOTTOM, 
			SS_BORDERSTYLE_SOLID, _BLACK );

	// Add Remarks.
	// Note: these lines must be placed after creating Show/Hide all priorities button to avoid to take in count these lines when defining
	//       range of row to be Show/Hide.
	lRow = pclSDCompressoSecondaryVessel->WriteRemarks( lRow, CD_TecBoxSecVssl_Name, CD_TecBoxSecVssl_Pointer );

	// Add possibility to change column size.
	// Remark: place this code after any initialization (like adding, removing, resizing, hiding/showing columns).
	CResizingColumnInfo *pclResizingColumnInfo = pclSDCompressoSecondaryVessel->GetResizingColumnInfo();
	pclResizingColumnInfo->ActivateFeature( true, false, RD_TecBoxVssl_FirstAvailRow, lLastDataRow );
	pclResizingColumnInfo->AddRangeColumn( CD_TecBoxSecVssl_PriceIndexRatioBorder, CD_TecBoxSecVssl_Pointer - 1, RD_TecBoxVssl_ColName, RD_TecBoxVssl_ColUnit );

	// Add possibility to click on title group to reset column width.
	CreateTitleGroupButtons( CD_TecBoxSecVssl_FirstColumn + 1, CD_TecBoxSecVssl_Pointer - 1, RD_TecBoxVssl_GroupName, pclSDCompressoSecondaryVessel );

	// We automatically select this product.
	CCellDescription *pclCellDescription = pclSDCompressoSecondaryVessel->GetCellDescription( CD_TecBoxSecVssl_CheckBox, lLastDataRow );

	if( NULL == pclCellDescription || NULL == dynamic_cast<CCellDescriptionProduct *>( pclCellDescription ) )
	{
		ASSERT( 0 );
		pclSSheet->SetBool( SSB_REDRAW, TRUE );
		return;
	}

	CCellDescriptionProduct *pclCellDescriptionProduct = dynamic_cast<CCellDescriptionProduct *>( pclCellDescription );
	LPARAM lpSelectedUserParamClicked = pclCellDescriptionProduct->GetUserParam();

	if( NULL == lpSelectedUserParamClicked )
	{
		ASSERT( 0 );
		pclSSheet->SetBool( SSB_REDRAW, TRUE );
		return;
	}
	
	CSelectedVssl *pclSelectedSecondaryVessel = ( CSelectedVssl * )pclCellDescriptionProduct->GetUserParam();

	if( NULL == dynamic_cast<CDB_Vessel*>( pclSelectedSecondaryVessel->GetProductIDPtr().MP ) )
	{
		ASSERT( 0 );
		pclSSheet->SetBool( SSB_REDRAW, TRUE );
		return;
	}

	CDB_Vessel *pclVessel = dynamic_cast<CDB_Vessel*>( pclSelectedSecondaryVessel->GetProductIDPtr().MP );

	// Save the new Compresso secondary vessel selection.
	m_arlpSelectedUserParam[SD_CompressoSecondaryVessel] = pclCellDescriptionProduct->GetUserParam();
	SetCurrentPMSelected( SD_CompressoSecondaryVessel, SD_TecBoxCompresso, pclCellDescriptionProduct );

	// Select Compresso secondary vessel (just highlight background).
	pclSSheet->SelectOneRow( lLastDataRow, pclSDCompressoSecondaryVessel->GetSelectionFrom(), pclSDCompressoSecondaryVessel->GetSelectionTo() );

	_FillAccessoryRows( SD_CompressoSecondaryVesselAccessory, SD_TecBoxCompresso, SD_CompressoSecondaryVessel, IDS_SSHEETSSELPM_CPVSSLACCGROUP, pclVessel );
	_FillIntermediateVsslSheet( SD_IntermediateVessel, SD_TecBoxCompresso, SD_CompressoSecondaryVessel, pclSelectedSecondaryVessel );

	pclSSheet->SetCheckBox( CD_TecBoxSecVssl_CheckBox, lLastDataRow, _T(""), true, true );

	pclSSheet->SetBool( SSB_REDRAW, TRUE );
}

void CRViewSSelPM::_InitCompressoSecondaryVsslHeaders( CSheetDescription *pclSDCompressoSecondaryVssl )
{
	if( NULL == pclSDCompressoSecondaryVssl || NULL == pclSDCompressoSecondaryVssl->GetSSheetPointer() )
	{
		return;
	}

	CSSheet *pclSSheet = pclSDCompressoSecondaryVssl->GetSSheetPointer();

	pclSSheet->Init();
	pclSSheet->SetBool( SSB_ALLOWDRAGDROP, FALSE );
	pclSSheet->SetBool( SSB_HORZSCROLLBAR, FALSE );
	pclSSheet->SetBool( SSB_VERTSCROLLBAR, FALSE );
	pclSSheet->SetBool( SSB_MOVEACTIVEONFOCUS, FALSE );
	pclSSheet->SetTextTip( SS_TT_STATUS_FLOATING, 250, NULL, _LIGHTYELLOW, _TAH_BLACK );

	// Set max rows.
	pclSSheet->SetMaxRows( RD_TecBoxVssl_FirstAvailRow - 1 );

	// All cells are static by default and filled with _T("").
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	pclSSheet->FormatStaticText( -1, -1, -1, -1, _T( "" ) );

	// Increase row height.
	double dRowHeight = 12.75;
	pclSSheet->SetRowHeight( RD_TecBoxVssl_FirstRow, dRowHeight * 0.5 );
	pclSSheet->SetRowHeight( RD_TecBoxVssl_GroupName, dRowHeight * 1.5 );
	pclSSheet->SetRowHeight( RD_TecBoxVssl_ColName, dRowHeight * 2 );
	pclSSheet->SetRowHeight( RD_TecBoxVssl_ColUnit, dRowHeight * 1.2 );

	// Initialize.
	pclSDCompressoSecondaryVssl->Init( 1, pclSSheet->GetMaxRows(), 1, CSheetDescription::SD_ParameterMode_Mono );
	pclSDCompressoSecondaryVssl->AddColumnInPixels( CD_TecBoxSecVssl_FirstColumn, m_mapSSheetColumnWidth[SD_CompressoSecondaryVessel][CD_TecBoxSecVssl_FirstColumn] );
	pclSDCompressoSecondaryVssl->AddColumnInPixels( CD_TecBoxSecVssl_CheckBox, m_mapSSheetColumnWidth[SD_CompressoSecondaryVessel][CD_TecBoxSecVssl_CheckBox] );

	if( true == TASApp.IsPMPriceIndexUsed() )
	{
		pclSDCompressoSecondaryVssl->AddColumnInPixels( CD_TecBoxSecVssl_PriceIndexRatioIcon, m_mapSSheetColumnWidth[SD_CompressoSecondaryVessel][CD_TecBoxSecVssl_PriceIndexRatioIcon] );
		pclSDCompressoSecondaryVssl->AddColumnInPixels( CD_TecBoxSecVssl_PriceIndexRatioValue, m_mapSSheetColumnWidth[SD_CompressoSecondaryVessel][CD_TecBoxSecVssl_PriceIndexRatioValue] );
		pclSDCompressoSecondaryVssl->AddColumnInPixels( CD_TecBoxSecVssl_PriceIndexRatioBorder, m_mapSSheetColumnWidth[SD_CompressoSecondaryVessel][CD_TecBoxSecVssl_PriceIndexRatioBorder] );
	}

	pclSDCompressoSecondaryVssl->AddColumnInPixels( CD_TecBoxSecVssl_Name, m_mapSSheetColumnWidth[SD_CompressoSecondaryVessel][CD_TecBoxSecVssl_Name] );
	pclSDCompressoSecondaryVssl->AddColumnInPixels( CD_TecBoxSecVssl_Volume, m_mapSSheetColumnWidth[SD_CompressoSecondaryVessel][CD_TecBoxSecVssl_Volume] );
	pclSDCompressoSecondaryVssl->AddColumnInPixels( CD_TecBoxSecVssl_MaxPressure, m_mapSSheetColumnWidth[SD_CompressoSecondaryVessel][CD_TecBoxSecVssl_MaxPressure] );
	pclSDCompressoSecondaryVssl->AddColumnInPixels( CD_TecBoxSecVssl_TempRange, m_mapSSheetColumnWidth[SD_CompressoSecondaryVessel][CD_TecBoxSecVssl_TempRange] );
	pclSDCompressoSecondaryVssl->AddColumnInPixels( CD_TecBoxSecVssl_Diameter, m_mapSSheetColumnWidth[SD_CompressoSecondaryVessel][CD_TecBoxSecVssl_Diameter] );
	pclSDCompressoSecondaryVssl->AddColumnInPixels( CD_TecBoxSecVssl_Height, m_mapSSheetColumnWidth[SD_CompressoSecondaryVessel][CD_TecBoxSecVssl_Height] );
	pclSDCompressoSecondaryVssl->AddColumnInPixels( CD_TecBoxSecVssl_Weight, m_mapSSheetColumnWidth[SD_CompressoSecondaryVessel][CD_TecBoxSecVssl_Weight] );
	pclSDCompressoSecondaryVssl->AddColumnInPixels( CD_TecBoxSecVssl_MaxWeight, m_mapSSheetColumnWidth[SD_CompressoSecondaryVessel][CD_TecBoxSecVssl_MaxWeight] );
	pclSDCompressoSecondaryVssl->AddColumnInPixels( CD_TecBoxSecVssl_Pointer, m_mapSSheetColumnWidth[SD_CompressoSecondaryVessel][CD_TecBoxSecVssl_Pointer] );

	// Must be placed after any call to 'AddColumnInPixels'. Because this method will add needed columns and TSpread will show them by default.
	// When calling the first time 'AddColumnInPixels' for the 'CD_TecBoxSecVssl_Name' column, this method will call TSpread::SetMaxCols() to adapt the
	// number of columns. All the column before 'CD_TecBoxSecVssl_Name' are set to a default width and are visible.
	if( false == TASApp.IsPMPriceIndexUsed() )
	{
		pclSSheet->ShowCol( CD_TecBoxSecVssl_PriceIndexRatioIcon, FALSE );
		pclSSheet->ShowCol( CD_TecBoxSecVssl_PriceIndexRatioValue, FALSE );
		pclSSheet->ShowCol( CD_TecBoxSecVssl_PriceIndexRatioBorder, FALSE );
	}

	// Set in which column parameter must be saved.
	pclSDCompressoSecondaryVssl->AddParameterColumn( CD_TecBoxSecVssl_Pointer );

	// Set the focus column.
	pclSDCompressoSecondaryVssl->SetActiveColumn( CD_TecBoxSecVssl_FirstColumn + 1 );

	// Set range for selection.
	pclSDCompressoSecondaryVssl->SetFocusColumnRange( CD_TecBoxSecVssl_FirstColumn + 1, CD_TecBoxSecVssl_Pointer - 1 );

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Add the main title.
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, _IMI_TITLE_GROUP3 );
	pclSDCompressoSecondaryVssl->SetUserVariable( _SDUV_TITLEFORECOLOR, ( pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor ) ) );
	pclSDCompressoSecondaryVssl->SetUserVariable( _SDUV_TITLEBACKCOLOR, ( pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor ) ) );

	pclSSheet->AddCellSpanW( CD_TecBoxSecVssl_FirstColumn + 1, RD_TecBoxVssl_GroupName, CD_TecBoxSecVssl_Pointer - ( CD_TecBoxSecVssl_FirstColumn + 1 ), 1 );
	pclSSheet->SetStaticText( CD_TecBoxSecVssl_FirstColumn + 1, RD_TecBoxVssl_GroupName, IDS_SSHEETSSELPM_CMPRSSSECVSSLGROUP );
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Column name.
	if( false == pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn ) )
	{
		return;
	}

	if( false == pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE ) )
	{
		return;
	}

	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap, (LPARAM)TRUE );

	if( true == TASApp.IsPMPriceIndexUsed() )
	{
		pclSSheet->AddCellSpan( CD_TecBoxSecVssl_PriceIndexRatioIcon, RD_TecBoxVssl_ColName, 3, 1 );
		pclSSheet->SetStaticText( CD_TecBoxSecVssl_PriceIndexRatioIcon, RD_TecBoxVssl_ColName, IDS_SSHEETSSELPM_PRICEINDEXRATIO );
	}

	pclSSheet->SetStaticText( CD_TecBoxSecVssl_Name, RD_TecBoxVssl_ColName, IDS_SSHEETSSELPM_VESSELNAME );
	pclSSheet->SetStaticText( CD_TecBoxSecVssl_Volume, RD_TecBoxVssl_ColName, IDS_SSHEETSSELPM_VESSELVOL );
	pclSSheet->SetStaticText( CD_TecBoxSecVssl_MaxPressure, RD_TecBoxVssl_ColName, IDS_SSHEETSSELPM_VESSELMAXPRESS );
	pclSSheet->SetStaticText( CD_TecBoxSecVssl_TempRange, RD_TecBoxVssl_ColName, IDS_SSHEETSSELPM_VESSELTEMPRANGE );
	pclSSheet->SetStaticText( CD_TecBoxSecVssl_Diameter, RD_TecBoxVssl_ColName, IDS_SSHEETSSELPM_VESSELDIAMETER );
	pclSSheet->SetStaticText( CD_TecBoxSecVssl_Height, RD_TecBoxVssl_ColName, IDS_SSHEETSSELPM_VESSELHEIGHT );
	pclSSheet->SetStaticText( CD_TecBoxSecVssl_Weight, RD_TecBoxVssl_ColName, IDS_SSHEETSSELPM_VESSELMASS );
	pclSSheet->SetStaticText( CD_TecBoxSecVssl_MaxWeight, RD_TecBoxVssl_ColName, IDS_SSHEETSSELPM_VESSELMAXWEIGHT );
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Column units.
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();

	if( true == TASApp.IsPMPriceIndexUsed() )
	{
		pclSSheet->SetStaticText( CD_TecBoxSecVssl_PriceIndexRatioValue, RD_TecBoxVssl_ColUnit, _T("%") );
	}

	pclSSheet->SetStaticText( CD_TecBoxSecVssl_Volume, RD_TecBoxVssl_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_VOLUME ) ).c_str() );
	pclSSheet->SetStaticText( CD_TecBoxSecVssl_MaxPressure, RD_TecBoxVssl_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_PRESSURE ) ).c_str() );
	pclSSheet->SetStaticText( CD_TecBoxSecVssl_TempRange, RD_TecBoxVssl_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_TEMPERATURE ) ).c_str() );
	pclSSheet->SetStaticText( CD_TecBoxSecVssl_Diameter, RD_TecBoxVssl_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_LENGTH ) ).c_str() );
	pclSSheet->SetStaticText( CD_TecBoxSecVssl_Height, RD_TecBoxVssl_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_LENGTH ) ).c_str() );
	pclSSheet->SetStaticText( CD_TecBoxSecVssl_Weight, RD_TecBoxVssl_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_MASS ) ).c_str() );
	pclSSheet->SetStaticText( CD_TecBoxSecVssl_MaxWeight, RD_TecBoxVssl_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_MASS ) ).c_str() );
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Draw black line below column header.
	pclSSheet->SetCellBorder( CD_TecBoxSecVssl_FirstColumn + 1, RD_TecBoxVssl_ColUnit, CD_TecBox_Pointer - 1, RD_TecBoxVssl_ColUnit, true, SS_BORDERTYPE_BOTTOM );
}

long CRViewSSelPM::_FillCompressoSecondaryVsslRow( CSheetDescription *pclSDCompressoSecondaryVssl, long lRow, CSelectedVssl *pclSelCompressoVssl, 
		CPMInputUser *pclPMInputUser, CSelectedPMBase *pclSelectedBestPriceProduct )
{
	// This method allow use to show secondary vessel for a Simply Compresso.

	if( NULL == pclSDCompressoSecondaryVssl || NULL == pclSDCompressoSecondaryVssl->GetSSheetPointer() 
			|| NULL == pclSelCompressoVssl || NULL == pclPMInputUser )
	{
		return -1;
	}

	CDB_Vessel *pclIntegratedVessel = dynamic_cast<CDB_Vessel *>( pclSelCompressoVssl->GetpData() );

	if( NULL == pclIntegratedVessel || NULL == dynamic_cast<CDB_Vessel*>( pclIntegratedVessel->GetSecVsslIDPtr().MP ) )
	{
		return -1;
	}

	CDB_Vessel *pclSecondaryVessel = dynamic_cast<CDB_Vessel*>( pclIntegratedVessel->GetSecVsslIDPtr().MP );

	CSSheet *pclSSheet = pclSDCompressoSecondaryVssl->GetSSheetPointer();

	// Add this row.
	// 'true' to specify that this row can be selected.
	pclSDCompressoSecondaryVssl->AddRows( 1, true );

	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

	// Add checkbox.
	pclSSheet->SetCheckBox( CD_TecBoxSecVssl_CheckBox, lRow, _T(""), false, true );

	// Set price index if used and defined.
	if( true == TASApp.IsPMPriceIndexUsed() )
	{
		// Remark: in case of Simply Compresso, the 'pclSelectedBestPriceProduct' is fill with the integrated vessel. There is no price for this 
		// product but well for the secondary vessel if exists.
		if( NULL != pclSelectedBestPriceProduct && 
				0 == _tcscmp( pclSelectedBestPriceProduct->GetProductIDPtr().ID, pclIntegratedVessel->GetIDPtr().ID ) )
		{
			pclSSheet->SetPictureCellWithID( IDI_BESTPRICE, CD_TecBoxSecVssl_PriceIndexRatioIcon, lRow, CSSheet::PictureCellType::Icon );
		}

		CString strPriceIndex = _T("-");

		// Remark: There is no price for the primary vessel because in this case this one is integrated in the Compresso.
		if( pclSelCompressoVssl->GetSecVesselPriceIndex() > 0.0 )
		{
			double dPriceIndex = pclSelCompressoVssl->GetSecVesselPriceIndex();
			strPriceIndex.Format( _T("%i"), (int)dPriceIndex );
		}

		pclSSheet->SetStaticText( CD_TecBoxSecVssl_PriceIndexRatioValue, lRow, strPriceIndex );
	}

	bool bBest = pclSelCompressoVssl->IsFlagSet( CSelectedPMBase::eBest );

	if( true == bBest )
	{
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_GREEN );
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, true );
	}

	// Set name.
	pclSSheet->SetStaticText( CD_TecBoxSecVssl_Name, lRow, pclSecondaryVessel->GetName() );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, false );

	// Set volume nominal.
	double dVesselVolume = pclSecondaryVessel->GetNominalVolume();
	pclSSheet->SetStaticText( CD_TecBoxSecVssl_Volume, lRow, WriteCUDouble( _U_VOLUME, dVesselVolume ) );

	// Set max pressure.
	pclSSheet->SetStaticText( CD_TecBoxSecVssl_MaxPressure, lRow, WriteCUDouble( _U_PRESSURE, pclSecondaryVessel->GetPmaxmax() ) );

	// Set temperature range.
	if( true == pclSelCompressoVssl->IsFlagSet( CSelectedPMBase::eTemperature ) )
	{
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_ORANGE );
	}

	pclSSheet->SetStaticText( CD_TecBoxSecVssl_TempRange, lRow, pclSecondaryVessel->GetTempRange() );

	// Restore normal fonts.
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

	// Set diameter.
	pclSSheet->SetStaticText( CD_TecBoxSecVssl_Diameter, lRow, WriteCUDouble( _U_LENGTH, pclSecondaryVessel->GetDiameter() ) );

	// Set height.
	pclSSheet->SetStaticText( CD_TecBoxSecVssl_Height, lRow, WriteCUDouble( _U_LENGTH, pclSecondaryVessel->GetHeight() ) );

	// Set weight.
	pclSSheet->SetStaticText( CD_TecBoxSecVssl_Weight, lRow, WriteCUDouble( _U_MASS, pclSecondaryVessel->GetWeight() ) );

	// Set max. weight.
	CString strMaxWeight = _T("-");
	double dMaxWeight = pclPMInputUser->GetMaxWeight( pclSecondaryVessel->GetWeight(), dVesselVolume, pclPMInputUser->GetMinTemperature() );

	if( -1.0 != dMaxWeight )
	{
		strMaxWeight = WriteCUDouble( _U_MASS, dMaxWeight );
	}

	pclSSheet->SetStaticText( CD_TecBoxSecVssl_MaxWeight, lRow, strMaxWeight );

	// Save parameter.
	CCellDescriptionProduct *pclCDProduct = CreateCellDescriptionProduct( pclSDCompressoSecondaryVssl->GetFirstParameterColumn(), lRow, 
			(LPARAM)pclSecondaryVessel, pclSDCompressoSecondaryVssl );

	if( NULL != pclCDProduct )
	{
		pclCDProduct->SetUserParam( (LPARAM)pclSelCompressoVssl );
	}

	pclSSheet->SetCellBorder( CD_TecBoxSecVssl_FirstColumn + 1, lRow, CD_TecBoxSecVssl_Pointer - 1, lRow, true, SS_BORDERTYPE_BOTTOM, 
			SS_BORDERSTYLE_SOLID, _GRAY );

	return lRow;
}

long CRViewSSelPM::_FillTecBoxTransferoSheet( CDS_SSelPMaint *pclEditedSSelPM )
{
	if( NULL == m_pclIndSelPMParams || NULL == m_pclIndSelPMParams->m_pclSelectPMList || NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser() 
			|| NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetPMWQSelectionPreferences() )
	{
		ASSERTA_RETURN( -1 );
	}

	// Call 'MultiSpreadBase' method to get a new sheet.
	CSheetDescription *pclSDTecBoxTransfero = CreateSSheet( SD_TecBoxTransfero );

	if( NULL == pclSDTecBoxTransfero || NULL == pclSDTecBoxTransfero->GetSSheetPointer() )
	{
		return -1;
	}

	CSSheet *pclSSheet = pclSDTecBoxTransfero->GetSSheetPointer();
	pclSDTecBoxTransfero->SetGroupSheetDescriptionID( SD_TecBoxTransfero );

	pclSSheet->SetBool( SSB_REDRAW, FALSE );

	_InitTecBoxTransferoHeaders( pclSDTecBoxTransfero );

	bool bRedundancy = m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetPMWQSelectionPreferences()->IsChecked( CPMWQPrefs::GeneralRedundancyTecBox );

	m_lSelectedRow = -1;
	m_lProductTotalCount = 0;
	m_lProductNotPriorityCount = 0;
	m_bShowAllPrioritiesShown = false;
	m_lShowAllPrioritiesButtonRow = 0;
	long lRow = RD_TecBox_FirstAvailRow;
	pclSDTecBoxTransfero->RestartRemarkGenerator();

	CSelectedPMBase* pclSelectedBestPriceProduct = NULL;

	if( true == TASApp.IsPMPriceIndexUsed() )
	{
		pclSelectedBestPriceProduct = m_pclIndSelPMParams->m_pclSelectPMList->GetBestPriceIndexProduct( CSelectPMList::BPIS_Transfero );
	}

	// Priority/NotaPriority loop.
	// Remark: Now we show all when user selects a specific pressure maintenance system.
	int iEndLoop = ( true == m_bIsPMTypeAll ) ? 2 : 1;

	for( int iLoop = 0; iLoop < iEndLoop; iLoop++ )
	{
		for( CSelectedPMBase *pclSelectedProduct = m_pclIndSelPMParams->m_pclSelectPMList->GetFirstProduct( CSelectPMList::PT_Transfero );
			 NULL != pclSelectedProduct; pclSelectedProduct = m_pclIndSelPMParams->m_pclSelectPMList->GetNextProduct( CSelectPMList::PT_Transfero ) )
		{
			// Skip not Priority when iLoop == 1
			if( true == m_bIsPMTypeAll && ( 0 == iLoop ) == pclSelectedProduct->IsFlagSet( CSelectedPMBase::eNotPriority ) )
			{
				continue;
			}

			CSelectedTransfero *pclSelectedTecBoxTransfero = dynamic_cast<CSelectedTransfero *>( pclSelectedProduct );

			if( NULL == pclSelectedTecBoxTransfero )
			{
				continue;
			}

			long lReturn = _FillTecBoxTransferoRow( pclSDTecBoxTransfero, lRow, pclSelectedTecBoxTransfero, m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser(),
					pclEditedSSelPM, pclSelectedBestPriceProduct );

			if( -1 == lReturn )
			{
				continue;
			}
			else
			{
				lRow = lReturn;
			}

			lRow++;
			m_lProductTotalCount++;
		}
	}

	long lLastDataRow = lRow - 1;
	pclSDTecBoxTransfero->SetUserVariable( _SDUV_TOTALPRODUCT_COUNT, m_lProductTotalCount );
	pclSDTecBoxTransfero->SetUserVariable( _SDUV_NOPRIORITYPRODUCT_COUNT, m_lProductNotPriorityCount );

	// Now we can create Show/Hide all priorities if needed.
	// Remarks: 2nd condition to avoid to have button if we have only no priority valves.
	if( true == m_bIsPMTypeAll )
	{
		if( true == m_bShowAllPrioritiesShown && m_lProductTotalCount > m_lProductNotPriorityCount )
		{
			// Create Show/Hide all priorities button in regards to current state!
			// HYS-1539 : The button state is the same for all sheet
			CCDButtonShowAllPriorities::ButtonState eState = ( true == GetShowAllPrioritiesFlag( SD_TecBoxTransfero ) ) ? 
					CCDButtonShowAllPriorities::ButtonState::HidePriorities : CCDButtonShowAllPriorities::ButtonState::ShowPriorities;

			CCDButtonShowAllPriorities *pclShowAllButton = CreateShowAllPrioritiesButton( CD_TecBox_FirstColumn, m_lShowAllPrioritiesButtonRow, 
					m_bShowAllPrioritiesShown, eState, m_lShowAllPrioritiesButtonRow + 1, pclSSheet->GetMaxRows(), pclSDTecBoxTransfero );

			// Here we must call 'ApplyInternalChange' to show or hide rows in regards to the current state!
			if( NULL != pclShowAllButton )
			{
				pclShowAllButton->ApplyInternalChange();
			}
		}
		else
		{
			if( m_lShowAllPrioritiesButtonRow != 0 )
			{
				pclSSheet->ShowRow( m_lShowAllPrioritiesButtonRow, false );
			}
		}
	}

	pclSSheet->SetCellBorder( CD_TecBox_FirstColumn + 1, lLastDataRow, CD_TecBox_Pointer - 1, lLastDataRow, true, SS_BORDERTYPE_BOTTOM, 
			SS_BORDERSTYLE_SOLID, _BLACK );

	// Add first the link to the excluded products if exist.
	lRow = _AddExcludedProductsLink( pclSDTecBoxTransfero, lRow, CSelectPMList::PT_Transfero );

	// Add Remarks.
	// Note: these lines must be placed after creating Show/Hide all priorities button to avoid to take in count these lines when defining
	//       range of row to be Show/Hide.
	lRow = pclSDTecBoxTransfero->WriteRemarks( lRow, CD_TecBox_FirstColumn + 1, CD_TecBox_Pointer );

	// Add possibility to change column size.
	// Remark: place this code after any initialization (like adding, removing, resizing, hiding/showing columns).
	CResizingColumnInfo *pclResizingColumnInfo = pclSDTecBoxTransfero->GetResizingColumnInfo();
	pclResizingColumnInfo->ActivateFeature( true, false, RD_TecBox_FirstAvailRow, lLastDataRow );
	pclResizingColumnInfo->AddRangeColumn( CD_TecBox_PriceIndexBorder, CD_TecBox_Pointer - 1, RD_TecBox_ColName, RD_TecBox_ColUnit );

	// Add possibility to click on title group to reset column width.
	CreateTitleGroupButtons( CD_TecBox_FirstColumn + 1, CD_TecBox_Pointer - 1, RD_TecBox_GroupName, pclSDTecBoxTransfero );

	// Create the button to allow to expand/collapse all the group.
	m_mapButtonExpandCollapseGroup[SD_TecBoxTransfero] = CreateExpandCollapseGroupButton( CD_TecBox_FirstColumn, RD_TecBox_GroupName, true,
			CCDButtonExpandCollapseGroup::ButtonState::CollapseRow, RD_TecBox_ColName, pclSSheet->GetMaxRows(), pclSDTecBoxTransfero );

	if( NULL != m_mapButtonExpandCollapseGroup[SD_TecBoxTransfero] )
	{
		m_mapButtonExpandCollapseGroup[SD_TecBoxTransfero]->SetShowStatus( true );
	}

	pclSSheet->SetBool( SSB_REDRAW, TRUE );
	return m_lSelectedRow;
}

void CRViewSSelPM::_InitTecBoxTransferoHeaders( CSheetDescription *pclSDTecBoxTransfero )
{
	if( NULL == pclSDTecBoxTransfero || NULL == pclSDTecBoxTransfero->GetSSheetPointer() )
	{
		return;
	}

	CSSheet *pclSSheet = pclSDTecBoxTransfero->GetSSheetPointer();

	pclSSheet->Init();
	pclSSheet->SetBool( SSB_ALLOWDRAGDROP, FALSE );
	pclSSheet->SetBool( SSB_HORZSCROLLBAR, FALSE );
	pclSSheet->SetBool( SSB_VERTSCROLLBAR, FALSE );
	pclSSheet->SetBool( SSB_MOVEACTIVEONFOCUS, TRUE );
	pclSSheet->SetTextTip( SS_TT_STATUS_FLOATING, 250, NULL, _LIGHTYELLOW, _TAH_BLACK );

	// To allow button of the combo box be always visible for the buffer list when existing.
	pclSSheet->SetButtonDrawMode( pclSSheet->GetButtonDrawMode() & SS_BDM_ALWAYSCOMBO  );

	// Set max rows.
	pclSSheet->SetMaxRows( RD_TecBox_FirstAvailRow - 1 );

	// All cells are static by default and filled with _T("").
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	pclSSheet->FormatStaticText( -1, -1, -1, -1, _T( "" ) );

	// Increase row height.
	double dRowHeight = 12.75;
	pclSSheet->SetRowHeight( RD_TecBox_FirstRow, dRowHeight * 0.5 );
	pclSSheet->SetRowHeight( RD_TecBox_GroupName, dRowHeight * 1.5 );
	pclSSheet->SetRowHeight( RD_TecBox_ColName, dRowHeight * 2 );
	pclSSheet->SetRowHeight( RD_TecBox_ColUnit, dRowHeight * 1.2 );

	// Initialize.
	pclSDTecBoxTransfero->Init( 1, pclSSheet->GetMaxRows(), 1, CSheetDescription::SD_ParameterMode_Mono );
	pclSDTecBoxTransfero->AddColumnInPixels( CD_TecBox_FirstColumn, m_mapSSheetColumnWidth[SD_TecBoxTransfero][CD_TecBox_FirstColumn] );
	pclSDTecBoxTransfero->AddColumnInPixels( CD_TecBox_CheckBox, m_mapSSheetColumnWidth[SD_TecBoxTransfero][CD_TecBox_CheckBox] );

	if( true == TASApp.IsPMPriceIndexUsed() )
	{
		pclSDTecBoxTransfero->AddColumnInPixels( CD_TecBox_PriceIndexIcon, m_mapSSheetColumnWidth[SD_TecBoxTransfero][CD_TecBox_PriceIndexIcon] );
		pclSDTecBoxTransfero->AddColumnInPixels( CD_TecBox_PriceIndexValue, m_mapSSheetColumnWidth[SD_TecBoxTransfero][CD_TecBox_PriceIndexValue] );
		pclSDTecBoxTransfero->AddColumnInPixels( CD_TecBox_PriceIndexBorder, m_mapSSheetColumnWidth[SD_TecBoxTransfero][CD_TecBox_PriceIndexBorder] );
	}

	pclSDTecBoxTransfero->AddColumnInPixels( CD_TecBox_Name, m_mapSSheetColumnWidth[SD_TecBoxTransfero][CD_TecBox_Name] );
	pclSDTecBoxTransfero->AddColumnInPixels( CD_TecBox_NbreOfDevice, m_mapSSheetColumnWidth[SD_TecBoxTransfero][CD_TecBox_NbreOfDevice] );
	pclSDTecBoxTransfero->AddColumnInPixels( CD_TecBox_PS, m_mapSSheetColumnWidth[SD_TecBoxTransfero][CD_TecBox_PS] );
	pclSDTecBoxTransfero->AddColumnInPixels( CD_TecBox_TempRange, m_mapSSheetColumnWidth[SD_TecBoxTransfero][CD_TecBox_TempRange] );
	pclSDTecBoxTransfero->AddColumnInPixels( CD_TecBox_MaxWaterMakeUpTemp, m_mapSSheetColumnWidth[SD_TecBoxTransfero][CD_TecBox_MaxWaterMakeUpTemp] );
	pclSDTecBoxTransfero->AddColumnInPixels( CD_TecBox_B, m_mapSSheetColumnWidth[SD_TecBoxTransfero][CD_TecBox_B] );
	pclSDTecBoxTransfero->AddColumnInPixels( CD_TecBox_H, m_mapSSheetColumnWidth[SD_TecBoxTransfero][CD_TecBox_H] );
	pclSDTecBoxTransfero->AddColumnInPixels( CD_TecBox_T, m_mapSSheetColumnWidth[SD_TecBoxTransfero][CD_TecBox_T] );
	pclSDTecBoxTransfero->AddColumnInPixels( CD_TecBox_Weight, m_mapSSheetColumnWidth[SD_TecBoxTransfero][CD_TecBox_Weight] );
	pclSDTecBoxTransfero->AddColumnInPixels( CD_TecBox_Standing, m_mapSSheetColumnWidth[SD_TecBoxTransfero][CD_TecBox_Standing] );
	pclSDTecBoxTransfero->AddColumnInPixels( CD_TecBox_ElectricalPower, m_mapSSheetColumnWidth[SD_TecBoxTransfero][CD_TecBox_ElectricalPower] );
	pclSDTecBoxTransfero->AddColumnInPixels( CD_TecBox_SupplyVoltage, m_mapSSheetColumnWidth[SD_TecBoxTransfero][CD_TecBox_SupplyVoltage] );
	pclSDTecBoxTransfero->AddColumnInPixels( CD_TecBox_Decibel, m_mapSSheetColumnWidth[SD_TecBoxTransfero][CD_TecBox_Decibel] );
	pclSDTecBoxTransfero->AddColumnInPixels( CD_TecBox_BufferVessel, m_mapSSheetColumnWidth[SD_TecBoxTransfero][CD_TecBox_BufferVessel] );
	pclSDTecBoxTransfero->AddColumnInPixels( CD_TecBox_IntegratedVessel, m_mapSSheetColumnWidth[SD_TecBoxTransfero][CD_TecBox_IntegratedVessel] );
	pclSDTecBoxTransfero->AddColumnInPixels( CD_TecBox_Pointer, m_mapSSheetColumnWidth[SD_TecBoxTransfero][CD_TecBox_Pointer] );
	
	// Must be placed after any call to 'AddColumnInPixels'. Because this method will add needed columns and TSpread will show them by default.
	// When calling the first time 'AddColumnInPixels' for the 'CD_TecBox_Name' column, this method will call TSpread::SetMaxCols() to adapt the
	// number of columns. All the column before 'CD_TecBox_Name' are set to a default width and are visible.
	if( false == TASApp.IsPMPriceIndexUsed() )
	{
		pclSSheet->ShowCol( CD_TecBox_PriceIndexIcon, FALSE );
		pclSSheet->ShowCol( CD_TecBox_PriceIndexValue, FALSE );
		pclSSheet->ShowCol( CD_TecBox_PriceIndexBorder, FALSE );
	}

	pclSSheet->ShowCol( CD_TecBox_IntegratedVessel, FALSE );
	
	// Set in which column parameter must be saved.
	pclSDTecBoxTransfero->AddParameterColumn( CD_TecBox_Pointer );

	// Set the focus column.
	pclSDTecBoxTransfero->SetActiveColumn( CD_TecBox_FirstColumn + 1 );

	// Set range for selection.
	pclSDTecBoxTransfero->SetFocusColumnRange( CD_TecBox_FirstColumn + 1, CD_TecBox_Pointer - 1 );

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Add the main title.
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, _IMI_TITLE_GROUP1 );
	pclSDTecBoxTransfero->SetUserVariable( _SDUV_TITLEFORECOLOR, ( pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor ) ) );
	pclSDTecBoxTransfero->SetUserVariable( _SDUV_TITLEBACKCOLOR, ( pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor ) ) );

	pclSSheet->AddCellSpanW( CD_TecBox_FirstColumn + 1, RD_TecBox_GroupName, CD_TecBox_Pointer - ( CD_TecBox_FirstColumn + 1 ), 1 );
	pclSSheet->SetStaticText( CD_TecBox_FirstColumn + 1, RD_TecBox_GroupName, IDS_SSHEETSSELPM_TFGROUP );
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Column name.
	if( false == pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn ) )
	{
		return;
	}

	if( false == pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE ) )
	{
		return;
	}

	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap, (LPARAM)TRUE );

	if( true == TASApp.IsPMPriceIndexUsed() )
	{
		pclSSheet->AddCellSpan( CD_TecBox_PriceIndexIcon, RD_TecBox_ColName, 3, 1 );
		pclSSheet->SetStaticText( CD_TecBox_PriceIndexIcon, RD_TecBox_ColName, IDS_SSHEETSSELPM_PRICEINDEX );
	}

	pclSSheet->SetStaticText( CD_TecBox_Name, RD_TecBox_ColName, IDS_SSHEETSSELPM_TFNAME );
	pclSSheet->SetStaticText( CD_TecBox_NbreOfDevice, RD_TecBox_ColName, IDS_SSHEETSSELPM_TBNUMBEROFDEVICES );
	pclSSheet->SetStaticText( CD_TecBox_PS, RD_TecBox_ColName, IDS_SSHEETSSELPM_CPPS );
	pclSSheet->SetStaticText( CD_TecBox_TempRange, RD_TecBox_ColName, IDS_SSHEETSSELPM_VESSELTEMPRANGE );
	pclSSheet->SetStaticText( CD_TecBox_MaxWaterMakeUpTemp, RD_TecBox_ColName, IDS_SSHEETSSELPM_MAXWATERMAKEUPTEMP );
	pclSSheet->SetStaticText( CD_TecBox_B, RD_TecBox_ColName, IDS_SSHEETSSELPM_CPWIDTH );
	pclSSheet->SetStaticText( CD_TecBox_H, RD_TecBox_ColName, IDS_SSHEETSSELPM_CPHEIGHT );
	pclSSheet->SetStaticText( CD_TecBox_T, RD_TecBox_ColName, IDS_SSHEETSSELPM_CPDEPTH );
	pclSSheet->SetStaticText( CD_TecBox_Weight, RD_TecBox_ColName, IDS_SSHEETSSELPM_CPMASS );
	//pclSSheet->SetStaticText( CD_TecBox_Standing, RD_TecBox_ColName, IDS_SSHEETSSELPM_TBSTANDING );
	pclSSheet->SetStaticText( CD_TecBox_ElectricalPower, RD_TecBox_ColName, IDS_SSHEETSSELPM_CPELECPOWER );
	pclSSheet->SetStaticText( CD_TecBox_SupplyVoltage, RD_TecBox_ColName, IDS_SSHEETSSELPM_CPELECVOLTAGE );
	pclSSheet->SetStaticText( CD_TecBox_Decibel, RD_TecBox_ColName, IDS_SSHEETSSELPM_TBSOUNDPRESSLEVEL );
	pclSSheet->SetStaticText( CD_TecBox_BufferVessel, RD_TecBox_ColName, IDS_SSHEETSSELPM_TBBUFFERVESSEL );
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Column units.
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap, (LPARAM)FALSE );

	if( true == TASApp.IsPMPriceIndexUsed() )
	{
		pclSSheet->SetStaticText( CD_TecBox_PriceIndexValue, RD_TecBox_ColUnit, _T("%") );
	}

	pclSSheet->SetStaticText( CD_TecBox_PS, RD_TecBox_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_PRESSURE ) ).c_str() );
	pclSSheet->SetStaticText( CD_TecBox_TempRange, RD_TecBox_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_TEMPERATURE ) ).c_str() );
	pclSSheet->SetStaticText( CD_TecBox_MaxWaterMakeUpTemp, RD_TecBox_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_TEMPERATURE ) ).c_str() );
	pclSSheet->SetStaticText( CD_TecBox_B, RD_TecBox_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_LENGTH ) ).c_str() );
	pclSSheet->SetStaticText( CD_TecBox_H, RD_TecBox_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_LENGTH ) ).c_str() );
	pclSSheet->SetStaticText( CD_TecBox_T, RD_TecBox_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_LENGTH ) ).c_str() );
	pclSSheet->SetStaticText( CD_TecBox_Weight, RD_TecBox_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_MASS ) ).c_str() );
	pclSSheet->SetStaticText( CD_TecBox_ElectricalPower, RD_TecBox_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_ELEC_POWER ) ).c_str() );
	pclSSheet->SetStaticText( CD_TecBox_Decibel, RD_TecBox_ColUnit, IDS_SSHEETSSELPM_TBDECIBELUNIT );
	pclSSheet->SetStaticText( CD_TecBox_SupplyVoltage, RD_TecBox_ColUnit, IDS_SSHEETSSELPM_TBVOLTUNIT );
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// By default we don't show this column.
	pclSSheet->ShowCol( CD_TecBox_MaxWaterMakeUpTemp, FALSE );

	// Draw black line below column header.
	pclSSheet->SetCellBorder( CD_TecBox_FirstColumn + 1, RD_TecBox_ColUnit, CD_TecBox_Pointer - 1, RD_TecBox_ColUnit, true, SS_BORDERTYPE_BOTTOM );
}

long CRViewSSelPM::_FillTecBoxTransferoRow( CSheetDescription *pclSDTecBoxTransfero, long lRow, CSelectedTransfero *pclSelectedTecBoxTransfero,
		CPMInputUser *pclPMInputUser, CDS_SSelPMaint *pclEditedSSelPM, CSelectedPMBase *pclSelectedBestPriceProduct, bool bForRejectedProduct )
{
	if( NULL == pclSDTecBoxTransfero || NULL == pclSDTecBoxTransfero->GetSSheetPointer() || NULL == pclSelectedTecBoxTransfero || NULL == pclPMInputUser )
	{
		return -1;
	}

	CDB_TecBox *pclTransferoTecBox = dynamic_cast<CDB_TecBox *>( pclSelectedTecBoxTransfero->GetpData() );

	if( NULL == pclTransferoTecBox )
	{
		return -1;
	}

	CSSheet *pclSSheet = pclSDTecBoxTransfero->GetSSheetPointer();

	bool fRedundancy = pclPMInputUser->GetPMWQSelectionPreferences()->IsChecked( CPMWQPrefs::GeneralRedundancyTecBox );
	bool fRedundancyDisabled = pclPMInputUser->GetPMWQSelectionPreferences()->IsDisabled( CPMWQPrefs::GeneralRedundancyTecBox );

	if( true == m_bIsPMTypeAll && true == pclSelectedTecBoxTransfero->IsFlagSet( CSelectedPMBase::eNotPriority ) )
	{
		m_lProductNotPriorityCount++;

		if( false == m_bShowAllPrioritiesShown )
		{
			// Add one empty not selectable row.
			pclSDTecBoxTransfero->AddRows( 1 );
			pclSDTecBoxTransfero->RemoveSelectableRow( lRow );

			pclSSheet->SetCellBorder( CD_TecBox_FirstColumn + 1, lRow, CD_TecBox_Pointer - 1, lRow, true, SS_BORDERTYPE_TOP | SS_BORDERTYPE_BOTTOM, 
					SS_BORDERSTYLE_SOLID, _BLACK );

			m_lShowAllPrioritiesButtonRow = lRow++;
			m_bShowAllPrioritiesShown = true;
		}
	}

	// Add this row.
	// 'true' to specify that this row can be selected.
	pclSDTecBoxTransfero->AddRows( 1, true );

	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

	// Add checkbox.
	pclSSheet->SetCheckBox(CD_TecBox_CheckBox, lRow, _T(""), false, true);

	// Set the price index if used.
	if( true == TASApp.IsPMPriceIndexUsed() )
	{
		if( NULL != pclSelectedBestPriceProduct && 
				0 == _tcscmp( pclSelectedBestPriceProduct->GetProductIDPtr().ID, pclSelectedTecBoxTransfero->GetProductIDPtr().ID ) )
		{
			pclSSheet->SetPictureCellWithID( IDI_BESTPRICE, CD_TecBox_PriceIndexIcon, lRow, CSSheet::PictureCellType::Icon );
		}

		CString str = _T("-");

		if( pclSelectedTecBoxTransfero->GetPriceIndex() > 0.0 )
		{
			str.Format( _T("%i"), (int)pclSelectedTecBoxTransfero->GetPriceIndex() );
		}

		pclSSheet->SetStaticText( CD_TecBox_PriceIndexValue, lRow, str );
	}

	bool bBest = pclSelectedTecBoxTransfero->IsFlagSet( CSelectedPMBase::eBest );

	if( true == bBest )
	{
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_GREEN );
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, true );
	}
	
	if( false == pclSelectedTecBoxTransfero->GetRejected() 
			&& 0 == CString( pclTransferoTecBox->GetFamilyID() ).Compare( _T("TBXFAM_TRANSFERO_TV_C") ) 
			&& true == pclTransferoTecBox->IsVariantHighFlowTemperature() )
	{
		// Need a tooltip because for the high flow variant of Transfero Connect, we can't use the degassing option with temperature above 90°C.
		// The same when temperature is below 0°C.
		if( pclPMInputUser->GetReturnTemperature() > pclTransferoTecBox->GetTmax() 
				|| pclPMInputUser->GetMinTemperature() < pclTransferoTecBox->GetTmin() )
		{
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_ORANGE );
		}
	}

	// Set name.
	pclSSheet->SetStaticText( CD_TecBox_Name, lRow, pclTransferoTecBox->GetName() );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, false );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );
	// HYS-1537 : Check if the preference is enabled
	pclSSheet->SetStaticText( CD_TecBox_NbreOfDevice, lRow, ( false == fRedundancy || fRedundancyDisabled == true ) ? _T("1" ) : _T( "2") );
	pclSSheet->SetStaticText( CD_TecBox_PS, lRow, WriteCUDouble( _U_PRESSURE, pclTransferoTecBox->GetPmaxmax() ) );

	if( true == pclSelectedTecBoxTransfero->IsFlagSet( CSelectedPMBase::eTemperature ) )
	{
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_ORANGE );
	}

	pclSSheet->SetStaticText( CD_TecBox_TempRange, lRow, pclTransferoTecBox->GetTempRange() );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

	if( true == pclTransferoTecBox->IsFctWaterMakeUp() )
	{
		pclSSheet->ShowCol( CD_TecBox_MaxWaterMakeUpTemp, TRUE );

		// Remark: for Transfero we don't have the min. admissible temperature for the integrated water make-up part.
		if( true == pclSelectedTecBoxTransfero->IsFlagSet( CSelectedPMBase::ePlenoWMUpMaxTemp ) )
		{
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_RED );
		}

		pclSSheet->SetStaticText( CD_TecBox_MaxWaterMakeUpTemp, lRow, WriteCUDouble( _U_TEMPERATURE, pclTransferoTecBox->GetTmaxWaterMakeUp() ) );
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );
	}

	pclSSheet->SetStaticText( CD_TecBox_B, lRow, WriteCUDouble( _U_LENGTH, pclTransferoTecBox->GetWidth() ) );
	pclSSheet->SetStaticText( CD_TecBox_H, lRow, WriteCUDouble( _U_LENGTH, pclTransferoTecBox->GetHeight() ) );
	pclSSheet->SetStaticText( CD_TecBox_T, lRow, WriteCUDouble( _U_LENGTH, pclTransferoTecBox->GetDepth() ) );
	pclSSheet->SetStaticText( CD_TecBox_Weight, lRow, WriteCUDouble( _U_MASS, pclTransferoTecBox->GetWeight() ) );
	pclSSheet->SetPictureCellWithID( pclTransferoTecBox->GetInstallationLayoutPictureID(), CD_TecBox_Standing, lRow, CSSheet::PictureCellType::Icon );
	pclSSheet->SetStaticText( CD_TecBox_ElectricalPower, lRow, WriteCUDouble( _U_ELEC_POWER, pclTransferoTecBox->GetPower() ) );
	pclSSheet->SetStaticText( CD_TecBox_SupplyVoltage, lRow, pclTransferoTecBox->GetPowerSupplyStr() );
	pclSSheet->SetStaticText( CD_TecBox_Decibel, lRow, WriteDouble( pclTransferoTecBox->GetSoundPressureLevel(), 1 ) );

	// When 'CSelectPMList::GetTransferoBufferVesselList' is called when selecting Transfero we fill 'IDPtrBufVssl' with the correct
	// buffer vessel only if this one is not integrated.
	CDB_Vessel *pSelectedBufferVessel = NULL;

	if( false == pclTransferoTecBox->IsVariantIntegratedBufferVessel() )
	{
		std::map<int, CSelectedTransfero::BufferVesselData> *pvecBufferVesselList = pclSelectedTecBoxTransfero->GetBufferVesselList();

		if( NULL == pvecBufferVesselList )
		{
			ASSERT( 0 );
			pclSSheet->SetStaticText( CD_TecBox_BufferVessel, lRow, _T("") );
		}
		else if( 1 == (int)pvecBufferVesselList->size() )
		{
			if( false == pvecBufferVesselList->at( pvecBufferVesselList->begin()->first ).m_bOnRequest )
			{
				pclSSheet->SetStaticText( CD_TecBox_BufferVessel, lRow, pvecBufferVesselList->at( pvecBufferVesselList->begin()->first ).m_pclVessel->GetName() );
				// HYS-872
				pSelectedBufferVessel = pvecBufferVesselList->at(pvecBufferVesselList->begin()->first).m_pclVessel;
			}
			else
			{
				pclSSheet->SetStaticText( CD_TecBox_BufferVessel, lRow, TASApp.LoadLocalizedString( IDS_SELECTPM_ONREQUEST ) );
			}
		}
		else
		{
			if (pvecBufferVesselList->size() > 0)
			{
				pclSSheet->FormatComboList( CD_TecBox_BufferVessel, lRow, SSS_ALIGN_CENTER | SSS_ALIGN_VCENTER );
				pclSSheet->ComboBoxSendMessage( CD_TecBox_BufferVessel, lRow, SS_CBM_RESETCONTENT, 0, 0 );
			}
			else if (!pclSelectedTecBoxTransfero->GetRejected())
			{
				// HYS-868: Add a information picture to tell user that have informations
				pclSSheet->SetPictureCellWithID( IDI_CELLINFO, CD_TecBox_BufferVessel, lRow, CSSheet::PictureCellType::Icon );
			}
			
			int iSelIndex = 0;
			
			for( auto &it : *pvecBufferVesselList )
			{
				// HYS-1025:
				if( NULL == pvecBufferVesselList->at( it.first ).m_pclVessel )
				{
					continue;
				}
				
				CString strBufferVesselName = pvecBufferVesselList->at( it.first ).m_pclVessel->GetName();
				pclSSheet->ComboBoxSendMessage( CD_TecBox_BufferVessel, lRow, SS_CBM_ADDSTRING, 0, (LPARAM)(LPCTSTR)strBufferVesselName );
				
				// HYS-919: Keep the selected buffer vessel when the selection is editing
				if( ( NULL != pclEditedSSelPM ) && ( NULL != pclEditedSSelPM->GetTecBoxCompTransfIDPtr().MP ) )
				{
					CDB_Vessel *pclEditedIntegratedVessel = dynamic_cast<CDB_Vessel *> (pclEditedSSelPM->GetTecBoxIntegratedVesselIDPtr().MP);

					if( (NULL != pclEditedIntegratedVessel) && ( 0 == StrCmp( pclEditedIntegratedVessel->GetName(), strBufferVesselName ) ) )
					{
						iSelIndex = it.first;
					}
				}
			}

			if( pvecBufferVesselList->size() > 0 )
			{
				pclSSheet->ComboBoxSendMessage(CD_TecBox_BufferVessel, lRow, SS_CBM_SETCURSEL, iSelIndex, 0);
				// HYS-872.
				pSelectedBufferVessel = pvecBufferVesselList->at(iSelIndex).m_pclVessel;
				// HYS-1025.
				ASSERT( NULL != pSelectedBufferVessel );
			}
		}
	}
	else
	{
		pclSSheet->SetStaticText( CD_TecBox_BufferVessel, lRow, TASApp.LoadLocalizedString( IDS_SSHEETSSELPM_TBBUFFERINTEGRATED ) );
	}

	// Save parameter.
	CCellDescriptionProduct *pclCDProduct = CreateCellDescriptionProduct( pclSDTecBoxTransfero->GetFirstParameterColumn(), lRow, 
			(LPARAM)pclTransferoTecBox, pclSDTecBoxTransfero );

	if( NULL != pclCDProduct )
	{
		pclCDProduct->SetUserParam( (LPARAM)pclSelectedTecBoxTransfero );
		// HYS-872
		pclCDProduct->SetSecondUserParam((LPARAM)pSelectedBufferVessel);
	}

	CDB_TecBox *pclEditedTecBoxTransfero = NULL;

	if( NULL != pclEditedSSelPM )
	{
		pclEditedTecBoxTransfero = dynamic_cast<CDB_TecBox *>( pclEditedSSelPM->GetTecBoxCompTransfIDPtr().MP );
	}

	if( NULL != pclEditedTecBoxTransfero && pclEditedTecBoxTransfero == pclTransferoTecBox )
	{
		m_lSelectedRow = lRow;
	}

	pclSSheet->SetCellBorder( CD_TecBox_FirstColumn + 1, lRow, CD_TecBox_Pointer - 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _GRAY );

	return lRow;
}

bool CRViewSSelPM::_FillTransferoCurves( CDB_TecBox *pclTransferoSelected )
{
	if( NULL == m_pclIndSelPMParams || NULL == m_pclIndSelPMParams->m_pclSelectPMList || NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERTA_RETURN( false );
	}

	if( NULL == pclTransferoSelected || false == pclTransferoSelected->IsFctPump() )
	{
		return false;
	}

	CDlgPMGraphsOutput::CPMInterface clOutputInterface;
	clOutputInterface.ClearOutput();

	double dPman = m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetTargetPressureForTecBox( pclTransferoSelected->GetTecBoxType() );
	double dVD = m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetVD();
	
	CDlgPMTChartHelper *pclTransferoCurve = m_pclIndSelPMParams->m_pclSelectPMList->GetTransferoCurves( pclTransferoSelected, dPman, dVD,
			clOutputInterface.GetCWndPMGraphsOutput() );

	if( NULL == pclTransferoCurve )
	{
		return false;
	}

	clOutputInterface.AddTChartDialog( pclTransferoCurve );

	double dp0 = m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetMinimumPressure();
	double dpa = m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetMinimumInitialPressure();
	double dpe = m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetFinalPressure( CDB_TecBox::etbtCompresso );
	double dPSV = m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetSafetyValveResponsePressure();
	HBITMAP hPressureGauge = m_pclIndSelPMParams->m_pclSelectPMList->GetPressureGauge( dp0, dpa, dpe, dPSV, clOutputInterface.GetCWndPMGraphsOutput() );

	if( NULL == hPressureGauge )
	{
		return false;
	}

	clOutputInterface.AddBitmap( hPressureGauge );	
	
	_SetPMGraphsOutputContext( false );

	return true;
}

void CRViewSSelPM::_FillTransferoVsslSheet( UINT uiSDGroupID, CDS_SSelPMaint *pclEditedSSelPM )
{
	if( NULL == m_pclIndSelPMParams || NULL == m_pclIndSelPMParams->m_pclSelectPMList || NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERT_RETURN;
	}

	// Get 'Parent' sheet identification.
	UINT uiAfterSheetDescriptionID = _GetAfterSheetDescriptionID( SD_TransferoVessel, uiSDGroupID );

	// Call 'MultiSpreadBase' method to get a new sheet.
	CSheetDescription *pclSDTransferoVssl = CreateSSheet( SD_TransferoVessel, uiAfterSheetDescriptionID );

	if( NULL == pclSDTransferoVssl || NULL == pclSDTransferoVssl->GetSSheetPointer() )
	{
		return;
	}

	pclSDTransferoVssl->SetGroupSheetDescriptionID( uiSDGroupID );
	pclSDTransferoVssl->SetParentSheetDescriptionID( SD_TransferoVessel );

	CSSheet *pclSSheet = pclSDTransferoVssl->GetSSheetPointer();

	pclSSheet->SetBool( SSB_REDRAW, FALSE );

	_InitTransferoVsslHeaders( pclSDTransferoVssl );

	m_lProductTotalCount = 0;
	m_lProductNotPriorityCount = 0;
	m_bShowAllPrioritiesShown = false;
	m_lShowAllPrioritiesButtonRow = 0;
	long lRow = RD_TecBoxVssl_FirstAvailRow;
	pclSDTransferoVssl->RestartRemarkGenerator();

	CSelectedPMBase *pclSelectedBestPriceProduct = NULL;

	if( true == TASApp.IsPMPriceIndexUsed() )
	{
		pclSelectedBestPriceProduct = m_pclIndSelPMParams->m_pclSelectPMList->GetLowestPriceObject( CSelectPMList::ProductType::PT_TransferoVessel );
	}

	// Priority/NotaPriority loop.
	for( int iLoop = 1; iLoop >= 0; --iLoop )
	{
		for( CSelectedPMBase *pclSelectedProduct = m_pclIndSelPMParams->m_pclSelectPMList->GetFirstProduct( CSelectPMList::PT_TransferoVessel );
				NULL != pclSelectedProduct; pclSelectedProduct = m_pclIndSelPMParams->m_pclSelectPMList->GetNextProduct( CSelectPMList::PT_TransferoVessel ) )
		{
			// Skip not Priority when iLoop == 1
			if( ( iLoop == 1 ) == pclSelectedProduct->IsFlagSet( CSelectedPMBase::eNotPriority ) )
			{
				continue;
			}

			CSelectedVssl *pclSelTransferoVssl = dynamic_cast<CSelectedVssl *>( pclSelectedProduct );

			if( NULL == pclSelTransferoVssl )
			{
				continue;
			}

			long lReturn = _FillTransferoVsslRow( pclSDTransferoVssl, lRow, pclSelTransferoVssl, m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser(),
					pclSelectedBestPriceProduct );

			if( -1 == lReturn )
			{
				continue;
			}
			else
			{
				lRow = lReturn;
			}

			lRow++;
			m_lProductTotalCount++;
		}
	}

	long lLastDataRow = lRow - 1;
	pclSDTransferoVssl->SetUserVariable( _SDUV_TOTALPRODUCT_COUNT, m_lProductTotalCount );
	pclSDTransferoVssl->SetUserVariable( _SDUV_NOPRIORITYPRODUCT_COUNT, m_lProductNotPriorityCount );

	// Now we can create Show/Hide all priorities if needed.
	// Remarks: 2nd condition to avoid to have button if we have only no priority valves.
	if( true == m_bShowAllPrioritiesShown && m_lProductTotalCount > m_lProductNotPriorityCount )
	{
		// Create Show/Hide all priorities button in regards to current state!
		// HYS-1539 : The button state is the same for all sheet
		CCDButtonShowAllPriorities::ButtonState eState = ( true == GetShowAllPrioritiesFlag( SD_TransferoVessel ) ) ? 
				CCDButtonShowAllPriorities::ButtonState::HidePriorities : CCDButtonShowAllPriorities::ButtonState::ShowPriorities;

		CCDButtonShowAllPriorities *pclShowAllButton = CreateShowAllPrioritiesButton( CD_TecBoxVssl_FirstColumn, m_lShowAllPrioritiesButtonRow,
				m_bShowAllPrioritiesShown, eState, m_lShowAllPrioritiesButtonRow + 1, pclSSheet->GetMaxRows(), pclSDTransferoVssl );

		// Here we must call 'ApplyInternalChange' to show or hide rows in regards to the current state!
		if( NULL != pclShowAllButton )
		{
			pclShowAllButton->ApplyInternalChange();
		}
	}
	else
	{
		if( m_lShowAllPrioritiesButtonRow != 0 )
		{
			pclSSheet->ShowRow( m_lShowAllPrioritiesButtonRow, false );
		}
	}

	pclSSheet->SetCellBorder( CD_TecBoxVssl_FirstColumn + 1, lLastDataRow, CD_TecBoxVssl_Pointer - 1, lLastDataRow, true, SS_BORDERTYPE_BOTTOM, 
			SS_BORDERSTYLE_SOLID, _BLACK );

	// Add first the link to the excluded products if exist.
	lRow = _AddExcludedProductsLink( pclSDTransferoVssl, lRow, CSelectPMList::PT_TransferoVessel );

	// Add Remarks.
	// Note: these lines must be placed after creating Show/Hide all priorities button to avoid to take in count these lines when defining
	//       range of row to be Show/Hide.
	lRow = pclSDTransferoVssl->WriteRemarks( lRow, CD_TecBoxVssl_FirstColumn + 1, CD_TecBoxVssl_Pointer );

	// Add possibility to change column size.
	// Remark: place this code after any initialization (like adding, removing, resizing, hiding/showing columns).
	CResizingColumnInfo *pclResizingColumnInfo = pclSDTransferoVssl->GetResizingColumnInfo();
	pclResizingColumnInfo->ActivateFeature( true, false, RD_TecBoxVssl_FirstAvailRow, lLastDataRow );
	pclResizingColumnInfo->AddRangeColumn( CD_TecBoxVssl_PriceIndexRatioBorder, CD_TecBoxVssl_Pointer - 1, RD_TecBoxVssl_ColName, RD_TecBoxVssl_ColUnit );

	// Add possibility to click on title group to reset column width.
	CreateTitleGroupButtons( CD_TecBoxVssl_FirstColumn + 1, CD_TecBoxVssl_Pointer - 1, RD_TecBoxVssl_GroupName, pclSDTransferoVssl );

	pclSSheet->SetBool( SSB_REDRAW, TRUE );
}

void CRViewSSelPM::_InitTransferoVsslHeaders( CSheetDescription *pclSDTransferoVssl )
{
	if( NULL == pclSDTransferoVssl || NULL == pclSDTransferoVssl->GetSSheetPointer() )
	{
		return;
	}

	CSSheet *pclSSheet = pclSDTransferoVssl->GetSSheetPointer();

	pclSSheet->Init();
	pclSSheet->SetBool( SSB_ALLOWDRAGDROP, FALSE );
	pclSSheet->SetBool( SSB_HORZSCROLLBAR, FALSE );
	pclSSheet->SetBool( SSB_VERTSCROLLBAR, FALSE );
	pclSSheet->SetBool( SSB_MOVEACTIVEONFOCUS, FALSE );
	pclSSheet->SetTextTip( SS_TT_STATUS_FLOATING, 250, NULL, _LIGHTYELLOW, _TAH_BLACK );

	// Set max rows.
	pclSSheet->SetMaxRows( RD_TecBoxVssl_FirstAvailRow - 1 );

	// All cells are static by default and filled with _T("").
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	pclSSheet->FormatStaticText( -1, -1, -1, -1, _T( "" ) );

	// Increase row height.
	double dRowHeight = 12.75;
	pclSSheet->SetRowHeight( RD_TecBoxVssl_FirstRow, dRowHeight * 0.5 );
	pclSSheet->SetRowHeight( RD_TecBoxVssl_GroupName, dRowHeight * 1.5 );
	pclSSheet->SetRowHeight( RD_TecBoxVssl_ColName, dRowHeight * 2 );
	pclSSheet->SetRowHeight( RD_TecBoxVssl_ColUnit, dRowHeight * 1.2 );

	// Initialize.
	pclSDTransferoVssl->Init( 1, pclSSheet->GetMaxRows(), 1, CSheetDescription::SD_ParameterMode_Mono );
	pclSDTransferoVssl->AddColumnInPixels( CD_TecBoxVssl_FirstColumn, m_mapSSheetColumnWidth[SD_TransferoVessel][CD_TecBoxVssl_FirstColumn] );
	pclSDTransferoVssl->AddColumnInPixels( CD_TecBoxVssl_CheckBox, m_mapSSheetColumnWidth[SD_TransferoVessel][CD_TecBoxVssl_CheckBox] );

	if( true == TASApp.IsPMPriceIndexUsed() )
	{
		pclSDTransferoVssl->AddColumnInPixels( CD_TecBoxVssl_PriceIndexRatioIcon, m_mapSSheetColumnWidth[SD_TransferoVessel][CD_TecBoxVssl_PriceIndexRatioIcon] );
		pclSDTransferoVssl->AddColumnInPixels( CD_TecBoxVssl_PriceIndexRatioValue, m_mapSSheetColumnWidth[SD_TransferoVessel][CD_TecBoxVssl_PriceIndexRatioValue] );
		pclSDTransferoVssl->AddColumnInPixels( CD_TecBoxVssl_PriceIndexRatioBorder, m_mapSSheetColumnWidth[SD_TransferoVessel][CD_TecBoxVssl_PriceIndexRatioBorder] );
	}

	pclSDTransferoVssl->AddColumnInPixels( CD_TecBoxVssl_Prim_Name, m_mapSSheetColumnWidth[SD_TransferoVessel][CD_TecBoxVssl_Prim_Name] );
	pclSDTransferoVssl->AddColumnInPixels( CD_TecBoxVssl_Prim_Volume, m_mapSSheetColumnWidth[SD_TransferoVessel][CD_TecBoxVssl_Prim_Volume] );
	pclSDTransferoVssl->AddColumnInPixels( CD_TecBoxVssl_Prim_MaxPressure, m_mapSSheetColumnWidth[SD_TransferoVessel][CD_TecBoxVssl_Prim_MaxPressure] );
	pclSDTransferoVssl->AddColumnInPixels( CD_TecBoxVssl_Prim_TempRange, m_mapSSheetColumnWidth[SD_TransferoVessel][CD_TecBoxVssl_Prim_TempRange] );
	pclSDTransferoVssl->AddColumnInPixels( CD_TecBoxVssl_Prim_Diameter, m_mapSSheetColumnWidth[SD_TransferoVessel][CD_TecBoxVssl_Prim_Diameter] );
	pclSDTransferoVssl->AddColumnInPixels( CD_TecBoxVssl_Prim_Height, m_mapSSheetColumnWidth[SD_TransferoVessel][CD_TecBoxVssl_Prim_Height] );
	pclSDTransferoVssl->AddColumnInPixels( CD_TecBoxVssl_Prim_Weight, m_mapSSheetColumnWidth[SD_TransferoVessel][CD_TecBoxVssl_Prim_Weight] );
	pclSDTransferoVssl->AddColumnInPixels( CD_TecBoxVssl_Prim_MaxWeight, m_mapSSheetColumnWidth[SD_TransferoVessel][CD_TecBoxVssl_Prim_MaxWeight] );
	pclSDTransferoVssl->AddColumnInPixels( CD_TecBoxVssl_Sec_Name, m_mapSSheetColumnWidth[SD_TransferoVessel][CD_TecBoxVssl_Sec_Name] );
	pclSDTransferoVssl->AddColumnInPixels( CD_TecBoxVssl_Sec_NbreOfVssl, m_mapSSheetColumnWidth[SD_TransferoVessel][CD_TecBoxVssl_Sec_NbreOfVssl] );
	pclSDTransferoVssl->AddColumnInPixels( CD_TecBoxVssl_Sec_Diameter, m_mapSSheetColumnWidth[SD_TransferoVessel][CD_TecBoxVssl_Sec_Diameter] );
	pclSDTransferoVssl->AddColumnInPixels( CD_TecBoxVssl_Sec_Height, m_mapSSheetColumnWidth[SD_TransferoVessel][CD_TecBoxVssl_Sec_Height] );
	pclSDTransferoVssl->AddColumnInPixels( CD_TecBoxVssl_Sec_Weight, m_mapSSheetColumnWidth[SD_TransferoVessel][CD_TecBoxVssl_Sec_Weight] );
	pclSDTransferoVssl->AddColumnInPixels( CD_TecBoxVssl_Sec_MaxWeight, m_mapSSheetColumnWidth[SD_TransferoVessel][CD_TecBoxVssl_Sec_MaxWeight] );
	pclSDTransferoVssl->AddColumnInPixels( CD_TecBoxVssl_Pointer, m_mapSSheetColumnWidth[SD_TransferoVessel][CD_TecBoxVssl_Pointer] );

	// Must be placed after any call to 'AddColumnInPixels'. Because this method will add needed columns and TSpread will show them by default.
	// When calling the first time 'AddColumnInPixels' for the 'CD_TecBoxVssl_Prim_Name' column, this method will call TSpread::SetMaxCols() to adapt the
	// number of columns. All the column before 'CD_TecBoxVssl_Prim_Name' are set to a default width and are visible.
	if( false == TASApp.IsPMPriceIndexUsed() )
	{
		pclSSheet->ShowCol( CD_TecBoxVssl_PriceIndexRatioIcon, FALSE );
		pclSSheet->ShowCol( CD_TecBoxVssl_PriceIndexRatioValue, FALSE );
		pclSSheet->ShowCol( CD_TecBoxVssl_PriceIndexRatioBorder, FALSE );
	}

	// Set in which column parameter must be saved.
	pclSDTransferoVssl->AddParameterColumn( CD_TecBoxVssl_Pointer );

	// Set the focus column.
	pclSDTransferoVssl->SetActiveColumn( CD_TecBoxVssl_FirstColumn + 1 );

	// Set range for selection.
	pclSDTransferoVssl->SetFocusColumnRange( CD_TecBoxVssl_FirstColumn + 1, CD_TecBoxVssl_Pointer - 1 );

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Add the main title.
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, _IMI_TITLE_GROUP3 );
	pclSDTransferoVssl->SetUserVariable( _SDUV_TITLEFORECOLOR, ( pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor ) ) );
	pclSDTransferoVssl->SetUserVariable( _SDUV_TITLEBACKCOLOR, ( pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor ) ) );

	pclSSheet->AddCellSpanW( CD_TecBoxVssl_FirstColumn + 1, RD_TecBoxVssl_GroupName, CD_TecBoxVssl_Pointer - ( CD_TecBoxVssl_FirstColumn + 1 ), 1 );
	pclSSheet->SetStaticText( CD_TecBoxVssl_FirstColumn + 1, RD_TecBoxVssl_GroupName, IDS_SSHEETSSELPM_TRSFROVSSLGROUP );
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Column name.
	if( false == pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn ) )
	{
		return;
	}

	if( false == pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE ) )
	{
		return;
	}

	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap, (LPARAM)TRUE );

	if( true == TASApp.IsPMPriceIndexUsed() )
	{
		pclSSheet->AddCellSpan( CD_TecBoxVssl_PriceIndexRatioIcon, RD_TecBoxVssl_ColName, 3, 1 );
		pclSSheet->SetStaticText( CD_TecBoxVssl_PriceIndexRatioIcon, RD_TecBoxVssl_ColName, IDS_SSHEETSSELPM_PRICEINDEXRATIO );
	}

	pclSSheet->SetStaticText( CD_TecBoxVssl_Prim_Name, RD_TecBoxVssl_ColName, IDS_SSHEETSSELPM_VESSELNAME );
	pclSSheet->SetStaticText( CD_TecBoxVssl_Prim_Volume, RD_TecBoxVssl_ColName, IDS_SSHEETSSELPM_VESSELVOL );
	pclSSheet->SetStaticText( CD_TecBoxVssl_Prim_MaxPressure, RD_TecBoxVssl_ColName, IDS_SSHEETSSELPM_VESSELMAXPRESS );
	pclSSheet->SetStaticText( CD_TecBoxVssl_Prim_TempRange, RD_TecBoxVssl_ColName, IDS_SSHEETSSELPM_VESSELTEMPRANGE );
	pclSSheet->SetStaticText( CD_TecBoxVssl_Prim_Diameter, RD_TecBoxVssl_ColName, IDS_SSHEETSSELPM_VESSELDIAMETER );
	pclSSheet->SetStaticText( CD_TecBoxVssl_Prim_Height, RD_TecBoxVssl_ColName, IDS_SSHEETSSELPM_VESSELHEIGHT );
	pclSSheet->SetStaticText( CD_TecBoxVssl_Prim_Weight, RD_TecBoxVssl_ColName, IDS_SSHEETSSELPM_VESSELMASS );
	pclSSheet->SetStaticText( CD_TecBoxVssl_Prim_MaxWeight, RD_TecBoxVssl_ColName, IDS_SSHEETSSELPM_VESSELMAXWEIGHT );
	pclSSheet->SetStaticText( CD_TecBoxVssl_Sec_Name, RD_TecBoxVssl_ColName, IDS_SSHEETSSELPM_VESSELNAME );
	pclSSheet->SetStaticText( CD_TecBoxVssl_Sec_NbreOfVssl, RD_TecBoxVssl_ColName, IDS_SSHEETSSELPM_VESSELSECQTY );
	pclSSheet->SetStaticText( CD_TecBoxVssl_Sec_Diameter, RD_TecBoxVssl_ColName, IDS_SSHEETSSELPM_VESSELDIAMETER );
	pclSSheet->SetStaticText( CD_TecBoxVssl_Sec_Height, RD_TecBoxVssl_ColName, IDS_SSHEETSSELPM_VESSELHEIGHT );
	pclSSheet->SetStaticText( CD_TecBoxVssl_Sec_Weight, RD_TecBoxVssl_ColName, IDS_SSHEETSSELPM_VESSELMASS );
	pclSSheet->SetStaticText( CD_TecBoxVssl_Sec_MaxWeight, RD_TecBoxVssl_ColName, IDS_SSHEETSSELPM_VESSELMAXWEIGHT );
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Column units.
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();

	if( true == TASApp.IsPMPriceIndexUsed() )
	{
		pclSSheet->SetStaticText( CD_TecBoxVssl_PriceIndexRatioValue, RD_TecBoxVssl_ColUnit, _T("%") );
	}
	
	pclSSheet->SetStaticText( CD_TecBoxVssl_Prim_Volume, RD_TecBoxVssl_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_VOLUME ) ).c_str() );
	pclSSheet->SetStaticText( CD_TecBoxVssl_Prim_MaxPressure, RD_TecBoxVssl_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_PRESSURE ) ).c_str() );
	pclSSheet->SetStaticText( CD_TecBoxVssl_Prim_TempRange, RD_TecBoxVssl_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_TEMPERATURE ) ).c_str() );
	pclSSheet->SetStaticText( CD_TecBoxVssl_Prim_Diameter, RD_TecBoxVssl_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_LENGTH ) ).c_str() );
	pclSSheet->SetStaticText( CD_TecBoxVssl_Prim_Height, RD_TecBoxVssl_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_LENGTH ) ).c_str() );
	pclSSheet->SetStaticText( CD_TecBoxVssl_Prim_Weight, RD_TecBoxVssl_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_MASS ) ).c_str() );
	pclSSheet->SetStaticText( CD_TecBoxVssl_Prim_MaxWeight, RD_TecBoxVssl_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_MASS ) ).c_str() );
	pclSSheet->SetStaticText( CD_TecBoxVssl_Sec_Diameter, RD_TecBoxVssl_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_LENGTH ) ).c_str() );
	pclSSheet->SetStaticText( CD_TecBoxVssl_Sec_Height, RD_TecBoxVssl_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_LENGTH ) ).c_str() );
	pclSSheet->SetStaticText( CD_TecBoxVssl_Sec_Weight, RD_TecBoxVssl_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_MASS ) ).c_str() );
	pclSSheet->SetStaticText( CD_TecBoxVssl_Sec_MaxWeight, RD_TecBoxVssl_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_MASS ) ).c_str() );
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Draw black line below column header.
	pclSSheet->SetCellBorder( CD_TecBoxVssl_FirstColumn + 1, RD_TecBoxVssl_ColUnit, CD_TecBox_Pointer - 1, RD_TecBoxVssl_ColUnit, true, SS_BORDERTYPE_BOTTOM );
}

long CRViewSSelPM::_FillTransferoVsslRow( CSheetDescription *pclSDTransferoVssl, long lRow, CSelectedVssl *pclSelTransferoVssl, 
		CPMInputUser *pclPMInputUser, CSelectedPMBase* pclSelectedBestPriceProduct, bool bForRejectedProduct )
{
	if( NULL == pclSDTransferoVssl || NULL == pclSDTransferoVssl->GetSSheetPointer() || NULL == pclSelTransferoVssl || NULL == pclPMInputUser )
	{
		return -1;
	}

	CDB_Vessel *pcldbVessel = dynamic_cast<CDB_Vessel *>( pclSelTransferoVssl->GetpData() );

	if( NULL == pcldbVessel )
	{
		return -1;
	}

	CSSheet *pclSSheet = pclSDTransferoVssl->GetSSheetPointer();

	if( true == pclSelTransferoVssl->IsFlagSet( CSelectedPMBase::eNotPriority ) )
	{
		m_lProductNotPriorityCount++;

		if( false == m_bShowAllPrioritiesShown )
		{
			// Add one empty not selectable row.
			pclSDTransferoVssl->AddRows( 1 );
			pclSDTransferoVssl->RemoveSelectableRow( lRow );

			pclSSheet->SetCellBorder( CD_TecBoxVssl_FirstColumn + 1, lRow, CD_TecBoxVssl_Pointer - 1, lRow, true, SS_BORDERTYPE_TOP | SS_BORDERTYPE_BOTTOM, 
					SS_BORDERSTYLE_SOLID, _BLACK );

			m_lShowAllPrioritiesButtonRow = lRow++;
			m_bShowAllPrioritiesShown = true;
		}
	}

	// Add this row.
	// 'true' to specify that this row can be selected.
	pclSDTransferoVssl->AddRows( 1, true );

	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

	// Add checkbox.
	pclSSheet->SetCheckBox(CD_TecBoxVssl_CheckBox, lRow, _T(""), false, true);

	// Primary vessel
	// Set the price index if used.
	if( true == TASApp.IsPMPriceIndexUsed() )
	{
		if( NULL != pclSelectedBestPriceProduct 
				&&  0 == _tcscmp( pclSelectedBestPriceProduct->GetProductIDPtr().ID, pclSelTransferoVssl->GetProductIDPtr().ID ) )
		{
			pclSSheet->SetPictureCellWithID( IDI_BESTPRICE, CD_TecBoxVssl_PriceIndexRatioIcon, lRow, CSSheet::PictureCellType::Icon );
		}

		CString str = _T("-");

		if( pclSelTransferoVssl->GetPriceIndex() > 0.0 )
		{
			double dPriceIndex = pclSelTransferoVssl->GetPriceIndex();
			dPriceIndex += pclSelTransferoVssl->GetSecVesselPriceIndex();
			str.Format( _T("%i"), (int)dPriceIndex );
		}

		pclSSheet->SetStaticText( CD_TecBoxVssl_PriceIndexRatioValue, lRow, str );
	}

	bool bBest = pclSelTransferoVssl->IsFlagSet( CSelectedPMBase::eBest );

	if( true == bBest )
	{
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_GREEN );
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, true );
	}

	// Set name.
	pclSSheet->SetStaticText( CD_TecBoxVssl_Prim_Name, lRow, pcldbVessel->GetName() );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, false );

	// Set volume nominal.
	double dVesselVolume = pcldbVessel->GetNominalVolume();
	pclSSheet->SetStaticText( CD_TecBoxVssl_Prim_Volume, lRow, WriteCUDouble( _U_VOLUME, dVesselVolume ) );

	// Set max pressure.
	pclSSheet->SetStaticText( CD_TecBoxVssl_Prim_MaxPressure, lRow, WriteCUDouble( _U_PRESSURE, pcldbVessel->GetPmaxmax() ) );

	// Set temperature range.
	if( true == pclSelTransferoVssl->IsFlagSet( CSelectedPMBase::eTemperature ) )
	{
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_ORANGE );
	}

	pclSSheet->SetStaticText( CD_TecBoxVssl_Prim_TempRange, lRow, pcldbVessel->GetTempRange() );

	// Restore normal fonts.
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

	// Set diameter.
	pclSSheet->SetStaticText( CD_TecBoxVssl_Prim_Diameter, lRow, WriteCUDouble( _U_LENGTH, pcldbVessel->GetDiameter() ) );

	// Set height.
	pclSSheet->SetStaticText( CD_TecBoxVssl_Prim_Height, lRow, WriteCUDouble( _U_LENGTH, pcldbVessel->GetHeight() ) );

	// Set weight.
	pclSSheet->SetStaticText( CD_TecBoxVssl_Prim_Weight, lRow, WriteCUDouble( _U_MASS, pcldbVessel->GetWeight() ) );

	// Set max. weight.
	CString strMaxWeight = _T("-");
	double dMaxWeight = pclPMInputUser->GetMaxWeight( pcldbVessel->GetWeight(), dVesselVolume, pclPMInputUser->GetMinTemperature() );

	if( -1.0 != dMaxWeight )
	{
		strMaxWeight = WriteCUDouble( _U_MASS, dMaxWeight );
	}

	pclSSheet->SetStaticText( CD_TecBoxVssl_Prim_MaxWeight, lRow, strMaxWeight );


	if( pclSelTransferoVssl->GetNbreOfVsslNeeded() > 1 )
	{
		// Set name.
		if( true == bBest )
		{
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_GREEN );
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, true );
		}

		CDB_Vessel *pcldbSecVessel = pcldbVessel->GetAssociatedSecondaryVessel();

		if( NULL != pcldbSecVessel )
		{
			pclSSheet->SetStaticText( CD_TecBoxVssl_Sec_Name, lRow, pcldbSecVessel->GetName() );
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, false );

			// Set quantity.
			if( true == pclSelTransferoVssl->IsFlagSet( CSelectedPMBase::eVslFlagsNbreOfVssl ) )
			{
				pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_ORANGE );
			}

			pclSSheet->SetStaticText( CD_TecBoxVssl_Sec_NbreOfVssl, lRow, WriteDouble( pclSelTransferoVssl->GetNbreOfVsslNeeded() - 1, 1, 0, 1 ) );

			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

			// Set diameter.
			pclSSheet->SetStaticText( CD_TecBoxVssl_Sec_Diameter, lRow, WriteCUDouble( _U_LENGTH, pcldbSecVessel->GetDiameter() ) );

			// Set height.
			pclSSheet->SetStaticText( CD_TecBoxVssl_Sec_Height, lRow, WriteCUDouble( _U_LENGTH, pcldbSecVessel->GetHeight() ) );

			// Set mass.
			pclSSheet->SetStaticText( CD_TecBoxVssl_Sec_Weight, lRow, WriteCUDouble( _U_MASS, pcldbSecVessel->GetWeight() ) );

			// Set max. weight.
			strMaxWeight = _T("-");
			dMaxWeight = pclPMInputUser->GetMaxWeight( pcldbVessel->GetWeight(), dVesselVolume, pclPMInputUser->GetMinTemperature() );

			if( -1.0 != dMaxWeight )
			{
				strMaxWeight = WriteCUDouble( _U_MASS, dMaxWeight );
			}

			pclSSheet->SetStaticText( CD_TecBoxVssl_Sec_MaxWeight, lRow, strMaxWeight );
		}
	}
	else
	{
		pclSSheet->SetStaticText( CD_TecBoxVssl_Sec_Name, lRow, _T("-") );
		pclSSheet->SetStaticText( CD_TecBoxVssl_Sec_NbreOfVssl, lRow, _T("-") );
		pclSSheet->SetStaticText( CD_TecBoxVssl_Sec_Diameter, lRow, _T("-") );
		pclSSheet->SetStaticText( CD_TecBoxVssl_Sec_Height, lRow, _T("-") );
		pclSSheet->SetStaticText( CD_TecBoxVssl_Sec_Weight, lRow, _T("-") );
		pclSSheet->SetStaticText( CD_TecBoxVssl_Sec_MaxWeight, lRow, _T("-") );
	}

	// Save parameter.
	CCellDescriptionProduct *pclCDProduct = CreateCellDescriptionProduct( pclSDTransferoVssl->GetFirstParameterColumn(), lRow, (LPARAM)pcldbVessel,
			pclSDTransferoVssl );

	if( NULL != pclCDProduct )
	{
		pclCDProduct->SetUserParam( (LPARAM)pclSelTransferoVssl );
	}

	pclSSheet->SetCellBorder( CD_TecBoxVssl_FirstColumn + 1, lRow, CD_TecBoxVssl_Pointer - 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _GRAY );

	return lRow;
}

void CRViewSSelPM::_FillAccessoryRows( UINT uiSheetDescriptionID, UINT uiSDGroupID, UINT uiSDParentID, int iIDSSheetTitle, CDB_Product *pSelectedProd, 
		CRank *prList, CDB_RuledTable *pclRuledTable )
{
	if( NULL == m_pclIndSelPMParams || NULL == m_pclIndSelPMParams->m_pTADB || NULL == m_pclIndSelPMParams->m_pclSelectPMList
			|| NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERT_RETURN;
	}

	vecCDCAccessoryList *pvecCDCAccessoryList = _GetvecCDAccessoryList( uiSheetDescriptionID );

	if( NULL == pvecCDCAccessoryList )
	{
		ASSERT_RETURN;
	}

	int iCount = 0;
	CRank rList;

	if( NULL == prList )
	{
		if( NULL == pSelectedProd )
		{
			return;
		}

		if( NULL == pclRuledTable )
		{
			pclRuledTable = (CDB_RuledTable *)( pSelectedProd->GetAccessoriesGroupIDPtr().MP );
		}

		if( NULL == pclRuledTable )
		{
			return;
		}

		iCount = m_pclIndSelPMParams->m_pTADB->GetAccessories( &rList, pclRuledTable, m_pclIndSelPMParams->m_eFilterSelection );
		prList = &rList;
	}
	else
	{
		iCount = prList->GetCount();
	}

	if( 0 == iCount )
	{
		return;
	}

	UINT uiAfterSheetDescriptionID = _GetAfterSheetDescriptionID( uiSheetDescriptionID, uiSDGroupID );

	if( -1 != _ConvertSD2SDG( uiSheetDescriptionID, uiSDGroupID ) )
	{
		uiSheetDescriptionID = _ConvertSD2SDG( uiSheetDescriptionID, uiSDGroupID );
	}

	if( -1 != _ConvertSD2SDG( uiAfterSheetDescriptionID, uiSDGroupID ) )
	{
		uiAfterSheetDescriptionID = _ConvertSD2SDG( uiAfterSheetDescriptionID, uiSDGroupID );
	}

	// Call 'MultiSpreadBase' method to get a new SSheet.
	CSheetDescription *pclSDAccessory = CreateSSheet( uiSheetDescriptionID, uiAfterSheetDescriptionID );

	if( NULL == pclSDAccessory || NULL == pclSDAccessory->GetSSheetPointer() )
	{
		return;
	}

	pclSDAccessory->SetGroupSheetDescriptionID( uiSDGroupID );

	UINT uiSDParentGroupID = _ConvertSD2SDG( uiSDParentID, uiSDGroupID );
	pclSDAccessory->SetParentSheetDescriptionID( uiSDParentGroupID );

	CSSheet *pclSSheet = pclSDAccessory->GetSSheetPointer();

	pclSSheet->SetBool( SSB_REDRAW, FALSE );

	pclSSheet->Init();
	pclSSheet->SetBool( SSB_ALLOWDRAGDROP, FALSE );
	pclSSheet->SetBool( SSB_HORZSCROLLBAR, FALSE );
	pclSSheet->SetBool( SSB_VERTSCROLLBAR, FALSE );
	pclSSheet->SetTextTip( SS_TT_STATUS_FLOATING, 250, NULL, _LIGHTYELLOW, _TAH_BLACK );

	// HYS-1783: Two lines by accessories, but two accessories by line (reason why we don't multiply iCount by 2).
	// If iCount is odd, we should also add 2 lines for name and description.
	if( 0 == iCount % 2 )
	{
		pclSSheet->SetMaxRows( RD_Accessory_FirstAvailRow + iCount );
	}
	else
	{
		pclSSheet->SetMaxRows( RD_Accessory_FirstAvailRow + iCount + 1 );
	}

	// All cells are static by default and filled with _T("").
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	pclSSheet->FormatStaticText( -1, -1, -1, -1, _T( "" ) );

	// Get sheet description of parent description sheet to retrieve width.
	CSheetDescription *pclSDParent = m_ViewDescription.GetFromSheetDescriptionID( uiSDParentGroupID );
	// It's absolutely not normal to have this pointer NULL.
	ASSERT( NULL != pclSDParent );

	// Take SSheet size and client area size and determine the right width to take in count.
	CRect rectSize = pclSDParent->GetSSheetPointer()->GetSheetSizeInPixels();

	// Prepare first column width (to match parent sheet).
	long lFirstColumnWidth = pclSDParent->GetSSheetPointer()->GetColWidthInPixelsW( 1 );

	// Do some correction in regards to some columns.
	long lTotalWidth = rectSize.Width() - lFirstColumnWidth;
	lTotalWidth -= pclSDParent->GetSSheetPointer()->GetColWidthInPixelsW( pclSDParent->GetSSheetPointer()->GetMaxCols() );

	// Try to create 2 columns in just the middle of the parent sheet.
	long lLeftWidth = ( long )( lTotalWidth / 2 );
	long lRightWidth = ( ( lTotalWidth % 2 ) > 0 ) ? lLeftWidth + 1 : lLeftWidth;

	// Set columns.
	// 'CSheetDescription::SD_ParameterMode_Multi' to specify that this sheet can have more than one column with parameter.
	pclSDAccessory->Init( 1, pclSSheet->GetMaxRows(), 2, CSheetDescription::SD_ParameterMode_Multi );

	pclSDAccessory->AddColumnInPixels( CD_Accessory_FirstColumn, lFirstColumnWidth );
	pclSDAccessory->AddColumnInPixels( CD_Accessory_Left, lLeftWidth );
	pclSDAccessory->AddColumnInPixels( CD_Accessory_Right, lRightWidth );
	pclSDAccessory->AddColumn( CD_Accessory_LastColumn, 0 );

	// These two columns can contain parameter.
	pclSDAccessory->AddParameterColumn( CD_Accessory_Left );
	pclSDAccessory->AddParameterColumn( CD_Accessory_Right );

	// Set the focus column (don't set on Left of Right (on a check box) )
	pclSDAccessory->SetActiveColumn( CD_Accessory_FirstColumn );

	// Set selectable rows.
	pclSDAccessory->SetSelectableRangeRow( RD_Accessory_FirstAvailRow, pclSSheet->GetMaxRows() - 1 );

	// Increase row height.
	double RowHeight = 12.75;
	pclSSheet->SetRowHeight( RD_Accessory_FirstRow, RowHeight * 0.5 );
	pclSSheet->SetRowHeight( RD_Accessory_GroupName, RowHeight * 1.5 );

	// Set title.
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, _IMI_TITLE_GROUP3 );

	pclSDAccessory->SetUserVariable( _SDUV_TITLEFORECOLOR, ( pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor ) ) );
	pclSDAccessory->SetUserVariable( _SDUV_TITLEBACKCOLOR, ( pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor ) ) );

	pclSSheet->AddCellSpanW( CD_Accessory_FirstColumn + 1, RD_Accessory_GroupName, CD_Accessory_LastColumn - ( CD_Accessory_FirstColumn + 1 ), 1 );
	pclSSheet->SetStaticText( CD_Accessory_FirstColumn + 1, RD_Accessory_GroupName, iIDSSheetTitle );

	// Fill accessories.
	long lRow = RD_Accessory_FirstAvailRow;

	CString str;
	LPARAM lparam;

	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_LEFT );

	// Left - Right.
	m_lProductTotalCount = 0;
	long lLeftOrRight = CD_Accessory_Left;
	BOOL bContinue = prList->GetFirst( str, lparam );

	while( TRUE == bContinue )
	{
		CDB_Product *pclAccessory = (CDB_Product *)lparam;
		VERIFY( NULL != pclAccessory );

		if( false == pclAccessory->IsAnAccessory() )
		{
			prList->GetNext( str, lparam );
			continue;
		}

		// HYS-1427: PAZ is an accessory with a dpu range. But we don't have this information in a 'CDB_Product'. For this case,
		// we use the Pmaxmax value to check if this PAZ can work with PSV.
		// HYS-1819: Lock shield valve accessory is not allowed when pSV is higher than PS limit
		if( ( 0 == CString( pclAccessory->GetFamilyID() ).Compare( _T("ACCFAM_PMPAZ") ) ) 
				|| ( 0 == CString( pclAccessory->GetFamilyID() ).Compare( _T("ACCFAM_LSV") ) ) )
		{
			if( pclAccessory->GetPmaxmax() < m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetSafetyValveResponsePressure() )
			{
				// HYS-1783: In the case of this bug, we did a "continue" without setting bContinue (the only condition for this loop). So,
				// if we are here and prList has no element we did a infinite loop. To fix that, we use bContinue to get the return of prList->GetNext()
				bContinue = prList->GetNext( str, lparam );

				// HYS-1783: We did the lRow +=2 to add this new accessory. If it is not available we have to go back at the previous one.
				// HYS-1821: We did the lRow +=2 to add this new accessory. If it is not available we have to go back at the previous one
				// ONLY if the current is the last one.
				if( FALSE == bContinue && lLeftOrRight == CD_Accessory_Left && lRow != RD_Accessory_FirstAvailRow )
				{
					lRow -= 2;
				}
				if( FALSE == bContinue )
				{
					// HYS-1783: If no more accessory is available, add the separator line.
					lRow++;
				}

				continue;
			}
		}
		
		// Create checkbox accessory.
		CString strName = _T("");

		if( true == pclRuledTable->IsByPair( pclAccessory->GetIDPtr().ID ) )
		{
			strName += _T("2x ");
		}

		strName += pclAccessory->GetName();

		CCDBCheckboxAccessory *pCheckbox = CreateCheckboxAccessory( lLeftOrRight, lRow, false, true, strName, pclAccessory, 
				pclRuledTable, pvecCDCAccessoryList, pclSDAccessory );

		if( NULL != pCheckbox )
		{
			pCheckbox->ApplyInternalChange();
		}

		// Description.
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleEllipses, (LPARAM)TRUE );
		pclSSheet->SetStaticText( lLeftOrRight, lRow + 1, pclAccessory->GetComment() );
		bContinue = prList->GetNext( str, lparam );

		// Restart left part.
		if( TRUE == bContinue && CD_Accessory_Right == lLeftOrRight )
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
		
		m_lProductTotalCount++;

		if( FALSE == bContinue )
		{
			lRow++;
		}
	}

	// HYS-1783: m_lProductTotalCount could be different from iCount. Set max row in this case.
	if( ( 0 == m_lProductTotalCount ) || ( lRow < pclSSheet->GetMaxRows() - 1 ) )
	{
		// HYS-1821: m_lProductTotalCount in place of iCount.
		if( 0 == m_lProductTotalCount % 2 )
		{
			pclSSheet->SetMaxRows( RD_Accessory_FirstAvailRow + m_lProductTotalCount );
		}
		else
		{
			pclSSheet->SetMaxRows( RD_Accessory_FirstAvailRow + m_lProductTotalCount + 1 );
		}
	}

	pclSDAccessory->SetUserVariable( _SDUV_TOTALPRODUCT_COUNT, m_lProductTotalCount );
	VerifyCheckboxAccessories( NULL, false, _GetvecCDAccessoryList( uiSDParentID ) );

	// Create the button to allow to expand/collapse all the group.
	m_mapButtonExpandCollapseGroup[uiSheetDescriptionID] = CreateExpandCollapseGroupButton( CD_Accessory_FirstColumn, RD_Accessory_GroupName, 
			true, CCDButtonExpandCollapseGroup::ButtonState::CollapseRow, RD_Accessory_FirstAvailRow, pclSSheet->GetMaxRows(), pclSDAccessory );

	if( NULL != m_mapButtonExpandCollapseGroup[uiSheetDescriptionID] )
	{
		m_mapButtonExpandCollapseGroup[uiSheetDescriptionID]->SetShowStatus( true );
	}

	pclSSheet->SetCellBorder( CD_Accessory_Left, lRow, CD_Accessory_LastColumn - 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );
	pclSSheet->SetBool( SSB_REDRAW, TRUE );
}

long CRViewSSelPM::_FillIntermediateVsslSheet( UINT uiSheetDescriptionID, UINT uiSDGroupID, UINT uiSDParentID, CSelectedVssl *pclSelectedVessel )
{
	if( NULL == m_pclIndSelPMParams || NULL == m_pclIndSelPMParams->m_pclSelectPMList || NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser() 
			|| NULL == pclSelectedVessel )
	{
		ASSERTA_RETURN( -1 );
	}

	CSelectPMList *pclSelectList = m_pclIndSelPMParams->m_pclSelectPMList;
	CPMInputUser *pclPMInputUser = pclSelectList->GetpclPMInputUser();

	// Test if intermediate vessels are needed.
	CDB_Vessel *pVssl = dynamic_cast<CDB_Vessel *>( pclSelectedVessel->GetpData() );

	if( NULL == pVssl || false == pclSelectList->IsIntermVesselNeeded( pVssl ) )
	{
		return -1;
	}
	
	/* HYS-843: we have to show the group header when no intermediate is selected */
	pclSelectList->GetIntermVsslList( pclSelectedVessel );
	
	UINT uiAfterSheetDescriptionID = _GetAfterSheetDescriptionID( uiSheetDescriptionID, uiSDGroupID );

	// Call 'MultiSpreadBase' method to get a new sheet.
	CSheetDescription *pclSDIntermVssl = CreateSSheet( uiSheetDescriptionID, uiAfterSheetDescriptionID );

	if( NULL == pclSDIntermVssl || NULL == pclSDIntermVssl->GetSSheetPointer() )
	{
		return -1;
	}

	pclSDIntermVssl->SetGroupSheetDescriptionID( uiSDGroupID );

	uiSDParentID = _ConvertSD2SDG( uiSDParentID, uiSDGroupID );
	pclSDIntermVssl->SetParentSheetDescriptionID( uiSDParentID );

	CSSheet *pclSSheet = pclSDIntermVssl->GetSSheetPointer();

	pclSSheet->SetBool( SSB_REDRAW, FALSE );

	_InitIntermediateVsslHeaders( pclSDIntermVssl );

	m_lSelectedRow = -1;
	m_lProductTotalCount = 0;
	m_lProductNotPriorityCount = 0;
	m_bShowAllPrioritiesShown = false;
	m_lShowAllPrioritiesButtonRow = 0;
	long lRow = RD_IntermVssl_FirstAvailRow;
	pclSDIntermVssl->RestartRemarkGenerator();

	// Priority/NotaPriority loop.
	for( int iLoop = 1; iLoop >= 0; --iLoop )
	{
		for( CSelectedPMBase *pclSelectedProduct = pclSelectList->GetFirstProduct( CSelectPMList::PT_IntermediateVessel );
				NULL != pclSelectedProduct; pclSelectedProduct = pclSelectList->GetNextProduct( CSelectPMList::PT_IntermediateVessel ) )
		{
			// Skip not Priority when iLoop == 1
			if( ( iLoop == 1 ) == pclSelectedProduct->IsFlagSet( CSelectedPMBase::eNotPriority ) )
			{
				continue;
			}

			CSelectedVssl *pclSelectedIntermVssl = dynamic_cast<CSelectedVssl *>( pclSelectedProduct );

			if( NULL == pclSelectedIntermVssl )
			{
				continue;
			}

			long lReturn = _FillIntermediateVsslRow( pclSDIntermVssl, lRow, pclSelectedIntermVssl, m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser() );

			if( -1 == lReturn )
			{
				continue;
			}
			else
			{
				lRow = lReturn;
			}

			lRow++;
			m_lProductTotalCount++;
		}
	}

	long lLastDataRow = lRow - 1;
	pclSDIntermVssl->SetUserVariable( _SDUV_TOTALPRODUCT_COUNT, m_lProductTotalCount );
	pclSDIntermVssl->SetUserVariable( _SDUV_NOPRIORITYPRODUCT_COUNT, m_lProductNotPriorityCount );

	// Now we can create Show/Hide all priorities if needed.
	// Remarks: 2nd condition to avoid to have button if we have only no priority valves.
	if( true == m_bShowAllPrioritiesShown && m_lProductTotalCount > m_lProductNotPriorityCount )
	{
		// Create Show/Hide all priorities button in regards to current state!
		// HYS-1539 : The button state is the same for all sheet
		CCDButtonShowAllPriorities::ButtonState eState = ( true == GetShowAllPrioritiesFlag( (int)uiSheetDescriptionID ) ) ? 
				CCDButtonShowAllPriorities::ButtonState::HidePriorities : CCDButtonShowAllPriorities::ButtonState::ShowPriorities;

		CCDButtonShowAllPriorities *pclShowAllButton = CreateShowAllPrioritiesButton( CD_IntermVssl_FirstColumn, m_lShowAllPrioritiesButtonRow,
				m_bShowAllPrioritiesShown, eState, m_lShowAllPrioritiesButtonRow + 1, pclSSheet->GetMaxRows(), pclSDIntermVssl );

		// Here we must call 'ApplyInternalChange' to show or hide rows in regards to the current state!
		if( NULL != pclShowAllButton )
		{
			pclShowAllButton->ApplyInternalChange();
		}
	}
	else
	{
		if( m_lShowAllPrioritiesButtonRow != 0 )
		{
			pclSSheet->ShowRow( m_lShowAllPrioritiesButtonRow, false );
		}
	}

	pclSSheet->SetCellBorder( CD_IntermVssl_CheckBox, lLastDataRow, CD_IntermVssl_Pointer - 1, lLastDataRow, true, SS_BORDERTYPE_BOTTOM, 
			SS_BORDERSTYLE_SOLID, _BLACK );

	// Add link to the excluded products if exist.
	lRow = _AddExcludedProductsLink( pclSDIntermVssl, lRow, CSelectPMList::PT_IntermediateVessel );

	// Add Remarks.
	// Note: these lines must be placed after creating Show/Hide all priorities button to avoid to take in count these lines when defining
	//       range of row to be Show/Hide.
	lRow = pclSDIntermVssl->WriteRemarks( lRow, CD_IntermVssl_Type, CD_IntermVssl_Pointer );

	// Add possibility to change column size.
	// Remark: place this code after any initialization (like adding, removing, resizing, hiding/showing columns).
	CResizingColumnInfo *pclResizingColumnInfo = pclSDIntermVssl->GetResizingColumnInfo();
	pclResizingColumnInfo->ActivateFeature( true, false, RD_IntermVssl_FirstAvailRow, lLastDataRow );
	pclResizingColumnInfo->AddRangeColumn( CD_IntermVssl_Type, CD_IntermVssl_Pointer - 1, RD_IntermVssl_ColName, RD_IntermVssl_ColUnit );

	// Add possibility to click on title group to reset column width.
	CreateTitleGroupButtons( CD_IntermVssl_Type, CD_IntermVssl_Pointer - 1, RD_IntermVssl_GroupName, pclSDIntermVssl );

	pclSSheet->SetBool( SSB_REDRAW, TRUE );
	return m_lSelectedRow;
}

void CRViewSSelPM::_InitIntermediateVsslHeaders( CSheetDescription *pclSDIntermVssl )
{
	if( NULL == pclSDIntermVssl || NULL == pclSDIntermVssl->GetSSheetPointer() )
	{
		return;
	}

	CSSheet *pclSSheet = pclSDIntermVssl->GetSSheetPointer();

	pclSSheet->Init();
	pclSSheet->SetBool( SSB_ALLOWDRAGDROP, FALSE );
	pclSSheet->SetBool( SSB_HORZSCROLLBAR, FALSE );
	pclSSheet->SetBool( SSB_VERTSCROLLBAR, FALSE );
	pclSSheet->SetBool( SSB_MOVEACTIVEONFOCUS, FALSE );
	pclSSheet->SetTextTip( SS_TT_STATUS_FLOATING, 250, NULL, _LIGHTYELLOW, _TAH_BLACK );

	// Set max rows.
	pclSSheet->SetMaxRows( RD_IntermVssl_FirstAvailRow - 1 );

	// All cells are static by default and filled with _T("").
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	pclSSheet->FormatStaticText( -1, -1, -1, -1, _T( "" ) );

	// Increase row height.
	double dRowHeight = 12.75;
	pclSSheet->SetRowHeight( RD_IntermVssl_FirstRow, dRowHeight * 0.5 );
	pclSSheet->SetRowHeight( RD_IntermVssl_GroupName, dRowHeight * 1.5 );
	pclSSheet->SetRowHeight( RD_IntermVssl_ColName, dRowHeight * 2.5 );
	pclSSheet->SetRowHeight( RD_IntermVssl_ColUnit, dRowHeight * 1.2 );

	// Initialize.
	pclSDIntermVssl->Init( 1, pclSSheet->GetMaxRows(), 1, CSheetDescription::SD_ParameterMode_Mono );

	pclSDIntermVssl->AddColumnInPixels( CD_IntermVssl_FirstColumn, m_mapSSheetColumnWidth[SD_IntermediateVessel][CD_IntermVssl_FirstColumn] );
	pclSDIntermVssl->AddColumnInPixels( CD_IntermVssl_CheckBox, m_mapSSheetColumnWidth[SD_IntermediateVessel][CD_IntermVssl_CheckBox] );
	pclSDIntermVssl->AddColumnInPixels( CD_IntermVssl_Type, m_mapSSheetColumnWidth[SD_IntermediateVessel][CD_IntermVssl_Type] );
	pclSDIntermVssl->AddColumnInPixels( CD_IntermVssl_NbreOfVssl, m_mapSSheetColumnWidth[SD_IntermediateVessel][CD_IntermVssl_NbreOfVssl] );
	pclSDIntermVssl->AddColumnInPixels( CD_IntermVssl_Volume, m_mapSSheetColumnWidth[SD_IntermediateVessel][CD_IntermVssl_Volume] );
	pclSDIntermVssl->AddColumnInPixels( CD_IntermVssl_MaxPressure, m_mapSSheetColumnWidth[SD_IntermediateVessel][CD_IntermVssl_MaxPressure] );
	pclSDIntermVssl->AddColumnInPixels( CD_IntermVssl_TempRange, m_mapSSheetColumnWidth[SD_IntermediateVessel][CD_IntermVssl_TempRange] );
	pclSDIntermVssl->AddColumnInPixels( CD_IntermVssl_Diameter, m_mapSSheetColumnWidth[SD_IntermediateVessel][CD_IntermVssl_Diameter] );
	pclSDIntermVssl->AddColumnInPixels( CD_IntermVssl_Height, m_mapSSheetColumnWidth[SD_IntermediateVessel][CD_IntermVssl_Height] );
	pclSDIntermVssl->AddColumnInPixels( CD_IntermVssl_Weight, m_mapSSheetColumnWidth[SD_IntermediateVessel][CD_IntermVssl_Weight] );
	pclSDIntermVssl->AddColumnInPixels( CD_IntermVssl_MaxWeight, m_mapSSheetColumnWidth[SD_IntermediateVessel][CD_IntermVssl_MaxWeight] );
	pclSDIntermVssl->AddColumnInPixels( CD_IntermVssl_Connection, m_mapSSheetColumnWidth[SD_IntermediateVessel][CD_IntermVssl_Connection] );
	pclSDIntermVssl->AddColumnInPixels( CD_IntermVssl_Pointer, m_mapSSheetColumnWidth[SD_IntermediateVessel][CD_IntermVssl_Pointer] );

	// Set in which column parameter must be saved.
	pclSDIntermVssl->AddParameterColumn( CD_IntermVssl_Pointer );

	// Set the focus column.
	pclSDIntermVssl->SetActiveColumn( CD_IntermVssl_CheckBox );

	// Set range for selection.
	pclSDIntermVssl->SetFocusColumnRange( CD_IntermVssl_CheckBox, CD_IntermVssl_Pointer - 1 );

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Add the main title.
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor,
									   _IMI_TITLE_GROUP3 );//(LPARAM)GetTitleBackgroundColor( pclSDIntermVssl ) );
	pclSDIntermVssl->SetUserVariable( _SDUV_TITLEFORECOLOR, ( pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor ) ) );
	pclSDIntermVssl->SetUserVariable( _SDUV_TITLEBACKCOLOR, ( pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor ) ) );

	pclSSheet->AddCellSpanW( CD_IntermVssl_FirstColumn + 1, RD_IntermVssl_GroupName, CD_IntermVssl_Pointer - ( CD_IntermVssl_FirstColumn + 1 ), 1 );
	pclSSheet->SetStaticText( CD_IntermVssl_FirstColumn + 1, RD_IntermVssl_GroupName, IDS_SSHEETSSELPM_STINTERMVSSLGROUP );
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Column name.
	if( false == pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn ) )
	{
		return;
	}

	if( false == pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE ) )
	{
		return;
	}

	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap, (LPARAM)TRUE );
	pclSSheet->SetStaticText( CD_IntermVssl_Type, RD_IntermVssl_ColName, IDS_SSHEETSSELPM_VESSELNAME );
	pclSSheet->SetStaticText( CD_IntermVssl_NbreOfVssl, RD_IntermVssl_ColName, IDS_SSHEETSSELPM_VESSELQTY );
	pclSSheet->SetStaticText( CD_IntermVssl_Volume, RD_IntermVssl_ColName, IDS_SSHEETSSELPM_VESSELVOL );
	pclSSheet->SetStaticText( CD_IntermVssl_MaxPressure, RD_IntermVssl_ColName, IDS_SSHEETSSELPM_VESSELMAXPRESS );
	pclSSheet->SetStaticText( CD_IntermVssl_TempRange, RD_IntermVssl_ColName, IDS_SSHEETSSELPM_VESSELTEMPRANGE );
	pclSSheet->SetStaticText( CD_IntermVssl_Diameter, RD_IntermVssl_ColName, IDS_SSHEETSSELPM_VESSELDIAMETER );
	pclSSheet->SetStaticText( CD_IntermVssl_Height, RD_IntermVssl_ColName, IDS_SSHEETSSELPM_VESSELHEIGHT );
	pclSSheet->SetStaticText( CD_IntermVssl_Weight, RD_IntermVssl_ColName, IDS_SSHEETSSELPM_VESSELMASS );
	pclSSheet->SetStaticText( CD_IntermVssl_MaxWeight, RD_IntermVssl_ColName, IDS_SSHEETSSELPM_VESSELMAXWEIGHT );
	pclSSheet->SetStaticText( CD_IntermVssl_Connection, RD_IntermVssl_ColName, IDS_SSHEETSSELPM_VESSELCONNECT );
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Column units.
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap, (LPARAM)FALSE );
	pclSSheet->SetStaticText( CD_IntermVssl_Volume, RD_IntermVssl_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_VOLUME ) ).c_str() );
	pclSSheet->SetStaticText( CD_IntermVssl_MaxPressure, RD_IntermVssl_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_PRESSURE ) ).c_str() );
	pclSSheet->SetStaticText( CD_IntermVssl_TempRange, RD_IntermVssl_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_TEMPERATURE ) ).c_str() );
	pclSSheet->SetStaticText( CD_IntermVssl_Diameter, RD_IntermVssl_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_LENGTH ) ).c_str() );
	pclSSheet->SetStaticText( CD_IntermVssl_Height, RD_IntermVssl_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_LENGTH ) ).c_str() );
	pclSSheet->SetStaticText( CD_IntermVssl_Weight, RD_IntermVssl_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_MASS ) ).c_str() );
	pclSSheet->SetStaticText( CD_IntermVssl_MaxWeight, RD_IntermVssl_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_MASS ) ).c_str() );
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Draw black line below column header.
	pclSSheet->SetCellBorder( CD_IntermVssl_CheckBox, RD_IntermVssl_ColUnit, CD_IntermVssl_Pointer - 1, RD_IntermVssl_ColUnit, true, SS_BORDERTYPE_BOTTOM );
}

long CRViewSSelPM::_FillIntermediateVsslRow( CSheetDescription *pclSDIntermVssl, long lRow, CSelectedVssl *pclSelectedIntermVssl, CPMInputUser *pclPMInputUser,
		bool bForRejectedProduct )
{
	if( NULL == pclSDIntermVssl || NULL == pclSDIntermVssl->GetSSheetPointer() || NULL == pclSelectedIntermVssl || NULL == pclPMInputUser )
	{
		ASSERTA_RETURN( -1 );
	}

	CDB_Vessel *pclInterVssl = dynamic_cast<CDB_Vessel *>( pclSelectedIntermVssl->GetpData() );

	if( NULL == pclInterVssl )
	{
		ASSERTA_RETURN( -1 );
	}

	CSSheet *pclSSheet = pclSDIntermVssl->GetSSheetPointer();

	if( true == pclSelectedIntermVssl->IsFlagSet( CSelectedPMBase::eNotPriority ) )
	{
		m_lProductNotPriorityCount++;

		if( false == m_bShowAllPrioritiesShown )
		{
			// Add one empty not selectable row.
			pclSDIntermVssl->AddRows( 1 );
			pclSDIntermVssl->RemoveSelectableRow( lRow );

			pclSSheet->SetCellBorder( CD_IntermVssl_CheckBox, lRow, CD_IntermVssl_Pointer - 1, lRow, true, SS_BORDERTYPE_TOP | SS_BORDERTYPE_BOTTOM, 
					SS_BORDERSTYLE_SOLID, _BLACK );

			m_lShowAllPrioritiesButtonRow = lRow++;
			m_bShowAllPrioritiesShown = true;
		}
	}

	// Add this row.
	// 'true' to specify that this row can be selected.
	pclSDIntermVssl->AddRows( 1, true );

	// Add checkbox.
	pclSSheet->SetCheckBox(CD_IntermVssl_CheckBox, lRow, _T(""), false, true);

	bool bBest = pclSelectedIntermVssl->IsFlagSet( CSelectedPMBase::eBest );

	if( true == bBest )
	{
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_GREEN );
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, true );
	}

	// Set name.
	pclSSheet->SetStaticText( CD_IntermVssl_Type, lRow, pclInterVssl->GetName() );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, false );

	// Set quantity.
	if( true == pclSelectedIntermVssl->IsFlagSet( CSelectedPMBase::eVslFlagsNbreOfVssl ) )
	{
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_ORANGE );
	}

	pclSSheet->SetStaticText( CD_IntermVssl_NbreOfVssl, lRow, WriteDouble( pclSelectedIntermVssl->GetNbreOfVsslNeeded(), 1, 0, 1 ) );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

	// Set volume nominal.
	double dVesselVolume = pclInterVssl->GetNominalVolume();
	pclSSheet->SetStaticText( CD_IntermVssl_Volume, lRow, WriteCUDouble( _U_VOLUME, dVesselVolume ) );

	// Set max pressure.
	pclSSheet->SetStaticText( CD_IntermVssl_MaxPressure, lRow, WriteCUDouble( _U_PRESSURE, pclInterVssl->GetPmaxmax() ) );

	// Set temperature range
	if( true == pclSelectedIntermVssl->IsFlagSet( CSelectedPMBase::eTemperature ) )
	{
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_ORANGE );
	}

	pclSSheet->SetStaticText( CD_IntermVssl_TempRange, lRow, pclInterVssl->GetTempRange() );

	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

	// Set diameter.
	pclSSheet->SetStaticText( CD_IntermVssl_Diameter, lRow, WriteCUDouble( _U_LENGTH, pclInterVssl->GetDiameter() ) );

	// Set height.
	pclSSheet->SetStaticText( CD_IntermVssl_Height, lRow, WriteCUDouble( _U_LENGTH, pclInterVssl->GetHeight() ) );

	// Set weight.
	pclSSheet->SetStaticText( CD_IntermVssl_Weight, lRow, WriteCUDouble( _U_MASS, pclInterVssl->GetWeight() ) );

	// Set max. weight.
	CString strMaxWeight = _T("-");
	double dMaxWeight = pclPMInputUser->GetMaxWeight( pclInterVssl->GetWeight(), dVesselVolume, pclPMInputUser->GetMinTemperature() );

	if( -1.0 != dMaxWeight )
	{
		strMaxWeight = WriteCUDouble( _U_MASS, dMaxWeight );
	}

	pclSSheet->SetStaticText( CD_IntermVssl_MaxWeight, lRow, strMaxWeight );

	// Set connection.
	pclSSheet->SetStaticText( CD_IntermVssl_Connection, lRow, pclInterVssl->GetConnect() );

	// Save parameter.
	CCellDescriptionProduct *pclCDProduct = CreateCellDescriptionProduct( pclSDIntermVssl->GetFirstParameterColumn(), lRow, (LPARAM)pclInterVssl,
			pclSDIntermVssl );

	if( NULL != pclCDProduct )
	{
		pclCDProduct->SetUserParam( (LPARAM)pclSelectedIntermVssl );
	}

	pclSSheet->SetCellBorder( CD_IntermVssl_CheckBox, lRow, CD_IntermVssl_Pointer - 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _GRAY );

	return lRow;
}

long CRViewSSelPM::_FillTecBoxVentoSheet( UINT uiSDGroupID, CSelectedPMBase *pclSelectedPM, CDS_SSelPMaint *pclEditedSSelPM, bool bForceRedForTitleBkg )
{
	if( NULL == m_pclIndSelPMParams || NULL == m_pclIndSelPMParams->m_pclSelectPMList || NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser() 
			|| NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetPMWQSelectionPreferences() )
	{
		ASSERTA_RETURN( -1 );
	}

	UINT uiSDGID = _ConvertSD2SDG( SD_TecBoxVento, uiSDGroupID );

	if( -1 == uiSDGID )
	{
		return -1;
	}

	// If there is no product (selected or rejected).
	if( false == m_pclIndSelPMParams->m_pclSelectPMList->IsProductExistInBothList( CSelectPMList::PT_Vento ) )
	{
		return -1;
	}

	// Get 'Parent' sheet identification.
	UINT uiAfterSheetDescriptionID = -1;

	if( -1 != uiSDGroupID )
	{
		uiAfterSheetDescriptionID = _GetAfterSheetDescriptionID( SD_TecBoxVento, uiSDGroupID );
	}

	// Call 'MultiSpreadBase' method to get a new sheet.
	CSheetDescription *pclSDTecBoxVento = CreateSSheet( uiSDGID, uiAfterSheetDescriptionID );

	if( NULL == pclSDTecBoxVento || NULL == pclSDTecBoxVento->GetSSheetPointer() )
	{
		return -1;
	}

	CPMInputUser *pclPMInputUser = m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser();

	CSSheet *pclSSheet = pclSDTecBoxVento->GetSSheetPointer();
	pclSDTecBoxVento->SetGroupSheetDescriptionID( uiSDGroupID );
	pclSDTecBoxVento->SetParentSheetDescriptionID( SD_TecBoxVento );

	pclSSheet->SetBool( SSB_REDRAW, FALSE );

	_InitTecBoxVentoHeaders( pclSDTecBoxVento, pclSelectedPM, bForceRedForTitleBkg );

	m_lSelectedRow = -1;
	m_lProductTotalCount = 0;
	m_lProductNotPriorityCount = 0;
	m_bShowAllPrioritiesShown = false;
	m_lShowAllPrioritiesButtonRow = 0;

	long lRow = RD_TecBox_FirstAvailRow;
	pclSDTecBoxVento->RestartRemarkGenerator();

	CSelectedPMBase *pclSelectedBestPriceProduct = NULL;

	if( true == TASApp.IsPMPriceIndexUsed() )
	{
		pclSelectedBestPriceProduct = m_pclIndSelPMParams->m_pclSelectPMList->GetLowestPriceObject( CSelectPMList::ProductType::PT_Vento );
	}

	// Priority/NotaPriority loop.
	// Remark: Now we show all when user selects a specific pressure maintenance system.
	int iEndLoop = ( true == m_bIsPMTypeAll ) ? 2 : 1;

	// Priority/NotaPriority loop.
	for( int iLoop = 0; iLoop < iEndLoop; iLoop++ )
	{
		for( CSelectedPMBase *pclSelectedProduct = m_pclIndSelPMParams->m_pclSelectPMList->GetFirstProduct( CSelectPMList::PT_Vento );
				NULL != pclSelectedProduct; pclSelectedProduct = m_pclIndSelPMParams->m_pclSelectPMList->GetNextProduct( CSelectPMList::PT_Vento ) )
		{
			// Skip not Priority when iLoop == 0
			if( true == m_bIsPMTypeAll && ( iLoop == 0 ) == pclSelectedProduct->IsFlagSet( CSelectedPMBase::eNotPriority ) )
			{
				continue;
			}

			CSelectedVento *pclSelectedTBVento = dynamic_cast<CSelectedVento *>( pclSelectedProduct );

			if( NULL == pclSelectedTBVento )
			{
				continue;
			}

			long lReturn = _FillTecBoxVentoRow( pclSDTecBoxVento, lRow, pclSelectedTBVento, m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser(),
					pclEditedSSelPM, pclSelectedBestPriceProduct );

			if( -1 == lReturn )
			{
				continue;
			}
			else
			{
				lRow = lReturn;
			}

			lRow++;
			m_lProductTotalCount++;
		}
	}

	long lLastDataRow = lRow - 1;
	pclSDTecBoxVento->SetUserVariable( _SDUV_TOTALPRODUCT_COUNT, m_lProductTotalCount );
	pclSDTecBoxVento->SetUserVariable( _SDUV_NOPRIORITYPRODUCT_COUNT, m_lProductNotPriorityCount );

	// Now we can create Show/Hide all priorities if needed.
	if( true == m_bIsPMTypeAll )
	{
		// Remarks: 2nd condition to avoid to have button if we have only no priority valves.
		if( true == m_bShowAllPrioritiesShown && m_lProductTotalCount > m_lProductNotPriorityCount )
		{
			// Create Show/Hide all priorities button in regards to current state!
			// HYS-1539 : The button state is the same for all sheet
			CCDButtonShowAllPriorities::ButtonState eState = ( true == GetShowAllPrioritiesFlag( (int)uiSDGID ) ) ? 
					CCDButtonShowAllPriorities::ButtonState::HidePriorities : CCDButtonShowAllPriorities::ButtonState::ShowPriorities;

			CCDButtonShowAllPriorities *pclShowAllButton = CreateShowAllPrioritiesButton( CD_TecBoxVento_FirstColumn, m_lShowAllPrioritiesButtonRow,
					m_bShowAllPrioritiesShown, eState, m_lShowAllPrioritiesButtonRow + 1, pclSSheet->GetMaxRows(), pclSDTecBoxVento );

			// Here we must call 'ApplyInternalChange' to show or hide rows in regards to the current state!
			if( NULL != pclShowAllButton )
			{
				pclShowAllButton->ApplyInternalChange();
			}
		}
		else
		{
			if( m_lShowAllPrioritiesButtonRow != 0 )
			{
				pclSSheet->ShowRow( m_lShowAllPrioritiesButtonRow, false );
			}
		}
	}

	pclSSheet->SetCellBorder( CD_TecBoxVento_FirstColumn + 1, lLastDataRow, CD_TecBoxVento_Pointer - 1, lLastDataRow, true, SS_BORDERTYPE_BOTTOM, 
			SS_BORDERSTYLE_SOLID, _BLACK );

	// Add link to the excluded products if exist.
	lRow = _AddExcludedProductsLink( pclSDTecBoxVento, lRow, CSelectPMList::PT_Vento );

	// Add Remarks.
	// Note: these lines must be placed after creating Show/Hide all priorities button to avoid to take in count these lines when defining
	//       range of row to be Show/Hide.
	lRow = pclSDTecBoxVento->WriteRemarks( lRow, CD_TecBoxVento_FirstColumn + 1, CD_TecBoxVento_Pointer );

	// Add possibility to change column size.
	// Remark: place this code after any initialization (like adding, removing, resizing, hiding/showing columns).
	CResizingColumnInfo *pclResizingColumnInfo = pclSDTecBoxVento->GetResizingColumnInfo();
	pclResizingColumnInfo->ActivateFeature( true, false, RD_TecBox_FirstAvailRow, lLastDataRow );
	pclResizingColumnInfo->AddRangeColumn( CD_TecBoxVento_PriceIndexBorder, CD_TecBoxVento_Pointer - 1, RD_TecBox_ColName, RD_TecBox_ColUnit );

	// Add possibility to click on title group to reset column width.
	CreateTitleGroupButtons( CD_TecBoxVento_FirstColumn + 1, CD_TecBoxVento_FirstColumn + 1, RD_TecBox_GroupName, pclSDTecBoxVento );

	pclSSheet->SetBool( SSB_REDRAW, TRUE );
	return m_lSelectedRow;
}

void CRViewSSelPM::_InitTecBoxVentoHeaders( CSheetDescription *pclSDTecBoxVento, CSelectedPMBase *pclSelectedPM, bool bForceRedForTitleBkg )
{
	if( NULL == pclSDTecBoxVento || NULL == pclSDTecBoxVento->GetSSheetPointer() || NULL == m_pclIndSelPMParams 
			|| NULL == m_pclIndSelPMParams->m_pclSelectPMList || NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERT_RETURN;
	}
	
	CPMInputUser *pclInputUser = m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser();

	CSSheet *pclSSheet = pclSDTecBoxVento->GetSSheetPointer();
	pclSSheet->Init();
	pclSSheet->SetBool( SSB_ALLOWDRAGDROP, FALSE );
	pclSSheet->SetBool( SSB_HORZSCROLLBAR, FALSE );
	pclSSheet->SetBool( SSB_VERTSCROLLBAR, FALSE );
	pclSSheet->SetBool( SSB_MOVEACTIVEONFOCUS, FALSE );
	pclSSheet->SetTextTip( SS_TT_STATUS_FLOATING, 250, NULL, _LIGHTYELLOW, _TAH_BLACK );

	// Set max rows.
	pclSSheet->SetMaxRows( RD_TecBox_FirstAvailRow - 1 );

	// All cells are static by default and filled with _T("").
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	pclSSheet->FormatStaticText( -1, -1, -1, -1, _T( "" ) );

	// Increase row height.
	double RowHeight = 12.75;
	pclSSheet->SetRowHeight( RD_TecBox_FirstRow, RowHeight * 0.5 );
	pclSSheet->SetRowHeight( RD_TecBox_GroupName, RowHeight * 1.5 );
	pclSSheet->SetRowHeight( RD_TecBox_ColName, RowHeight * 2 );
	pclSSheet->SetRowHeight( RD_TecBox_ColUnit, RowHeight * 1.2 );

	// Initialize.
	pclSDTecBoxVento->Init( 1, pclSSheet->GetMaxRows(), 1, CSheetDescription::SD_ParameterMode_Mono );
	pclSDTecBoxVento->AddColumnInPixels( CD_TecBoxVento_FirstColumn, m_mapSSheetColumnWidth[SD_TecBoxVento][CD_TecBoxVento_FirstColumn] );
	pclSDTecBoxVento->AddColumnInPixels( CD_TecBoxVento_CheckBox, m_mapSSheetColumnWidth[SD_TecBoxVento][CD_TecBoxVento_CheckBox] );

	// Show price index for Vento only if there is no pressurisation (Vento selected alone).
	if( true == TASApp.IsPMPriceIndexUsed() && false == pclInputUser->IsPressurisationSystemExist() )
	{
		pclSDTecBoxVento->AddColumnInPixels( CD_TecBoxVento_PriceIndexIcon, m_mapSSheetColumnWidth[SD_TecBoxVento][CD_TecBoxVento_PriceIndexIcon] );
		pclSDTecBoxVento->AddColumnInPixels( CD_TecBoxVento_PriceIndexValue, m_mapSSheetColumnWidth[SD_TecBoxVento][CD_TecBoxVento_PriceIndexValue] );
		pclSDTecBoxVento->AddColumnInPixels( CD_TecBoxVento_PriceIndexBorder, m_mapSSheetColumnWidth[SD_TecBoxVento][CD_TecBoxVento_PriceIndexBorder] );
	}

	pclSDTecBoxVento->AddColumnInPixels( CD_TecBoxVento_Name, m_mapSSheetColumnWidth[SD_TecBoxVento][CD_TecBoxVento_Name] );
	pclSDTecBoxVento->AddColumnInPixels( CD_TecBoxVento_NbreOfDevice, m_mapSSheetColumnWidth[SD_TecBoxVento][CD_TecBoxVento_NbreOfDevice] );
	pclSDTecBoxVento->AddColumnInPixels( CD_TecBoxVento_MaxPressure, m_mapSSheetColumnWidth[SD_TecBoxVento][CD_TecBoxVento_MaxPressure] );
	pclSDTecBoxVento->AddColumnInPixels( CD_TecBoxVento_TempRange, m_mapSSheetColumnWidth[SD_TecBoxVento][CD_TecBoxVento_TempRange] );
	pclSDTecBoxVento->AddColumnInPixels( CD_TecBoxVento_MaxWaterMakeUpTemp, m_mapSSheetColumnWidth[SD_TecBoxPleno][CD_TecBoxVento_MaxWaterMakeUpTemp] );
	pclSDTecBoxVento->AddColumnInPixels( CD_TecBoxVento_Width, m_mapSSheetColumnWidth[SD_TecBoxVento][CD_TecBoxVento_Width] );
	pclSDTecBoxVento->AddColumnInPixels( CD_TecBoxVento_Height, m_mapSSheetColumnWidth[SD_TecBoxVento][CD_TecBoxVento_Height] );
	pclSDTecBoxVento->AddColumnInPixels( CD_TecBoxVento_Depth, m_mapSSheetColumnWidth[SD_TecBoxVento][CD_TecBoxVento_Depth] );
	pclSDTecBoxVento->AddColumnInPixels( CD_TecBoxVento_Weight, m_mapSSheetColumnWidth[SD_TecBoxVento][CD_TecBoxVento_Weight] );
	pclSDTecBoxVento->AddColumnInPixels( CD_TecBoxVento_Standing, m_mapSSheetColumnWidth[SD_TecBoxVento][CD_TecBoxVento_Standing] );
	pclSDTecBoxVento->AddColumnInPixels( CD_TecBoxVento_ElectricalPower, m_mapSSheetColumnWidth[SD_TecBoxVento][CD_TecBoxVento_ElectricalPower] );
	pclSDTecBoxVento->AddColumnInPixels( CD_TecBoxVento_SupplyVoltage, m_mapSSheetColumnWidth[SD_TecBoxVento][CD_TecBoxVento_SupplyVoltage] );
	pclSDTecBoxVento->AddColumnInPixels( CD_TecBoxVento_VNd, m_mapSSheetColumnWidth[SD_TecBoxVento][CD_TecBoxVento_VNd] );
	pclSDTecBoxVento->AddColumnInPixels( CD_TecBoxVento_SPL, m_mapSSheetColumnWidth[SD_TecBoxVento][CD_TecBoxVento_SPL] );
	pclSDTecBoxVento->AddColumnInPixels( CD_TecBoxVento_dpu, m_mapSSheetColumnWidth[SD_TecBoxVento][CD_TecBoxVento_dpu] );
	pclSDTecBoxVento->AddColumnInPixels( CD_TecBoxVento_qNwm, m_mapSSheetColumnWidth[SD_TecBoxVento][CD_TecBoxVento_qNwm] );
	pclSDTecBoxVento->AddColumnInPixels( CD_TecBoxVento_IP, m_mapSSheetColumnWidth[SD_TecBoxVento][CD_TecBoxVento_IP] );
	pclSDTecBoxVento->AddColumnInPixels( CD_TecBoxVento_Pointer, m_mapSSheetColumnWidth[SD_TecBoxVento][CD_TecBoxVento_Pointer] );
	
	// Must be placed after any call to 'AddColumnInPixels'. Because this method will add needed columns and TSpread will show them by default.
	// When calling the first time 'AddColumnInPixels' for the 'CD_TecBoxVento_Name' column, this method will call TSpread::SetMaxCols() to adapt the
	// number of columns. All the column before 'CD_TecBoxVento_Name' are set to a default width and are visible.
	if( false == TASApp.IsPMPriceIndexUsed() || true == pclInputUser->IsPressurisationSystemExist() )
	{
		pclSSheet->ShowCol( CD_TecBoxVento_PriceIndexIcon, FALSE );
		pclSSheet->ShowCol( CD_TecBoxVento_PriceIndexValue, FALSE );
		pclSSheet->ShowCol( CD_TecBoxVento_PriceIndexBorder, FALSE );
	}
	
	// Set in which column parameter must be saved.
	pclSDTecBoxVento->AddParameterColumn( CD_TecBoxVento_Pointer );

	// Set the focus column.
	pclSDTecBoxVento->SetActiveColumn( CD_TecBoxVento_FirstColumn + 1 );

	// Set range for selection.
	pclSDTecBoxVento->SetFocusColumnRange( CD_TecBoxVento_FirstColumn + 1, CD_TecBoxVento_Pointer - 1 );

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Add the main title.
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );
	int iBackColor;

	if( true == bForceRedForTitleBkg )
	{
		iBackColor = _TAH_TITLE_MAIN_REEDIT;
	}
	else
	{
		iBackColor = ( NULL != pclSelectedPM ) ? _IMI_TITLE_GROUP3 : _IMI_TITLE_GROUP1;
	}

	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, iBackColor );
	pclSDTecBoxVento->SetUserVariable( _SDUV_TITLEFORECOLOR, ( pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor ) ) );
	pclSDTecBoxVento->SetUserVariable( _SDUV_TITLEBACKCOLOR, ( pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor ) ) );

	pclSSheet->AddCellSpanW( CD_TecBoxVento_FirstColumn + 1, RD_TecBox_GroupName, CD_TecBoxVento_Pointer - ( CD_TecBoxVento_FirstColumn + 1 ), 1 );
	pclSSheet->SetStaticText( CD_TecBoxVento_FirstColumn + 1, RD_TecBox_GroupName, IDS_SSHEETSSELPM_VGROUP );
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Column name.
	if( false == pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn ) )
	{
		return;
	}

	if( false == pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE ) )
	{
		return;
	}

	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap, (LPARAM)TRUE );

	// Show price index for Vento only if there is no pressurisation (Vento selected alone).
	if( true == TASApp.IsPMPriceIndexUsed() && false == pclInputUser->IsPressurisationSystemExist() )
	{
		pclSSheet->AddCellSpan( CD_TecBoxVento_PriceIndexIcon, RD_TecBox_ColName, 3, 1 );
		pclSSheet->SetStaticText( CD_TecBoxVento_PriceIndexIcon, RD_TecBox_ColName, IDS_SSHEETSSELPM_PRICEINDEX );
	}

	pclSSheet->SetStaticText( CD_TecBoxVento_Name, RD_TecBox_ColName, IDS_SSHEETSSELPM_TFNAME );
	pclSSheet->SetStaticText( CD_TecBoxVento_NbreOfDevice, RD_TecBox_ColName, IDS_SSHEETSSELPM_TBNUMBEROFDEVICES );
	pclSSheet->SetStaticText( CD_TecBoxVento_MaxPressure, RD_TecBox_ColName, IDS_SSHEETSSELPM_CPPS );
	pclSSheet->SetStaticText( CD_TecBoxVento_TempRange, RD_TecBox_ColName, IDS_SSHEETSSELPM_VESSELTEMPRANGE );
	pclSSheet->SetStaticText( CD_TecBoxVento_MaxWaterMakeUpTemp, RD_TecBox_ColName, IDS_SSHEETSSELPM_MAXWATERMAKEUPTEMP );
	pclSSheet->SetStaticText( CD_TecBoxVento_Width, RD_TecBox_ColName, IDS_SSHEETSSELPM_CPWIDTH );
	pclSSheet->SetStaticText( CD_TecBoxVento_Height, RD_TecBox_ColName, IDS_SSHEETSSELPM_CPHEIGHT );
	pclSSheet->SetStaticText( CD_TecBoxVento_Depth, RD_TecBox_ColName, IDS_SSHEETSSELPM_CPDEPTH );
	pclSSheet->SetStaticText( CD_TecBoxVento_Weight, RD_TecBox_ColName, IDS_SSHEETSSELPM_CPMASS );
	pclSSheet->SetStaticText( CD_TecBoxVento_ElectricalPower, RD_TecBox_ColName, IDS_SSHEETSSELPM_CPELECPOWER );
	pclSSheet->SetStaticText( CD_TecBoxVento_SupplyVoltage, RD_TecBox_ColName, IDS_SSHEETSSELPM_CPELECVOLTAGE );
	pclSSheet->SetStaticText( CD_TecBoxVento_VNd, RD_TecBox_ColName, IDS_SSHEETSSELPM_PVVND );
	pclSSheet->SetStaticText( CD_TecBoxVento_SPL, RD_TecBox_ColName, IDS_SSHEETSSELPM_TBSOUNDPRESSLEVEL );
	pclSSheet->SetStaticText( CD_TecBoxVento_dpu, RD_TecBox_ColName, IDS_SSHEETSSELPM_PVDPU );
	pclSSheet->SetStaticText( CD_TecBoxVento_qNwm, RD_TecBox_ColName, IDS_SSHEETSSELPM_PVQNWM );
	pclSSheet->SetStaticText( CD_TecBoxVento_IP, RD_TecBox_ColName, IDS_SSHEETSSELPM_TBIP );
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Column units.
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap, (LPARAM)FALSE );

	// Show price index for Vento only if there is no pressurisation (Vento selected alone).
	if( true == TASApp.IsPMPriceIndexUsed() && false == pclInputUser->IsPressurisationSystemExist() )
	{
		pclSSheet->SetStaticText( CD_TecBoxVento_PriceIndexValue, RD_TecBox_ColUnit, _T("%") );
	}

	pclSSheet->SetStaticText( CD_TecBoxVento_MaxPressure, RD_TecBox_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_PRESSURE ) ).c_str() );
	pclSSheet->SetStaticText( CD_TecBoxVento_TempRange, RD_TecBox_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_TEMPERATURE ) ).c_str() );
	pclSSheet->SetStaticText( CD_TecBoxVento_MaxWaterMakeUpTemp, RD_TecBox_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_TEMPERATURE ) ).c_str() );
	pclSSheet->SetStaticText( CD_TecBoxVento_Width, RD_TecBox_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_LENGTH ) ).c_str() );
	pclSSheet->SetStaticText( CD_TecBoxVento_Height, RD_TecBox_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_LENGTH ) ).c_str() );
	pclSSheet->SetStaticText( CD_TecBoxVento_Depth, RD_TecBox_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_LENGTH ) ).c_str() );
	pclSSheet->SetStaticText( CD_TecBoxVento_Weight, RD_TecBox_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_MASS ) ).c_str() );
	pclSSheet->SetStaticText( CD_TecBoxVento_ElectricalPower, RD_TecBox_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_ELEC_POWER ) ).c_str() );
	pclSSheet->SetStaticText( CD_TecBoxVento_SupplyVoltage, RD_TecBox_ColUnit, IDS_SSHEETSSELPM_TBVOLTUNIT );
	pclSSheet->SetStaticText( CD_TecBoxVento_VNd, RD_TecBox_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_VOLUME ) ).c_str() );
	pclSSheet->SetStaticText( CD_TecBoxVento_SPL, RD_TecBox_ColUnit, IDS_SSHEETSSELPM_TBDECIBELUNIT );
	pclSSheet->SetStaticText( CD_TecBoxVento_dpu, RD_TecBox_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_PRESSURE ) ).c_str() );
	pclSSheet->SetStaticText( CD_TecBoxVento_qNwm, RD_TecBox_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_FLOW ) ).c_str() );
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// By default we don't show this column.
	pclSSheet->ShowCol( CD_TecBoxVento_MaxWaterMakeUpTemp, FALSE );

	// Draw black line below column header.
	pclSSheet->SetCellBorder( CD_TecBoxVento_FirstColumn + 1, RD_TecBox_ColUnit, CD_TecBoxVento_Pointer - 1, RD_TecBox_ColUnit, true, SS_BORDERTYPE_BOTTOM );
}

long CRViewSSelPM::_FillTecBoxVentoRow( CSheetDescription *pclSDTecBoxVento, long lRow, CSelectedVento *pclSelectedTBVento, CPMInputUser *pclPMInputUser,
		CDS_SSelPMaint *pclEditedSSelPM, CSelectedPMBase* pclSelectedBestPriceProduct, bool bForRejectedProduct )
{
	if( NULL == pclSDTecBoxVento || NULL == pclSDTecBoxVento->GetSSheetPointer() || NULL == pclSelectedTBVento || NULL == pclPMInputUser )
	{
		ASSERTA_RETURN( -1 );
	}

	CDB_TBPlenoVento *pclTecBoxVento = dynamic_cast<CDB_TBPlenoVento *>( pclSelectedTBVento->GetpData() );

	if( NULL == pclTecBoxVento )
	{
		ASSERTA_RETURN( -1 );
	}

	CSSheet *pclSSheet = pclSDTecBoxVento->GetSSheetPointer();

	if( true == m_bIsPMTypeAll && true == pclSelectedTBVento->IsFlagSet( CSelectedPMBase::eNotPriority ) )
	{
		m_lProductNotPriorityCount++;

		if( false == m_bShowAllPrioritiesShown )
		{
			// Add one empty not selectable row.
			pclSDTecBoxVento->AddRows( 1 );
			pclSDTecBoxVento->RemoveSelectableRow( lRow );

			pclSSheet->SetCellBorder( CD_TecBoxVento_FirstColumn + 1, lRow, CD_TecBoxVento_Pointer - 1, lRow, true, 
					SS_BORDERTYPE_TOP | SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );

			m_lShowAllPrioritiesButtonRow = lRow++;
			m_bShowAllPrioritiesShown = true;
		}
	}

	// Add this row.
	// 'true' to specify that this row can be selected.
	pclSDTecBoxVento->AddRows( 1, true );

	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

	// Add checkbox.
	pclSSheet->SetCheckBox( CD_TecBoxVento_CheckBox, lRow, _T(""), false, true );

	// Show price index for Vento only if there is no pressurisation (Vento selected alone).
	if( true == TASApp.IsPMPriceIndexUsed() && false == pclPMInputUser->IsPressurisationSystemExist() )
	{
		if( NULL != pclSelectedBestPriceProduct 
				&&  0 == _tcscmp( pclSelectedBestPriceProduct->GetProductIDPtr().ID, pclSelectedTBVento->GetProductIDPtr().ID ) )
		{
			pclSSheet->SetPictureCellWithID( IDI_BESTPRICE, CD_TecBoxVento_PriceIndexIcon, lRow, CSSheet::PictureCellType::Icon );
		}

		CString str = _T("-");

		if( pclSelectedTBVento->GetPriceIndex() > 0.0 )
		{
			str.Format( _T("%i"), (int)pclSelectedTBVento->GetPriceIndex() );
		}

		pclSSheet->SetStaticText( CD_TecBoxVento_PriceIndexValue, lRow, str );
	}

	bool bBest = pclSelectedTBVento->IsFlagSet( CSelectedPMBase::eBest );

	// We not flag best solution when selection is done without pressurisation system.
	if( true == pclPMInputUser->IsPressurisationSystemExist() && true == bBest )
	{
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_GREEN );
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, true );
	}

	// Set name.
	pclSSheet->SetStaticText( CD_TecBoxVento_Name, lRow, pclTecBoxVento->GetName() );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, false );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

	int iNbrOfVento = pclSelectedTBVento->GetNumberOfVento();
	bool bRedundancy = pclPMInputUser->GetPMWQSelectionPreferences()->IsChecked( CPMWQPrefs::GeneralRedundancyTecBox );
	bool bRedundancyDisabled = pclPMInputUser->GetPMWQSelectionPreferences()->IsDisabled( CPMWQPrefs::GeneralRedundancyTecBox );
	// HYS-1537 : Check if the preference is enabled
	iNbrOfVento *= ( false == bRedundancy || true == bRedundancyDisabled ) ? 1 : 2;
	
	CString str;
	str.Format( _T("%i"), iNbrOfVento );

	if( true == pclSelectedTBVento->IsFlagSet( CSelectedPMBase::eVentoMaxParallel ) )
	{
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_ORANGE );
	}

	pclSSheet->SetStaticText( CD_TecBoxVento_NbreOfDevice, lRow, str );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

	pclSSheet->SetStaticText( CD_TecBoxVento_MaxPressure, lRow, WriteCUDouble( _U_PRESSURE, pclTecBoxVento->GetPmaxmax() ) );

	if( true == pclSelectedTBVento->IsFlagSet( CSelectedPMBase::eTemperature ) ||
		true == pclSelectedTBVento->IsFlagSet( CSelectedPMBase::eVentoMaxTempConnectPoint ) ||
		true == pclSelectedTBVento->IsFlagSet( CSelectedPMBase::eVentoMinTemp ) )
	{
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_RED );
	}

	pclSSheet->SetStaticText( CD_TecBoxVento_TempRange, lRow, pclTecBoxVento->GetTempRange() );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

	if( true == pclTecBoxVento->IsFctWaterMakeUp() )
	{
		pclSSheet->ShowCol( CD_TecBoxVento_MaxWaterMakeUpTemp, TRUE );

		// Remark: for Vento we don't have the min. admissible temperature for the integrated water make-up part.
		if( true == pclSelectedTBVento->IsFlagSet( CSelectedPMBase::ePlenoWMUpMaxTemp ) )
		{
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_RED );
		}

		pclSSheet->SetStaticText( CD_TecBoxVento_MaxWaterMakeUpTemp, lRow, WriteCUDouble( _U_TEMPERATURE, pclTecBoxVento->GetTmaxWaterMakeUp() ) );
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );
	}
	else if( FALSE == pclSSheet->IsColHidden( CD_TecBoxVento_MaxWaterMakeUpTemp ) )
	{
		pclSSheet->SetStaticText( CD_TecBoxVento_MaxWaterMakeUpTemp, lRow, _T("-") );
	}

	pclSSheet->SetStaticText( CD_TecBoxVento_Width, lRow, WriteCUDouble( _U_LENGTH, pclTecBoxVento->GetWidth() ) );
	pclSSheet->SetStaticText( CD_TecBoxVento_Height, lRow, WriteCUDouble( _U_LENGTH, pclTecBoxVento->GetHeight() ) );
	pclSSheet->SetStaticText( CD_TecBoxVento_Depth, lRow, WriteCUDouble( _U_LENGTH, pclTecBoxVento->GetDepth() ) );
	pclSSheet->SetStaticText( CD_TecBoxVento_Weight, lRow, WriteCUDouble( _U_MASS, pclTecBoxVento->GetWeight() ) );
	pclSSheet->SetPictureCellWithID( pclTecBoxVento->GetInstallationLayoutPictureID(), CD_TecBoxVento_Standing, lRow, CSSheet::PictureCellType::Icon );
	pclSSheet->SetStaticText( CD_TecBoxVento_ElectricalPower, lRow, WriteCUDouble( _U_ELEC_POWER, pclTecBoxVento->GetPower() ) );
	pclSSheet->SetStaticText( CD_TecBoxVento_SupplyVoltage, lRow, pclTecBoxVento->GetPowerSupplyStr() );
	pclSSheet->SetStaticText( CD_TecBoxVento_VNd, lRow, WriteCUDouble( _U_VOLUME, pclTecBoxVento->GetSystemVolume() ) );
	pclSSheet->SetStaticText( CD_TecBoxVento_SPL, lRow, WriteDouble( pclTecBoxVento->GetSoundPressureLevel(), 1 ) );
	pclSSheet->SetStaticText( CD_TecBoxVento_dpu, lRow, pclTecBoxVento->GetWorkingPressureRange().c_str() );

	if( -1.0 != pclTecBoxVento->GetFlowWaterMakeUp() )
	{
		pclSSheet->SetStaticText( CD_TecBoxVento_qNwm, lRow, WriteCUDouble( _U_FLOW, pclTecBoxVento->GetFlowWaterMakeUp() ) );
	}
	else
	{
		pclSSheet->SetStaticText( CD_TecBoxVento_qNwm, lRow, _T("-") );
	}

	pclSSheet->SetStaticText( CD_TecBoxVento_IP, lRow, pclTecBoxVento->GetIPxx() );

	// Save parameter.
	CCellDescriptionProduct *pclCDProduct = CreateCellDescriptionProduct( pclSDTecBoxVento->GetFirstParameterColumn(), lRow, (LPARAM)pclTecBoxVento,
			pclSDTecBoxVento );

	if( NULL != pclCDProduct )
	{
		pclCDProduct->SetUserParam( (LPARAM)pclSelectedTBVento );
	}

	CDB_TBPlenoVento *pclEditedVento = NULL;

	if( NULL != pclEditedSSelPM )
	{
		pclEditedVento = dynamic_cast<CDB_TBPlenoVento *>( pclEditedSSelPM->GetTecBoxVentoIDPtr().MP );
	}

	if( NULL != pclEditedVento && pclEditedVento == pclTecBoxVento )
	{
		m_lSelectedRow = lRow;
	}

	pclSSheet->SetCellBorder( CD_TecBoxVento_FirstColumn + 1, lRow, CD_TecBoxVento_Pointer - 1, lRow, true, SS_BORDERTYPE_BOTTOM, 
			SS_BORDERSTYLE_SOLID, _GRAY );

	return lRow;
}

void CRViewSSelPM::_VerifyPlenoSheet( CSelectedPMBase *pclSelectedProduct, UINT uiSDGroupID )
{
	if( NULL == m_pclIndSelPMParams || NULL == m_pclIndSelPMParams->m_pclSelectPMList || NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERT_RETURN;
	}

	// If user selects a Vento with a Statico/Compresso/Transfero, we have to check if this one already contains a water make-up system. 
	// If it's the case, we hide the Pleno sheet. Otherwise, we let the Pleno sheet displayed and we show only Pleno compatible with the Vento.
	//
	// If user selects a Vento alone, we have to select Pleno that are compatible with the current selected Vento.
	//
	// If user unselects a Vento with a Statico/Compresso/Transfero, we have to show all Pleno available with the vessel, Compresso or Transfero.
	//
	// If user unselects a Vento alone, we have to show all Pleno available in regards to the combo choice.

	if( NULL != pclSelectedProduct )
	{
		// A Vento is selected.

		if( true == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->IsPressurisationSystemExist() )
		{
			// Vento is selected with a Statico, Compresso or Transfero.

			UINT uiPlenoSDGID = _ConvertSD2SDG( SD_TecBoxPleno, uiSDGroupID );

			if( CSelectedPMBase::ST_Vento == pclSelectedProduct->GetSelectedType() 
					&& NULL != dynamic_cast<CDB_TBPlenoVento *>( (CData*)pclSelectedProduct->GetProductIDPtr().MP ) )
			{
				// HYS-1832: Exception with vento selected with Simply Compresso SWM. We still show Pleno compatible with the Simply Compresso
				
				CDB_TecBox* pclTechBox = NULL;
				UINT uiSheetDescriptionID = -1;
				bool bShowPlenoForVento = true;
				// Find the parent's group
				GetTecBoxCompressoSelected( &pclTechBox );
				if( ( NULL != pclTechBox ) && ( 0 == CString( pclTechBox->GetIDPtr().ID ).Compare( _T( "SIMP_COMP_2_1_80SWM" ) ) ) )
				{
					bShowPlenoForVento = false;
				}
				
				if( true == bShowPlenoForVento )
				{
					// First of all: clear Pleno sheet and its accessories.
					_ClearPreviousSelectedProduct( SD_TecBoxPleno, uiSDGroupID );

					// '_ClearPreviousSelectedProduct' clear only sheets linked to Pleno (accessories or so on) but not the Pleno sheet itself.
					m_ViewDescription.RemoveOneSheetDescription( uiPlenoSDGID );

					CDB_TBPlenoVento *pclVento = dynamic_cast<CDB_TBPlenoVento *>( (CData*)pclSelectedProduct->GetProductIDPtr().MP );

					if( true == pclVento->IsFctWaterMakeUp() )
					{
						// If Vento is selected and has internal water make-up device, we don't show Pleno in a separate sheet.
						return;
					}
					else
					{
						// Show only Pleno compatible with the Vento.
						m_pclIndSelPMParams->m_pclSelectPMList->SelectVentoPleno( pclSelectedProduct, true );

						_FillTecBoxPlenoSheet( uiSDGroupID, pclSelectedProduct );
					}
				}
			}
		}
		else
		{
			// Vento is selected alone.

			CDB_Product *pclProduct = NULL;
			long lRow = 0;
			LPARAM lParam = 0;
				
			if( true == GetProductSelected( SD_TecBoxVento, SD_TecBoxVento, &pclProduct, &lRow, &lParam ) && NULL != lParam )
			{
				pclSelectedProduct = (CSelectedPMBase *)lParam;
			}

			m_pclIndSelPMParams->m_pclSelectPMList->SelectVentoPleno( pclSelectedProduct, true );

			// First of all: clear Pleno sheet and its accessories.
			// -1 because in this case Pleno are not belonging to a group.
			_ClearPreviousSelectedProduct( SD_TecBoxPleno, -1 );
			
			// '_ClearPreviousSelectedProduct' clear only sheets linked to Pleno (accessories or so on) but not the Pleno sheet itself.
			m_ViewDescription.RemoveOneSheetDescription( SD_TecBoxPleno );

			// -1 because in this case Pleno are not belonging to a group (Selection is not done with Statico, Compresso or Transfero).
			_FillTecBoxPlenoSheet( -1, pclSelectedProduct );
		}
	}
	else
	{
		// A Vento is unselected.

		if( true == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->IsPressurisationSystemExist() )
		{
			// Vento was selected with a Statico, Compresso or Transfero.

			// First of all: clear Pleno sheet and its accessories.
			_ClearPreviousSelectedProduct( SD_TecBoxPleno, uiSDGroupID );
			
			// '_ClearPreviousSelectedProduct' clear only sheets linked to Pleno (accessories or so on) but not the Pleno sheet itself.
			UINT uiPlenoSDGID = _ConvertSD2SDG( SD_TecBoxPleno, uiSDGroupID );
			m_ViewDescription.RemoveOneSheetDescription( uiPlenoSDGID );

			// We can thus show Pleno compatible with the selected Statico, Compresso or Transfero.
			LPARAM lpParam = NULL;

			if( uiSDGroupID == SD_ExpansionVessel )
			{
				CDB_Vessel *pclVessel = NULL;
				GetExpansionVesselSelected( &pclVessel, NULL, &lpParam );
			}
			else if( uiSDGroupID == SD_ExpansionVesselMembrane )
			{
				CDB_Vessel *pclVessel = NULL;
				GetExpansionVesselSelected( &pclVessel, NULL, &lpParam );
			}
			else if( uiSDGroupID == SD_TecBoxCompresso || uiSDGroupID == SD_TecBoxTransfero )
			{
				CDB_TecBox *pclTechBox = NULL;
				GetTecBoxSelected( &pclTechBox, NULL, NULL, &lpParam );
			}

			CSelectedPMBase *pclSelected = NULL;

			if( NULL != lpParam )
			{
				pclSelected = (CSelectedPMBase *)lpParam;

				// Add Pleno that are compatible with the current selected Statico, Compresso or Transfero.
				m_pclIndSelPMParams->m_pclSelectPMList->SelectVentoPleno( pclSelected, true );

				_FillTecBoxPlenoSheet( uiSDGroupID, pclSelectedProduct );
			}
		}
		else
		{
			// Vento was selected alone.

			// First of all: clear Pleno sheet and its accessories.
			// -1 because in this case Pleno are not belonging to a group.
			_ClearPreviousSelectedProduct( SD_TecBoxPleno, -1 );
			
			// '_ClearPreviousSelectedProduct' clear only sheets linked to Pleno (accessories or so on) but not the Pleno sheet itself.
			m_ViewDescription.RemoveOneSheetDescription( SD_TecBoxPleno );

			// 'NULL' and 'true' because in this case we select Pleno alone in regards to the current combo choice.
			m_pclIndSelPMParams->m_pclSelectPMList->SelectVentoPleno( NULL, true );

			// -1 because in this case Pleno are not belonging to a group (Selection is not done with Statico, Compresso or Transfero).
			_FillTecBoxPlenoSheet( -1, NULL );
		}
	}
}

void CRViewSSelPM::_VerifyPlenoRefillSheet( UINT uiSDGroupID )
{
	if( NULL == m_pclIndSelPMParams || NULL == m_pclIndSelPMParams->m_pclSelectPMList || NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERT_RETURN;
	}

	LPARAM lpParam = 0;
	CDB_TBPlenoVento *pclPlenoWMProtection = NULL;
	CDB_TBPlenoVento *pclTecBoxPlenoSelected = NULL;
	CDB_Set *pclWTCombination = NULL;
	bool bPlenoSelected = GetTecBoxPlenoSelected( &pclTecBoxPlenoSelected, uiSDGroupID, NULL, &lpParam );
	bool bPlenoWMProtectionSelected = GetTecBoxPlenoWMProtectionSelected( &pclPlenoWMProtection, uiSDGroupID, NULL, &lpParam );
	bool bAtLeastOneDeviceSelectedHasVacuumDegassingFunction = _IsAtLeastOneSelectedDeviceHasVacuumDegassingFunction( uiSDGroupID );

	if( NULL != pclTecBoxPlenoSelected && NULL == pclPlenoWMProtection )
	{
		if( 0 != lpParam )
		{
			
		}
		
		if( true == m_ViewDescription.IsSheetDescriptionExist( SD_PlenoRefill ) )
		{
			_ClearPreviousSelectedProduct( SD_PlenoRefill, -1 );
			m_ViewDescription.RemoveOneSheetDescription( SD_PlenoRefill );
		}
		
		m_pclIndSelPMParams->m_pclSelectPMList->SelectPlenoRefill( pclTecBoxPlenoSelected, bAtLeastOneDeviceSelectedHasVacuumDegassingFunction, pclWTCombination );
		_FillPlenoRefillSheet( uiSDGroupID );
	}
	else if( NULL == pclTecBoxPlenoSelected && 0 != lpParam )
	{
		// Here we have a water treatment combination for the Pleno (the case for TV connect with P BA4 P + P AB5 R).
		CSelectedWaterTreatmentCombination *pclSelectedWTCombination = (CSelectedWaterTreatmentCombination*)lpParam;
		pclWTCombination = pclSelectedWTCombination->GetWTCombination();
		
		if( true == m_ViewDescription.IsSheetDescriptionExist( SD_PlenoRefill ) )
		{
			_ClearPreviousSelectedProduct( SD_PlenoRefill, -1 );
			m_ViewDescription.RemoveOneSheetDescription( SD_PlenoRefill );
		}
		
		m_pclIndSelPMParams->m_pclSelectPMList->SelectPlenoRefill( pclTecBoxPlenoSelected, bAtLeastOneDeviceSelectedHasVacuumDegassingFunction, pclWTCombination );
		_FillPlenoRefillSheet( uiSDGroupID );
	}
}

long CRViewSSelPM::_FillTecBoxPlenoSheet( UINT uiSDGroupID, CSelectedPMBase *pclSelectedPM, bool bOnlyWMProtecModule, CDS_SSelPMaint *pclEditedSSelPM, bool bForceRedForTitleBkg )
{
	if( NULL == m_pclIndSelPMParams || NULL == m_pclIndSelPMParams->m_pclSelectPMList || NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser() 
			|| NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetPMWQSelectionPreferences() )
	{
		ASSERTA_RETURN( -1 );
	}
	
	CSelectPMList::ProductType PTCurrent = ( true == bOnlyWMProtecModule ) ? CSelectPMList::ProductType::PT_Pleno_Protec : CSelectPMList::ProductType::PT_Pleno;
	SheetDescription SCurrentSheet = ( true == bOnlyWMProtecModule ) ? SD_PlenoWaterMakeUpProtection : SD_TecBoxPleno;
	UINT uiSDGID = _ConvertSD2SDG( SCurrentSheet, uiSDGroupID );

	if( -1 == uiSDGID )
	{
		return -1;
	}
	
	// If there is no product (selected or rejected).
	if( false == m_pclIndSelPMParams->m_pclSelectPMList->IsProductExistInBothList( PTCurrent ) )
	{
		return -1;
	}

	CPMInputUser *pclPMInputUser = m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser();

	// Get 'Parent' sheet identification.
	UINT uiAfterSheetDescriptionID = -1;

	if( -1 != uiSDGroupID )
	{
		uiAfterSheetDescriptionID = _GetAfterSheetDescriptionID( SCurrentSheet, uiSDGroupID );
	}

	if( -1 != _ConvertSD2SDG( uiAfterSheetDescriptionID, uiSDGroupID ) )
	{
		uiAfterSheetDescriptionID = _ConvertSD2SDG( uiAfterSheetDescriptionID, uiSDGroupID );
	}

	// Call 'MultiSpreadBase' method to get a new sheet.

	CSheetDescription *pclSDTecBoxPleno = CreateSSheet( uiSDGID, uiAfterSheetDescriptionID );

	if( NULL == pclSDTecBoxPleno || NULL == pclSDTecBoxPleno->GetSSheetPointer() )
	{
		return -1;
	}

	CSSheet *pclSSheet = pclSDTecBoxPleno->GetSSheetPointer();
	pclSDTecBoxPleno->SetGroupSheetDescriptionID( uiSDGroupID );
	pclSDTecBoxPleno->SetParentSheetDescriptionID( SCurrentSheet );

	pclSSheet->SetBool( SSB_REDRAW, FALSE );

	_InitTecBoxPlenoHeaders( pclSDTecBoxPleno, pclSelectedPM, bForceRedForTitleBkg, bOnlyWMProtecModule );

	m_lSelectedRow = -1;
	m_lProductTotalCount = 0;
	m_lProductNotPriorityCount = 0;
	m_bShowAllPrioritiesShown = false;
	m_lShowAllPrioritiesButtonRow = 0;
	long lRow = RD_TecBox_FirstAvailRow;
	pclSDTecBoxPleno->RestartRemarkGenerator();

	CSelectedPMBase* pclSelectedBestPriceProduct = NULL;

	// For Pleno only if there is no pressurisation (Pleno selected alone).
	if( true == TASApp.IsPMPriceIndexUsed() && false == pclPMInputUser->IsPressurisationSystemExist() )
	{
		pclSelectedBestPriceProduct = m_pclIndSelPMParams->m_pclSelectPMList->GetLowestPriceObject( PTCurrent );
	}
	// HYS-1121: Get the water make up protection
	CDB_TecBox *pclSelectedTecBox = ( NULL != pclSelectedPM ) ? dynamic_cast<CDB_TecBox *>( pclSelectedPM->GetProductIDPtr().MP ) : NULL;
	CTable *pclPlenoTable = ( NULL != pclSelectedTecBox ) ? dynamic_cast<CTable *>( pclSelectedTecBox->GetPlenoIDPtr().MP ) : NULL;

	for( CSelectedPMBase *pclSelectedTBPleno = m_pclIndSelPMParams->m_pclSelectPMList->GetFirstProduct( PTCurrent );
			NULL != pclSelectedTBPleno; pclSelectedTBPleno = m_pclIndSelPMParams->m_pclSelectPMList->GetNextProduct( PTCurrent ) )
	{
		if( true == bOnlyWMProtecModule && NULL != pclPlenoTable )
		{
			bool bPlenoFound = false;
			for( IDPTR PlenoIDPtr = pclPlenoTable->GetFirst(); '\0' != *PlenoIDPtr.ID; PlenoIDPtr = pclPlenoTable->GetNext() )
			{
				CDB_TBPlenoVento *pclTecBoxPleno = dynamic_cast<CDB_TBPlenoVento*>( PlenoIDPtr.MP );
				if( true == pclSelectedTBPleno->GetProductIDPtr().IDMatch( pclTecBoxPleno->GetIDPtr().ID ) )
				{
					bPlenoFound = true;
					break;
				}
			}
			if( false == bPlenoFound )
			{
				continue;
			}
		}
		long lReturn = _FillTecBoxPlenoRow( pclSDTecBoxPleno, lRow, pclSelectedTBPleno, m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser(),
				pclEditedSSelPM, pclSelectedBestPriceProduct );

		if( -1 == lReturn )
		{
			continue;
		}
		else
		{
			lRow = lReturn;
		}

		lRow++;
		m_lProductTotalCount++;
	}

	long lLastDataRow = lRow - 1;
	pclSDTecBoxPleno->SetUserVariable( _SDUV_TOTALPRODUCT_COUNT, m_lProductTotalCount );
	pclSDTecBoxPleno->SetUserVariable( _SDUV_NOPRIORITYPRODUCT_COUNT, m_lProductNotPriorityCount );

	pclSSheet->SetCellBorder( CD_TecBoxPleno_FirstColumn + 1, lLastDataRow, CD_TecBoxPleno_Pointer - 1, lLastDataRow, true, 
			SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );

	// Add first the link to the excluded products if exist.
	lRow = _AddExcludedProductsLink( pclSDTecBoxPleno, lRow, PTCurrent );

	// Add Remarks.
	// Note: these lines must be placed after creating Show/Hide all priorities button to avoid to take in count these lines when defining
	//       range of row to be Show/Hide.
	lRow = pclSDTecBoxPleno->WriteRemarks( lRow, CD_TecBoxPleno_FirstColumn + 1, CD_TecBoxPleno_Pointer );

	// Add possibility to change column size.
	// Remark: place this code after any initialization (like adding, removing, resizing, hiding/showing columns).
	CResizingColumnInfo *pclResizingColumnInfo = pclSDTecBoxPleno->GetResizingColumnInfo();
	pclResizingColumnInfo->ActivateFeature( true, false, RD_TecBox_FirstAvailRow, lLastDataRow );
	pclResizingColumnInfo->AddRangeColumn( CD_TecBoxPleno_PriceIndexBorder, CD_TecBoxPleno_Pointer - 1, RD_TecBox_ColName, RD_TecBox_ColUnit );

	// Add possibility to click on title group to reset column width.
	CreateTitleGroupButtons( CD_TecBoxPleno_FirstColumn + 1, CD_TecBoxPleno_Pointer - 1, RD_TecBox_GroupName, pclSDTecBoxPleno );

	pclSSheet->SetBool( SSB_REDRAW, TRUE );
	return m_lSelectedRow;
}

void CRViewSSelPM::_InitTecBoxPlenoHeaders( CSheetDescription *pclSDTecBoxPleno, CSelectedPMBase *pclSelectedPM, bool bForceRedForTitleBkg, bool bOnlyWMProtecModule )
{
	if( NULL == pclSDTecBoxPleno || NULL == pclSDTecBoxPleno->GetSSheetPointer() || NULL == m_pclIndSelPMParams 
			|| NULL == m_pclIndSelPMParams->m_pclSelectPMList || NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERT( 0 );
		return;
	}
	
	CPMInputUser *pclInputUser = m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser();

	CSSheet *pclSSheet = pclSDTecBoxPleno->GetSSheetPointer();
	pclSSheet->Init();
	pclSSheet->SetBool( SSB_ALLOWDRAGDROP, FALSE );
	pclSSheet->SetBool( SSB_HORZSCROLLBAR, FALSE );
	pclSSheet->SetBool( SSB_VERTSCROLLBAR, FALSE );
	pclSSheet->SetBool( SSB_MOVEACTIVEONFOCUS, FALSE );
	pclSSheet->SetTextTip( SS_TT_STATUS_FLOATING, 250, NULL, _LIGHTYELLOW, _TAH_BLACK );

	// Set max rows.
	pclSSheet->SetMaxRows( RD_TecBox_FirstAvailRow - 1 );

	// All cells are static by default and filled with _T("").
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	pclSSheet->FormatStaticText( -1, -1, -1, -1, _T( "" ) );

	// Increase row height.
	double dRowHeight = 12.75;
	pclSSheet->SetRowHeight( RD_TecBox_FirstRow, dRowHeight * 0.5 );
	pclSSheet->SetRowHeight( RD_TecBox_GroupName, dRowHeight * 1.5 );
	pclSSheet->SetRowHeight( RD_TecBox_ColName, dRowHeight * 2 );
	pclSSheet->SetRowHeight( RD_TecBox_ColUnit, dRowHeight * 1.2 );

	// Initialize.
	pclSDTecBoxPleno->Init( 1, pclSSheet->GetMaxRows(), 1, CSheetDescription::SD_ParameterMode_Mono );
	pclSDTecBoxPleno->AddColumnInPixels( CD_TecBoxPleno_FirstColumn, m_mapSSheetColumnWidth[SD_TecBoxPleno][CD_TecBoxPleno_FirstColumn] );
	pclSDTecBoxPleno->AddColumnInPixels( CD_TecBoxPleno_CheckBox, m_mapSSheetColumnWidth[SD_TecBoxPleno][CD_TecBoxPleno_CheckBox] );

	// Show price index for Pleno only if there is no pressurisation (Pleno selected alone).
	if( true == TASApp.IsPMPriceIndexUsed() && false == pclInputUser->IsPressurisationSystemExist() )
	{
		pclSDTecBoxPleno->AddColumnInPixels( CD_TecBoxPleno_PriceIndexIcon, m_mapSSheetColumnWidth[SD_TecBoxPleno][CD_TecBoxPleno_PriceIndexIcon] );
		pclSDTecBoxPleno->AddColumnInPixels( CD_TecBoxPleno_PriceIndexValue, m_mapSSheetColumnWidth[SD_TecBoxPleno][CD_TecBoxPleno_PriceIndexValue] );
		pclSDTecBoxPleno->AddColumnInPixels( CD_TecBoxPleno_PriceIndexBorder, m_mapSSheetColumnWidth[SD_TecBoxPleno][CD_TecBoxPleno_PriceIndexBorder] );
	}

	pclSDTecBoxPleno->AddColumnInPixels( CD_TecBoxPleno_Name, m_mapSSheetColumnWidth[SD_TecBoxPleno][CD_TecBoxPleno_Name] );
	pclSDTecBoxPleno->AddColumnInPixels( CD_TecBoxPleno_NbreOfDevice, m_mapSSheetColumnWidth[SD_TecBoxPleno][CD_TecBoxPleno_NbreOfDevice] );
	pclSDTecBoxPleno->AddColumnInPixels( CD_TecBoxPleno_MaxPressure, m_mapSSheetColumnWidth[SD_TecBoxPleno][CD_TecBoxPleno_MaxPressure] );
	pclSDTecBoxPleno->AddColumnInPixels( CD_TecBoxPleno_TempRange, m_mapSSheetColumnWidth[SD_TecBoxPleno][CD_TecBoxPleno_TempRange] );
	pclSDTecBoxPleno->AddColumnInPixels( CD_TecBoxPleno_Width, m_mapSSheetColumnWidth[SD_TecBoxPleno][CD_TecBoxPleno_Width] );
	pclSDTecBoxPleno->AddColumnInPixels( CD_TecBoxPleno_Height, m_mapSSheetColumnWidth[SD_TecBoxPleno][CD_TecBoxPleno_Height] );
	pclSDTecBoxPleno->AddColumnInPixels( CD_TecBoxPleno_Depth, m_mapSSheetColumnWidth[SD_TecBoxPleno][CD_TecBoxPleno_Depth] );
	pclSDTecBoxPleno->AddColumnInPixels( CD_TecBoxPleno_Weight, m_mapSSheetColumnWidth[SD_TecBoxPleno][CD_TecBoxPleno_Weight] );
	pclSDTecBoxPleno->AddColumnInPixels( CD_TecBoxPleno_Standing, m_mapSSheetColumnWidth[SD_TecBoxPleno][CD_TecBoxPleno_Standing] );
	pclSDTecBoxPleno->AddColumnInPixels( CD_TecBoxPleno_ElectricalPower, m_mapSSheetColumnWidth[SD_TecBoxPleno][CD_TecBoxPleno_ElectricalPower] );
	pclSDTecBoxPleno->AddColumnInPixels( CD_TecBoxPleno_SupplyVoltage, m_mapSSheetColumnWidth[SD_TecBoxPleno][CD_TecBoxPleno_SupplyVoltage] );
	pclSDTecBoxPleno->AddColumnInPixels( CD_TecBoxPleno_Kvs, m_mapSSheetColumnWidth[SD_TecBoxPleno][CD_TecBoxPleno_Kvs] );
	pclSDTecBoxPleno->AddColumnInPixels( CD_TecBoxPleno_SPL, m_mapSSheetColumnWidth[SD_TecBoxPleno][CD_TecBoxPleno_SPL] );
	pclSDTecBoxPleno->AddColumnInPixels( CD_TecBoxPleno_dpu, m_mapSSheetColumnWidth[SD_TecBoxPleno][CD_TecBoxPleno_dpu] );
	pclSDTecBoxPleno->AddColumnInPixels( CD_TecBoxPleno_qNwm, m_mapSSheetColumnWidth[SD_TecBoxPleno][CD_TecBoxPleno_qNwm] );
	pclSDTecBoxPleno->AddColumnInPixels( CD_TecBoxPleno_IP, m_mapSSheetColumnWidth[SD_TecBoxPleno][CD_TecBoxPleno_IP] );
	pclSDTecBoxPleno->AddColumnInPixels( CD_TecBoxPleno_Pointer, m_mapSSheetColumnWidth[SD_TecBoxPleno][CD_TecBoxPleno_Pointer] );
	
	// Must be placed after any call to 'AddColumnInPixels'. Because this method will add needed columns and TSpread will show them by default.
	// When calling the first time 'AddColumnInPixels' for the 'CD_TecBoxPleno_Name' column, this method will call TSpread::SetMaxCols() to adapt the
	// number of columns. All the column before 'CD_TecBoxPleno_Name' are set to a default width and are visible.
	if( false == TASApp.IsPMPriceIndexUsed() || true == pclInputUser->IsPressurisationSystemExist() )
	{
		pclSSheet->ShowCol( CD_TecBoxPleno_PriceIndexIcon, FALSE );
		pclSSheet->ShowCol( CD_TecBoxPleno_PriceIndexValue, FALSE );
		pclSSheet->ShowCol( CD_TecBoxPleno_PriceIndexBorder, FALSE );
	}
	
	// Set in which column parameter must be saved.
	pclSDTecBoxPleno->AddParameterColumn( CD_TecBoxPleno_Pointer );

	// Set the focus column.
	pclSDTecBoxPleno->SetActiveColumn( CD_TecBoxPleno_FirstColumn + 1 );

	// Set range for selection.
	pclSDTecBoxPleno->SetFocusColumnRange( CD_TecBoxPleno_FirstColumn + 1, CD_TecBoxPleno_Pointer - 1 );


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Add the main title.
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );
	int iBackColor;

	if( true == bForceRedForTitleBkg )
	{
		iBackColor = _TAH_TITLE_MAIN_REEDIT;
	}
	else
	{
		iBackColor = ( NULL != pclSelectedPM ) ? _IMI_TITLE_GROUP3 : _IMI_TITLE_GROUP1;
	}

	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, iBackColor );
	pclSDTecBoxPleno->SetUserVariable( _SDUV_TITLEFORECOLOR, ( pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor ) ) );
	pclSDTecBoxPleno->SetUserVariable( _SDUV_TITLEBACKCOLOR, ( pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor ) ) );

	pclSSheet->AddCellSpanW( CD_TecBoxPleno_FirstColumn + 1, RD_TecBox_GroupName, CD_TecBoxPleno_Pointer - ( CD_TecBoxPleno_FirstColumn + 1 ), 1 );
	if( true == bOnlyWMProtecModule )
	{
		pclSSheet->SetStaticText( CD_TecBoxPleno_FirstColumn + 1, RD_TecBox_GroupName, IDS_SELECT_PLENO_WMPROTECTION_TITLE );
	}
	else
	{
		pclSSheet->SetStaticText( CD_TecBoxPleno_FirstColumn + 1, RD_TecBox_GroupName, IDS_SSHEETSSELPM_PGROUP );
	}
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Column name.
	if( false == pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn ) )
	{
		return;
	}

	if( false == pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE ) )
	{
		return;
	}

	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap, (LPARAM)TRUE );

	// Show price index for Pleno only if there is no pressurisation (Pleno selected alone).
	if( true == TASApp.IsPMPriceIndexUsed() && false == pclInputUser->IsPressurisationSystemExist() )
	{
		pclSSheet->AddCellSpan( CD_TecBoxPleno_PriceIndexIcon, RD_TecBox_ColName, 3, 1 );
		pclSSheet->SetStaticText( CD_TecBoxPleno_PriceIndexIcon, RD_TecBox_ColName, IDS_SSHEETSSELPM_PRICEINDEX );
	}

	pclSSheet->SetStaticText( CD_TecBoxPleno_Name, RD_TecBox_ColName, IDS_SSHEETSSELPM_TFNAME );
	pclSSheet->SetStaticText( CD_TecBoxPleno_NbreOfDevice, RD_TecBox_ColName, IDS_SSHEETSSELPM_TBNUMBEROFDEVICES );
	pclSSheet->SetStaticText( CD_TecBoxPleno_MaxPressure, RD_TecBox_ColName, IDS_SSHEETSSELPM_CPPS );
	pclSSheet->SetStaticText( CD_TecBoxPleno_TempRange, RD_TecBox_ColName, IDS_SSHEETSSELPM_VESSELTEMPRANGE );
	pclSSheet->SetStaticText( CD_TecBoxPleno_Width, RD_TecBox_ColName, IDS_SSHEETSSELPM_CPWIDTH );
	pclSSheet->SetStaticText( CD_TecBoxPleno_Height, RD_TecBox_ColName, IDS_SSHEETSSELPM_CPHEIGHT );
	pclSSheet->SetStaticText( CD_TecBoxPleno_Depth, RD_TecBox_ColName, IDS_SSHEETSSELPM_CPDEPTH );
	pclSSheet->SetStaticText( CD_TecBoxPleno_Weight, RD_TecBox_ColName, IDS_SSHEETSSELPM_CPMASS );
	pclSSheet->SetStaticText( CD_TecBoxPleno_ElectricalPower, RD_TecBox_ColName, IDS_SSHEETSSELPM_CPELECPOWER );
	pclSSheet->SetStaticText( CD_TecBoxPleno_SupplyVoltage, RD_TecBox_ColName, IDS_SSHEETSSELPM_CPELECVOLTAGE );
	pclSSheet->SetStaticText( CD_TecBoxPleno_Kvs, RD_TecBox_ColName, IDS_SSHEETSSELPM_PVKVS );
	pclSSheet->SetStaticText( CD_TecBoxPleno_SPL, RD_TecBox_ColName, IDS_SSHEETSSELPM_TBSOUNDPRESSLEVEL );
	pclSSheet->SetStaticText( CD_TecBoxPleno_dpu, RD_TecBox_ColName, IDS_SSHEETSSELPM_PVDPU );
	pclSSheet->SetStaticText( CD_TecBoxPleno_qNwm, RD_TecBox_ColName, IDS_SSHEETSSELPM_PVQNWM );
	pclSSheet->SetStaticText( CD_TecBoxPleno_IP, RD_TecBox_ColName, IDS_SSHEETSSELPM_TBIP );
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Column units.
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap, (LPARAM)FALSE );

	// Show price index unit for Pleno only if there is no pressurisation (Pleno selected alone).
	if( true == TASApp.IsPMPriceIndexUsed() && false == pclInputUser->IsPressurisationSystemExist() )
	{
		pclSSheet->SetStaticText( CD_TecBoxPleno_PriceIndexValue, RD_TecBox_ColUnit, _T("%") );
	}

	pclSSheet->SetStaticText( CD_TecBoxPleno_MaxPressure, RD_TecBox_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_PRESSURE ) ).c_str() );
	pclSSheet->SetStaticText( CD_TecBoxPleno_TempRange, RD_TecBox_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_TEMPERATURE ) ).c_str() );
	pclSSheet->SetStaticText( CD_TecBoxPleno_Width, RD_TecBox_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_LENGTH ) ).c_str() );
	pclSSheet->SetStaticText( CD_TecBoxPleno_Height, RD_TecBox_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_LENGTH ) ).c_str() );
	pclSSheet->SetStaticText( CD_TecBoxPleno_Depth, RD_TecBox_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_LENGTH ) ).c_str() );
	pclSSheet->SetStaticText( CD_TecBoxPleno_Weight, RD_TecBox_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_MASS ) ).c_str() );
	pclSSheet->SetStaticText( CD_TecBoxPleno_ElectricalPower, RD_TecBox_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_ELEC_POWER ) ).c_str() );
	pclSSheet->SetStaticText( CD_TecBoxPleno_SupplyVoltage, RD_TecBox_ColUnit, IDS_SSHEETSSELPM_TBVOLTUNIT );
	pclSSheet->SetStaticText( CD_TecBoxPleno_Kvs, RD_TecBox_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _C_KVCVCOEFF ) ).c_str() );
	pclSSheet->SetStaticText( CD_TecBoxPleno_SPL, RD_TecBox_ColUnit, IDS_SSHEETSSELPM_TBDECIBELUNIT );
	pclSSheet->SetStaticText( CD_TecBoxPleno_dpu, RD_TecBox_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_PRESSURE ) ).c_str() );
	pclSSheet->SetStaticText( CD_TecBoxPleno_qNwm, RD_TecBox_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_FLOW ) ).c_str() );
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Draw black line below column header.
	pclSSheet->SetCellBorder( CD_TecBoxPleno_FirstColumn + 1, RD_TecBox_ColUnit, CD_TecBoxPleno_Pointer - 1, RD_TecBox_ColUnit, true, SS_BORDERTYPE_BOTTOM );
}

long CRViewSSelPM::_FillTecBoxPlenoRow( CSheetDescription *pclSDTecBoxPleno, long lRow, CSelectedPMBase *pclSelectedTBPleno,
		CPMInputUser *pclPMInputUser, CDS_SSelPMaint *pclEditedSSelPM, CSelectedPMBase* pclSelectedBestPriceProduct, bool bForRejectedProduct )
{
	if( NULL == pclSDTecBoxPleno || NULL == pclSDTecBoxPleno->GetSSheetPointer() || NULL == pclSelectedTBPleno || NULL == pclPMInputUser )
	{
		ASSERTA_RETURN( -1 );
	}

	// It exists now in the DB a Pleno that is in fact a combination of two modules "P BA4 P" + "P ABR5-R". We want to show this product as
	// one product but with specific data for each of both devices.
	bool bWTCombination = ( CSelectedPMBase::ST_WaterTreatmentCombination == pclSelectedTBPleno->GetSelectedType() ) ? true : false;

	CDB_TBPlenoVento *pclFirstTecBoxPleno = NULL;
	CDB_TBPlenoVento *pclSecondTecBoxPleno = NULL;
	CSelectedWaterTreatmentCombination *pclSelectedWTCombination = NULL;
	CDB_Set *pWTCombination = NULL;
	bool bDoubleInfo = false;
	bool bNoInfo = false;

	if( false == bWTCombination )
	{
		// Only one device.
		pclFirstTecBoxPleno = dynamic_cast<CDB_TBPlenoVento *>( pclSelectedTBPleno->GetpData() );
	}
	else
	{
		// Combination of two devices.
		pclSelectedWTCombination = dynamic_cast<CSelectedWaterTreatmentCombination*>( pclSelectedTBPleno );

		if( NULL == pclSelectedWTCombination )
		{
			return -1;
		}

		pWTCombination = pclSelectedWTCombination->GetWTCombination();

		if( NULL == pWTCombination )
		{
			return -1;
		}

		if( NULL != pclSelectedWTCombination->GetSelectedFirst() )
		{
			pclFirstTecBoxPleno = dynamic_cast<CDB_TBPlenoVento *>( pclSelectedWTCombination->GetSelectedFirst()->GetpData() );
		}

		if( NULL != pclSelectedWTCombination->GetSelectedSecond() )
		{
			pclSecondTecBoxPleno = dynamic_cast<CDB_TBPlenoVento *>( pclSelectedWTCombination->GetSelectedSecond()->GetpData() );
		}

		// The 'fDoubleInfo' variable is set to 'true' if there is 2 devices for this water treatment product.
		if( NULL != pclFirstTecBoxPleno && NULL != pclSecondTecBoxPleno )
		{
			bDoubleInfo = true;
		}
		
		// It exists also the possibility to choose no water make-up treatment but just the cartridge. In this case we effectively have 
		// an empty object in the DB that give the possibility to the user when clicking on it to choose a cartridge in the deployed sheet.
		if( NULL == pclFirstTecBoxPleno && NULL == pclSecondTecBoxPleno )
		{
			bNoInfo = true;
		}
	}

	if( false == bWTCombination && NULL == pclFirstTecBoxPleno )
	{
		return -1;
	}

	CSSheet *pclSSheet = pclSDTecBoxPleno->GetSSheetPointer();

	// Add this row.
	// 'true' to specify that this row can be selected.
	pclSDTecBoxPleno->AddRows( 1, true );

	// Double the row height if there is 2 products.
	if( true == bDoubleInfo )
	{
		long lHeight = pclSSheet->GetRowHeightInPixelsW( pclSSheet->GetMaxRows() );
		pclSSheet->SetRowHeightInPixels( pclSSheet->GetMaxRows(), 2 * lHeight );
	}

	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

	// Add checkbox.
	pclSSheet->SetCheckBox(CD_TecBoxPleno_CheckBox, lRow, _T(""), false, true);

	// Show price index for Pleno only if there is no pressurisation (Pleno selected alone).
	if( true == TASApp.IsPMPriceIndexUsed() && false == pclPMInputUser->IsPressurisationSystemExist() )
	{
		if( NULL != pclSelectedBestPriceProduct 
				&& 0 == _tcscmp( pclSelectedBestPriceProduct->GetProductIDPtr().ID, pclSelectedTBPleno->GetProductIDPtr().ID ) )
		{
			pclSSheet->SetPictureCellWithID( IDI_BESTPRICE, CD_TecBoxPleno_PriceIndexIcon, lRow, CSSheet::PictureCellType::Icon );
		}

		CString str = _T("-");

		if( pclSelectedTBPleno->GetPriceIndex() > 0.0 )
		{
			str.Format( _T("%i"), (int)pclSelectedTBPleno->GetPriceIndex() );
		}

		pclSSheet->SetStaticText( CD_TecBoxPleno_PriceIndexValue, lRow, str );
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// NAME
	CString strName;

	if( false == bWTCombination )
	{
		// Only one device.
		strName = pclFirstTecBoxPleno->GetName();
	}
	else
	{
		// Combination of two devices.

		if( pWTCombination->GetName().GetLength() > 0 )
		{
			// If the combination has a unique name, we use it.
			std::wstring *pStr = GetpXmlStrTab()->GetIDSStr( pWTCombination->GetName().GetBuffer() );
			
			if( NULL != pStr )
			{
				strName = pStr->c_str();
			}
			else
			{
				strName = pWTCombination->GetName();
			}
		}
		else
		{
			// Otherwise we take the name of the first device.
			strName = pclFirstTecBoxPleno->GetName();
			
			if( true == bDoubleInfo )
			{
				// And add the name of the second device if exist.
				strName += CString( _T("\r\n") ) + pclSecondTecBoxPleno->GetName();
			}
		}
	}
	
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap, (LPARAM)TRUE );
	pclSSheet->SetStaticText( CD_TecBoxPleno_Name, lRow, strName );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, false );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// REDUNDANCY
	bool bRedundancy = pclPMInputUser->GetPMWQSelectionPreferences()->IsChecked( CPMWQPrefs::GeneralRedundancyTecBox );
	bool bRedundancyDisabled = pclPMInputUser->GetPMWQSelectionPreferences()->IsDisabled( CPMWQPrefs::GeneralRedundancyTecBox );
	CString strRedundancy;
	
	if( false == bWTCombination )
	{
		// Only one device.
		// HYS-1537 : Check if the preference is enabled
		strRedundancy = ( false == bRedundancy || true == bRedundancyDisabled ) ? _T("1" ) : _T( "2");
	}
	else
	{
		// Combination of two devices.

		if( false == bNoInfo )
		{
			// Take the redundancy of the first device.
			// HYS-1537 : Check if the preference is enabled
			strRedundancy = ( false == bRedundancy || true == bRedundancyDisabled ) ? _T("1" ) : _T( "2");

			if( true == bDoubleInfo )
			{
				// And add the redundancy of the second device if exist.
				strRedundancy += _T("\r\n");
				// HYS-1537 : Check if the preference is enabled
				strRedundancy += ( false == bRedundancy || true == bRedundancyDisabled ) ? _T("1" ) : _T( "2");
			}

		}
		else
		{
			strRedundancy = _T("");
		}
	}

	pclSSheet->SetStaticText( CD_TecBoxPleno_NbreOfDevice, lRow, strRedundancy );
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// PMAXMAX
	CString strPmaxmax;

	if( false == bWTCombination )
	{
		// Only one device.
		strPmaxmax = WriteCUDouble( _U_PRESSURE, pclFirstTecBoxPleno->GetPmaxmax() );
	}
	else
	{
		// Combination of two devices.

		if( false == bNoInfo )
		{
			// Take the Pmaxmax of the first device.
			strPmaxmax = WriteCUDouble( _U_PRESSURE, pclFirstTecBoxPleno->GetPmaxmax() );

			if( true == bDoubleInfo )
			{
				// And add the Pmaxmax of the second device if exist.
				strPmaxmax += CString( _T("\r\n") ) + WriteCUDouble( _U_PRESSURE, pclSecondTecBoxPleno->GetPmaxmax() );
			}
		}
		else
		{
			strPmaxmax = _T("");
		}
	}

	pclSSheet->SetStaticText( CD_TecBoxPleno_MaxPressure, lRow, strPmaxmax );
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// TEMPERATURE
	// Check first if we need to show temperature in red or not.
	bool bRed = false;

	if( false == bWTCombination )
	{
		// Only one device.

		if( true == pclSelectedTBPleno->IsFlagSet( CSelectedPMBase::eTemperature ) 
				|| true == pclSelectedTBPleno->IsFlagSet( CSelectedPMBase::ePlenoWMUpMaxTemp ) 
				|| true == pclSelectedTBPleno->IsFlagSet( CSelectedPMBase::ePlenoWMUpMinTemp ) )
		{
			bRed = true;
		}
	}
	else
	{
		// Combination of two devices.

		if( NULL != pclSelectedWTCombination->GetSelectedFirst() )
		{
			// Check for the first device.

			if( true == pclSelectedWTCombination->GetSelectedFirst()->IsFlagSet( CSelectedPMBase::eTemperature ) 
					|| true == pclSelectedWTCombination->GetSelectedFirst()->IsFlagSet( CSelectedPMBase::ePlenoWMUpMaxTemp ) 
					|| true == pclSelectedWTCombination->GetSelectedFirst()->IsFlagSet( CSelectedPMBase::ePlenoWMUpMinTemp ) )
			{
				bRed = true;
			}
		}

		if( false == bRed && NULL != pclSelectedWTCombination->GetSelectedSecond() )
		{
			// If no error on the first device and a second one exist, we check the second device.

			if( true == pclSelectedWTCombination->GetSelectedSecond()->IsFlagSet( CSelectedPMBase::eTemperature ) 
					|| true == pclSelectedWTCombination->GetSelectedSecond()->IsFlagSet( CSelectedPMBase::ePlenoWMUpMaxTemp ) 
					|| true == pclSelectedWTCombination->GetSelectedSecond()->IsFlagSet( CSelectedPMBase::ePlenoWMUpMinTemp ) )
			{
				bRed = true;
			}
		}
	}

	if( true == bRed )
	{
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_RED );
	}

	CString strTempRange;

	if( false == bWTCombination )
	{
		// Only one device.
		strTempRange = pclFirstTecBoxPleno->GetTempRange();
	}
	else
	{
		// Combination of two devices.

		if( false == bNoInfo )
		{
			// Take the temperature range of the first device.
			strTempRange = pclFirstTecBoxPleno->GetTempRange();

			if( true == bDoubleInfo )
			{
				// And add the temperature range of the second device if exist.
				strTempRange += _T("\r\n") + pclSecondTecBoxPleno->GetTempRange();
			}
		}
		else
		{
			strTempRange = _T("");
		}
	}

	pclSSheet->SetStaticText( CD_TecBoxPleno_TempRange, lRow, strTempRange );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// WIDTH
	CString strWidth;

	if( false == bWTCombination )
	{
		// Only one device.
		strWidth = WriteCUDouble( _U_LENGTH, pclFirstTecBoxPleno->GetWidth() );
	}
	else
	{
		// Combination of two devices.

		if( false == bNoInfo )
		{
			// Take the width of the first device.
			strWidth = WriteCUDouble( _U_LENGTH, pclFirstTecBoxPleno->GetWidth() );

			if( true == bDoubleInfo )
			{
				// And add the width of the second device if exist.
				strWidth += CString( _T("\r\n") ) + WriteCUDouble( _U_LENGTH, pclSecondTecBoxPleno->GetWidth() );
			}
		}
		else
		{
			strWidth = _T("");
		}
	}

	pclSSheet->SetStaticText( CD_TecBoxPleno_Width, lRow, strWidth );
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// HEIGHT
	CString strHeight;

	if( false == bWTCombination )
	{
		// Only one device.
		strHeight = WriteCUDouble( _U_LENGTH, pclFirstTecBoxPleno->GetHeight() );
	}
	else
	{
		// Combination of two devices.

		if( false == bNoInfo )
		{
			// Take the height of the first device.
			strHeight = WriteCUDouble( _U_LENGTH, pclFirstTecBoxPleno->GetHeight() );

			if( true == bDoubleInfo )
			{
				// And add the height of the second device if exist.
				strHeight += CString( _T("\r\n") ) + WriteCUDouble( _U_LENGTH, pclSecondTecBoxPleno->GetHeight() );
			}
		}
		else
		{
			strHeight = _T("");
		}
	}

	pclSSheet->SetStaticText( CD_TecBoxPleno_Height, lRow, strHeight );
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// DEPTH
	CString strDepth;

	if( false == bWTCombination )
	{
		// Only one device.
		strDepth = WriteCUDouble( _U_LENGTH, pclFirstTecBoxPleno->GetDepth() );
	}
	else
	{
		// Combination of two devices.

		if( false == bNoInfo )
		{
			// Take the depth of the first device.
			strDepth = WriteCUDouble( _U_LENGTH, pclFirstTecBoxPleno->GetDepth() );

			if( true == bDoubleInfo )
			{
				// And add the depth of the second device if exist.
				strDepth += CString( _T("\r\n") ) + WriteCUDouble( _U_LENGTH, pclSecondTecBoxPleno->GetDepth() );
			}
		}
		else
		{
			strDepth = _T("");
		}
	}

	pclSSheet->SetStaticText( CD_TecBoxPleno_Depth, lRow, strDepth );
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// WEIGHT
	CString strWeight;

	if( false == bWTCombination )
	{
		// Only one device.
		strWeight = WriteCUDouble( _U_MASS, pclFirstTecBoxPleno->GetWeight() );
	}
	else
	{
		// Combination of two devices.

		if( false == bNoInfo )
		{
			// Take the weight of the first device.
			strWeight = WriteCUDouble( _U_MASS, pclFirstTecBoxPleno->GetWeight() );

			if( true == bDoubleInfo )
			{
				// And add the weight of the second device if exist.
				strWeight += CString( _T("\r\n") ) + WriteCUDouble( _U_MASS, pclSecondTecBoxPleno->GetWeight() );
			}
		}
		else
		{
			strWeight = _T("");
		}
	}

	pclSSheet->SetStaticText( CD_TecBoxPleno_Weight, lRow, strWeight );
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// STANDING

	// Only one device. Don't need to show standing for both devices.
	if( false == bNoInfo )
	{
		pclSSheet->SetPictureCellWithID( pclFirstTecBoxPleno->GetInstallationLayoutPictureID(), CD_TecBoxPleno_Standing, lRow, 
				CSSheet::PictureCellType::Icon );
	}
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// ELECTRICAL POWER
	CString strPower;

	if( false == bWTCombination )
	{
		// Only one device.
		strPower = WriteCUDouble( _U_ELEC_POWER, pclFirstTecBoxPleno->GetPower() );
	}
	else
	{
		// Combination of two devices.

		if( false == bNoInfo )
		{
			// Take the power of the first device.
			strPower = WriteCUDouble( _U_ELEC_POWER, pclFirstTecBoxPleno->GetPower() );

			if( true == bDoubleInfo )
			{
				// And add the power of the second device if exist.
				strPower += CString( _T("\r\n") ) + WriteCUDouble( _U_ELEC_POWER, pclSecondTecBoxPleno->GetPower() );
			}
		}
		else
		{
			strPower = _T("");
		}
	}

	pclSSheet->SetStaticText( CD_TecBoxPleno_ElectricalPower, lRow, strPower );
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// POWER SUPPLY
	CString strPowerSupply;

	if( false == bWTCombination )
	{
		// Only one device.
		strPowerSupply = pclFirstTecBoxPleno->GetPowerSupplyStr();
	}
	else
	{
		// Combination of two devices.

		if( false == bNoInfo )
		{
			// Take the power supply of the first device.
			strPowerSupply = pclFirstTecBoxPleno->GetPowerSupplyStr();

			if( true == bDoubleInfo )
			{
				// And add the power supply of the second device if exist.
				strPowerSupply += CString( _T("\r\n") ) + pclSecondTecBoxPleno->GetPowerSupplyStr();
			}
		}
		else
		{
			strPowerSupply = _T("");
		}
	}

	pclSSheet->SetStaticText( CD_TecBoxPleno_SupplyVoltage, lRow, strPowerSupply );
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// KVS
	CString strKvs;

	if( false == bWTCombination )
	{
		// Only one device.

		// Kvs is only for Pleno P/PI.
		if( -1.0 != pclFirstTecBoxPleno->GetKvs() )
		{
			strKvs = WriteCUDouble( _C_KVCVCOEFF, pclFirstTecBoxPleno->GetKvs() );
		}
		else
		{
			strKvs = _T("-");
		}
	}
	else
	{
		// Combination of two devices.

		if( false == bNoInfo )
		{
			// Take the kvs of the first device.

			if( -1.0 != pclFirstTecBoxPleno->GetKvs() )
			{
				strKvs = WriteCUDouble( _C_KVCVCOEFF, pclFirstTecBoxPleno->GetKvs() );
			}
			else
			{
				strKvs = _T("-");
			}

			if( true == bDoubleInfo )
			{
				// And add the kvs of the second device if exist.

				if( -1.0 != pclSecondTecBoxPleno->GetKvs() )
				{
					strKvs += CString( _T("\r\n") ) + WriteCUDouble( _C_KVCVCOEFF, pclSecondTecBoxPleno->GetKvs() );
				}
				else
				{
					strKvs += CString( _T("\r\n-") );
				}
			}
		}
		else
		{
			strKvs = _T("");
		}
	}

	pclSSheet->SetStaticText( CD_TecBoxPleno_Kvs, lRow, strKvs );
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// SOUND PRESSURE LEVEL
	CString strSPL;

	if( false == bWTCombination )
	{
		// Only one device.

		// Sound pressure level is only for PI9/6.
		if( 0.0 != pclFirstTecBoxPleno->GetSoundPressureLevel() )
		{
			strSPL.Format( _T("%.f"), pclFirstTecBoxPleno->GetSoundPressureLevel() );
		}
		else
		{
			strSPL = _T("-");
		}
	}
	else
	{
		// Combination of two devices.

		if( false == bNoInfo )
		{
			// Take the sound pressure level of the first device.
			// Sound pressure level is only for PI9/6.

			if( 0.0 != pclFirstTecBoxPleno->GetSoundPressureLevel() )
			{
				strSPL.Format( _T("%.f"), pclFirstTecBoxPleno->GetSoundPressureLevel() );
			}
			else
			{
				strSPL = _T("-");
			}

			if( true == bDoubleInfo )
			{
				// And add the sound pressure level of the second device if exist.
				// Sound pressure level is only for PI9/6.

				if( 0.0 != pclSecondTecBoxPleno->GetSoundPressureLevel() )
				{
					CString strTemp;
					strTemp.Format( _T("%.f"), pclSecondTecBoxPleno->GetSoundPressureLevel() );
					strSPL += CString( _T("\r\n") ) + strTemp;
				}
				else
				{
					strSPL += CString( _T("\r\n-") );
				}
			}
		}
		else
		{
			strSPL = _T("");
		}
	}
	
	pclSSheet->SetStaticText( CD_TecBoxPleno_SPL, lRow, strSPL );
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// DPU
	CString strDPp;

	if( false == bWTCombination )
	{
		// Only one device.
		strDPp = pclFirstTecBoxPleno->GetWorkingPressureRange().c_str();
	}
	else
	{
		// Combination of two devices.

		if( false == bNoInfo )
		{
			// Take the DPp of the first device.
			strDPp = pclFirstTecBoxPleno->GetWorkingPressureRange().c_str();

			if( true == bDoubleInfo )
			{
				// And add the DPp of the second device if exist.
				strDPp += CString( _T("\r\n") ) + pclSecondTecBoxPleno->GetWorkingPressureRange().c_str();
			}
		}
		else
		{
			strDPp = _T( "" );
		}
	}

	pclSSheet->SetStaticText( CD_TecBoxPleno_dpu, lRow, strDPp );
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// QNWM
	CString strVNS;

	if( false == bWTCombination )
	{
		// Only one device.
		// 'qNwm' is only for PI9/6.

		if( -1.0 != pclFirstTecBoxPleno->GetFlowWaterMakeUp() )
		{
			strVNS = WriteCUDouble( _U_FLOW, pclFirstTecBoxPleno->GetFlowWaterMakeUp() );
		}
		else
		{
			strVNS = _T("-");
		}
	}
	else
	{
		// Combination of two devices.

		if( false == bNoInfo )
		{
			// Take the 'qNwm' of the first device.
			// 'qNwm' is only for PI9/6.

			if( -1.0 != pclFirstTecBoxPleno->GetFlowWaterMakeUp() )
			{
				strVNS = WriteCUDouble( _U_FLOW, pclFirstTecBoxPleno->GetFlowWaterMakeUp() );
			}
			else
			{
				strVNS = _T("-");
			}

			if( true == bDoubleInfo )
			{
				// And add the 'qNwm' of the second device if exist.
				// 'qNwm' is only for PI9/6.

				if( -1.0 != pclSecondTecBoxPleno->GetFlowWaterMakeUp() )
				{
					strVNS += CString( _T("\r\n") ) + WriteCUDouble( _U_FLOW, pclSecondTecBoxPleno->GetFlowWaterMakeUp() );
				}
				else
				{
					strVNS += CString( _T("\r\n-") );
				}
			}
		}
		else
		{
			strVNS = _T("");
		}
	}

	pclSSheet->SetStaticText( CD_TecBoxPleno_qNwm, lRow, strVNS );
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// IP
	CString strIP;

	if( false == bWTCombination )
	{
		// Only one device.
		strIP = pclFirstTecBoxPleno->GetIPxx();
	}
	else
	{
		// Combination of two devices.

		if( false == bNoInfo )
		{
			// Take the IP of the first device.
			strIP = pclFirstTecBoxPleno->GetIPxx();

			if( true == bDoubleInfo )
			{
				// And add the IP of the second device if exist.
				strIP += _T("\r\n") + pclSecondTecBoxPleno->GetIPxx();
			}
		}
		else
		{
			strIP = _T("");
		}
	}

	pclSSheet->SetStaticText( CD_TecBoxPleno_IP, lRow, strIP );
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	// Save parameter.
	LPARAM lpProduct = ( false == bWTCombination ) ? (LPARAM)pclFirstTecBoxPleno : (LPARAM)0;

	CCellDescriptionProduct *pclCDProduct = CreateCellDescriptionProduct( pclSDTecBoxPleno->GetFirstParameterColumn(), lRow, lpProduct,
			pclSDTecBoxPleno );

	if( NULL != pclCDProduct )
	{
		if( false == bWTCombination )
		{
			pclCDProduct->SetUserParam( (LPARAM)pclSelectedTBPleno );
		}
		else
		{
			pclCDProduct->SetUserParam( (LPARAM)pclSelectedWTCombination );
		}
	}

	CDB_TBPlenoVento *pclEditedPleno = NULL;

	if( NULL != pclEditedSSelPM )
	{
		pclEditedPleno = dynamic_cast<CDB_TBPlenoVento *>( pclEditedSSelPM->GetTecBoxPlenoIDPtr().MP );
	}

	if( NULL != pclEditedPleno && pclEditedPleno == pclFirstTecBoxPleno )
	{
		m_lSelectedRow = lRow;
	}

	pclSSheet->SetCellBorder( CD_TecBoxPleno_FirstColumn + 1, lRow, CD_TecBoxPleno_Pointer - 1, lRow, true, SS_BORDERTYPE_BOTTOM, 
			SS_BORDERSTYLE_SOLID, _GRAY );

	return lRow;
}

void CRViewSSelPM::_PrepareTBPlenoAccessories( CDB_Set *pclWTCombination, CRank *prList, CDB_RuledTable **pclRuledTable )
{
	try
	{
		if( NULL == m_pclIndSelPMParams )
		{
			HYSELECT_THROW( _T("Internal error: 'm_pclIndSelPMParams' can't be NULL.") );
		}
		else if( NULL == m_pclIndSelPMParams->m_pTADB )
		{
			HYSELECT_THROW( _T("Internal error: 'm_pclIndSelPMParams->m_pTADB' can't be NULL.") );
		}

		if( NULL == pclWTCombination || NULL == prList )
		{
			return;
		}

		CRank rListFirst;
		CRank rListSecond;
		CDB_RuledTable *pclFirstRuledTable = NULL;
		CDB_RuledTable *pclSecondRuledTable = NULL;

		if( NULL != pclWTCombination->GetFirstIDPtr().MP 
				&& NULL != dynamic_cast<CDB_TBPlenoVento*>( (CData*)( pclWTCombination->GetFirstIDPtr().MP ) ) )
		{
			CDB_TBPlenoVento* pclPleno = dynamic_cast<CDB_TBPlenoVento*>( (CData*)( pclWTCombination->GetFirstIDPtr().MP ) );						
			pclFirstRuledTable = dynamic_cast<CDB_RuledTable*>( (CData*)pclPleno->GetAccessoriesGroupIDPtr().MP );
		}

		if( NULL != pclWTCombination->GetSecondIDPtr().MP 
				&& NULL != dynamic_cast<CDB_TBPlenoVento*>( (CData*)( pclWTCombination->GetSecondIDPtr().MP ) ) )
		{
			CDB_TBPlenoVento* pclPleno = dynamic_cast<CDB_TBPlenoVento*>( pclWTCombination->GetSecondIDPtr().MP );
			pclSecondRuledTable = dynamic_cast<CDB_RuledTable*>( (CData*)pclPleno->GetAccessoriesGroupIDPtr().MP );
		}

		IDPTR RuledTableTempIDPtr = m_pclIndSelPMParams->m_pTADB->Get( _T("RULEDTABLEMERGED") );

		if( _NULL_IDPTR != RuledTableTempIDPtr )
		{
			m_pclIndSelPMParams->m_pTADB->Access().Remove( RuledTableTempIDPtr );
			m_pclIndSelPMParams->m_pTADB->DeleteObject( RuledTableTempIDPtr );
		}

		m_pclIndSelPMParams->m_pTADB->CreateObject( RuledTableTempIDPtr, CLASS( CDB_RuledTable ), _T("RULEDTABLEMERGED") );
		m_pclIndSelPMParams->m_pTADB->Access().Insert( RuledTableTempIDPtr );

		CDB_RuledTable *pclRuledTableMerged = (CDB_RuledTable *)( RuledTableTempIDPtr.MP );

		// This table doesn't contain objects that belong to it.
		pclRuledTableMerged->SetTrueParent( false );

		if( NULL != pclFirstRuledTable )
		{
			pclRuledTableMerged->CopyFrom( pclFirstRuledTable );

			if( NULL != pclSecondRuledTable )
			{
				pclRuledTableMerged->MergeWith( pclSecondRuledTable );
			}
		}
		else if( NULL != pclSecondRuledTable )
		{
			pclRuledTableMerged->CopyFrom( pclSecondRuledTable );
		}

		m_pclIndSelPMParams->m_pTADB->GetAccessories( prList, pclRuledTableMerged, m_pclIndSelPMParams->m_eFilterSelection );

		if( NULL != pclRuledTable )
		{
			*pclRuledTable = pclRuledTableMerged;
		}
	}
	catch( CHySelectException &clHySelectException )
	{
		clHySelectException.AddMessage( _T("Error in 'CRViewSSelPM::_PrepareTBPlenoAccessories'."), __LINE__, __FILE__ );
		throw;
	}
}

long CRViewSSelPM::_FillPlenoRefillSheet( UINT uiSDGroupID )
{
	if( NULL == m_pclIndSelPMParams || NULL == m_pclIndSelPMParams->m_pclSelectPMList || NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser() 
			|| NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetPMWQSelectionPreferences() )
	{
		ASSERTA_RETURN( -1 );
	}

	UINT uiSDGID = _ConvertSD2SDG( SD_PlenoRefill, uiSDGroupID );

	if( -1 == uiSDGID )
	{
		return -1;
	}

	CPMInputUser *pclPMInputUser = m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser();

	// If user has not asked softening or desalination, we don't show Pleno Refill.
	if( 0 == CString( pclPMInputUser->GetWaterMakeUpTypeID() ).Compare( _T("WMUP_TYPE_NONE") ) 
			|| 0 == CString( pclPMInputUser->GetWaterMakeUpTypeID() ).Compare( _T("WMUP_TYPE_STD") ) )
	{
		return -1;
	}

	// Get 'Parent' sheet identification.
	UINT uiAfterSheetDescriptionID = -1;

	if( -1 != uiSDGroupID )
	{
		uiAfterSheetDescriptionID = _GetAfterSheetDescriptionID( SD_PlenoRefill, uiSDGroupID );
	}

	if( -1 != _ConvertSD2SDG( uiAfterSheetDescriptionID, uiSDGroupID ) )
	{
		uiAfterSheetDescriptionID = _ConvertSD2SDG( uiAfterSheetDescriptionID, uiSDGroupID );
	}

	// Call 'MultiSpreadBase' method to get a new sheet.

	CSheetDescription *pclSDPlenoRefill = CreateSSheet( uiSDGID, uiAfterSheetDescriptionID );

	if( NULL == pclSDPlenoRefill || NULL == pclSDPlenoRefill->GetSSheetPointer() )
	{
		return -1;
	}

	CSSheet *pclSSheet = pclSDPlenoRefill->GetSSheetPointer();
	pclSDPlenoRefill->SetGroupSheetDescriptionID( uiSDGroupID );
	pclSDPlenoRefill->SetParentSheetDescriptionID( SD_TecBoxPleno );

	pclSSheet->SetBool( SSB_REDRAW, FALSE );

	_InitPlenoRefillHeaders( pclSDPlenoRefill );

	// To compute max volume for the cartridge.
	m_dTotalHardnessOfSystem = pclPMInputUser->GetTotalHardnessOfSystem();

	m_lSelectedRow = -1;
	m_lProductTotalCount = 0;
	long lRow = RD_TecBox_FirstAvailRow;
	pclSDPlenoRefill->RestartRemarkGenerator();

	for( CSelectedPMBase *pclSelectedPlenoRefill = m_pclIndSelPMParams->m_pclSelectPMList->GetLastProduct( CSelectPMList::PT_PlenoRefill );
			NULL != pclSelectedPlenoRefill; pclSelectedPlenoRefill = m_pclIndSelPMParams->m_pclSelectPMList->GetPrevProduct( CSelectPMList::PT_PlenoRefill ) )
	{
		long lReturn = _FillPlenoRefillRow( pclSDPlenoRefill, lRow, pclSelectedPlenoRefill, pclPMInputUser );

		if( -1 == lReturn )
		{
			continue;
		}
		else
		{
			lRow = lReturn;
		}

		lRow++;
		m_lProductTotalCount++;
	}

	long lLastDataRow = lRow - 1;
	pclSDPlenoRefill->SetUserVariable( _SDUV_TOTALPRODUCT_COUNT, m_lProductTotalCount );

	pclSSheet->SetCellBorder( CD_PlenoRefill_FirstColumn + 1, lLastDataRow, CD_PlenoRefill_Pointer - 1, lLastDataRow, true, 
			SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );

	// Add first the link to the excluded products if exist.
	lRow = _AddExcludedProductsLink( pclSDPlenoRefill, lRow, CSelectPMList::PT_PlenoRefill );

	// Add Remarks.
	// Note: these lines must be placed after creating Show/Hide all priorities button to avoid to take in count these lines when defining
	//       range of row to be Show/Hide.
	lRow = pclSDPlenoRefill->WriteRemarks( lRow, CD_PlenoRefill_FirstColumn + 1, CD_PlenoRefill_Pointer );

	// Add possibility to change column size.
	// Remark: place this code after any initialization (like adding, removing, resizing, hiding/showing columns).
	CResizingColumnInfo *pclResizingColumnInfo = pclSDPlenoRefill->GetResizingColumnInfo();
	pclResizingColumnInfo->ActivateFeature( true, false, RD_TecBox_FirstAvailRow, lLastDataRow );
	pclResizingColumnInfo->AddRangeColumn( CD_PlenoRefill_PriceIndexBorder, CD_PlenoRefill_Pointer - 1, RD_TecBox_ColName, RD_TecBox_ColUnit );

	// Add possibility to click on title group to reset column width.
	CreateTitleGroupButtons( CD_PlenoRefill_FirstColumn + 1, CD_PlenoRefill_Pointer - 1, RD_TecBox_GroupName, pclSDPlenoRefill );

	pclSSheet->SetBool( SSB_REDRAW, TRUE );
	return m_lSelectedRow;
}

void CRViewSSelPM::_InitPlenoRefillHeaders( CSheetDescription *pclSDPlenoRefill )
{
	if( NULL == pclSDPlenoRefill || NULL == pclSDPlenoRefill->GetSSheetPointer() || NULL == m_pclIndSelPMParams 
			|| NULL == m_pclIndSelPMParams->m_pclSelectPMList || NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERT_RETURN;
	}
	
	CPMInputUser *pclInputUser = m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser();

	CSSheet *pclSSheet = pclSDPlenoRefill->GetSSheetPointer();
	pclSSheet->Init();
	pclSSheet->SetBool( SSB_ALLOWDRAGDROP, FALSE );
	pclSSheet->SetBool( SSB_HORZSCROLLBAR, FALSE );
	pclSSheet->SetBool( SSB_VERTSCROLLBAR, FALSE );
	pclSSheet->SetBool( SSB_MOVEACTIVEONFOCUS, FALSE );
	pclSSheet->SetTextTip( SS_TT_STATUS_FLOATING, 250, NULL, _LIGHTYELLOW, _TAH_BLACK );

	// Set max rows.
	pclSSheet->SetMaxRows( RD_TecBox_FirstAvailRow - 1 );

	// All cells are static by default and filled with _T("").
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	pclSSheet->FormatStaticText( -1, -1, -1, -1, _T( "" ) );

	// Increase row height.
	double dRowHeight = 12.75;
	pclSSheet->SetRowHeight( RD_TecBox_FirstRow, dRowHeight * 0.5 );
	pclSSheet->SetRowHeight( RD_TecBox_GroupName, dRowHeight * 1.5 );
	pclSSheet->SetRowHeight( RD_TecBox_ColName, dRowHeight * 2 );
	pclSSheet->SetRowHeight( RD_TecBox_ColUnit, dRowHeight * 1.2 );

	// Initialize.
	pclSDPlenoRefill->Init( 1, pclSSheet->GetMaxRows(), 1, CSheetDescription::SD_ParameterMode_Mono );
	pclSDPlenoRefill->AddColumnInPixels( CD_PlenoRefill_FirstColumn, m_mapSSheetColumnWidth[SD_PlenoRefill][CD_PlenoRefill_FirstColumn] );
	pclSDPlenoRefill->AddColumnInPixels( CD_PlenoRefill_CheckBox, m_mapSSheetColumnWidth[SD_PlenoRefill][CD_PlenoRefill_CheckBox] );

	// Show price index for Pleno only if there is no pressurisation (Pleno selected alone).
	if( true == TASApp.IsPMPriceIndexUsed() && false == pclInputUser->IsPressurisationSystemExist() )
	{
		pclSDPlenoRefill->AddColumnInPixels( CD_PlenoRefill_PriceIndexIcon, m_mapSSheetColumnWidth[SD_PlenoRefill][CD_PlenoRefill_PriceIndexIcon] );
		pclSDPlenoRefill->AddColumnInPixels( CD_PlenoRefill_PriceIndexValue, m_mapSSheetColumnWidth[SD_PlenoRefill][CD_PlenoRefill_PriceIndexValue] );
		pclSDPlenoRefill->AddColumnInPixels( CD_PlenoRefill_PriceIndexBorder, m_mapSSheetColumnWidth[SD_PlenoRefill][CD_PlenoRefill_PriceIndexBorder] );
	}

	pclSDPlenoRefill->AddColumnInPixels( CD_PlenoRefill_Name, m_mapSSheetColumnWidth[SD_PlenoRefill][CD_PlenoRefill_Name] );
	pclSDPlenoRefill->AddColumnInPixels( CD_PlenoRefill_NbreOfDevice, m_mapSSheetColumnWidth[SD_PlenoRefill][CD_PlenoRefill_NbreOfDevice] );
	pclSDPlenoRefill->AddColumnInPixels( CD_PlenoRefill_MaxPressure, m_mapSSheetColumnWidth[SD_PlenoRefill][CD_PlenoRefill_MaxPressure] );
	pclSDPlenoRefill->AddColumnInPixels( CD_PlenoRefill_TempRange, m_mapSSheetColumnWidth[SD_PlenoRefill][CD_PlenoRefill_TempRange] );
	pclSDPlenoRefill->AddColumnInPixels( CD_PlenoRefill_Functions, m_mapSSheetColumnWidth[SD_PlenoRefill][CD_PlenoRefill_Functions] );
	pclSDPlenoRefill->AddColumnInPixels( CD_PlenoRefill_Capacity, m_mapSSheetColumnWidth[SD_PlenoRefill][CD_PlenoRefill_Capacity] );
	pclSDPlenoRefill->AddColumnInPixels( CD_PlenoRefill_MaxVolume, m_mapSSheetColumnWidth[SD_PlenoRefill][CD_PlenoRefill_MaxVolume] );
	pclSDPlenoRefill->AddColumnInPixels( CD_PlenoRefill_MaxFlow, m_mapSSheetColumnWidth[SD_PlenoRefill][CD_PlenoRefill_MaxFlow] );
	pclSDPlenoRefill->AddColumnInPixels( CD_PlenoRefill_SuplyWaterPressureRange, m_mapSSheetColumnWidth[SD_PlenoRefill][CD_PlenoRefill_SuplyWaterPressureRange] );
	pclSDPlenoRefill->AddColumnInPixels( CD_PlenoRefill_InConnectAndSize, m_mapSSheetColumnWidth[SD_PlenoRefill][CD_PlenoRefill_InConnectAndSize] );
	pclSDPlenoRefill->AddColumnInPixels( CD_PlenoRefill_OutConnectAndSize, m_mapSSheetColumnWidth[SD_PlenoRefill][CD_PlenoRefill_OutConnectAndSize] );
	pclSDPlenoRefill->AddColumnInPixels( CD_PlenoRefill_Height, m_mapSSheetColumnWidth[SD_PlenoRefill][CD_PlenoRefill_Height] );
	pclSDPlenoRefill->AddColumnInPixels( CD_PlenoRefill_WidthWoConnect, m_mapSSheetColumnWidth[SD_PlenoRefill][CD_PlenoRefill_WidthWoConnect] );
	pclSDPlenoRefill->AddColumnInPixels( CD_PlenoRefill_WidthWConnect, m_mapSSheetColumnWidth[SD_PlenoRefill][CD_PlenoRefill_WidthWConnect] );
	pclSDPlenoRefill->AddColumnInPixels( CD_PlenoRefill_Weight, m_mapSSheetColumnWidth[SD_PlenoRefill][CD_PlenoRefill_Weight] );
	pclSDPlenoRefill->AddColumnInPixels( CD_PlenoRefill_Pointer, m_mapSSheetColumnWidth[SD_PlenoRefill][CD_PlenoRefill_Pointer] );
	
	// Must be placed after any call to 'AddColumnInPixels'. Because this method will add needed columns and TSpread will show them by default.
	// When calling the first time 'AddColumnInPixels' for the 'CD_PlenoRefill_Name' column, this method will call TSpread::SetMaxCols() to adapt the
	// number of columns. All the column before 'CD_PlenoRefill_Name' are set to a default width and are visible.
	if( false == TASApp.IsPMPriceIndexUsed() || true == pclInputUser->IsPressurisationSystemExist() )
	{
		pclSSheet->ShowCol( CD_PlenoRefill_PriceIndexIcon, FALSE );
		pclSSheet->ShowCol( CD_PlenoRefill_PriceIndexValue, FALSE );
		pclSSheet->ShowCol( CD_PlenoRefill_PriceIndexBorder, FALSE );
	}
	
	// Set in which column parameter must be saved.
	pclSDPlenoRefill->AddParameterColumn( CD_PlenoRefill_Pointer );

	// Set the focus column.
	pclSDPlenoRefill->SetActiveColumn( CD_PlenoRefill_FirstColumn + 1 );

	// Set range for selection.
	pclSDPlenoRefill->SetFocusColumnRange( CD_PlenoRefill_FirstColumn + 1, CD_PlenoRefill_Pointer - 1 );


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Add the main title.
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, _IMI_TITLE_GROUP3 );
	pclSDPlenoRefill->SetUserVariable( _SDUV_TITLEFORECOLOR, ( pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor ) ) );
	pclSDPlenoRefill->SetUserVariable( _SDUV_TITLEBACKCOLOR, ( pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor ) ) );

	pclSSheet->AddCellSpanW( CD_PlenoRefill_FirstColumn + 1, RD_TecBox_GroupName, CD_PlenoRefill_Pointer - ( CD_PlenoRefill_FirstColumn + 1 ), 1 );
	pclSSheet->SetStaticText( CD_PlenoRefill_FirstColumn + 1, RD_TecBox_GroupName, IDS_SSHEETSSELPM_PRGROUP );
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Column name.
	if( false == pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn ) )
	{
		return;
	}

	if( false == pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE ) )
	{
		return;
	}

	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap, (LPARAM)TRUE );

	// Show price index for Pleno refill only if there is no pressurisation.
	if( true == TASApp.IsPMPriceIndexUsed() && false == pclInputUser->IsPressurisationSystemExist() )
	{
		pclSSheet->AddCellSpan( CD_PlenoRefill_PriceIndexIcon, RD_TecBox_ColName, 3, 1 );
		pclSSheet->SetStaticText( CD_PlenoRefill_PriceIndexIcon, RD_TecBox_ColName, IDS_SSHEETSSELPM_PRICEINDEX );
	}

	pclSSheet->SetStaticText( CD_PlenoRefill_Name, RD_TecBox_ColName, IDS_SSHEETSSELPM_TFNAME );
	pclSSheet->SetStaticText( CD_PlenoRefill_NbreOfDevice, RD_TecBox_ColName, IDS_SSHEETSSELPM_TBNUMBEROFDEVICES );
	pclSSheet->SetStaticText( CD_PlenoRefill_MaxPressure, RD_TecBox_ColName, IDS_SSHEETSSELPM_CPPS );
	pclSSheet->SetStaticText( CD_PlenoRefill_TempRange, RD_TecBox_ColName, IDS_SSHEETSSELPM_VESSELTEMPRANGE );
	pclSSheet->SetStaticText( CD_PlenoRefill_Functions, RD_TecBox_ColName, IDS_SSHEETSSELPM_PRFUNCTIONS );
	pclSSheet->SetStaticText( CD_PlenoRefill_Capacity, RD_TecBox_ColName, IDS_SSHEETSSELPM_PRCAPACITY );
	pclSSheet->SetStaticText( CD_PlenoRefill_MaxVolume, RD_TecBox_ColName, IDS_SSHEETSSELPM_PRMAXVOL );
	pclSSheet->SetStaticText( CD_PlenoRefill_MaxFlow, RD_TecBox_ColName, IDS_SSHEETSSELPM_PRMAXFLOW );
	pclSSheet->SetStaticText( CD_PlenoRefill_SuplyWaterPressureRange, RD_TecBox_ColName, IDS_SSHEETSSELPM_PRSUPPLYWATERPRESSRANGE );
	pclSSheet->SetStaticText( CD_PlenoRefill_InConnectAndSize, RD_TecBox_ColName, IDS_SSHEETSSELPM_PRCONNECTIN );
	pclSSheet->SetStaticText( CD_PlenoRefill_OutConnectAndSize, RD_TecBox_ColName, IDS_SSHEETSSELPM_PRCONNECTOUT );
	pclSSheet->SetStaticText( CD_PlenoRefill_Height, RD_TecBox_ColName, IDS_SSHEETSSELPM_CPHEIGHT );
	pclSSheet->SetStaticText( CD_PlenoRefill_WidthWoConnect, RD_TecBox_ColName, IDS_SSHEETSSELPM_PRWIDTHWOCONNECT );
	pclSSheet->SetStaticText( CD_PlenoRefill_WidthWConnect, RD_TecBox_ColName, IDS_SSHEETSSELPM_PRWIDTHWCONNECT );
	pclSSheet->SetStaticText( CD_PlenoRefill_Weight, RD_TecBox_ColName, IDS_SSHEETSSELPM_CPMASS );
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Column units.
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap, (LPARAM)FALSE );

	// Show price index unit for Pleno only if there is no pressurisation (Pleno selected alone).
	if( true == TASApp.IsPMPriceIndexUsed() && false == pclInputUser->IsPressurisationSystemExist() )
	{
		pclSSheet->SetStaticText( CD_PlenoRefill_PriceIndexValue, RD_TecBox_ColUnit, _T("%") );
	}

	pclSSheet->SetStaticText( CD_PlenoRefill_MaxPressure, RD_TecBox_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_PRESSURE ) ).c_str() );
	pclSSheet->SetStaticText( CD_PlenoRefill_TempRange, RD_TecBox_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_TEMPERATURE ) ).c_str() );

	CString str = GetNameOf( pUnitDB->GetDefaultUnit( _U_VOLUME ) ).c_str() + CString( _T(" / ") );
	str += GetNameOf( pUnitDB->GetDefaultUnit( _U_WATERHARDNESS ) ).c_str();
	pclSSheet->SetStaticText( CD_PlenoRefill_Capacity, RD_TecBox_ColUnit, str );

	pclSSheet->SetStaticText( CD_PlenoRefill_MaxVolume, RD_TecBox_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_VOLUME ) ).c_str() );
	pclSSheet->SetStaticText( CD_PlenoRefill_MaxFlow, RD_TecBox_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_FLOW ) ).c_str() );
	pclSSheet->SetStaticText( CD_PlenoRefill_SuplyWaterPressureRange, RD_TecBox_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_PRESSURE ) ).c_str() );
	pclSSheet->SetStaticText( CD_PlenoRefill_Height, RD_TecBox_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_LENGTH ) ).c_str() );
	pclSSheet->SetStaticText( CD_PlenoRefill_WidthWoConnect, RD_TecBox_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_LENGTH ) ).c_str() );
	pclSSheet->SetStaticText( CD_PlenoRefill_WidthWConnect, RD_TecBox_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_LENGTH ) ).c_str() );
	pclSSheet->SetStaticText( CD_PlenoRefill_Weight, RD_TecBox_ColUnit, GetNameOf( pUnitDB->GetDefaultUnit( _U_MASS ) ).c_str() );
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Draw black line below column header.
	pclSSheet->SetCellBorder( CD_PlenoRefill_FirstColumn + 1, RD_TecBox_ColUnit, CD_PlenoRefill_Pointer - 1, RD_TecBox_ColUnit, true, SS_BORDERTYPE_BOTTOM );
}

long CRViewSSelPM::_FillPlenoRefillRow( CSheetDescription *pclSDPlenoRefill, long lRow, CSelectedPMBase *pclSelectedPlenoRefill, CPMInputUser *pclPMInputUser, bool bForRejectedProduct )
{
	if( NULL == pclSDPlenoRefill || NULL == pclSDPlenoRefill->GetSSheetPointer() || NULL == pclSelectedPlenoRefill || NULL == pclPMInputUser )
	{
		ASSERTA_RETURN( -1 );
	}

	bool bWTCombination = ( CSelectedPMBase::ST_WaterTreatmentCombination == pclSelectedPlenoRefill->GetSelectedType() ) ? true : false;

	CDB_PlenoRefill *pclPlenoRefill = dynamic_cast<CDB_PlenoRefill *>( pclSelectedPlenoRefill->GetpData() );

	if( NULL == pclPlenoRefill )
	{
		return -1;
	}

	CSSheet *pclSSheet = pclSDPlenoRefill->GetSSheetPointer();

	// Add this row.
	// 'true' to specify that this row can be selected.
	pclSDPlenoRefill->AddRows( 1, true );

	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

	// Add checkbox.
	pclSSheet->SetCheckBox( CD_PlenoRefill_CheckBox, lRow, _T(""), false, true);

	// Set name.
	pclSSheet->SetStaticText( CD_PlenoRefill_Name, lRow, pclPlenoRefill->GetName() );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, false );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

	bool fRedundancy = pclPMInputUser->GetPMWQSelectionPreferences()->IsChecked( CPMWQPrefs::GeneralRedundancyTecBox );
	bool fRedundancyDisabled = pclPMInputUser->GetPMWQSelectionPreferences()->IsDisabled( CPMWQPrefs::GeneralRedundancyTecBox );
	// HYS-1537 : Check if the preference is enabled
	pclSSheet->SetStaticText( CD_PlenoRefill_NbreOfDevice, lRow, ( false == fRedundancy || true == fRedundancyDisabled ) ? _T("1" ) : _T( "2") );

	pclSSheet->SetStaticText( CD_PlenoRefill_MaxPressure, lRow, WriteCUDouble( _U_PRESSURE, pclPlenoRefill->GetPmaxmax() ) );

	if( true == pclSelectedPlenoRefill->IsFlagSet( CSelectedPMBase::ePRefillWMUpMinTemp ) ||
		true == pclSelectedPlenoRefill->IsFlagSet( CSelectedPMBase::ePRefillWMUpMaxTemp ) )
	{
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_RED );
	}

	pclSSheet->SetStaticText( CD_PlenoRefill_TempRange, lRow, pclPlenoRefill->GetTempRange() );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

	pclSSheet->SetStaticText( CD_PlenoRefill_Functions, lRow, pclPlenoRefill->GetFunctionsStr() );
	pclSSheet->SetStaticText( CD_PlenoRefill_Capacity, lRow, pclPlenoRefill->GetCapacityStr() );

	// HYS-36: if we have the selection of a Pleno Refill alone (Without pressurisation system and thus without system volume
	//         and installed power) we take the best case of total hardness of the system to 0 °dH.
	double dMaxVolume = -1.0;

	if( false == pclPMInputUser->IsPressurisationSystemExist() )
	{
		dMaxVolume = pclPlenoRefill->GetCapacity() / pclPMInputUser->GetWaterMakeUpWaterHardness();
	}
	else
	{
		dMaxVolume = pclPlenoRefill->GetMaxVolume( pclPMInputUser->GetWaterMakeUpWaterHardness(), m_dTotalHardnessOfSystem );
	}

	CString str = ( -1.0 == dMaxVolume ) ? _T("-") : WriteCUDouble( _U_VOLUME, dMaxVolume );
	pclSSheet->SetStaticText( CD_PlenoRefill_MaxVolume, lRow, str );

	double dMaxFlow = pclPlenoRefill->GetMaxFlow();
	str = ( -1.0 == dMaxFlow ) ? _T("-") : WriteCUDouble( _U_FLOW, dMaxFlow );
	pclSSheet->SetStaticText( CD_PlenoRefill_MaxFlow, lRow, str );
	pclSSheet->SetStaticText( CD_PlenoRefill_SuplyWaterPressureRange, lRow, pclPlenoRefill->GetSupplyWaterPressureRangeStr() );
	pclSSheet->SetStaticText( CD_PlenoRefill_InConnectAndSize, lRow, pclPlenoRefill->GetInConnectSizeStr() );
	pclSSheet->SetStaticText( CD_PlenoRefill_OutConnectAndSize, lRow, pclPlenoRefill->GetOutConnectSizeStr() );
	pclSSheet->SetStaticText( CD_PlenoRefill_Height, lRow, WriteCUDouble( _U_LENGTH, pclPlenoRefill->GetHeight() ) );

	double dWidth = pclPlenoRefill->GetWidth( false );
	str = ( dWidth > 0.0 ) ? WriteCUDouble( _U_LENGTH, dWidth ) : _T("-");
	pclSSheet->SetStaticText( CD_PlenoRefill_WidthWoConnect, lRow, str );

	dWidth = pclPlenoRefill->GetWidth( true );
	str = ( dWidth > 0.0 ) ? WriteCUDouble( _U_LENGTH, dWidth ) : _T("-");
	pclSSheet->SetStaticText( CD_PlenoRefill_WidthWConnect, lRow, str );

	pclSSheet->SetStaticText( CD_PlenoRefill_Weight, lRow, WriteCUDouble( _U_MASS, pclPlenoRefill->GetWeight() ) );

	// Save parameter.
	CCellDescriptionProduct *pclCDProduct = CreateCellDescriptionProduct( pclSDPlenoRefill->GetFirstParameterColumn(), lRow, (LPARAM)pclPlenoRefill,
			pclSDPlenoRefill );

	if( NULL != pclCDProduct )
	{
		pclCDProduct->SetUserParam( (LPARAM)pclSelectedPlenoRefill );
	}

	pclSSheet->SetCellBorder( CD_PlenoRefill_FirstColumn + 1, lRow, CD_PlenoRefill_Pointer - 1, lRow, true, SS_BORDERTYPE_BOTTOM, 
			SS_BORDERSTYLE_SOLID, _GRAY );

	return lRow;
}

void CRViewSSelPM::_VerifyPlenoRefillAccessories( UINT uiSDGroupID )
{
	// HYS-1445: need a special verification for Pleno Refill with P BAR 4 and Statico.
	// Because Statico has no integrated water meter (As it's the case in Transfero and Compresso) and
	// other Pleno for Statico have all now a BrainCube with an integrated water meter.
	// But P BAR 4 has no integrated water meter. So, if user choices a Pleno Refill we must automatically select
	// the WM 24 accessory (That is a water meter). The water meter allows to prevent when the cardtridge 
	// in the Pleno Refill must be replaced.
	if( SD_ExpansionVessel != uiSDGroupID && SD_ExpansionVesselMembrane != uiSDGroupID )
	{
		return;
	}

	CDB_TBPlenoVento *pclTecBoxPlenoSelected = NULL;
	LPARAM lpParam = 0;

	if( false == GetTecBoxPlenoSelected( &pclTecBoxPlenoSelected, uiSDGroupID, NULL, &lpParam ) )
	{
		return;
	}

	if( NULL == pclTecBoxPlenoSelected )
	{
		return;
	}

	if( 0 == CString( pclTecBoxPlenoSelected->GetFamilyID() ).Compare( _T("TBXFAM_PLENO_PIX_CONN" ) ) 
			|| 0 == CString( pclTecBoxPlenoSelected->GetFamilyID() ).Compare( _T("TBXFAM_PLENO_PI_CONNECT" ) ) )
	{
		return;
	}

	vecCDCAccessoryList *pvecCDCAccessoryList = _GetvecCDAccessoryList( SD_PlenoRefillAccessory );

	if( NULL == pvecCDCAccessoryList )
	{
		return;
	}

	for( auto &iter : *pvecCDCAccessoryList )
	{
		if( 0 == CString( iter->GetAccessoryPointer()->GetIDPtr().ID ).Compare( _T("PR_ELECTWATERMETER") ) 
				|| 0 == CString( iter->GetAccessoryPointer()->GetIDPtr().ID ).Compare( _T("PR_ELECTWMPOWER") ) )
		{
			iter->SetCheckStatus( true );
			iter->SetEnableStatus( false );
			iter->ApplyInternalChange();
		}
	}
}

long CRViewSSelPM::_AddExcludedProductsLink( CSheetDescription *pclSheetDescription, long lRow, CSelectPMList::ProductType eProductType )
{
	if( NULL == m_pclIndSelPMParams || NULL == m_pclIndSelPMParams->m_pclSelectPMList )
	{
		ASSERTA_RETURN( lRow );
	}

	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() )
	{
		return lRow;
	}

	if( false == m_pclIndSelPMParams->m_pclSelectPMList->IsProductExist( eProductType, CSelectPMList::WL_Rejected ) )
	{
		return lRow;
	}

	CSSheet *pclSheet = pclSheetDescription->GetSSheetPointer();
	long lFromColumn;
	long lToColumn;
	pclSheetDescription->GetFocusColumnRange( lFromColumn, lToColumn );

	if( lToColumn - lFromColumn < 0 )
	{
		return lRow;
	}

	pclSheetDescription->AddRows( 1 );

	pclSheet->AddCellSpan( lFromColumn, lRow, lToColumn - lFromColumn + 1, 1 );

	pclSheet->SetTextPatternProperty( CSSheet::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_LEFT );
	pclSheet->SetStaticText( lFromColumn, lRow, TASApp.LoadLocalizedString( IDS_RVIEWSSELPM_REJECTEDROW ) );

	CreateCellDescriptionLink( pclSheetDescription->GetFirstParameterColumn(), lRow, pclSheetDescription, (LPARAM)eProductType );

	pclSheet->SetCellBorder( lFromColumn, lRow, lToColumn, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );

	return ++lRow;
}

void CRViewSSelPM::_GetCurrentSheetDescriptionAndGroupID( UINT *puiSheetDescriptionID, UINT *puiSDGroupID )
{
	if( NULL == puiSheetDescriptionID && NULL == puiSDGroupID )
	{
		return;
	}

	CDB_Vessel *pclVessel = NULL;
	CDB_TecBox *pclTecBox = NULL;
	CDB_TBPlenoVento *pclPlenoVento = NULL;
	*puiSheetDescriptionID = 0;
	*puiSDGroupID = -1;

	if( true == GetExpansionVesselSelected( &pclVessel ) )
	{
		*puiSheetDescriptionID = SD_ExpansionVessel;
		*puiSDGroupID = SD_ExpansionVessel;
	}
	else if( true == GetExpansionVesselMembraneSelected( &pclVessel ) )
	{
		*puiSheetDescriptionID = SD_ExpansionVesselMembrane;
		*puiSDGroupID = SD_ExpansionVesselMembrane;
	}
	else if( true == GetTecBoxCompressoSelected( &pclTecBox ) )
	{
		*puiSheetDescriptionID = SD_TecBoxCompresso;
		*puiSDGroupID = SD_TecBoxCompresso;
	}
	else if( true == GetTecBoxTransferoSelected( &pclTecBox ) )
	{
		*puiSheetDescriptionID = SD_TecBoxTransfero;
		*puiSDGroupID = SD_TecBoxTransfero;
	}
	else if( true == GetTecBoxVentoSelected( &pclPlenoVento, -1 ) )
	{
		*puiSheetDescriptionID = SD_TecBoxVento;
	}
	else if( true == GetTecBoxPlenoSelected( &pclPlenoVento, -1 ) )
	{
		*puiSheetDescriptionID = SD_TecBoxPleno;
	}
}

UINT CRViewSSelPM::_ConvertSD2SDG( UINT uiSDIDToConvert, UINT uiSDIDGroup )
{
	if( SD_TecBoxVento != uiSDIDToConvert && SD_TecBoxVentoAccessory != uiSDIDToConvert &&  SD_TecBoxPleno != uiSDIDToConvert 
			&& SD_TecBoxPlenoAccessory != uiSDIDToConvert )
	{
		return uiSDIDToConvert;
	}

	switch( uiSDIDGroup )
	{
		case SD_ExpansionVessel:
			uiSDIDToConvert |= SDG_ExpansionVessel;
			break;

		case SD_ExpansionVesselMembrane:
			uiSDIDToConvert |= SDG_ExpansionVesselMembrane;
			break;

		case SD_TecBoxCompresso:
			uiSDIDToConvert |= SDG_Compresso;
			break;

		case SD_TecBoxTransfero:
			uiSDIDToConvert |= SDG_Transfero;
			break;
	}

	return uiSDIDToConvert;
}

void CRViewSSelPM::_ClearPreviousSelectedProduct( UINT uiProductSheetDescriptionID, UINT uiProductSDGroupID )
{
	// This method allows to clear the selections (if exist) from the current sheet description clicked to the end of the group.
	CSheetDescription *pclSheetDescription = m_ViewDescription.GetFromSheetDescriptionID( _ConvertSD2SDG( uiProductSheetDescriptionID, uiProductSDGroupID ) );

	if( NULL == pclSheetDescription )
	{
		return;
	}

	UINT uiProductSDParentID = -1;
	bool bAllItemsInGroup = false;
	bool bAllItemsInParent = false;

	if( SD_ExpansionVessel == uiProductSheetDescriptionID || SD_ExpansionVesselMembrane == uiProductSheetDescriptionID 
			|| SD_TecBoxCompresso == uiProductSheetDescriptionID || SD_TecBoxTransfero == uiProductSheetDescriptionID )
	{
		// If we click on a vessel or a TecBox, we clear all items in the same group.
		bAllItemsInGroup = true;
	}
	else if( -1 != pclSheetDescription->GetParentSheetDescriptionID() )
	{
		// Otherwise we clear all sheets belonging at the same parent (and sub-parent if exist like accessories sheet).
		uiProductSDParentID = pclSheetDescription->GetParentSheetDescriptionID();
		bAllItemsInParent = true;
	}

	if( -1 == uiProductSDGroupID )
	{
		uiProductSDGroupID = pclSheetDescription->GetGroupSheetDescriptionID();
	}

	CSheetDescription *pclLoopSheetDescription = pclSheetDescription;
	CSheetDescription *pclNextSheetDescription;

	std::vector<UINT> vecParent = { uiProductSDParentID };

	while( NULL != pclLoopSheetDescription )
	{
		bool bFound = false;

		if( NULL != pclLoopSheetDescription->GetSSheetPointer() )
		{
			if( true == bAllItemsInGroup && uiProductSDGroupID == pclLoopSheetDescription->GetGroupSheetDescriptionID() )
			{
				bFound = true;
			}
			else if( true == bAllItemsInParent )
			{
				for( int iLoop = 0; iLoop < (int)vecParent.size(); iLoop++ )
				{
					if( vecParent[iLoop] == pclLoopSheetDescription->GetParentSheetDescriptionID() )
					{
						bFound = true;
					}
				}

				if( true == bFound )
				{
					bool fAddParent = true;
					
					for( int iLoop = 0; iLoop < (int)vecParent.size(); iLoop++ )
					{
						if( vecParent[iLoop] == pclLoopSheetDescription->GetSheetDescriptionID() )
						{
							fAddParent = false;
						}
					}

					if( true == fAddParent )
					{
						vecParent.push_back( pclLoopSheetDescription->GetSheetDescriptionID() );
					}
				}
			}
		}

		if( true == bFound )
		{
			UINT uiLoopSheetDescriptionID = GETWITHOUTGROUPID( pclLoopSheetDescription->GetSheetDescriptionID() );
			CSSheet *pclSSheet = pclLoopSheetDescription->GetSSheetPointer();

			// To be sure to clean all previous data concerning focus.
			KillCurrentFocus();

			// Unselect row where current selection is set (yellow light).
			// Remark: 'KillCurrentFocus' unselect only rows where focus is on (blue light).
			pclSSheet->UnSelectMultipleRows();

			if( SD_ExpansionVessel == uiProductSheetDescriptionID )
			{
				CDB_Vessel *pclVessel = NULL;
				long lRow = 0;
				GetExpansionVesselSelected( &pclVessel, &lRow );

				if( NULL != pclVessel )
				{
					pclSSheet->SetCheckBox( CD_ExpansionVessel_CheckBox, lRow, _T(""), false, true );
				}
			}
			else if( SD_ExpansionVesselMembrane == uiProductSheetDescriptionID )
			{
				CDB_Vessel *pclVessel = NULL;
				long lRow = 0;
				GetExpansionVesselMembraneSelected( &pclVessel, &lRow );

				if( NULL != pclVessel )
				{
					pclSSheet->SetCheckBox( CD_ExpansionVessel_CheckBox, lRow, _T(""), false, true );
				}
			}
			else if( SD_IntermediateVessel == uiProductSheetDescriptionID )
			{
				CDB_Vessel *pclVessel = NULL;
				long lRow = 0;
				GetIntermediateVesselSelected( &pclVessel, &lRow );

				if( NULL != pclVessel )
				{
					pclSSheet->SetCheckBox( CD_IntermVssl_CheckBox, lRow, _T(""), false, true );
				}
			}
			else if( SD_TecBoxCompresso == uiProductSheetDescriptionID )
			{
				CDB_TecBox *pclTecBox = NULL;
				long lRow = 0;
				GetTecBoxCompressoSelected( &pclTecBox, &lRow );

				if( NULL != pclTecBox )
				{
					pclSSheet->SetCheckBox( CD_TecBox_CheckBox, lRow, _T(""), false, true );
				}
			}
			else if( SD_TecBoxTransfero == uiProductSheetDescriptionID )
			{
				CDB_TecBox *pclTecBox = NULL;
				long lRow = 0;
				GetTecBoxTransferoSelected(&pclTecBox, &lRow);
			
				if( NULL != pclTecBox )
				{
					pclSSheet->SetCheckBox( CD_TecBox_CheckBox, lRow, _T(""), false, true );
				}
			}
			else if( SD_CompressoVessel == uiProductSheetDescriptionID )
			{
				CDB_Vessel *pclVessel = NULL;
				long lRow = 0;
				GetCompressoVesselSelected( &pclVessel, &lRow );
				
				if( NULL != pclVessel )
				{
					pclSSheet->SetCheckBox( CD_TecBoxVssl_CheckBox, lRow, _T(""), false, true );
				}
			}
			else if( SD_CompressoSecondaryVessel == uiProductSheetDescriptionID )
			{
				CDB_Vessel *pclVessel = NULL;
				long lRow = 0;
				GetCompressoSecondaryVesselSelected( &pclVessel, &lRow );
				
				if( NULL != pclVessel )
				{
					pclSSheet->SetCheckBox( CD_TecBoxSecVssl_CheckBox, lRow, _T(""), false, true );
				}
			}
			else if( SD_TransferoVessel == uiProductSheetDescriptionID )
			{
				CDB_Vessel *pclVessel = NULL;
				long lRow = 0;
				GetTransferoVesselSelected( &pclVessel, &lRow );
				
				if( NULL != pclVessel )
				{
					pclSSheet->SetCheckBox( CD_TecBoxVssl_CheckBox, lRow, _T(""), false, true );
				}
			}
			else if( SD_TecBoxVento == uiProductSheetDescriptionID )
			{
				CDB_TBPlenoVento *pclTecBoxVento = NULL;
				long lRow = -1;
				GetTecBoxVentoSelected( &pclTecBoxVento, uiProductSDGroupID, &lRow );
				
				if( NULL != pclTecBoxVento )
				{
					pclSSheet->SetCheckBox( CD_TecBoxVento_CheckBox, lRow, _T(""), false, true );
				}
			}
			else if( SD_TecBoxPleno == uiProductSheetDescriptionID )
			{
				CDB_TBPlenoVento *pclTecBoxPleno = NULL;
				long lRow = 0;
				long lpParam = NULL;
				GetTecBoxPlenoSelected( &pclTecBoxPleno, uiProductSDGroupID, &lRow, &lpParam );

				if( NULL != pclTecBoxPleno || NULL != lpParam )
				{
					pclSSheet->SetCheckBox( CD_TecBoxPleno_CheckBox, lRow, _T(""), false, true );
				}
			}
			else if( SD_PlenoWaterMakeUpProtection == uiProductSheetDescriptionID )
			{
				CDB_TBPlenoVento *pclTecBoxPleno = NULL;
				long lRow = 0;
				long lpParam = NULL;
				GetTecBoxPlenoWMProtectionSelected( &pclTecBoxPleno, uiProductSDGroupID, &lRow, &lpParam );

				if( NULL != pclTecBoxPleno || NULL != lpParam )
				{
					pclSSheet->SetCheckBox( CD_TecBoxPleno_CheckBox, lRow, _T(""), false, true );
				}
			}
			else if( SD_PlenoRefill == uiProductSheetDescriptionID )
			{
				CDB_PlenoRefill *pclPlenoRefill = NULL;
				long lRow = -1;
				GetPlenoRefillSelected( &pclPlenoRefill, uiProductSDGroupID, &lRow );

				if( NULL != pclPlenoRefill )
				{
					pclSSheet->SetCheckBox( CD_PlenoRefill_CheckBox, lRow, _T(""), false, true );
				}
			}

			// Reset the current pressure maintenance product selected.
			m_arlpSelectedUserParam[uiLoopSheetDescriptionID] = 0;
			SetCurrentPMSelected( uiLoopSheetDescriptionID, uiProductSDGroupID, NULL );

			// Delete Expand/Collapse rows button if exist.
			// Remark: 'm_mapButtonExpandCollapseRows[iSheetDescriptionID]' is set to NULL in method!
			if( NULL != m_mapButtonExpandCollapseRows[uiLoopSheetDescriptionID] )
			{
				DeleteExpandCollapseRowsButton( m_mapButtonExpandCollapseRows[uiLoopSheetDescriptionID], pclLoopSheetDescription );
			}

			// Show Show/Hide all priorities button if exist.
			if( NULL != m_mapButtonShowAllPriorities[uiLoopSheetDescriptionID] )
			{
				m_mapButtonShowAllPriorities[uiLoopSheetDescriptionID]->ApplyInternalChange();
			}

			// Clear accessories list linked to the products.
			std::vector<UINT> vecAcccessorySheetDescriptionIDList;
			_GetAccessorySheetDescriptionID( uiLoopSheetDescriptionID, vecAcccessorySheetDescriptionIDList );

			for( auto &iterAccessorySheetDescriptionID : vecAcccessorySheetDescriptionIDList )
			{
				if( true == m_ViewDescription.IsSheetDescriptionExist( _ConvertSD2SDG( iterAccessorySheetDescriptionID, uiProductSDGroupID ) ) )
				{
					if( NULL != _GetvecCDAccessoryList( iterAccessorySheetDescriptionID ) )
					{
						_GetvecCDAccessoryList( iterAccessorySheetDescriptionID )->clear();
					}

					// Remove accessory sheet linked to the pressure maintenance products.
					m_mapButtonExpandCollapseGroup[iterAccessorySheetDescriptionID] = NULL;
					m_ViewDescription.RemoveOneSheetDescription( _ConvertSD2SDG( iterAccessorySheetDescriptionID, uiProductSDGroupID ) );
				}
			}

			// To avoid to remove the sheet concerned by the product.
			pclNextSheetDescription = m_ViewDescription.GetNextSheetDescription( pclLoopSheetDescription );

			if( uiLoopSheetDescriptionID != uiProductSheetDescriptionID )
			{
				m_ViewDescription.RemoveOneSheetDescription( _ConvertSD2SDG( uiLoopSheetDescriptionID, uiProductSDGroupID ) );
			}
		}
		else
		{
			pclNextSheetDescription = m_ViewDescription.GetNextSheetDescription( pclLoopSheetDescription );
		}

		if( true == bAllItemsInGroup || true == bAllItemsInParent )
		{
			pclLoopSheetDescription = pclNextSheetDescription;
		}
		else
		{
			pclLoopSheetDescription = NULL;
		}
	}
}

void CRViewSSelPM::_GetAccessorySheetDescriptionID( UINT uiProductSheetDescriptionID, std::vector<UINT> &vecAccessorySheetDescriptionIDList )
{
	vecAccessorySheetDescriptionIDList.clear();

	if( SD_ExpansionVessel == uiProductSheetDescriptionID )
	{
		vecAccessorySheetDescriptionIDList.push_back( SD_ExpansionVesselAccessory );
	}
	else if( SD_ExpansionVesselMembrane == uiProductSheetDescriptionID )
	{
		vecAccessorySheetDescriptionIDList.push_back( SD_ExpansionVesselMembraneAccessory );
	}
	else if( SD_TecBoxCompresso == uiProductSheetDescriptionID )
	{
		vecAccessorySheetDescriptionIDList.push_back( SD_TecBoxCompressoAccessory );
	}
	else if( SD_CompressoVessel == uiProductSheetDescriptionID )
	{
		vecAccessorySheetDescriptionIDList.push_back( SD_CompressoVesselAccessory );
	}
	else if( SD_TecBoxTransfero == uiProductSheetDescriptionID )
	{
		vecAccessorySheetDescriptionIDList.push_back( SD_TecBoxTransferoAccessory );
		vecAccessorySheetDescriptionIDList.push_back( SD_TecBoxTransferoBufferVesselAccessory );
	}
	else if( SD_TransferoVessel == uiProductSheetDescriptionID )
	{
		vecAccessorySheetDescriptionIDList.push_back( SD_TransferoVesselAccessory );
	}
	else if( SD_IntermediateVessel == uiProductSheetDescriptionID )
	{
		vecAccessorySheetDescriptionIDList.push_back( SD_IntermediateVesselAcc );
	}
	else if( SD_TecBoxVento == uiProductSheetDescriptionID )
	{
		vecAccessorySheetDescriptionIDList.push_back( SD_TecBoxVentoAccessory );
	}
	else if( SD_TecBoxPleno == uiProductSheetDescriptionID || SD_PlenoWaterMakeUpProtection == uiProductSheetDescriptionID )
	{
		vecAccessorySheetDescriptionIDList.push_back( SD_TecBoxPlenoAccessory );
	}
	else if( SD_PlenoRefill == uiProductSheetDescriptionID )
	{
		vecAccessorySheetDescriptionIDList.push_back( SD_PlenoRefillAccessory );
	}
}

CRViewSSelPM::vecCDCAccessoryList *CRViewSSelPM::_GetvecCDAccessoryList( UINT uiSheetDescriptionID )
{
	vecCDCAccessoryList *pvecCDCAccessoryList = NULL;

	switch( uiSheetDescriptionID )
	{
		case SD_ExpansionVessel:
		case SD_ExpansionVesselAccessory:
			pvecCDCAccessoryList = &m_mapAccessoryList[SD_ExpansionVessel];
			break;

		case SD_ExpansionVesselMembrane:
		case SD_ExpansionVesselMembraneAccessory:
			pvecCDCAccessoryList = &m_mapAccessoryList[SD_ExpansionVesselMembrane];
			break;

		case SD_TecBoxCompresso:
		case SD_TecBoxCompressoAccessory:
			pvecCDCAccessoryList = &m_mapAccessoryList[SD_TecBoxCompresso];
			break;

		case SD_CompressoVessel:
		case SD_CompressoVesselAccessory:
			pvecCDCAccessoryList = &m_mapAccessoryList[SD_CompressoVessel];
			break;

		case SD_CompressoSecondaryVessel:
		case SD_CompressoSecondaryVesselAccessory:
			pvecCDCAccessoryList = &m_mapAccessoryList[SD_CompressoSecondaryVessel];
			break;

		case SD_TecBoxTransfero:
		case SD_TecBoxTransferoAccessory:
			pvecCDCAccessoryList = &m_mapAccessoryList[SD_TecBoxTransfero];
			break;

		case SD_TransferoVessel:
		case SD_TransferoVesselAccessory:
			pvecCDCAccessoryList = &m_mapAccessoryList[SD_TransferoVessel];
			break;

		case SD_IntermediateVessel:
		case SD_IntermediateVesselAcc:
			pvecCDCAccessoryList = &m_mapAccessoryList[SD_IntermediateVessel];
			break;

		case SD_TecBoxVento:
		case SD_TecBoxVentoAccessory:
			pvecCDCAccessoryList = &m_mapAccessoryList[SD_TecBoxVento];
			break;

		case SD_TecBoxPleno:
		case SD_TecBoxPlenoAccessory:
			pvecCDCAccessoryList = &m_mapAccessoryList[SD_TecBoxPleno];
			break;

		case SD_PlenoRefill:
		case SD_PlenoRefillAccessory:
			pvecCDCAccessoryList = &m_mapAccessoryList[SD_PlenoRefill];
			break;

		case SD_TecBoxTransferoBufferVesselAccessory:
			pvecCDCAccessoryList = &m_mapAccessoryList[SD_TecBoxTransferoBufferVesselAccessory];
			break;
	}

	return pvecCDCAccessoryList;
}

int CRViewSSelPM::_GetVesselSheetDescriptionID( UINT uiProductSheetDescriptionID )
{
	UINT iReturn = -1;

	if( SD_TecBoxCompresso == uiProductSheetDescriptionID )
	{
		iReturn = SD_CompressoVessel;
	}
	else if( SD_TecBoxTransfero == uiProductSheetDescriptionID )
	{
		iReturn = SD_TransferoVessel;
	}

	return iReturn;
}

UINT CRViewSSelPM::_GetAfterSheetDescriptionID( UINT uiNewSheetDescriptionID, UINT uiSDGroupID )
{
	UINT uiAfterSheetDescriptionID = 0xFFFFFFFF;

	switch( uiNewSheetDescriptionID )
	{
		case SD_ComputedData:
			uiAfterSheetDescriptionID = 0xFFFFFFFF;
			break;

		case SD_ExpansionVessel:
			if( true == m_ViewDescription.IsSheetDescriptionExist( SD_PriceIndex ) )
			{
				uiAfterSheetDescriptionID = SD_PriceIndex;
			}
			else
			{
				uiAfterSheetDescriptionID = SD_ComputedData;
			}
			break;

		case SD_ExpansionVesselAccessory:
			uiAfterSheetDescriptionID = SD_ExpansionVessel;
			break;

		case SD_ExpansionVesselMembrane:
			if( true == m_ViewDescription.IsSheetDescriptionExist( SD_ExpansionVesselAccessory ) )
			{
				uiAfterSheetDescriptionID = SD_ExpansionVesselAccessory;
			}
			else if( true == m_ViewDescription.IsSheetDescriptionExist( SD_ExpansionVessel ) )
			{
				uiAfterSheetDescriptionID = SD_ExpansionVessel;
			}
			else if( true == m_ViewDescription.IsSheetDescriptionExist( SD_PriceIndex ) )
			{
				uiAfterSheetDescriptionID = SD_PriceIndex;
			}
			else
			{
				uiAfterSheetDescriptionID = SD_ComputedData;
			}
			break;

		case SD_ExpansionVesselMembraneAccessory:
			uiAfterSheetDescriptionID = SD_ExpansionVesselMembrane;
			break;

		case SD_TecBoxCompressoAccessory:
			uiAfterSheetDescriptionID = SD_TecBoxCompresso;
			break;

		case SD_CompressoVessel:
			if( true == m_ViewDescription.IsSheetDescriptionExist( SD_TecBoxCompressoAccessory ) )
			{
				uiAfterSheetDescriptionID = SD_TecBoxCompressoAccessory;
			}
			else
			{
				uiAfterSheetDescriptionID = SD_TecBoxCompresso;
			}

			break;

		case SD_CompressoVesselAccessory:
			uiAfterSheetDescriptionID = SD_CompressoVessel;
			break;

		case SD_CompressoSecondaryVessel:
			if( true == m_ViewDescription.IsSheetDescriptionExist( SD_TecBoxCompressoAccessory ) )
			{
				uiAfterSheetDescriptionID = SD_TecBoxCompressoAccessory;
			}
			else
			{
				uiAfterSheetDescriptionID = SD_TecBoxCompresso;
			}

			break;

		case SD_CompressoSecondaryVesselAccessory:
			uiAfterSheetDescriptionID = SD_CompressoSecondaryVessel;
			break;

		case SD_TecBoxTransferoAccessory:
			uiAfterSheetDescriptionID = SD_TecBoxTransfero;
			break;
		
		case SD_TecBoxTransferoBufferVesselAccessory:

			uiAfterSheetDescriptionID = ( true == m_ViewDescription.IsSheetDescriptionExist( SD_TecBoxTransferoAccessory ) ) ? SD_TecBoxTransferoAccessory : SD_TecBoxTransfero;
			break;

		case SD_TransferoVesselAccessory:

			uiAfterSheetDescriptionID = SD_TransferoVessel;
			break;

		case SD_IntermediateVessel:
			switch( uiSDGroupID )
			{
				case SD_ExpansionVessel:
					if( true == m_ViewDescription.IsSheetDescriptionExist( SD_ExpansionVesselAccessory ) )
					{
						uiAfterSheetDescriptionID = SD_ExpansionVesselAccessory;
					}
					else
					{
						uiAfterSheetDescriptionID = SD_ExpansionVessel;
					}

					break;

				case SD_ExpansionVesselMembrane:
					if( true == m_ViewDescription.IsSheetDescriptionExist( SD_ExpansionVesselMembraneAccessory ) )
					{
						uiAfterSheetDescriptionID = SD_ExpansionVesselMembraneAccessory;
					}
					else
					{
						uiAfterSheetDescriptionID = SD_ExpansionVesselMembrane;
					}

					break;

				case SD_TecBoxCompresso:
					if( true == m_ViewDescription.IsSheetDescriptionExist( SD_CompressoSecondaryVesselAccessory ) )
					{
						uiAfterSheetDescriptionID = SD_CompressoSecondaryVesselAccessory;
					}
					else if( true == m_ViewDescription.IsSheetDescriptionExist( SD_CompressoSecondaryVessel ) )
					{
						uiAfterSheetDescriptionID = SD_CompressoSecondaryVessel;
					}
					else if( true == m_ViewDescription.IsSheetDescriptionExist( SD_CompressoVesselAccessory ) )
					{
						uiAfterSheetDescriptionID = SD_CompressoVesselAccessory;
					}
					else if( true == m_ViewDescription.IsSheetDescriptionExist( SD_CompressoVessel ) )
					{
						uiAfterSheetDescriptionID = SD_CompressoVessel;
					}
					else if( true == m_ViewDescription.IsSheetDescriptionExist( SD_TecBoxCompressoAccessory ) )
					{
						uiAfterSheetDescriptionID = SD_TecBoxCompressoAccessory;
					}
					else
					{
						uiAfterSheetDescriptionID = SD_TecBoxCompresso;
					}

					break;

				case SD_TecBoxTransfero:
					if( true == m_ViewDescription.IsSheetDescriptionExist( SD_TransferoVesselAccessory ) )
					{
						uiAfterSheetDescriptionID = SD_TransferoVesselAccessory;
					}
					else if( true == m_ViewDescription.IsSheetDescriptionExist( SD_TransferoVessel ) )
					{
						uiAfterSheetDescriptionID = SD_TransferoVessel;
					}
					else if( true == m_ViewDescription.IsSheetDescriptionExist( SD_TecBoxTransferoAccessory ) )
					{
						uiAfterSheetDescriptionID = SD_TecBoxTransferoAccessory;
					}
					else
					{
						uiAfterSheetDescriptionID = SD_TecBoxTransfero;
					}

					break;
			}

			break;

		case SD_IntermediateVesselAcc:
			uiAfterSheetDescriptionID = SD_IntermediateVessel;
			break;

		case SD_PlenoWaterMakeUpProtection:
		case SD_TecBoxVento:
		case SD_TecBoxPleno:
			if( SD_TecBoxPleno == uiNewSheetDescriptionID )
			{
				if( true == m_ViewDescription.IsSheetDescriptionExist( SD_TecBoxVentoAccessory ) )
				{
					uiAfterSheetDescriptionID = SD_TecBoxVentoAccessory;

					// Intentionally break here!
					break;
				}
				else if( true == m_ViewDescription.IsSheetDescriptionExist( SDG_ExpansionVessel + SD_TecBoxVento ) ||
						 true == m_ViewDescription.IsSheetDescriptionExist( SDG_ExpansionVesselMembrane + SD_TecBoxVento ) ||
						 true == m_ViewDescription.IsSheetDescriptionExist( SDG_Compresso + SD_TecBoxVento ) || 
						 true == m_ViewDescription.IsSheetDescriptionExist( SDG_Transfero + SD_TecBoxVento ) )
				{
					uiAfterSheetDescriptionID = SD_TecBoxVento;

					// Intentionally break here!
					break;
				}
			}
			else if( SD_PlenoWaterMakeUpProtection == uiNewSheetDescriptionID )
			{
				if( true == m_ViewDescription.IsSheetDescriptionExist( SD_TecBoxPlenoAccessory ) )
				{
					uiAfterSheetDescriptionID = SD_TecBoxPlenoAccessory;

					// Intentionally break here!
					break;
				}
				else
				{
					uiAfterSheetDescriptionID = SD_TecBoxPleno;

					// Intentionally break here!
					break;
				}
			}

			switch( uiSDGroupID )
			{
				case SD_ExpansionVessel:
					if( true == m_ViewDescription.IsSheetDescriptionExist( SD_IntermediateVesselAcc ) )
					{
						uiAfterSheetDescriptionID = SD_IntermediateVesselAcc;
					}
					else if( true == m_ViewDescription.IsSheetDescriptionExist( SD_IntermediateVessel ) )
					{
						uiAfterSheetDescriptionID = SD_IntermediateVessel;
					}
					else if( true == m_ViewDescription.IsSheetDescriptionExist( SD_ExpansionVesselAccessory ) )
					{
						uiAfterSheetDescriptionID = SD_ExpansionVesselAccessory;
					}
					else
					{
						uiAfterSheetDescriptionID = SD_ExpansionVessel;
					}

					break;

				case SD_ExpansionVesselMembrane:
					if( true == m_ViewDescription.IsSheetDescriptionExist( SD_IntermediateVesselAcc ) )
					{
						uiAfterSheetDescriptionID = SD_IntermediateVesselAcc;
					}
					else if( true == m_ViewDescription.IsSheetDescriptionExist( SD_IntermediateVessel ) )
					{
						uiAfterSheetDescriptionID = SD_IntermediateVessel;
					}
					else if( true == m_ViewDescription.IsSheetDescriptionExist( SD_ExpansionVesselMembraneAccessory ) )
					{
						uiAfterSheetDescriptionID = SD_ExpansionVesselMembraneAccessory;
					}
					else
					{
						uiAfterSheetDescriptionID = SD_ExpansionVesselMembrane;
					}

					break;

				case SD_TecBoxCompresso:
					if( true == m_ViewDescription.IsSheetDescriptionExist( SD_IntermediateVesselAcc ) )
					{
						uiAfterSheetDescriptionID = SD_IntermediateVesselAcc;
					}
					else if( true == m_ViewDescription.IsSheetDescriptionExist( SD_IntermediateVessel ) )
					{
						uiAfterSheetDescriptionID = SD_IntermediateVessel;
					}
					else if( true == m_ViewDescription.IsSheetDescriptionExist( SD_CompressoSecondaryVesselAccessory ) )
					{
						uiAfterSheetDescriptionID = SD_CompressoSecondaryVesselAccessory;
					}
					else if( true == m_ViewDescription.IsSheetDescriptionExist( SD_CompressoSecondaryVessel ) )
					{
						uiAfterSheetDescriptionID = SD_CompressoSecondaryVessel;
					}
					else if( true == m_ViewDescription.IsSheetDescriptionExist( SD_CompressoVesselAccessory ) )
					{
						uiAfterSheetDescriptionID = SD_CompressoVesselAccessory;
					}
					else if( true == m_ViewDescription.IsSheetDescriptionExist( SD_CompressoVessel ) )
					{
						uiAfterSheetDescriptionID = SD_CompressoVessel;
					}
					else if( true == m_ViewDescription.IsSheetDescriptionExist( SD_TecBoxCompressoAccessory ) )
					{
						uiAfterSheetDescriptionID = SD_TecBoxCompressoAccessory;
					}
					else
					{
						uiAfterSheetDescriptionID = SD_TecBoxCompresso;
					}

					break;

				case SD_TecBoxTransfero:
					if( true == m_ViewDescription.IsSheetDescriptionExist( SD_IntermediateVesselAcc ) )
					{
						uiAfterSheetDescriptionID = SD_IntermediateVesselAcc;
					}
					else if( true == m_ViewDescription.IsSheetDescriptionExist( SD_IntermediateVessel ) )
					{
						uiAfterSheetDescriptionID = SD_IntermediateVessel;
					}
					else if( true == m_ViewDescription.IsSheetDescriptionExist( SD_TransferoVesselAccessory ) )
					{
						uiAfterSheetDescriptionID = SD_TransferoVesselAccessory;
					}
					else if( true == m_ViewDescription.IsSheetDescriptionExist( SD_TransferoVessel ) )
					{
						uiAfterSheetDescriptionID = SD_TransferoVessel;
					}
					else if( true == m_ViewDescription.IsSheetDescriptionExist( SD_TecBoxTransferoAccessory ) )
					{
						uiAfterSheetDescriptionID = SD_TecBoxTransferoAccessory;
					}
					else
					{
						uiAfterSheetDescriptionID = SD_TecBoxTransfero;
					}

					break;
			}

			break;

		case SD_TecBoxVentoAccessory:
			uiAfterSheetDescriptionID = SD_TecBoxVento;
			break;

		case SD_TecBoxPlenoAccessory:
			uiAfterSheetDescriptionID = SD_TecBoxPleno;
			break;

		case SD_PlenoRefill:
			if( true == m_ViewDescription.IsSheetDescriptionExist( SDG_ExpansionVessel + SD_TecBoxPlenoAccessory ) ||
				true == m_ViewDescription.IsSheetDescriptionExist( SDG_ExpansionVesselMembrane + SD_TecBoxPlenoAccessory ) ||
				true == m_ViewDescription.IsSheetDescriptionExist( SDG_Compresso + SD_TecBoxPlenoAccessory ) || 
				true == m_ViewDescription.IsSheetDescriptionExist( SDG_Transfero + SD_TecBoxPlenoAccessory ) )
			{
				uiAfterSheetDescriptionID = SD_TecBoxPlenoAccessory;
			}
			else if( true == m_ViewDescription.IsSheetDescriptionExist( SD_PlenoWaterMakeUpProtection ) )
			{
				uiAfterSheetDescriptionID = SD_PlenoWaterMakeUpProtection;
			}
			else
			{
				uiAfterSheetDescriptionID = SD_TecBoxPleno;
			}
			break;

		case SD_PlenoRefillAccessory:
			uiAfterSheetDescriptionID = SD_PlenoRefill;
			break;
	}

	return uiAfterSheetDescriptionID;
}

void CRViewSSelPM::_CollapseOtherGroups( UINT uiSheetDescriptionID )
{
	CDB_Vessel *pclExpansionVessel;
	CDB_TecBox *pclTechBox;
	std::vector<UINT> vecSDToCollapse;

	if( SD_ExpansionVessel == uiSheetDescriptionID )
	{
		vecSDToCollapse.push_back( SD_ExpansionVesselMembrane );
		vecSDToCollapse.push_back( SD_TecBoxCompresso );
		vecSDToCollapse.push_back( SD_TecBoxTransfero );

		if( true == GetExpansionVesselMembraneSelected( &pclExpansionVessel ) && NULL != pclExpansionVessel )
		{
			_ClearPreviousSelectedProduct( SD_ExpansionVesselMembrane, -1 );
		}

		if( true == GetTecBoxCompressoSelected( &pclTechBox ) && NULL != pclTechBox )
		{
			_ClearPreviousSelectedProduct( SD_TecBoxCompresso, -1 );
		}

		if( true == GetTecBoxTransferoSelected( &pclTechBox ) && NULL != pclTechBox )
		{
			_ClearPreviousSelectedProduct( SD_TecBoxTransfero, -1 );
		}
	}
	else if( SD_ExpansionVesselMembrane == uiSheetDescriptionID )
	{
		vecSDToCollapse.push_back( SD_ExpansionVessel );
		vecSDToCollapse.push_back( SD_TecBoxCompresso );
		vecSDToCollapse.push_back( SD_TecBoxTransfero );

		if( true == GetExpansionVesselSelected( &pclExpansionVessel ) && NULL != pclExpansionVessel )
		{
			_ClearPreviousSelectedProduct( SD_ExpansionVessel, -1 );
		}

		if( true == GetTecBoxCompressoSelected( &pclTechBox ) && NULL != pclTechBox )
		{
			_ClearPreviousSelectedProduct( SD_TecBoxCompresso, -1 );
		}

		if( true == GetTecBoxTransferoSelected( &pclTechBox ) && NULL != pclTechBox )
		{
			_ClearPreviousSelectedProduct( SD_TecBoxTransfero, -1 );
		}
	}
	else if( SD_TecBoxCompresso == uiSheetDescriptionID )
	{
		vecSDToCollapse.push_back( SD_ExpansionVessel );
		vecSDToCollapse.push_back( SD_ExpansionVesselMembrane );
		vecSDToCollapse.push_back( SD_TecBoxTransfero );

		if( true == GetExpansionVesselSelected( &pclExpansionVessel ) )
		{
			_ClearPreviousSelectedProduct( SD_ExpansionVessel, -1 );
		}

		if( true == GetExpansionVesselMembraneSelected( &pclExpansionVessel ) && NULL != pclExpansionVessel )
		{
			_ClearPreviousSelectedProduct( SD_ExpansionVesselMembrane, -1 );
		}

		if( true == GetTecBoxTransferoSelected( &pclTechBox ) && NULL != pclTechBox )
		{
			_ClearPreviousSelectedProduct( SD_TecBoxTransfero, -1 );
		}
	}
	else if( SD_TecBoxTransfero == uiSheetDescriptionID )
	{
		vecSDToCollapse.push_back( SD_ExpansionVessel );
		vecSDToCollapse.push_back( SD_ExpansionVesselMembrane );
		vecSDToCollapse.push_back( SD_TecBoxCompresso );

		if( true == GetExpansionVesselSelected( &pclExpansionVessel ) )
		{
			_ClearPreviousSelectedProduct( SD_ExpansionVessel, -1 );
		}

		if( true == GetExpansionVesselMembraneSelected( &pclExpansionVessel ) && NULL != pclExpansionVessel )
		{
			_ClearPreviousSelectedProduct( SD_ExpansionVesselMembrane, -1 );
		}

		if( true == GetTecBoxCompressoSelected( &pclTechBox ) && NULL != pclTechBox )
		{
			_ClearPreviousSelectedProduct( SD_TecBoxCompresso, -1 );
		}
	}
	else
	{
		return;
	}

	// Run all groups.
	for( std::vector<UINT>::iterator iterGroups = vecSDToCollapse.begin(); iterGroups != vecSDToCollapse.end(); iterGroups++ )
	{
		if( 0 == m_mapButtonExpandCollapseGroup.count( *iterGroups ) || NULL == m_mapButtonExpandCollapseGroup[*iterGroups] )
		{
			continue;
		}

		CCDButtonExpandCollapseGroup *pclButton = m_mapButtonExpandCollapseGroup[*iterGroups];

		if( CCDButtonExpandCollapseGroup::ButtonState::CollapseRow == pclButton->GetButtonState() )
		{
			pclButton->ToggleButtonState( true );
		}
	}
}

CString CRViewSSelPM::_OnTextTipFetchTemperatureHelper( CSelectedPMBase *pclSelectedPM, CDB_Product *pclPMProduct, CPMInputUser *pclInputUser )
{
	if( NULL == pclSelectedPM || NULL == pclPMProduct || NULL == pclInputUser || false == pclSelectedPM->IsFlagSet( CSelectedPMBase::eTemperature ) )
	{
		return _T( "" );
	}

	CString str( _T( "" ) );
	CString strTemp1;
	CString strTemp2;
	CString strHelp;
	std::vector<CString> vecTempMsg;
	int iErrorCode = pclSelectedPM->GetFlagError( CSelectedPMBase::eTemperature );

	for( int iLoop = ( int )CSelectedPMBase::TEC_FirstError; iLoop <= ( int )CSelectedPMBase::TEC_LastError; )
	{
		if( CSelectedPMBase::TEC_SystemReturnTempBiggerTBMaxTemp == ( iErrorCode & iLoop ) )
		{
			// Return temperature of the system is above the max. admissible temperature of this device.
			strTemp1 = WriteCUDouble( _U_TEMPERATURE, pclInputUser->GetReturnTemperature(), true );
			strTemp2 = WriteCUDouble( _U_TEMPERATURE, pclPMProduct->GetTmax(), true );
			strHelp = strTemp1 + _T(" > ") + strTemp2;
			FormatString( str, IDS_RVIEWSSELPM_TTSYSRETTEMPBIGGERTBMAXTEMP, strHelp );
			vecTempMsg.push_back( str );
		}
		else if( CSelectedPMBase::TEC_SystemMinTempLowerTBMinTemp == ( iErrorCode & iLoop ) )
		{
			// Min. temperature that can be reached when the system is out of operation\r\nis below the min. admissible temperature of this device (%1).
			strTemp1 = WriteCUDouble( _U_TEMPERATURE, pclInputUser->GetMinTemperature(), true );
			strTemp2 = WriteCUDouble( _U_TEMPERATURE, pclPMProduct->GetTmin(), true );
			strHelp = strTemp1 + _T(" < ") + strTemp2;
			FormatString( str, IDS_RVIEWSSELPM_TTSYSMINTEMPLOWERTBMINTEMP, strHelp );
			vecTempMsg.push_back( str );
		}

		iLoop <<= 1;
	}

	str = _T( "" );
	bool bAddPrefix = ( 1 == ( int )vecTempMsg.size() ) ? false : true;

	for( int iLoop = 0; iLoop < ( int )vecTempMsg.size(); iLoop++ )
	{
		if( iLoop > 0 )
		{
			str += _T("\r\n");
		}

		if( true == bAddPrefix )
		{
			str += _T(" - ");
		}

		CString strToAdd = vecTempMsg[iLoop];

		if( true == bAddPrefix )
		{
			int iStart = 0;
			int iFind = 0;

			while( -1 != ( iFind = strToAdd.Find( _T("\r\n"), iStart ) ) )
			{
				iFind += 2;
				strToAdd.Insert( iFind, _T("   ") );
				iStart = iFind;
			}
		}

		str += strToAdd;
	}

	return str;
}

void CRViewSSelPM::_ReadAllColumnWidth( CDS_IndSelParameter *pclIndSelParameter )
{
	// By default and before reading registry saved column width force reset column width for all sheets.
	for( UINT ui = SD_First; ui < SD_Last; ui++ )
	{
		ResetColumnWidth( ui );
	}

	std::map<UINT, short> mapSDIDVersion;
	mapSDIDVersion[CW_RVIEWSSELPM_SHEETID_COMPUTEDATA] = CW_RVIEWSSELPM_COMPUTEDDATA_VERSION;
	mapSDIDVersion[CW_RVIEWSSELPM_SHEETID_PRICEINDEX] = CW_RVIEWSSELPM_PRICEINDEX_VERSION;
	mapSDIDVersion[CW_RVIEWSSELPM_SHEETID_EXPANSIONVESSEL] = CW_RVIEWSSELPM_EXPANSIONVESSEL_VERSION;
	mapSDIDVersion[CW_RVIEWSSELPM_SHEETID_EXPANSIONVESSELMEMBRANE] = CW_RVIEWSSELPM_EXPANSIONVESSELMEMBRANE_VERSION;
	mapSDIDVersion[CW_RVIEWSSELPM_SHEETID_TECBOXCOMPRESSO] = CW_RVIEWSSELPM_COMPRESSO_VERSION;
	mapSDIDVersion[CW_RVIEWSSELPM_SHEETID_TECBOXTRANSFERO] = CW_RVIEWSSELPM_TRANSFERO_VERSION;
	mapSDIDVersion[CW_RVIEWSSELPM_SHEETID_INTERMEDIATEVESSEL] = CW_RVIEWSSELPM_INTERMEDIATEVESSEL_VERSION;
	mapSDIDVersion[CW_RVIEWSSELPM_SHEETID_COMPRESSOVESSEL] = CW_RVIEWSSELPM_COMPRESSO_VERSION;
	mapSDIDVersion[CW_RVIEWSSELPM_SHEETID_COMPRESSOSECVESSEL] = CW_RVIEWSSELPM_COMPRESSO_VERSION;
	mapSDIDVersion[CW_RVIEWSSELPM_SHEETID_TRANSFEROVESSEL] = CW_RVIEWSSELPM_TRANSFERO_VERSION;
	mapSDIDVersion[CW_RVIEWSSELPM_SHEETID_TECBOXPLENO] = CW_RVIEWSSELPM_PLENO_VERSION;
	mapSDIDVersion[CW_RVIEWSSELPM_SHEETID_TECBOXVENTO] = CW_RVIEWSSELPM_VENTO_VERSION;
	mapSDIDVersion[CW_RVIEWSSELPM_SHEETID_PLENOREFILL] = CW_RVIEWSSELPM_PLENO_VERSION;

	// Container window sheet ID to sheetdescription of this rightview.
	std::map<UINT, short> mapCWtoRW;
	mapCWtoRW[CW_RVIEWSSELPM_SHEETID_COMPUTEDATA] = SD_ComputedData;
	mapCWtoRW[CW_RVIEWSSELPM_SHEETID_PRICEINDEX] = SD_PriceIndex;
	mapCWtoRW[CW_RVIEWSSELPM_SHEETID_EXPANSIONVESSEL] = SD_ExpansionVessel;
	mapCWtoRW[CW_RVIEWSSELPM_SHEETID_EXPANSIONVESSELMEMBRANE] = SD_ExpansionVesselMembrane;
	mapCWtoRW[CW_RVIEWSSELPM_SHEETID_TECBOXCOMPRESSO] = SD_TecBoxCompresso;
	mapCWtoRW[CW_RVIEWSSELPM_SHEETID_TECBOXTRANSFERO] = SD_TecBoxTransfero;
	mapCWtoRW[CW_RVIEWSSELPM_SHEETID_INTERMEDIATEVESSEL] = SD_IntermediateVessel;
	mapCWtoRW[CW_RVIEWSSELPM_SHEETID_COMPRESSOVESSEL] = SD_CompressoVessel;
	mapCWtoRW[CW_RVIEWSSELPM_SHEETID_COMPRESSOSECVESSEL] = SD_CompressoSecondaryVessel;
	mapCWtoRW[CW_RVIEWSSELPM_SHEETID_TRANSFEROVESSEL] = SD_TransferoVessel;
	mapCWtoRW[CW_RVIEWSSELPM_SHEETID_TECBOXPLENO] = SD_TecBoxPleno;
	mapCWtoRW[CW_RVIEWSSELPM_SHEETID_TECBOXVENTO] = SD_TecBoxVento;
	mapCWtoRW[CW_RVIEWSSELPM_SHEETID_PLENOREFILL] = SD_PlenoRefill;
	
	// Access to the 'RViewSSelPM' window.
	CDS_IndSelParameter::CCWWindow *pclCWWindow = pclIndSelParameter->GetWindowColumnWidth( CW_WINDOWID_INDSELPRESSUREMAINTENANCE, true );

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

void CRViewSSelPM::_WriteAllColumnWidth( CDS_IndSelParameter *pclIndSelParameter )
{
	std::map<UINT, short> mapSDIDVersion;
	mapSDIDVersion[CW_RVIEWSSELPM_SHEETID_COMPUTEDATA] = CW_RVIEWSSELPM_COMPUTEDDATA_VERSION;
	mapSDIDVersion[CW_RVIEWSSELPM_SHEETID_PRICEINDEX] = CW_RVIEWSSELPM_PRICEINDEX_VERSION;
	mapSDIDVersion[CW_RVIEWSSELPM_SHEETID_EXPANSIONVESSEL] = CW_RVIEWSSELPM_EXPANSIONVESSEL_VERSION;
	mapSDIDVersion[CW_RVIEWSSELPM_SHEETID_EXPANSIONVESSELMEMBRANE] = CW_RVIEWSSELPM_EXPANSIONVESSELMEMBRANE_VERSION;
	mapSDIDVersion[CW_RVIEWSSELPM_SHEETID_TECBOXCOMPRESSO] = CW_RVIEWSSELPM_COMPRESSO_VERSION;
	mapSDIDVersion[CW_RVIEWSSELPM_SHEETID_TECBOXTRANSFERO] = CW_RVIEWSSELPM_TRANSFERO_VERSION;
	mapSDIDVersion[CW_RVIEWSSELPM_SHEETID_INTERMEDIATEVESSEL] = CW_RVIEWSSELPM_INTERMEDIATEVESSEL_VERSION;
	mapSDIDVersion[CW_RVIEWSSELPM_SHEETID_COMPRESSOVESSEL] = CW_RVIEWSSELPM_COMPRESSO_VERSION;
	mapSDIDVersion[CW_RVIEWSSELPM_SHEETID_COMPRESSOSECVESSEL] = CW_RVIEWSSELPM_COMPRESSO_VERSION;
	mapSDIDVersion[CW_RVIEWSSELPM_SHEETID_TRANSFEROVESSEL] = CW_RVIEWSSELPM_TRANSFERO_VERSION;
	mapSDIDVersion[CW_RVIEWSSELPM_SHEETID_TECBOXPLENO] = CW_RVIEWSSELPM_PLENO_VERSION;
	mapSDIDVersion[CW_RVIEWSSELPM_SHEETID_TECBOXVENTO] = CW_RVIEWSSELPM_VENTO_VERSION;
	mapSDIDVersion[CW_RVIEWSSELPM_SHEETID_PLENOREFILL] = CW_RVIEWSSELPM_PLENO_VERSION;

	// Container window sheet ID to sheetdescription of this rightview.
	std::map<UINT, short> mapCWtoRW;
	mapCWtoRW[CW_RVIEWSSELPM_SHEETID_COMPUTEDATA] = SD_ComputedData;
	mapCWtoRW[CW_RVIEWSSELPM_SHEETID_PRICEINDEX] = SD_PriceIndex;
	mapCWtoRW[CW_RVIEWSSELPM_SHEETID_EXPANSIONVESSEL] = SD_ExpansionVessel;
	mapCWtoRW[CW_RVIEWSSELPM_SHEETID_EXPANSIONVESSELMEMBRANE] = SD_ExpansionVesselMembrane;
	mapCWtoRW[CW_RVIEWSSELPM_SHEETID_TECBOXCOMPRESSO] = SD_TecBoxCompresso;
	mapCWtoRW[CW_RVIEWSSELPM_SHEETID_TECBOXTRANSFERO] = SD_TecBoxTransfero;
	mapCWtoRW[CW_RVIEWSSELPM_SHEETID_INTERMEDIATEVESSEL] = SD_IntermediateVessel;
	mapCWtoRW[CW_RVIEWSSELPM_SHEETID_COMPRESSOVESSEL] = SD_CompressoVessel;
	mapCWtoRW[CW_RVIEWSSELPM_SHEETID_COMPRESSOSECVESSEL] = SD_CompressoSecondaryVessel;
	mapCWtoRW[CW_RVIEWSSELPM_SHEETID_TRANSFEROVESSEL] = SD_TransferoVessel;
	mapCWtoRW[CW_RVIEWSSELPM_SHEETID_TECBOXPLENO] = SD_TecBoxPleno;
	mapCWtoRW[CW_RVIEWSSELPM_SHEETID_TECBOXVENTO] = SD_TecBoxVento;
	mapCWtoRW[CW_RVIEWSSELPM_SHEETID_PLENOREFILL] = SD_PlenoRefill;

	// Access to the 'RViewSSelPM' window.
	CDS_IndSelParameter::CCWWindow *pclCWWindow = pclIndSelParameter->GetWindowColumnWidth( CW_WINDOWID_INDSELPRESSUREMAINTENANCE, true );

	for( std::map<UINT, short>::iterator iter = mapSDIDVersion.begin(); iter != mapSDIDVersion.end(); iter++ )
	{
		CDS_IndSelParameter::CCWSheet *pclCWSheet = pclCWWindow->GetCWSheet( iter->first, true );
		pclCWSheet->SetVersion( iter->second );
		pclCWSheet->GetMap() = m_mapSSheetColumnWidth[mapCWtoRW[iter->first]];
	}
}

void CRViewSSelPM::_SetPMGraphsOutputContext( bool bShowGraphs )
{
	CToolsDockablePane *pToolsDockablePane = pMainFrame->GetpToolsDockablePane();

	if( NULL == pToolsDockablePane || NULL == pToolsDockablePane->GetSafeHwnd() )
	{
		return;
	}

	CDlgPMGraphsOutput::CPMInterface clOutputInterface;
	int iContextID;

	if( NULL == pDlgLeftTabSelManager || false == pDlgLeftTabSelManager->GetToolsDockablePaneContextID( ProductSubCategory::PSC_PM_ExpansionVessel, iContextID ) )
	{
		return;
	}

	CToolsDockablePane::ContextParameters *pclContextParameters = pToolsDockablePane->GetContextByID( iContextID );

	if( NULL == pclContextParameters )
	{
		return;
	}

	long lCurrentHeight;

	if( true == clOutputInterface.GetCurrentHeight( &lCurrentHeight ) )
	{
		pToolsDockablePane->ChangeDlgOutputHeight( pclContextParameters->m_iContextID, lCurrentHeight );
	}

	if( true == bShowGraphs && TRUE == pclContextParameters->m_bIsPaneAutoHide )
	{
		pToolsDockablePane->Slide( TRUE, TRUE );
	}
}

void CRViewSSelPM::_UpdateCurrentSelectionPriceIndex( void )
{
	if( NULL == m_pclIndSelPMParams || NULL == m_pclIndSelPMParams->m_pTADB || NULL == m_pclIndSelPMParams->m_pclSelectPMList 
			|| NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERT_RETURN;
	}

	if( false == m_ViewDescription.IsSheetDescriptionExist( SD_PriceIndex ) || NULL == m_ViewDescription.GetFromSheetDescriptionID( SD_PriceIndex ) 
			|| NULL == m_ViewDescription.GetFromSheetDescriptionID( SD_PriceIndex )->GetSSheetPointer() )
	{
		return;
	}

	CPMInputUser *pclInputUser = m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser();

	_ResetCurrentSelectionPriceIndex();

	int iColumnID = CD_PriceIndex_ExpansionVessel;
	CSelectPMList::BestPriceIndexSolution eBPISolution;
	double dTotalPrice = 0.0;

	// If an expansion vessel is selected...
	if( 0 != m_arlpSelectedUserParam[SD_ExpansionVessel] )
	{
		// We can directly update the price index of the current selection.
		CDB_Vessel *pclExpansionVessel = NULL;

		if( true == GetExpansionVesselSelected( &pclExpansionVessel ) && NULL != pclExpansionVessel )
		{
			CSelectedVssl *pclSelectedVessel = dynamic_cast<CSelectedVssl *>( (CSelectedPMBase*)m_arlpSelectedUserParam[SD_ExpansionVessel] );

			if( NULL != pclSelectedVessel )
			{
				double dPrice = m_pclIndSelPMParams->m_pTADB->GetPrice( pclExpansionVessel->GetArtNum( true ) );

				if( dPrice  > 0.0 && pclSelectedVessel->GetNbreOfVsslNeeded() > 0 )
				{
					dTotalPrice += dPrice * pclSelectedVessel->GetNbreOfVsslNeeded();
				}
			}
		}

		eBPISolution = CSelectPMList::BPIS_ExpansionVessel;
	}
	else if( true == m_pclIndSelPMParams->m_bIsVesselMembraneTypeExistInPMTypeCombo && 0 != m_arlpSelectedUserParam[SD_ExpansionVesselMembrane] )
	{
		// We can directly update the price index of the current selection.
		CDB_Vessel *pclExpansionVesselMembrane = NULL;

		if( true == GetExpansionVesselMembraneSelected( &pclExpansionVesselMembrane ) && NULL != pclExpansionVesselMembrane )
		{
			CSelectedVssl *pclSelectedVessel = dynamic_cast<CSelectedVssl *>( (CSelectedPMBase*)m_arlpSelectedUserParam[SD_ExpansionVesselMembrane] );

			if( NULL != pclSelectedVessel )
			{
				double dPrice = m_pclIndSelPMParams->m_pTADB->GetPrice( pclExpansionVesselMembrane->GetArtNum( true ) );

				if( dPrice > 0.0 && pclSelectedVessel->GetNbreOfVsslNeeded() > 0 )
				{
					dTotalPrice += dPrice * pclSelectedVessel->GetNbreOfVsslNeeded();
				}
			}
		}

		eBPISolution = CSelectPMList::BPIS_ExpansionVesselMembrane;
	}
	else if( 0 != m_arlpSelectedUserParam[SD_TecBoxCompresso] && 0 != m_arlpSelectedUserParam[SD_CompressoVessel] )
	{
		// Price of the TecBox.
		CDB_TecBox *pclTecBoxCompresso = NULL;

		if( true == GetTecBoxCompressoSelected( &pclTecBoxCompresso ) && NULL != pclTecBoxCompresso )
		{
			double dPrice = m_pclIndSelPMParams->m_pTADB->GetPrice( pclTecBoxCompresso->GetArtNum( true ) );

			if( dPrice > 0.0 )
			{
				dTotalPrice += dPrice;
			}
		}

		// Price of the primary vessel.
		CDB_Vessel *pclCompressoVessel = NULL;

		if( true == GetCompressoVesselSelected( &pclCompressoVessel ) && NULL != pclCompressoVessel )
		{
			double dPrice = m_pclIndSelPMParams->m_pTADB->GetPrice( pclCompressoVessel->GetArtNum( true ) );

			if( dPrice > 0.0 )
			{
				dTotalPrice += dPrice;
			}
		}

		// Price of the secondary vessel.
		CSelectedVssl *pclSelectedVessel = dynamic_cast<CSelectedVssl *>( (CSelectedPMBase*)m_arlpSelectedUserParam[SD_CompressoVessel] );

		if( NULL != pclSelectedVessel && pclSelectedVessel->GetNbreOfVsslNeeded() > 1 && NULL != pclCompressoVessel 
			&& NULL != pclCompressoVessel->GetAssociatedSecondaryVessel() )
		{
			double dPrice = m_pclIndSelPMParams->m_pTADB->GetPrice( pclCompressoVessel->GetAssociatedSecondaryVessel()->GetArtNum( true ) );

			if( dPrice > 0.0 )
			{
				dTotalPrice += ( dPrice * ( pclSelectedVessel->GetNbreOfVsslNeeded() - 1 ) );
			}
		}

		eBPISolution = CSelectPMList::BPIS_Compresso;
		iColumnID = CD_PriceIndex_Compresso;
	}
	else if( 0 != m_arlpSelectedUserParam[SD_TecBoxTransfero] && 0 != m_arlpSelectedUserParam[SD_TransferoVessel] )
	{
		// Price of the TecBox.
		CDB_TecBox *pclTecBoxTransfero = NULL;

		if( true == GetTecBoxTransferoSelected( &pclTecBoxTransfero ) && NULL != pclTecBoxTransfero )
		{
			double dPrice = m_pclIndSelPMParams->m_pTADB->GetPrice( pclTecBoxTransfero->GetArtNum( true ) );

			if( dPrice > 0.0 )
			{
				dTotalPrice += dPrice;
			}
		}

		// Price of the primary vessel.
		CDB_Vessel *pclTransferoVessel = NULL;

		if( true == GetTransferoVesselSelected( &pclTransferoVessel ) && NULL != pclTransferoVessel )
		{
			double dPrice = m_pclIndSelPMParams->m_pTADB->GetPrice( pclTransferoVessel->GetArtNum( true ) );

			if( dPrice > 0.0 )
			{
				dTotalPrice += dPrice;
			}
		}

		// Price of the secondary vessel.
		CSelectedVssl *pclSelectedVessel = dynamic_cast<CSelectedVssl *>( (CSelectedPMBase*)m_arlpSelectedUserParam[SD_TransferoVessel] );

		if( NULL != pclSelectedVessel && pclSelectedVessel->GetNbreOfVsslNeeded() > 1 && NULL != pclTransferoVessel 
			&& NULL != pclTransferoVessel->GetAssociatedSecondaryVessel() )
		{
			double dPrice = m_pclIndSelPMParams->m_pTADB->GetPrice( pclTransferoVessel->GetAssociatedSecondaryVessel()->GetArtNum( true ) );

			if( dPrice > 0.0 )
			{
				dTotalPrice += ( dPrice * ( pclSelectedVessel->GetNbreOfVsslNeeded() - 1 ) );
			}
		}

		eBPISolution = CSelectPMList::BPIS_Transfero;
		iColumnID = CD_PriceIndex_Transfero;
	}
	
	// Check the total price of the current selection.
	
	if( dTotalPrice <= 0.0 )
	{
		return;
	}

	double dSolutionLowestPrice = DBL_MAX;

	if( true == m_bIsPMTypeAll )
	{
		if( dSolutionLowestPrice > m_pclIndSelPMParams->m_pclSelectPMList->GetBPILowestPrice( CSelectPMList::BPIS_ExpansionVessel ) )
		{
			dSolutionLowestPrice = m_pclIndSelPMParams->m_pclSelectPMList->GetBPILowestPrice( CSelectPMList::BPIS_ExpansionVessel );
		}

		if( true == m_pclIndSelPMParams->m_bIsVesselMembraneTypeExistInPMTypeCombo 
			&& dSolutionLowestPrice > m_pclIndSelPMParams->m_pclSelectPMList->GetBPILowestPrice( CSelectPMList::BPIS_ExpansionVesselMembrane ) )
		{
			dSolutionLowestPrice = m_pclIndSelPMParams->m_pclSelectPMList->GetBPILowestPrice( CSelectPMList::BPIS_ExpansionVesselMembrane );
		}
		
		if( dSolutionLowestPrice > m_pclIndSelPMParams->m_pclSelectPMList->GetBPILowestPrice( CSelectPMList::BPIS_Compresso ) )
		{
			dSolutionLowestPrice = m_pclIndSelPMParams->m_pclSelectPMList->GetBPILowestPrice( CSelectPMList::BPIS_Compresso );
		}
		
		if( dSolutionLowestPrice > m_pclIndSelPMParams->m_pclSelectPMList->GetBPILowestPrice( CSelectPMList::BPIS_Transfero ) )
		{
			dSolutionLowestPrice = m_pclIndSelPMParams->m_pclSelectPMList->GetBPILowestPrice( CSelectPMList::BPIS_Transfero );
		}
	}
	else
	{
		dSolutionLowestPrice = m_pclIndSelPMParams->m_pclSelectPMList->GetBPILowestPrice( eBPISolution );
	}

	double dCurrentSelection = dTotalPrice / dSolutionLowestPrice * 100.0;

	CString str;
	str.Format( _T("%i"), (int)dCurrentSelection );

	CSSheet *pclSSheet = m_ViewDescription.GetFromSheetDescriptionID( SD_PriceIndex )->GetSSheetPointer();
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

	pclSSheet->SetStaticText( iColumnID, RD_PriceIndex_CurrentSelection, str );
}

void CRViewSSelPM::_ResetCurrentSelectionPriceIndex( void )
{
	if( false == m_ViewDescription.IsSheetDescriptionExist( SD_PriceIndex ) || NULL == m_ViewDescription.GetFromSheetDescriptionID( SD_PriceIndex )
			|| NULL == m_ViewDescription.GetFromSheetDescriptionID( SD_PriceIndex )->GetSSheetPointer() )
	{
		return;
	}

	CSSheet *pclSSheet = m_ViewDescription.GetFromSheetDescriptionID( SD_PriceIndex )->GetSSheetPointer();
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
	pclSSheet->SetStaticText( CD_PriceIndex_ExpansionVessel, RD_PriceIndex_CurrentSelection, _T("-") );
	pclSSheet->SetStaticText( CD_PriceIndex_Compresso, RD_PriceIndex_CurrentSelection, _T("-") );
	pclSSheet->SetStaticText( CD_PriceIndex_Transfero, RD_PriceIndex_CurrentSelection, _T("-") );
}

bool CRViewSSelPM::_IsAtLeastOneSelectedDeviceHasVacuumDegassingFunction( UINT uiSDGroupID )
{
	bool bIsAtLeastOne = false;

	// Check first the Compresso (Vessels have no vacuum degassing integrated).
	CDB_TecBox *pclTechBox = NULL;

	if( true == GetTecBoxCompressoSelected( &pclTechBox ) )
	{
		if( true == pclTechBox->IsFctDegassing() )
		{
			bIsAtLeastOne = true;
		}
	}

	if( false == bIsAtLeastOne )
	{
		if( true == GetTecBoxTransferoSelected( &pclTechBox ) )
		{
			if( true == pclTechBox->IsFctDegassing() )
			{
				bIsAtLeastOne = true;
			}
		}
	}

	if( false == bIsAtLeastOne )
	{
		CDB_TBPlenoVento *pclTecBoxVentoSelected = NULL;

		if( true == GetTecBoxVentoSelected( &pclTecBoxVentoSelected, uiSDGroupID ) )
		{
			if( true == pclTecBoxVentoSelected->IsFctDegassing() )
			{
				bIsAtLeastOne = true;
			}
		}
	}

	if( false == bIsAtLeastOne )
	{
		CDB_TBPlenoVento *pclTecBoxPlenoSelected = NULL;

		if( true == GetTecBoxPlenoSelected( &pclTecBoxPlenoSelected, uiSDGroupID ) )
		{
			if( true == pclTecBoxPlenoSelected->IsFctDegassing() )
			{
				bIsAtLeastOne = true;
			}
		}
	}
	
	return bIsAtLeastOne;
}

CString CRViewSSelPM::_GetSumOfIndexes( int iStartIndex, int iEndIndex, CString strPrefix )
{
	CString str;
	str.Format( _T("(%s%i"), strPrefix, iStartIndex );

	for( int iLoopIndex = iStartIndex + 1; iLoopIndex < iEndIndex; iLoopIndex++ )
	{
		str.Format( _T("%s+%i"), str, iLoopIndex );
	}

	str += _T(")");
	return str;
}