#include "stdafx.h"
#include "afxctl.h"

#include "TASelect.h"
#include "MainFrm.h"
#include "DataStruct.h"
#include "Hydronic.h"

#include "DlgLeftTabSelManager.h"
#include "DlgInfoSSelDpC.h"

#include "RViewSSelSS.h"
#include "RViewSSelDpC.h"

CRViewSSelDpC *pRViewSSelDpC = NULL;
CRViewSSelDpC::CRViewSSelDpC() : CRViewSSelSS( CMainFrame::RightViewList::eRVSSelDpC, false )
{
	m_pclIndSelDpCParams = NULL;
	m_pclDlgInfoSSelDpC = NULL;
	_Init();
	pRViewSSelDpC = this;
}

CRViewSSelDpC::~CRViewSSelDpC()
{
	pRViewSSelDpC = NULL;
}

void CRViewSSelDpC::Reset()
{
	if( NULL != m_pclDlgInfoSSelDpC && NULL != m_pclDlgInfoSSelDpC->GetSafeHwnd() )
	{
		m_pclDlgInfoSSelDpC->Reset();
	}

	_Init();
	CRViewSSelSS::Reset();
}

void CRViewSSelDpC::Suggest( CProductSelelectionParameters *pclProductSelectionParameters, LPARAM lpParam )
{
	CRViewSSelSS::Suggest( pclProductSelectionParameters, lpParam );

	if( NULL == pclProductSelectionParameters || NULL == dynamic_cast<CIndSelDpCParams*>( pclProductSelectionParameters ) )
	{
		ASSERT_RETURN;
	}

	m_pclIndSelDpCParams = dynamic_cast<CIndSelDpCParams*>( pclProductSelectionParameters );
	m_pclDlgInfoSSelDpC = (CDlgInfoSSelDpC *)lpParam;
	
	// To remove all current displayed sheets.
	Reset();

	BeginWaitCursor();
	CWnd::SetRedraw( FALSE );

	long lDpCRowSelected = -1;
	CDS_SSelDpC *pSelectedDpC = NULL;

	if( NULL != m_pclIndSelDpCParams->m_SelIDPtr.MP )
	{
		pSelectedDpC = reinterpret_cast<CDS_SSelDpC *>( (CData*)( m_pclIndSelDpCParams->m_SelIDPtr.MP ) );

		if( NULL == pSelectedDpC )
		{
			ASSERT( 0 );
		}

		lDpCRowSelected = _FillDpControllerRows( pSelectedDpC );
	}
	else
	{
		lDpCRowSelected = _FillDpControllerRows();
	}

	// Verify if sheet description has been well created.
	CSheetDescription *pclSheetDescriptionDpC = m_ViewDescription.GetFromSheetDescriptionID( SD_DpController );

	if( NULL != pclSheetDescriptionDpC && NULL != pclSheetDescriptionDpC->GetSSheetPointer() )
	{
		CSSheet *pclSSheet = pclSheetDescriptionDpC->GetSSheetPointer();

		if( lDpCRowSelected > -1 && NULL != pSelectedDpC )
		{
			CCellDescriptionProduct *pclCDDpController = FindCDProduct( lDpCRowSelected, ( LPARAM )( dynamic_cast<CDB_TAProduct *>( pSelectedDpC->GetDpCIDPtr().MP ) ),
					pclSheetDescriptionDpC );

			if( NULL != pclCDDpController && NULL != pclCDDpController->GetProduct() )
			{
				// If we are in edition mode we simulate a click on the product.
				OnClickProduct( pclSheetDescriptionDpC, pclCDDpController, pclSheetDescriptionDpC->GetActiveColumn(), lDpCRowSelected );

				// Allow to check if we need to change the 'Show all priorities' button or not.
				CheckShowAllPrioritiesButtonState( pclSheetDescriptionDpC, lDpCRowSelected );

				// Verify accessories on Dp controller.
				CAccessoryList *pclDpCAccessoryList = pSelectedDpC->GetDpCAccessoryList();

				if( pclDpCAccessoryList->GetCount() > 0 )
				{
					CAccessoryList::AccessoryItem rAccessoryItem = pclDpCAccessoryList->GetFirst();

					while( rAccessoryItem.IDPtr.MP != NULL )
					{
						VerifyCheckboxAccessories( (CDB_Product * )rAccessoryItem.IDPtr.MP, true, &m_vecDpCAccessoryList );
						rAccessoryItem = pclDpCAccessoryList->GetNext();
					}
				}

				// Verify if user has selected a balancing valve.
				IDPTR ValveIDPtr = pSelectedDpC->GetProductIDPtr();
				CSheetDescription *pclSheetDescriptionBv = m_ViewDescription.GetFromSheetDescriptionID( SD_BalancingValve );

				if( _NULL_IDPTR != ValveIDPtr && NULL != ValveIDPtr.MP && NULL != pclSheetDescriptionBv )
				{
					CDB_TAProduct *pEditedTAP = dynamic_cast<CDB_TAProduct *>( ValveIDPtr.MP );

					if( NULL != pEditedTAP )
					{
						// Find row number where is the balancing valve.
						long lBvRowSelected = _GetRowOfEditedBv( pclSheetDescriptionBv, pEditedTAP );

						if( lBvRowSelected != -1 )
						{
							CCellDescriptionProduct *pclCDBalancingValve = FindCDProduct( lBvRowSelected, ( LPARAM )pEditedTAP, pclSheetDescriptionBv );

							if( NULL != pclCDBalancingValve && NULL != pclCDBalancingValve->GetProduct() )
							{
								// Simulate a click on the edited balancing valve.
								OnClickProduct( pclSheetDescriptionBv, pclCDBalancingValve, pclSheetDescriptionBv->GetActiveColumn(), lBvRowSelected );

								// Allow to check if we need to change the 'Show all priorities' button or not.
								CheckShowAllPrioritiesButtonState( pclSheetDescriptionBv, lBvRowSelected );

								// Verify accessories on balancing valve.
								CAccessoryList *pclBvAccessoryList = pSelectedDpC->GetAccessoryList();

								if( pclBvAccessoryList->GetCount() > 0 )
								{
									CAccessoryList::AccessoryItem rAccessoryItem = pclBvAccessoryList->GetFirst();

									while( rAccessoryItem.IDPtr.MP != NULL )
									{
										VerifyCheckboxAccessories( (CDB_Product * )rAccessoryItem.IDPtr.MP, true, &m_vecBvAccessoryList );
										rAccessoryItem = pclBvAccessoryList->GetNext();
									}
								}
							}
						}
					}
				}
			}
		}
		else
		{
			// Define the first product row as the active cell and set a visual focus.
			PrepareAndSetNewFocus( pclSheetDescriptionDpC, CD_DpController_Name, RD_DpController_FirstAvailRow, 0 );
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

void CRViewSSelDpC::FillInSelected( CDS_SelProd *pSelectedProductToFill )
{
	if( NULL == m_pclIndSelDpCParams )
	{
		ASSERT_RETURN;
	}

	CDS_SSelDpC *pSelectedDpCToFill = dynamic_cast<CDS_SSelDpC *>( pSelectedProductToFill );

	if( NULL == pSelectedDpCToFill )
	{
		ASSERT_RETURN;
	}

	// HYS-987: Clear previous selected Dp controller accessories if we are not in edition mode.
	CAccessoryList *pclDpCAccessoryToFill = pSelectedDpCToFill->GetDpCAccessoryList();

	// Clear previous balancing valve accessoriesif we are not in edition mode.
	CAccessoryList *pclBvAccessoryToFill = pSelectedDpCToFill->GetAccessoryList();
	bool bIsEditionMode = false;
	if( false == m_pclIndSelDpCParams->m_bEditModeRunning )
	{
		pclDpCAccessoryToFill->Clear();
		pclBvAccessoryToFill->Clear();
	}
	else
	{
		bIsEditionMode = true;
	}

	// Fill data for Dp controller.
	CDB_DpController *pclCurrentDpControllerSelected = GetCurrentDpControllerSelected();

	if( NULL != pclCurrentDpControllerSelected && NULL != m_pclIndSelDpCParams->m_pclSelectDpCList )
	{
		// Save Dp controller IDPtr.
		pSelectedDpCToFill->SetDpCIDPtr( pclCurrentDpControllerSelected->GetIDPtr() );

		// HYS-987: If we are not in Edition mode we fill the accessory list. In edition mode we juste update accessory list
		if( false == bIsEditionMode )
		{
			// Retrieve selected accessory and add it.
			for( vecCDCAccessoryListIter vecIter = m_vecDpCAccessoryList.begin(); vecIter != m_vecDpCAccessoryList.end(); vecIter++ )
			{
				CCDBCheckboxAccessory *pCDBCheckboxAccessory = *vecIter;

				if( NULL != pCDBCheckboxAccessory && true == pCDBCheckboxAccessory->GetCheckStatus() && NULL != pCDBCheckboxAccessory->GetAccessoryPointer() )
				{
					pclDpCAccessoryToFill->Add( pCDBCheckboxAccessory->GetAccessoryPointer()->GetIDPtr(), CAccessoryList::_AT_Accessory,
						pCDBCheckboxAccessory->GetRuledTable() );
				}
			}
		}
		else
		{
			UpdateAccessoryList( m_vecDpCAccessoryList, pclDpCAccessoryToFill, CAccessoryList::_AT_Accessory );
		}

		// Selected pipe informations.
		if( NULL != m_pclIndSelDpCParams->m_pclSelectDpCList->GetSelectPipeList() )
		{
			m_pclIndSelDpCParams->m_pclSelectDpCList->GetSelectPipeList()->GetMatchingPipe( pclCurrentDpControllerSelected->GetSizeKey(), 
					*pSelectedDpCToFill->GetpSelPipe() );
		}
	}

	// Fill data for balancing valve.
	CDB_TAProduct *pclCurrentSelectedBalancingValve = GetCurrentBalancingValveSelected();

	if( NULL != pclCurrentSelectedBalancingValve && NULL != m_pclIndSelDpCParams->m_pclSelectDpCList 
			&& NULL != m_pclIndSelDpCParams->m_pclSelectDpCList->GetBvSelected() )
	{
		// Save balancing valve IDPtr.
		pSelectedDpCToFill->SetProductIDPtr( pclCurrentSelectedBalancingValve->GetIDPtr() );

		CSelectedValve *pclSelectedBv = GetSelectProduct<CSelectedValve>( pclCurrentSelectedBalancingValve, 
				m_pclIndSelDpCParams->m_pclSelectDpCList->GetBvSelected() );

		if( NULL != pclSelectedBv )
		{
			pSelectedDpCToFill->SetOpening( pclSelectedBv->GetH() );
			
			// HYS-987: If we are not in Edition mode we fill the accessory list. In edition mode we just update accessory list
			if( false == bIsEditionMode )
			{
				// Retrieve selected accessory and add it.
				for( vecCDCAccessoryListIter vecIter = m_vecBvAccessoryList.begin(); vecIter != m_vecBvAccessoryList.end(); vecIter++ )
				{
					CCDBCheckboxAccessory *pCDBCheckboxAccessory = *vecIter;

					if( NULL != pCDBCheckboxAccessory && true == pCDBCheckboxAccessory->GetCheckStatus() && NULL != pCDBCheckboxAccessory->GetAccessoryPointer() )
					{
						pclBvAccessoryToFill->Add( pCDBCheckboxAccessory->GetAccessoryPointer()->GetIDPtr(), CAccessoryList::_AT_Accessory,
							pCDBCheckboxAccessory->GetRuledTable() );
					}
				}
			}
			else
			{
				UpdateAccessoryList( m_vecBvAccessoryList, pclBvAccessoryToFill, CAccessoryList::_AT_Accessory );
			}
		}
	}
}

void CRViewSSelDpC::OnNewDocument( CDS_IndSelParameter *pclIndSelParameter )
{
	_ReadAllColumnWidth( pclIndSelParameter );
}

void CRViewSSelDpC::SaveSelectionParameters( CDS_IndSelParameter *pclIndSelParameter )
{
	_WriteAllColumnWidth( pclIndSelParameter );
}

CDB_DpController *CRViewSSelDpC::GetCurrentDpControllerSelected()
{
	CDB_DpController *pclCurrentDpControllerSelected = NULL;
	CSheetDescription *pclSheetDescriptionDpC = m_ViewDescription.GetFromSheetDescriptionID( SD_DpController );

	if( NULL != pclSheetDescriptionDpC )
	{
		// Retrieve the current selected Dp controller.
		CCellDescriptionProduct *pclCDCurrentDpControllerSelected = NULL;
		LPARAM lpPointer;

		if( true == pclSheetDescriptionDpC->GetUserVariable( _SDUV_SELECTEDPRODUCT, lpPointer ) )
		{
			pclCDCurrentDpControllerSelected = (CCellDescriptionProduct *)lpPointer;
		}

		if( NULL != pclCDCurrentDpControllerSelected && NULL != pclCDCurrentDpControllerSelected->GetProduct() )
		{
			pclCurrentDpControllerSelected = dynamic_cast<CDB_DpController *>( ( CData * )pclCDCurrentDpControllerSelected->GetProduct() );
		}
	}

	return pclCurrentDpControllerSelected;
}

void CRViewSSelDpC::SetCurrentDpControllerSelected( CCellDescriptionProduct *pclCDCurrentDpControllerSelected )
{
	// Try to retrieve sheet description linked to Dp controller.
	CSheetDescription *pclSheetDescriptionDpC = m_ViewDescription.GetFromSheetDescriptionID( SD_DpController );

	if( NULL != pclSheetDescriptionDpC )
	{
		pclSheetDescriptionDpC->SetUserVariable( _SDUV_SELECTEDPRODUCT, (LPARAM)pclCDCurrentDpControllerSelected );
	}
}

CDB_TAProduct *CRViewSSelDpC::GetCurrentBalancingValveSelected()
{
	CDB_TAProduct *pclCurrentBalancingValveSelected = NULL;
	CSheetDescription *pclSheetDescriptionBv = m_ViewDescription.GetFromSheetDescriptionID( SD_BalancingValve );

	if( NULL != pclSheetDescriptionBv )
	{
		// Retrieve the current selected balancing valve.
		CCellDescriptionProduct *pclCDCurrentBalancingValveSelected = NULL;
		LPARAM lpPointer;

		if( true == pclSheetDescriptionBv->GetUserVariable( _SDUV_SELECTEDPRODUCT, lpPointer ) )
		{
			pclCDCurrentBalancingValveSelected = (CCellDescriptionProduct *)lpPointer;
		}

		if( NULL != pclCDCurrentBalancingValveSelected && NULL != pclCDCurrentBalancingValveSelected->GetProduct() )
		{
			pclCurrentBalancingValveSelected = dynamic_cast<CDB_TAProduct *>( (CData *)pclCDCurrentBalancingValveSelected->GetProduct() );
		}
	}

	return pclCurrentBalancingValveSelected;
}

void CRViewSSelDpC::SetCurrentBalancingValveSelected( CCellDescriptionProduct *pclCDCurrentBalancingValveSelected )
{
	// Try to retrieve sheet description linked to balancing valve.
	CSheetDescription *pclSheetDescriptionBv = m_ViewDescription.GetFromSheetDescriptionID( SD_BalancingValve );

	if( NULL != pclSheetDescriptionBv )
	{
		pclSheetDescriptionBv->SetUserVariable( _SDUV_SELECTEDPRODUCT, ( LPARAM )pclCDCurrentBalancingValveSelected );
	}
}

void CRViewSSelDpC::UpdateDpCPictureAndInfos( void )
{
	if( NULL == m_pclIndSelDpCParams || NULL == m_pclIndSelDpCParams->m_pclSelectDpCList )
	{
		ASSERT_RETURN;
	}
		
	if( NULL == m_pclDlgInfoSSelDpC || NULL == m_pclDlgInfoSSelDpC->GetSafeHwnd() )
	{
		return;
	}

	// Check current Dp controller selected.
	CDB_DpController *pclDpControllerSelected = GetCurrentDpControllerSelected();

	if( NULL == pclDpControllerSelected )
	{
		return;
	}

	CSelectedValve *pSelectedTAP = GetSelectProduct<CSelectedValve>( pclDpControllerSelected, m_pclIndSelDpCParams->m_pclSelectDpCList );

	m_pclDlgInfoSSelDpC->Reset();

	double dDpToStab = 0.0;

	if( true == m_pclIndSelDpCParams->m_bIsGroupDpbranchOrKvChecked )
	{
		if( eDpStab::DpStabOnBranch == m_pclIndSelDpCParams->m_eDpStab )
		{
			dDpToStab = m_pclIndSelDpCParams->m_dDpBranch;
		}
		else if( eDpStab::DpStabOnCV == m_pclIndSelDpCParams->m_eDpStab )
		{
			dDpToStab = CalcDp( m_pclIndSelDpCParams->m_dFlow, m_pclIndSelDpCParams->m_dKv, m_pclIndSelDpCParams->m_WC.GetDens() );
		}
	}

	m_pclDlgInfoSSelDpC->SetSchemePict(	m_pclIndSelDpCParams->m_eDpStab, m_pclIndSelDpCParams->m_eMvLoc, m_pclIndSelDpCParams->m_eDpCLoc, 
			m_pclIndSelDpCParams->m_bIsGroupDpbranchOrKvChecked, dDpToStab, m_pclIndSelDpCParams->m_dKv, m_pclIndSelDpCParams->m_bOnlyForSet );

	if( NULL != pSelectedTAP )
	{
		bool bValidRange = ( false == pSelectedTAP->IsFlagSet( CSelectedBase::eValveDpToLarge ) 
				&& false == pSelectedTAP->IsFlagSet( CSelectedBase::eValveDpToSmall ) );

		m_pclDlgInfoSSelDpC->UpdateInfoDpC( pclDpControllerSelected, pSelectedTAP->GetDpMin(), bValidRange );
	}

	m_pclDlgInfoSSelDpC->Invalidate();
	m_pclDlgInfoSSelDpC->UpdateWindow();
}

void CRViewSSelDpC::UpdateBvInfos( void )
{
	if( NULL == m_pclIndSelDpCParams || NULL == m_pclIndSelDpCParams->m_pclSelectDpCList || NULL == m_pclIndSelDpCParams->m_pclSelectDpCList->GetBvSelected() )
	{
		return;
	}

	UpdateDpCPictureAndInfos();

	// Check current balancing valve selected.
	CDB_TAProduct *pclBalancingValveSelected = GetCurrentBalancingValveSelected();

	if( NULL == pclBalancingValveSelected )
	{
		return;
	}

	CSelectedValve *pSelectedTAP = GetSelectProduct<CSelectedValve>( pclBalancingValveSelected, m_pclIndSelDpCParams->m_pclSelectDpCList->GetBvSelected() );

	if( NULL == pSelectedTAP )
	{
		return;
	}

	if( NULL != m_pclDlgInfoSSelDpC && NULL != m_pclDlgInfoSSelDpC->GetSafeHwnd() )
	{
		m_pclDlgInfoSSelDpC->UpdateInfoMV( pclBalancingValveSelected, pSelectedTAP->GetDp(), pSelectedTAP->GetDpFullOpen(), pSelectedTAP->GetH() );
		m_pclDlgInfoSSelDpC->Invalidate();
		m_pclDlgInfoSSelDpC->UpdateWindow();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PROTECTED MEMBERS
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CRViewSSelDpC::OnTextTipFetch( CSheetDescription *pclSheetDescription, long lColumn, long lRow, WORD *pwMultiLine, SHORT *pnTipWidth, TCHAR *pstrTipText,
		BOOL *pbShowTip )
{
	if( NULL == m_pclIndSelDpCParams || NULL == m_pclIndSelDpCParams->m_pclSelectDpCList || false == m_bInitialised || NULL == pclSheetDescription )
	{
		ASSERTA_RETURN( false );
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

	// Prepare some variables.
	CDB_TAProduct *pTAP = dynamic_cast<CDB_TAProduct *>( ( CData * )GetCDProduct( lColumn, lRow, pclSheetDescription ) );
	CSSheet *pclSSheet = pclSheetDescription->GetSSheetPointer();
	CDS_TechnicalParameter *pclTechParam = m_pclIndSelDpCParams->m_pTADS->GetpTechParams();

	CDB_DpController *pclDpController = NULL;
	CDB_RegulatingValve *pclRegulatingValve = NULL;
	CSelectedValve *pSelectedTAP = NULL;

	switch( pclSheetDescription->GetSheetDescriptionID() )
	{
		// Mouse cursor has passed over a Dp Controller.
		case SD_DpController:
			pclDpController = dynamic_cast<CDB_DpController *>( pTAP );
			pSelectedTAP = GetSelectProduct<CSelectedValve>( pTAP, m_pclIndSelDpCParams->m_pclSelectDpCList );

			if( CD_DpController_DpMin == lColumn )
			{
				// Check what is the color of the text.
				if( NULL != pSelectedTAP && NULL != pclSSheet && _ORANGE == pclSSheet->GetForeColor( lColumn, lRow ) )
				{
					CString str2 = WriteCUDouble( _U_DIFFPRESS, pSelectedTAP->GetDpMin() );
					str2 += CString( _T(" < ") ) + WriteCUDouble( _U_DIFFPRESS, pclTechParam->GetDpCMinDp(), true );
					FormatString( str, IDS_SSHEETSSELDPC_DPMINERROR, str2 );
				}
			}
			else if( CD_DpController_DplRange == lColumn )
			{
				if( NULL != pSelectedTAP )
				{
					CString str2 = WriteCUDouble( _U_DIFFPRESS, m_pclIndSelDpCParams->m_pclSelectDpCList->GetDpToStab(), false, 3, 0 );

					if( true == pSelectedTAP->IsFlagSet( CSelectedBase::eValveDpToLarge ) && -1.0 != pclDpController->GetDplmax() )
					{
						str2 += CString( _T(" > ") ) + WriteCUDouble( _U_DIFFPRESS, pclDpController->GetDplmax(), true, 3, 0 );
						FormatString( str, IDS_SSHEETSSELDPC_DPLRANGEERRORH, str2 );
					}
					else if( true == pSelectedTAP->IsFlagSet( CSelectedBase::eValveDpToSmall ) && -1.0 != pclDpController->GetDplmin() )
					{
						str2 += CString( _T(" < ") ) + WriteCUDouble( _U_DIFFPRESS, pclDpController->GetDplmin(), true, 3, 0 );
						FormatString( str, IDS_SSHEETSSELDPC_DPLRANGEERRORL, str2 );
					}
				}
			}
			else if( CD_DpController_DpMax == lColumn )
			{
				if( NULL != pclDpController && m_pclIndSelDpCParams->m_dDpMax > pclDpController->GetDpmax() )
				{
					CString str2 = WriteCUDouble( _U_DIFFPRESS, m_pclIndSelDpCParams->m_dDpMax, true, 3, 0 );
					CString str3 = WriteCUDouble( _U_DIFFPRESS, pclDpController->GetDpmax(), true, 3, 0 );
					FormatString( str, IDS_PRODUCTSELECTION_ERROR_DPMAX, str2, str3 );
				}
			}
			else if( CD_DpController_TemperatureRange == lColumn )
			{
				if( NULL != pSelectedTAP )
				{
					if( true == pSelectedTAP->IsFlagSet( CSelectedBase::eTemperature ) )
					{
						FormatString( str, IDS_SSHEETSSEL_TEMPERROR, pTAP->GetTempRange() );
					}
				}
			}
			else if( CD_DpController_PipeLinDp == lColumn )
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
			else if( CD_DpController_PipeV == lColumn )
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

			if( true == str.IsEmpty() && NULL != pclDpController )
			{
				if( 0 != _tcslen( pclDpController->GetComment() ) )
				{
					str = pclDpController->GetComment();
				}
			}

			break;

		// Mouse cursor has passed over a DpC accessory.
		case SD_DpCAccessories:
			if( lColumn > CD_DpCAccessory_FirstColumn && lColumn < CD_DpCAccessory_LastColumn )
			{
				TextTipFetchEllipsesHelper( lColumn, lRow, pclSheetDescription, pnTipWidth, &str );
			}

			break;

		// Mouse cursor has passed over a balancing valve.
		case SD_BalancingValve:
			pclRegulatingValve = dynamic_cast<CDB_RegulatingValve *>( pTAP );
			pSelectedTAP = GetSelectProduct<CSelectedValve>( pclRegulatingValve, m_pclIndSelDpCParams->m_pclSelectDpCList->GetBvSelected() );

			if( CD_BalancingValve_Preset == lColumn )
			{
				// Check if flag is set.
				if( NULL != pSelectedTAP && true == pSelectedTAP->IsFlagSet( CSelectedBase::eValveSetting ) )
				{
					if( NULL != pclRegulatingValve && NULL != pclRegulatingValve->GetValveCharacteristic() )
					{
						CString str2 = pclRegulatingValve->GetValveCharacteristic()->GetSettingString( pSelectedTAP->GetH() );

						double dMinRecommendedSetting = pclRegulatingValve->GetValveCharacteristic()->GetMinRecSetting();
						str2 += _T(" < ") + pclRegulatingValve->GetValveCharacteristic()->GetSettingString( dMinRecommendedSetting );
						FormatString( str, IDS_SSHEETSSEL_SETTINGERROR, str2 );
					}
				}
			}
			else if( CD_BalancingValve_Dp == lColumn )
			{
				// Check if flag is set.
				if( NULL != pSelectedTAP && true == pSelectedTAP->IsFlagSet( CSelectedBase::eDp ) )
				{
					CString str2 = WriteCUDouble( _U_DIFFPRESS, pSelectedTAP->GetDp() );

					if( pSelectedTAP->GetDp() < pclTechParam->GetValvMinDp( pclRegulatingValve->GetTypeIDPtr().ID ) )
					{
						str2 += CString( _T(" < ") ) + WriteCUDouble( _U_DIFFPRESS, pclTechParam->GetValvMinDp( pclRegulatingValve->GetTypeIDPtr().ID ), true );

						// "Dp is below the minimum value defined in technical parameters ( %1 )"
						FormatString( str, IDS_SSHEETSSEL_DPERRORL, str2 );
					}
					else if( pSelectedTAP->GetDp() > pclTechParam->VerifyValvMaxDp( pclRegulatingValve ) )
					{
						// Either the max Dp is defined for the valve or we take the max Dp defined in the technical parameters.
						// This is why the message is different in regards to these both cases.
						double dDpMax = pclRegulatingValve->GetDpmax();

						// IDS_SSHEETSSEL_DPERRORH: "Dp is above the maximum value defined in technical parameters ( %1 )"
						// IDS_SSHEETSSEL_DPERRORH2: "Dp is above the maximum value defined for this valve ( %1 )"
						int iMsgID = ( dDpMax <= 0.0 ) ? IDS_SSHEETSSEL_DPERRORH : IDS_SSHEETSSEL_DPERRORH2;

						str2 += CString( _T(" > ") ) + WriteCUDouble( _U_DIFFPRESS, pclTechParam->VerifyValvMaxDp( pclRegulatingValve ), true );
						FormatString( str, iMsgID, str2 );
					}
				}
			}
			else if( CD_BalancingValve_DpFullOpening == lColumn )
			{
				// Check if flag is set.
				if( NULL != pSelectedTAP && true == pSelectedTAP->IsFlagSet( CSelectedBase::eValveFullODp ) )
				{
					CString str2 = WriteCUDouble( _U_DIFFPRESS, pSelectedTAP->GetDpFullOpen() );

					if( pSelectedTAP->GetDpFullOpen() < pclTechParam->GetValvMinDp() )
					{
						str2 += CString( _T(" < ") ) + WriteCUDouble( _U_DIFFPRESS, pclTechParam->GetValvMinDp(), true );

						// "Pressure drop on valve fully open is below the minimum value defined in technical parameters ( %1 )"
						FormatString( str, IDS_SSHEETSSEL_DPFOERROR, str2 );
					}
				}
			}
			else if( CD_BalancingValve_DpHalfOpening == lColumn )
			{
				// Check if flag is set.
				if( NULL != pSelectedTAP && true == pSelectedTAP->IsFlagSet( CSelectedBase::eValveHalfODp ) )
				{
					CString str2 = WriteCUDouble( _U_DIFFPRESS, pSelectedTAP->GetDpHalfOpen() );

					if( pSelectedTAP->GetDpHalfOpen() < pclTechParam->GetValvMinDp() )
					{
						str2 += CString( _T(" < ") ) + WriteCUDouble( _U_DIFFPRESS, pclTechParam->GetValvMinDp(), true );
						FormatString( str, IDS_SSHEETSSEL_DPHOERRORL, str2 );
					}
					else if( pSelectedTAP->GetDpHalfOpen() > pclTechParam->VerifyValvMaxDp( pclRegulatingValve ) )
					{
						str2 += CString( _T(" > ") ) + WriteCUDouble( _U_DIFFPRESS, pclTechParam->VerifyValvMaxDp( pclRegulatingValve ), true );
						FormatString( str, IDS_SSHEETSSEL_DPHOERRORH, str2 );
					}
				}
			}
			else if( CD_BalancingValve_TemperatureRange == lColumn )
			{
				if( NULL != pSelectedTAP )
				{
					if( true == pSelectedTAP->IsFlagSet( CSelectedBase::eTemperature ) )
					{
						FormatString( str, IDS_SSHEETSSEL_TEMPERROR, pTAP->GetTempRange() );
					}
				}
			}
			else if( CD_BalancingValve_PipeLinDp == lColumn )
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
			else if( CD_BalancingValve_PipeV == lColumn )
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

			if( true == str.IsEmpty() && NULL != pclRegulatingValve )
			{
				if( 0 != _tcslen( pclRegulatingValve->GetComment() ) )
				{
					str = pclRegulatingValve->GetComment();
				}
			}

			break;

		// Mouse cursor has passed over a BV accessory.
		case SD_BvAccessories:
			if( lColumn > CD_BVAccessory_FirstColumn && lColumn < CD_BVAccessory_LastColumn )
			{
				TextTipFetchEllipsesHelper( lColumn, lRow, pclSheetDescription, pnTipWidth, &str );
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

bool CRViewSSelDpC::OnClickProduct( CSheetDescription *pclSheetDescription, CCellDescriptionProduct *pclCellDescriptionProduct, long lColumn, long lRow )
{
	// Sanity check.
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() || NULL == pclCellDescriptionProduct )
	{
		return false;
	}

	bool bNeedRefresh = true;
	CSSheet *pclSSheet = pclSheetDescription->GetSSheetPointer();
	pclSSheet->SetBool( SSB_REDRAW, FALSE );

	CDB_DpController *pDpController = dynamic_cast<CDB_DpController *>( ( CData * )pclCellDescriptionProduct->GetProduct() );
	CDB_RegulatingValve *pBalancingValve = dynamic_cast<CDB_RegulatingValve *>( ( CData * )pclCellDescriptionProduct->GetProduct() );

	if( NULL != pDpController )
	{
		// User has clicked on a Dp controller.
		_ClickOnDpController( pclSheetDescription, pDpController, pclCellDescriptionProduct, lColumn, lRow );
	}
	else if( NULL != pBalancingValve )
	{
		// User has clicked on a balancing valve.
		_ClickOnBalancingValve( pclSheetDescription, pBalancingValve, pclCellDescriptionProduct, lColumn, lRow );
	}
	else
	{
		bNeedRefresh = false;
	}

	pclSSheet->SetBool( SSB_REDRAW, TRUE );
	return bNeedRefresh;
}

bool CRViewSSelDpC::ResetColumnWidth( short nSheetDescriptionID )
{
	TSpread clTSpread;

	if( FALSE == clTSpread.Create( WS_CHILD, CRect( 0, 0, 0, 0 ), this, 0 ) )
	{
		ASSERT( 0 );
		return false;
	}

	switch( nSheetDescriptionID )
	{
		case SD_DpController:
			m_mapSSheetColumnWidth[SD_DpController][CD_DpController_FirstColumn] = clTSpread.ColWidthToLogUnits( 4 );
			m_mapSSheetColumnWidth[SD_DpController][CD_DpController_Box] = clTSpread.ColWidthToLogUnits( 2 );
			m_mapSSheetColumnWidth[SD_DpController][CD_DpController_CheckBox] = clTSpread.ColWidthToLogUnits( 2 );
			m_mapSSheetColumnWidth[SD_DpController][CD_DpController_Name] = clTSpread.ColWidthToLogUnits( 14 );
			m_mapSSheetColumnWidth[SD_DpController][CD_DpController_Material] = clTSpread.ColWidthToLogUnits( 12 );
			m_mapSSheetColumnWidth[SD_DpController][CD_DpController_Connection] = clTSpread.ColWidthToLogUnits( 14 );
			m_mapSSheetColumnWidth[SD_DpController][CD_DpController_Version] = clTSpread.ColWidthToLogUnits( 16 );
			m_mapSSheetColumnWidth[SD_DpController][CD_DpController_PN] = clTSpread.ColWidthToLogUnits( 6 );
			m_mapSSheetColumnWidth[SD_DpController][CD_DpController_Size] = clTSpread.ColWidthToLogUnits( 10 );
			m_mapSSheetColumnWidth[SD_DpController][CD_DpController_Preset] = clTSpread.ColWidthToLogUnits( 6 );
			m_mapSSheetColumnWidth[SD_DpController][CD_DpController_DpMin] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_DpController][CD_DpController_DplRange] = clTSpread.ColWidthToLogUnits( 10 );
			m_mapSSheetColumnWidth[SD_DpController][CD_DpController_DpMax] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_DpController][CD_DpController_TemperatureRange] = clTSpread.ColWidthToLogUnits( 8 );
			m_mapSSheetColumnWidth[SD_DpController][CD_DpController_Separator] = clTSpread.ColWidthToLogUnits( 1 );
			m_mapSSheetColumnWidth[SD_DpController][CD_DpController_PipeSize] = clTSpread.ColWidthToLogUnits( 8 );
			m_mapSSheetColumnWidth[SD_DpController][CD_DpController_PipeLinDp] = clTSpread.ColWidthToLogUnits( 8 );
			m_mapSSheetColumnWidth[SD_DpController][CD_DpController_PipeV] = clTSpread.ColWidthToLogUnits( 8 );
			m_mapSSheetColumnWidth[SD_DpController][CD_DpController_Pointer] = clTSpread.ColWidthToLogUnits( 1 );
			break;

		case SD_BalancingValve:
			m_mapSSheetColumnWidth[SD_BalancingValve][CD_BalancingValve_FirstColumn] = clTSpread.ColWidthToLogUnits( 4 );
			m_mapSSheetColumnWidth[SD_BalancingValve][CD_BalancingValve_Box] = clTSpread.ColWidthToLogUnits( 2 );
			m_mapSSheetColumnWidth[SD_BalancingValve][CD_BalancingValve_CheckBox] = clTSpread.ColWidthToLogUnits( 2 );
			m_mapSSheetColumnWidth[SD_BalancingValve][CD_BalancingValve_Name] = clTSpread.ColWidthToLogUnits( 14 );
			m_mapSSheetColumnWidth[SD_BalancingValve][CD_BalancingValve_Material] = clTSpread.ColWidthToLogUnits( 12 );
			m_mapSSheetColumnWidth[SD_BalancingValve][CD_BalancingValve_Connection] = clTSpread.ColWidthToLogUnits( 14 );
			m_mapSSheetColumnWidth[SD_BalancingValve][CD_BalancingValve_Version] = clTSpread.ColWidthToLogUnits( 12 );
			m_mapSSheetColumnWidth[SD_BalancingValve][CD_BalancingValve_PN] = clTSpread.ColWidthToLogUnits( 6 );
			m_mapSSheetColumnWidth[SD_BalancingValve][CD_BalancingValve_Size] = clTSpread.ColWidthToLogUnits( 10 );
			m_mapSSheetColumnWidth[SD_BalancingValve][CD_BalancingValve_Preset] = clTSpread.ColWidthToLogUnits( 6 );
			m_mapSSheetColumnWidth[SD_BalancingValve][CD_BalancingValve_DpSignal] = clTSpread.ColWidthToLogUnits( 6 );
			m_mapSSheetColumnWidth[SD_BalancingValve][CD_BalancingValve_Dp] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_BalancingValve][CD_BalancingValve_DpFullOpening] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_BalancingValve][CD_BalancingValve_DpHalfOpening] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_BalancingValve][CD_BalancingValve_TemperatureRange] = clTSpread.ColWidthToLogUnits( 8 );
			m_mapSSheetColumnWidth[SD_BalancingValve][CD_BalancingValve_Separator] = clTSpread.ColWidthToLogUnits( 1 );
			m_mapSSheetColumnWidth[SD_BalancingValve][CD_BalancingValve_PipeSize] = clTSpread.ColWidthToLogUnits( 8 );
			m_mapSSheetColumnWidth[SD_BalancingValve][CD_BalancingValve_PipeLinDp] = clTSpread.ColWidthToLogUnits( 8 );
			m_mapSSheetColumnWidth[SD_BalancingValve][CD_BalancingValve_PipeV] = clTSpread.ColWidthToLogUnits( 8 );
			m_mapSSheetColumnWidth[SD_BalancingValve][CD_BalancingValve_Pointer] = clTSpread.ColWidthToLogUnits( 1 );
			break;
	}

	if( clTSpread.GetSafeHwnd() != NULL )
	{
		clTSpread.DestroyWindow();
	}

	return true;
}

bool CRViewSSelDpC::IsSelectionReady( void )
{
	bool bReady = false;
	CDB_DpController *pclSelectedDpController = GetCurrentDpControllerSelected();

	if( pclSelectedDpController != NULL )
	{
		bReady = ( false == pclSelectedDpController->IsDeleted() );

		CDB_TAProduct *pclSelectedBv = GetCurrentBalancingValveSelected();

		if ( bReady && NULL != pclSelectedBv )
		{
			bReady = ( false == pclSelectedBv->IsDeleted() );
		}

		// If we are in the package selection mode AND the Dp controller is part of a set BUT there is not yet a balancing valve selected...
		if( NULL != m_pclIndSelDpCParams && true == m_pclIndSelDpCParams->m_bOnlyForSet && true == pclSelectedDpController->IsPartOfaSet() 
				&& NULL == pclSelectedBv )
		{
			bReady = false;
		}
	}

	return bReady;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PRIVATE MEMBERS
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CRViewSSelDpC::_Init( void )
{
	m_lDpCSelectedRow = 0;
	m_lBvSelectedRow = 0;
	m_vecDpCAccessoryList.clear();
	m_vecBvAccessoryList.clear();
	m_pCDBExpandCollapseRowsDpC = NULL;
	m_pCDBExpandCollapseGroupDpCAccessory = NULL;
	m_pCDBShowAllPrioritiesDpC = NULL;
	m_pCDBExpandCollapseRowsBv = NULL;
	m_pCDBExpandCollapseGroupBvAccessory = NULL;
	m_pCDBShowAllPrioritiesBv = NULL;
}

void CRViewSSelDpC::_ClickOnDpController( CSheetDescription *pclSheetDescriptionDpC, CDB_DpController *pDpControllerClicked,
		CCellDescriptionProduct *pclCellDescriptionProduct, long lColumn, long lRow )
{
	if( NULL == m_pclIndSelDpCParams || NULL == m_pclIndSelDpCParams->m_pclSelectDpCList )
	{
		ASSERT( 0 );
		return;
	}

	CSSheet *pclSSheet = pclSheetDescriptionDpC->GetSSheetPointer();

	m_lBvSelectedRow = 0;
	m_pCDBExpandCollapseGroupDpCAccessory = NULL;
	m_pCDBExpandCollapseRowsBv = NULL;
	m_pCDBExpandCollapseGroupBvAccessory = NULL;
	m_pCDBShowAllPrioritiesBv = NULL;

	SetCurrentBalancingValveSelected( NULL );

	// Memorize previous focus row.
	long lCurrentFocusedRow = GetCurrentFocusedRow();

	// To be sure to clean all previous data concerning focus.
	KillCurrentFocus();

	// Unselect row where current selection is set (yellow light).
	// Remark: 'KillCurrentFocus' unselect only rows where focus is on (blue light).
	pclSSheet->UnSelectMultipleRows();

	// By default clear accessories list.
	m_vecDpCAccessoryList.clear();

	LPARAM lDpControllerCount;
	pclSheetDescriptionDpC->GetUserVariable( _SDUV_TOTALPRODUCT_COUNT, lDpControllerCount );

	// Retrieve the current selected control valve if exist.
	CDB_DpController *pclCurrentDpControllerSelected = GetCurrentDpControllerSelected();

	// If there is already one Dp controller selected and user click on the current one...
	// Remark: 'm_pCDBExpandCollapseRowsDpC' is not created if there is only one Dp controller. Thus we need to check first if there is only one Dp controller.
	//         Otherwise we have to check if user has clicked on the current one.
	if( NULL != pclCurrentDpControllerSelected && ( ( 1 == lDpControllerCount ) || ( NULL != m_pCDBExpandCollapseRowsDpC
			&& lRow == m_pCDBExpandCollapseRowsDpC->GetCellPosition().y ) ) )
	{
		// Change focus state (selected to normal) and delete Expand/Collapse rows button.

		// Change box button to open state for Dp controller.
		if( true == m_pclIndSelDpCParams->m_bOnlyForSet && true == pclCurrentDpControllerSelected->IsPartOfaSet() )
		{
			CCDButtonBox *pCDButtonBox = GetCDButtonBox( CD_DpController_Box, lRow, pclSheetDescriptionDpC );

			if( NULL != pCDButtonBox )
			{
				pCDButtonBox->SetButtonState( CCDButtonBox::ButtonState::BoxOpened,  true );
			}
		}

		// Uncheck checkbox.
		pclSSheet->SetCheckBox(CD_DpController_CheckBox, lRow, _T(""), false, true);

		// Reset current product selected.
		SetCurrentDpControllerSelected( NULL );

		// Delete Expand/Collapse rows button if exist.
		// Remark: 'm_pCDBExpandCollapseRowsDpC' is set to NULL in method!
		DeleteExpandCollapseRowsButton( m_pCDBExpandCollapseRowsDpC, pclSheetDescriptionDpC );

		// Show Show/Hide all priorities button if exist.
		if( NULL != m_pCDBShowAllPrioritiesDpC )
		{
			m_pCDBShowAllPrioritiesDpC->ApplyInternalChange();
		}

		// Set focus on Dp controller currently selected.
		PrepareAndSetNewFocus( pclSheetDescriptionDpC, pclSheetDescriptionDpC->GetActiveColumn(), lRow, 0 );

		// Remove all sheets after Dp controller.
		m_ViewDescription.RemoveAllSheetAfter( SD_DpController );

		m_lDpCSelectedRow = 0;
		UpdateDpCPictureAndInfos();

		// Notify a change in the selection.
		SelectionHasChanged( IsSelectionReady() );
	}
	else
	{
		// If a Dp controller is already selected...
		if( NULL != pclCurrentDpControllerSelected )
		{
			// Change box button to open state for previous Dp controller.
			if( true == m_pclIndSelDpCParams->m_bOnlyForSet && true == pclCurrentDpControllerSelected->IsPartOfaSet() )
			{
				CCDButtonBox *pCDButtonBox = GetCDButtonBox( CD_DpController_Box, m_lDpCSelectedRow, pclSheetDescriptionDpC );

				if( NULL != pCDButtonBox )
				{
					pCDButtonBox->SetButtonState( CCDButtonBox::ButtonState::BoxOpened,  true );
				}
			}

			// uncheck checkbox
			pclSSheet->SetCheckBox(CD_DpController_CheckBox, m_lDpCSelectedRow, _T(""), false, true);

			// Remove all sheets after Dp controller.
			m_ViewDescription.RemoveAllSheetAfter( SD_DpController );
		}

		// Check checkbox.
		pclSSheet->SetCheckBox(CD_DpController_CheckBox, lRow, _T(""), true, true);

		// Save new Dp controller selection.
		SetCurrentDpControllerSelected( pclCellDescriptionProduct );

		// Update DlgInfoSSelDpC.
		UpdateDpCPictureAndInfos();

		// Delete Expand/Collapse rows button if exist.
		if( NULL != m_pCDBExpandCollapseRowsDpC )
		{
			// Remark: 'm_pCDBExpandCollapseRows' is set to NULL in method!
			DeleteExpandCollapseRowsButton( m_pCDBExpandCollapseRowsDpC, pclSheetDescriptionDpC );
		}

		// Create Expand/Collapse rows button if needed...
		LPARAM lDpCTotalCount;
		pclSheetDescriptionDpC->GetUserVariable( _SDUV_TOTALPRODUCT_COUNT, lDpCTotalCount );

		if( lDpCTotalCount > 1 )
		{
			m_pCDBExpandCollapseRowsDpC = CreateExpandCollapseRowsButton( CD_DpController_FirstColumn, lRow, true, CCDButtonExpandCollapseRows::ButtonState::CollapseRow,
										  pclSheetDescriptionDpC->GetFirstSelectableRow(), pclSheetDescriptionDpC->GetLastSelectableRow( false ),
										  pclSheetDescriptionDpC );

			// Show button.
			if( NULL != m_pCDBExpandCollapseRowsDpC )
			{
				m_pCDBExpandCollapseRowsDpC->SetShowStatus( true );
			}
		}

		// Select Dp controller.
		pclSSheet->SelectOneRow( lRow, pclSheetDescriptionDpC->GetSelectionFrom(), pclSheetDescriptionDpC->GetSelectionTo() );

		// Fill accessories available for the current Dp controller.
		_FillDpCAccessoryRows();

		// Fill corresponding balancing valve list.
		CSelectedValve *pSelectedTAP = GetSelectProduct<CSelectedValve>( pDpControllerClicked, m_pclIndSelDpCParams->m_pclSelectDpCList );
		_FillBalancingValveRows( pSelectedTAP );

		// Memorize Dp controller row selected.
		// Remark: Do it here because we need this variable when calling the '_ClickOnBalancingValve' below.
		m_lDpCSelectedRow = lRow;

		bool bSetFocus = true;

		// If user has selected DpC-MV set mode and we have only one balancing valve...
		CSheetDescription *pSheetDescriptionBv = m_ViewDescription.GetFromSheetDescriptionID( SD_BalancingValve );

		if( NULL != pSheetDescriptionBv && true == m_pclIndSelDpCParams->m_bOnlyForSet && true == pDpControllerClicked->IsPartOfaSet() )
		{
			// Verify first if we have only one balancing valve.
			LPARAM lBvTotalCount;
			pSheetDescriptionBv->GetUserVariable( _SDUV_TOTALPRODUCT_COUNT, lBvTotalCount );

			if( 1 == lBvTotalCount )
			{
				// ... we can than automatically select balancing valve and close boxes in front of valve.

				// Retrieve balancing valve.
				CCellDescriptionProduct *pclCDProduct = NULL;
				long lBvRow = pSheetDescriptionBv->GetFirstSelectableRow();
				long lColParam = pSheetDescriptionBv->GetFirstParameterColumn();
				LPARAM lProduct = GetCDProduct( lColParam, lBvRow, pSheetDescriptionBv, &pclCDProduct );

				if( NULL != pclCDProduct )
				{
					// Simulate a click on the edited balancing valve.
					_ClickOnBalancingValve( pSheetDescriptionBv, dynamic_cast<CDB_RegulatingValve *>( ( CData * )lProduct ), pclCDProduct, 
							pSheetDescriptionBv->GetSelectionFrom(), lBvRow, false );

					// Change box button to close state for Dp controller.
					CCDButtonBox *pCDButtonBox = GetCDButtonBox( CD_DpController_Box, lRow, pclSheetDescriptionDpC );

					if( NULL != pCDButtonBox )
					{
						pCDButtonBox->SetButtonState( CCDButtonBox::ButtonState::BoxClosed,  true );
					}

					// Change box button to close state for balancing valve selected.
					pCDButtonBox = GetCDButtonBox( CD_BalancingValve_Box, lBvRow, pSheetDescriptionBv );

					if( NULL != pCDButtonBox )
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
			bool bShiftPressed;

			if( DNFERRC_FocusOnNextGroupOK == DefineNextFocusedRow( pclSheetDescriptionDpC, CD_DpController_FirstColumn, lRow, false, lNewFocusedRow,
					pclNextSheetDescription, bShiftPressed ) )
			{
				long lNewFocusedCol = pclNextSheetDescription->GetFirstParameterColumn();
				PrepareAndSetNewFocus( pclNextSheetDescription, lNewFocusedCol, lNewFocusedRow );
			}
			else
			{
				// Because we have changed current selection background in yellow, if there is no other sheet following current one, we
				// must set focus on current selection.
				PrepareAndSetNewFocus( pclSheetDescriptionDpC, lColumn, lRow );
			}
		}

		// Notify a change in the selection.
		SelectionHasChanged( IsSelectionReady() );
	}
}

long CRViewSSelDpC::_FillDpControllerRows( CDS_SSelDpC *pEditedDpC )
{
	if( NULL == m_pclIndSelDpCParams || NULL == m_pclIndSelDpCParams->m_pclSelectDpCList )
	{
		ASSERTA_RETURN( 0 );
	}

	CDB_TAProduct *pEditedTAP = NULL;

	if( NULL != pEditedDpC && _NULL_IDPTR != pEditedDpC->GetDpCIDPtr() )
	{
		pEditedTAP = dynamic_cast<CDB_TAProduct *>( pEditedDpC->GetDpCIDPtr().MP );
	}

	CSelectedBase *pclSelectedProduct = m_pclIndSelDpCParams->m_pclSelectDpCList->GetFirst<CSelectedBase>();

	if( NULL == pclSelectedProduct )
	{
		return 0;
	}

	// Call 'MultiSpreadBase' method to get a new SSheet.
	CSheetDescription *pclSheetDescriptionDpC = CreateSSheet( SD_DpController );

	if( NULL == pclSheetDescriptionDpC || NULL == pclSheetDescriptionDpC->GetSSheetPointer() )
	{
		return 0;
	}

	CSSheet *pclSSheet = pclSheetDescriptionDpC->GetSSheetPointer();

	pclSSheet->SetBool( SSB_REDRAW, FALSE );

	pclSSheet->Init();
	pclSSheet->SetBool( SSB_ALLOWDRAGDROP, FALSE );
	pclSSheet->SetBool( SSB_HORZSCROLLBAR, FALSE );
	pclSSheet->SetBool( SSB_VERTSCROLLBAR, FALSE );
	pclSSheet->SetBool( SSB_MOVEACTIVEONFOCUS, FALSE );
	pclSSheet->SetTextTip( SS_TT_STATUS_FLOATING, 250, NULL, _LIGHTYELLOW, _TAH_BLACK );

	// Initialize DpC sheet and fill header.
	_InitAndFillDpCHeader( pclSheetDescriptionDpC, pclSSheet );

	long lRetRow = -1;
	long lRow = RD_DpController_FirstAvailRow;

	long lValveTotalCount = 0;
	long lValveNotPriorityCount = 0;
	bool bShowAllPrioritiesShown = false;
	long lShowAllPrioritiesButtonRow = 0;

	pclSheetDescriptionDpC->SetUserVariable( _SDUV_TOTALPRODUCT_COUNT, 0 );
	pclSheetDescriptionDpC->SetUserVariable( _SDUV_NOPRIORITYPRODUCT_COUNT, 0 );

	bool bAtLeastOneDpcPartOfASet = false;
	pclSheetDescriptionDpC->RestartRemarkGenerator();

	for( CSelectedValve *pclSelectedDpController = m_pclIndSelDpCParams->m_pclSelectDpCList->GetFirst<CSelectedValve>(); NULL != pclSelectedDpController;
			pclSelectedDpController = m_pclIndSelDpCParams->m_pclSelectDpCList->GetNext<CSelectedValve>() )
	{
		CDB_DpController *pDpController = dynamic_cast<CDB_DpController *>( pclSelectedDpController->GetpData() );

		if( NULL == pDpController )
		{
			continue;
		}

		if( true == pDpController->IsPartOfaSet() )
		{
			bAtLeastOneDpcPartOfASet = true;
		}

		if( true == pclSelectedDpController->IsFlagSet( CSelectedBase::eNotPriority ) )
		{
			lValveNotPriorityCount++;

			if( false == bShowAllPrioritiesShown )
			{
				// Add one empty not selectable row.
				pclSheetDescriptionDpC->AddRows( 1 );
				pclSheetDescriptionDpC->RemoveSelectableRow( lRow );

				pclSSheet->SetCellBorder( CD_DpController_CheckBox, lRow, CD_DpController_Separator - 1, lRow, true, SS_BORDERTYPE_TOP | SS_BORDERTYPE_BOTTOM, 
						SS_BORDERSTYLE_SOLID, _BLACK );
				
				pclSSheet->SetCellBorder( CD_DpController_PipeSize, lRow, CD_DpController_Pointer - 1, lRow, true, SS_BORDERTYPE_TOP | SS_BORDERTYPE_BOTTOM,
						SS_BORDERSTYLE_SOLID, _BLACK );

				lShowAllPrioritiesButtonRow = lRow++;
				bShowAllPrioritiesShown = true;
			}
		}

		long lRetVal = _FillOneDpCRow( pclSheetDescriptionDpC, pclSSheet, pclSelectedDpController, lRow, pEditedTAP );

		if( lRetVal > 0 )
		{
			lRetRow = lRetVal;
		}

		lRow++;
		lValveTotalCount++;
	}

	long lLastDataRow = lRow - 1;

	// If we are not in selection by package mode OR if there is no valve that belongs to a set...
	if( false == m_pclIndSelDpCParams->m_bOnlyForSet || false == bAtLeastOneDpcPartOfASet )
	{
		pclSSheet->ShowCol( CD_DpController_Box, FALSE );
	}

	pclSheetDescriptionDpC->SetUserVariable( _SDUV_TOTALPRODUCT_COUNT, lValveTotalCount );
	pclSheetDescriptionDpC->SetUserVariable( _SDUV_NOPRIORITYPRODUCT_COUNT, lValveNotPriorityCount );

	// Now we can create Show/Hide all priorities if needed.
	// Remarks: 2nd condition to avoid to have button if we have only no priority valves.
	if( true == bShowAllPrioritiesShown && lValveTotalCount > lValveNotPriorityCount )
	{
		// Create Show/Hide all priorities button in regards to current state!
		CCDButtonShowAllPriorities::ButtonState eState = ( true == GetShowAllPrioritiesFlag( SD_DpController ) ) ? 
				CCDButtonShowAllPriorities::ButtonState::HidePriorities : CCDButtonShowAllPriorities::ButtonState::ShowPriorities;
		
		CCDButtonShowAllPriorities *pclShowAllButton = CreateShowAllPrioritiesButton( CD_DpController_FirstColumn, lShowAllPrioritiesButtonRow, 
				bShowAllPrioritiesShown, eState, lShowAllPrioritiesButtonRow + 1, pclSSheet->GetMaxRows(), pclSheetDescriptionDpC );

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

	// Try to merge only if there is more than one Dp controller.
	if( lValveTotalCount > 2 || ( 2 == lValveTotalCount && lValveNotPriorityCount != 1 ) )
	{
		vector<long> vecColumnList;
		vecColumnList.push_back( CD_DpController_PipeSize );
		vecColumnList.push_back( CD_DpController_PipeLinDp );
		vecColumnList.push_back( CD_DpController_PipeV );
		pclSSheet->MergeRowsWithSameValues( RD_DpController_FirstAvailRow, lLastDataRow, vecColumnList );
	}

	pclSSheet->SetCellBorder( CD_DpController_CheckBox, lLastDataRow, CD_DpController_Separator - 1, lLastDataRow, true, SS_BORDERTYPE_BOTTOM, 
			SS_BORDERSTYLE_SOLID, _BLACK );

	// Needed by pipe merging
	pclSSheet->SetCellBorder( CD_DpController_PipeSize, lLastDataRow, CD_DpController_Pointer - 1, lLastDataRow, true, SS_BORDERTYPE_BOTTOM, 
			SS_BORDERSTYLE_SOLID, _BLACK );

	// Add Remarks.
	// Note: these lines must be placed after creating Show/Hide all priorities button to avoid to take in count these lines when defining
	//       range of row to be Show/Hide.
	lRow = pclSheetDescriptionDpC->WriteRemarks( lRow, CD_DpController_CheckBox, CD_DpController_Separator );

	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, ( LPARAM )FALSE );

	pclSSheet->SetBool( SSB_REDRAW, TRUE );

	// Set that there is no selection at now.
	SetCurrentDpControllerSelected( NULL );

	// Add possibility to change column size.
	// Remark: place this code after any initialization (like adding, removing, resizing, hiding/showing columns).
	CResizingColumnInfo *pclResizingColumnInfo = pclSheetDescriptionDpC->GetResizingColumnInfo();
	pclResizingColumnInfo->ActivateFeature( true, false, RD_DpController_FirstAvailRow, lLastDataRow );
	pclResizingColumnInfo->AddRangeColumn( CD_DpController_Name, CD_DpController_TemperatureRange, RD_DpController_ColName, RD_DpController_Unit );
	pclResizingColumnInfo->AddRangeColumn( CD_DpController_PipeSize, CD_DpController_PipeV, RD_DpController_ColName, RD_DpController_Unit );

	// Add possibility to click on title group to reset column width.
	CreateTitleGroupButtons( CD_DpController_CheckBox, CD_DpController_PipeV, RD_DpController_GroupName, pclSheetDescriptionDpC );

	return lRetRow;
}

void CRViewSSelDpC::_InitAndFillDpCHeader( CSheetDescription *pclSheetDescriptionDpC, CSSheet *pclSSheet )
{
	if( NULL == m_pclIndSelDpCParams || NULL == m_pclIndSelDpCParams->m_pPipeDB )
	{
		ASSERT_RETURN;
	}

	// Set title.
	pclSSheet->SetMaxRows( RD_DpController_FirstAvailRow - 1 );

	// All cells are static by default and filled with _T("").
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	pclSSheet->FormatStaticText( -1, -1, -1, -1, _T( "" ) );

	// Increase row height.
	double dRowHeight = 12.75;
	pclSSheet->SetRowHeight( RD_DpController_FirstRow, dRowHeight * 0.5 );
	pclSSheet->SetRowHeight( RD_DpController_GroupName, dRowHeight * 1.5 );
	pclSSheet->SetRowHeight( RD_DpController_ColName, dRowHeight * 2 );
	pclSSheet->SetRowHeight( RD_DpController_Unit, dRowHeight * 1.2 );

	// Set columns.
	pclSheetDescriptionDpC->Init( 1, pclSSheet->GetMaxRows(), 1, CSheetDescription::SD_ParameterMode_Mono );
	pclSheetDescriptionDpC->AddColumnInPixels( CD_DpController_FirstColumn, m_mapSSheetColumnWidth[SD_DpController][CD_DpController_FirstColumn] );
	pclSheetDescriptionDpC->AddColumnInPixels( CD_DpController_Box, m_mapSSheetColumnWidth[SD_DpController][CD_DpController_Box] );
	pclSheetDescriptionDpC->AddColumnInPixels( CD_DpController_CheckBox, m_mapSSheetColumnWidth[SD_DpController][CD_DpController_CheckBox] );
	pclSheetDescriptionDpC->AddColumnInPixels( CD_DpController_Name, m_mapSSheetColumnWidth[SD_DpController][CD_DpController_Name] );
	pclSheetDescriptionDpC->AddColumnInPixels( CD_DpController_Material, m_mapSSheetColumnWidth[SD_DpController][CD_DpController_Material] );
	pclSheetDescriptionDpC->AddColumnInPixels( CD_DpController_Connection, m_mapSSheetColumnWidth[SD_DpController][CD_DpController_Connection] );
	pclSheetDescriptionDpC->AddColumnInPixels( CD_DpController_Version, m_mapSSheetColumnWidth[SD_DpController][CD_DpController_Version] );
	pclSheetDescriptionDpC->AddColumnInPixels( CD_DpController_PN, m_mapSSheetColumnWidth[SD_DpController][CD_DpController_PN] );
	pclSheetDescriptionDpC->AddColumnInPixels( CD_DpController_Size, m_mapSSheetColumnWidth[SD_DpController][CD_DpController_Size] );
	pclSheetDescriptionDpC->AddColumnInPixels( CD_DpController_Preset, m_mapSSheetColumnWidth[SD_DpController][CD_DpController_Preset] );
	pclSheetDescriptionDpC->AddColumnInPixels( CD_DpController_DpMin, m_mapSSheetColumnWidth[SD_DpController][CD_DpController_DpMin] );
	pclSheetDescriptionDpC->AddColumnInPixels( CD_DpController_DplRange, m_mapSSheetColumnWidth[SD_DpController][CD_DpController_DplRange] );

	if( true == m_pclIndSelDpCParams->m_bIsDpMaxChecked )
	{
		pclSheetDescriptionDpC->AddColumnInPixels( CD_DpController_DpMax, m_mapSSheetColumnWidth[SD_DpController][CD_DpController_DpMax] );
	}

	pclSheetDescriptionDpC->AddColumnInPixels( CD_DpController_TemperatureRange, m_mapSSheetColumnWidth[SD_DpController][CD_DpController_TemperatureRange] );
	pclSheetDescriptionDpC->AddColumnInPixels( CD_DpController_Separator, m_mapSSheetColumnWidth[SD_DpController][CD_DpController_Separator] );
	pclSheetDescriptionDpC->AddColumnInPixels( CD_DpController_PipeSize, m_mapSSheetColumnWidth[SD_DpController][CD_DpController_PipeSize] );
	pclSheetDescriptionDpC->AddColumnInPixels( CD_DpController_PipeLinDp, m_mapSSheetColumnWidth[SD_DpController][CD_DpController_PipeLinDp] );
	pclSheetDescriptionDpC->AddColumnInPixels( CD_DpController_PipeV, m_mapSSheetColumnWidth[SD_DpController][CD_DpController_PipeV] );
	pclSheetDescriptionDpC->AddColumnInPixels( CD_DpController_Pointer, m_mapSSheetColumnWidth[SD_DpController][CD_DpController_Pointer] );

	// Set in which column parameter must be saved.
	pclSheetDescriptionDpC->AddParameterColumn( CD_DpController_Pointer );

	// Set the focus column.
	pclSheetDescriptionDpC->SetActiveColumn( CD_DpController_Name );

	// Set range for selection.
	pclSheetDescriptionDpC->SetFocusColumnRange( CD_DpController_CheckBox, CD_DpController_TemperatureRange );


	// Hide columns corresponding to user selected combos.
	pclSSheet->ShowCol( CD_DpController_Material, ( true == m_pclIndSelDpCParams->m_strComboMaterialID.IsEmpty() ) ? TRUE : FALSE );
	pclSSheet->ShowCol( CD_DpController_Connection, ( true == m_pclIndSelDpCParams->m_strComboConnectID.IsEmpty() ) ? TRUE : FALSE );
	pclSSheet->ShowCol( CD_DpController_Version, ( true == m_pclIndSelDpCParams->m_strComboVersionID.IsEmpty() ) ? TRUE : FALSE );
	pclSSheet->ShowCol( CD_DpController_PN, ( true == m_pclIndSelDpCParams->m_strComboPNID.IsEmpty() ) ? TRUE : FALSE );

	if( false == m_pclIndSelDpCParams->m_bIsGroupDpbranchOrKvChecked )
	{
		pclSSheet->ShowCol( CD_DpController_Preset, FALSE );
	}

	if( false == m_pclIndSelDpCParams->m_bIsDpMaxChecked )
	{
		pclSSheet->ShowCol( CD_DpController_DpMax, FALSE );
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Row name.

	// Add the main title.
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, ( LPARAM )GetTitleBackgroundColor( pclSheetDescriptionDpC ) );
	pclSheetDescriptionDpC->SetUserVariable( _SDUV_TITLEFORECOLOR, ( pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor ) ) );
	pclSheetDescriptionDpC->SetUserVariable( _SDUV_TITLEBACKCOLOR, ( pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor ) ) );

	pclSSheet->AddCellSpanW( CD_DpController_CheckBox, RD_DpController_GroupName, CD_DpController_Separator - CD_DpController_CheckBox, 1 );
	pclSSheet->SetStaticText( CD_DpController_CheckBox, RD_DpController_GroupName, IDS_SSHEETSSELDPC_DPCGROUP );
	pclSSheet->AddCellSpanW( CD_DpController_PipeSize, RD_DpController_GroupName, CD_DpController_Pointer - CD_DpController_PipeSize, 1 );

	CTable *pclTable = dynamic_cast<CTable *>( m_pclIndSelDpCParams->m_pPipeDB->Get( m_pclIndSelDpCParams->m_strPipeSeriesID ).MP );
	ASSERT( NULL != pclTable );

	pclSSheet->SetStaticText( CD_DpController_PipeSize, RD_DpController_GroupName, pclTable->GetName() );

	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, ( LPARAM )FALSE );

	pclSSheet->SetStaticText( CD_DpController_Name, RD_DpController_ColName, IDS_SSHEETSSEL_NAME );
	pclSSheet->SetStaticText( CD_DpController_Material, RD_DpController_ColName, IDS_SSHEETSSEL_MATERIAL );
	pclSSheet->SetStaticText( CD_DpController_Connection, RD_DpController_ColName, IDS_SSHEETSSEL_CONNECT );
	pclSSheet->SetStaticText( CD_DpController_Version, RD_DpController_ColName, IDS_SSHEETSSEL_VERSION );
	pclSSheet->SetStaticText( CD_DpController_PN, RD_DpController_ColName, IDS_SSHEETSSEL_PN );
	pclSSheet->SetStaticText( CD_DpController_Size, RD_DpController_ColName, IDS_SSHEETSSEL_SIZE );
	pclSSheet->SetStaticText( CD_DpController_Preset, RD_DpController_ColName, IDS_SSHEETSSEL_PRESET );
	pclSSheet->SetStaticText( CD_DpController_DpMin, RD_DpController_ColName, IDS_SHEETHDR_DPMIN );
	pclSSheet->SetStaticText( CD_DpController_DplRange, RD_DpController_ColName, IDS_SHEETHDR_DPLRANGE );

	if( true == m_pclIndSelDpCParams->m_bIsDpMaxChecked )
	{
		pclSSheet->SetStaticText( CD_DpController_DpMax, RD_DpController_ColName, IDS_RVIEWSSELDPC_DPMAX );
	}

	pclSSheet->SetStaticText( CD_DpController_TemperatureRange, RD_DpController_ColName, IDS_SSHEETSSEL_COLTEMPRANGE );

	pclSSheet->SetStaticText( CD_DpController_Separator, RD_DpController_ColName, _T( "" ) );

	// Pipes.
	pclSSheet->SetStaticText( CD_DpController_PipeSize, RD_DpController_ColName, IDS_SSHEETSSEL_PIPESIZE );
	pclSSheet->SetStaticText( CD_DpController_PipeLinDp, RD_DpController_ColName, IDS_SSHEETSSEL_PIPELINDP );
	pclSSheet->SetStaticText( CD_DpController_PipeV, RD_DpController_ColName, IDS_SSHEETSSEL_PIPEV );
	//
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Row units.
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();
	pclSSheet->SetStaticText( CD_DpController_Preset, RD_DpController_Unit, IDS_SHEETHDR_TURNSPOS );
	pclSSheet->SetStaticText( CD_DpController_DpMin, RD_DpController_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ) ).c_str() );
	pclSSheet->SetStaticText( CD_DpController_DplRange, RD_DpController_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ) ).c_str() );

	if( true == m_pclIndSelDpCParams->m_bIsDpMaxChecked )
	{
		pclSSheet->SetStaticText( CD_DpController_DpMax, RD_DpController_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ) ).c_str() );
	}

	pclSSheet->SetStaticText( CD_DpController_TemperatureRange, RD_DpController_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_TEMPERATURE ) ).c_str() );
	pclSSheet->SetStaticText( CD_DpController_PipeLinDp, RD_DpController_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_LINPRESSDROP ) ).c_str() );
	pclSSheet->SetStaticText( CD_DpController_PipeV, RD_DpController_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_VELOCITY ) ).c_str() );
	//
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Draw black line below column header.
	pclSSheet->SetCellBorder( CD_DpController_CheckBox, RD_DpController_Unit, CD_DpController_Separator - 1, RD_DpController_Unit, true, SS_BORDERTYPE_BOTTOM );
	pclSSheet->SetCellBorder( CD_DpController_PipeSize, RD_DpController_Unit, CD_DpController_Pointer - 1, RD_DpController_Unit, true, SS_BORDERTYPE_BOTTOM );
}

long CRViewSSelDpC::_FillOneDpCRow( CSheetDescription *pclSheetDescriptionDpC, CSSheet *pclSSheet, CSelectedValve *pSelectedTAP, long lRow,
		CDB_TAProduct *pEditedTAP )
{
	if( NULL == m_pclIndSelDpCParams || NULL == m_pclIndSelDpCParams->m_pclSelectDpCList || NULL == m_pclIndSelDpCParams->m_pclSelectDpCList->GetSelectPipeList() )
	{
		ASSERTA_RETURN( -1 );
	}
	
	// No need to do sanity check on 'pclSheetDescriptionDpC' and 'pclSSheet', it has been already done in '_FillDpControllerRows'.
	CDS_TechnicalParameter *pTechParam = m_pclIndSelDpCParams->m_pTADS->GetpTechParams();

	CDB_DpController *pDpController = dynamic_cast<CDB_DpController *>( pSelectedTAP->GetpData() );

	if( NULL == pDpController )
	{
		ASSERTA_RETURN( -1 );
	}

	long lRetRow = 0;
	bool bAtLeastOneError = false;
	bool bBest = pSelectedTAP->IsFlagSet( CSelectedBase::eBest );

	// Add this row.
	// 'true' to specify that this row can be selected.
	pclSheetDescriptionDpC->AddRows( 1, true );

	// If user wants a DpC-Mv set, we add an opened box in front of the line.
	if( true == m_pclIndSelDpCParams->m_bOnlyForSet && true == pDpController->IsPartOfaSet() )
	{
		CCDButtonBox *pCDButtonBox = CreateCellDescriptionBox( CD_DpController_Box, lRow, true, CCDButtonBox::ButtonState::BoxOpened, pclSheetDescriptionDpC );

		if( NULL != pCDButtonBox )
		{
			pCDButtonBox->ApplyInternalChange();
		}
	}

	// First columns will be set at the end!

	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, ( LPARAM )_BLACK );

	// Add checkbox.
	pclSSheet->SetCheckBox(CD_DpController_CheckBox, lRow, _T(""), false, true);

	// We can set the setting when user has chosen the balancing valve.
	pclSSheet->SetStaticText( CD_DpController_Preset, lRow, _T("-") );

	// Set DpMin.
	if( pSelectedTAP->GetDpMin() < pTechParam->GetDpCMinDp() )
	{
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, ( LPARAM )_ORANGE );
	}

	pclSSheet->SetStaticText( CD_DpController_DpMin, lRow, WriteDouble( CDimValue::SItoCU( _U_DIFFPRESS, pSelectedTAP->GetDpMin() ), 3, 0 ) );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, ( LPARAM )_BLACK );

	// Set DplRange.
	CString	str = pDpController->GetFormatedDplRange( false ).c_str();

	if( ( true == pSelectedTAP->IsFlagSet( CSelectedBase::eValveDpToLarge ) || true == pSelectedTAP->IsFlagSet( CSelectedBase::eValveDpToSmall ) ) )
	{
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, ( LPARAM )_RED );
		bAtLeastOneError = true;
	}

	pclSSheet->SetStaticText( CD_DpController_DplRange, lRow, str );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, ( LPARAM )_BLACK );

	// Set Dp max.
	if( true == m_pclIndSelDpCParams->m_bIsDpMaxChecked )
	{
		if( pDpController->GetDpmax() < m_pclIndSelDpCParams->m_dDpMax )
		{
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, ( LPARAM )_RED );
			bAtLeastOneError = true;
		}

		pclSSheet->SetStaticText( CD_DpController_DpMax, lRow, WriteDouble( CDimValue::SItoCU( _U_DIFFPRESS, pDpController->GetDpmax() ), 3, 0 ) );
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, ( LPARAM )_BLACK );
	}

	// Set the temperature range.
	if( true == pSelectedTAP->IsFlagSet( CSelectedBase::eTemperature ) )
	{
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, ( LPARAM )_RED );
		bAtLeastOneError = true;
	}

	pclSSheet->SetStaticText( CD_DpController_TemperatureRange, lRow, ( ( CDB_TAProduct * )pSelectedTAP->GetProductIDPtr().MP )->GetTempRange() );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, ( LPARAM )_BLACK );

	// Now we can set first columns in regards to current status (error, best or normal).
	if( true == bAtLeastOneError )
	{
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, ( LPARAM )_RED );
	}
	else if( true == pSelectedTAP->IsFlagSet( CSelectedBase::eBest ) )
	{
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, ( LPARAM )_GREEN );
	}

	// In addition of current flag, we have possibility that valve has the flag 'Not available' or 'Deleted'. In that case, we show valve name in red with "*" around it and
	// symbol '!' or '!!' after.
	if( true == pDpController->IsDeleted() )
	{
		pclSheetDescriptionDpC->WriteTextWithFlags( CString( pDpController->GetName() ), CD_DpController_Name, lRow, 
				CSheetDescription::RemarkFlags::Deleted, _T(""), _RED );
	}
	else if( false == pDpController->IsAvailable() )
	{
		pclSheetDescriptionDpC->WriteTextWithFlags( CString( pDpController->GetName() ), CD_DpController_Name, lRow, 
				CSheetDescription::RemarkFlags::NotAvailable, _T(""), _RED );
	}
	else
	{
		pclSSheet->SetStaticText( CD_DpController_Name, lRow, pDpController->GetName() );
	}

	if( true == bAtLeastOneError )
	{
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, ( LPARAM )_BLACK );
	}
	else if( true == pSelectedTAP->IsFlagSet( CSelectedBase::eBest ) )
	{
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, ( LPARAM )_GREEN );
	}

	pclSSheet->SetStaticText( CD_DpController_Material, lRow, pDpController->GetBodyMaterial() );
	pclSSheet->SetStaticText( CD_DpController_Connection, lRow, pDpController->GetConnect() );
	pclSSheet->SetStaticText( CD_DpController_Version, lRow, pDpController->GetVersion() );
	pclSSheet->SetStaticText( CD_DpController_PN, lRow, pDpController->GetPN().c_str() );
	pclSSheet->SetStaticText( CD_DpController_Size, lRow, pDpController->GetSize() );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, ( LPARAM )_BLACK );

	// Set pipe size.
	CSelectPipe selPipe( m_pclIndSelDpCParams );
	m_pclIndSelDpCParams->m_pclSelectDpCList->GetSelectPipeList()->GetMatchingPipe( pDpController->GetSizeKey(), selPipe );
	pclSSheet->SetStaticText( CD_DpController_PipeSize, lRow, selPipe.GetpPipe()->GetName() );

	// Set the LinDp to orange if it is above or below the technical parameters limits.
	if( selPipe.GetLinDp() > pTechParam->GetPipeMaxDp() || selPipe.GetLinDp() < pTechParam->GetPipeMinDp() )
	{
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, ( LPARAM )_ORANGE );
	}

	pclSSheet->SetStaticText( CD_DpController_PipeLinDp, lRow, WriteCUDouble( _U_LINPRESSDROP, selPipe.GetLinDp() ) );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, ( LPARAM )_BLACK );

	// Set the Velocity to orange if it is above the technical parameter limit.
	// Orange if it is below the dMinVel.
	if( selPipe.GetU() > pTechParam->GetPipeMaxVel() || selPipe.GetU() < pTechParam->GetPipeMinVel() )
	{
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, ( LPARAM )_ORANGE );
	}

	pclSSheet->SetStaticText( CD_DpController_PipeV, lRow, WriteCUDouble( _U_VELOCITY, selPipe.GetU() ) );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, ( LPARAM )_BLACK );

	// Save parameter.
	CreateCellDescriptionProduct( pclSheetDescriptionDpC->GetFirstParameterColumn(), lRow, ( LPARAM )pDpController, pclSheetDescriptionDpC );

	if( NULL != pEditedTAP && pEditedTAP == pDpController )
	{
		lRetRow = lRow;
	}

	pclSSheet->SetCellBorder( CD_DpController_CheckBox, lRow, CD_DpController_Separator - 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _GRAY );
	pclSSheet->SetCellBorder( CD_DpController_PipeSize, lRow, CD_DpController_Pointer - 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _GRAY );

	return lRetRow;
}

void CRViewSSelDpC::_UpdateDpCPresetColumn( bool bReset )
{
	if( NULL == m_pclIndSelDpCParams || NULL == m_pclIndSelDpCParams->m_pclSelectDpCList )
	{
		ASSERT( 0 );
		return;
	}

	if( false == m_pclIndSelDpCParams->m_bIsGroupDpbranchOrKvChecked )
	{
		return;
	}

	CSheetDescription *pclSheetDescriptionDpC = m_ViewDescription.GetFromSheetDescriptionID( SD_DpController );
	
	if( NULL == pclSheetDescriptionDpC || NULL == pclSheetDescriptionDpC->GetSSheetPointer() )
	{
		return;
	}
	
	CSSheet *pclSSheet = pclSheetDescriptionDpC->GetSSheetPointer();

	CSelectDpCList *pSelectedDpCList = m_pclIndSelDpCParams->m_pclSelectDpCList;

	if( NULL == pSelectedDpCList )
	{
		return;
	}

	// We remove current selection to reset it at the end of the update.
	// Remark: it's to force internal variables to maintain the right color under selection.
	pclSSheet->UnSelectMultipleRows();

	// By default, clean full opening remark if exist.
	pclSheetDescriptionDpC->RemoveOneRemark( CSheetDescription::RemarkFlags::FullOpening );
	// Clean all others (keep internal variables).
	pclSheetDescriptionDpC->ClearRemarks();

	// Loop all products.
	CCellDescription *pclCellDescription = pclSheetDescriptionDpC->GetFirstCellDescription( RVSCellDescription::CD_Product );

	while( NULL != pclCellDescription )
	{
		CCellDescriptionProduct *pCDProduct = dynamic_cast<CCellDescriptionProduct*>( pclCellDescription );

		if( NULL == pCDProduct )
		{
			ASSERT_CONTINUE;
		}

		CDB_DpController *pclDpController = dynamic_cast<CDB_DpController*>( (CData*)pCDProduct->GetProduct() );

		if( NULL == pclDpController )
		{
			ASSERT_CONTINUE;
		}

		CDB_DpCCharacteristic *pDpCChar = pclDpController->GetDpCCharacteristic();

		if( NULL == pDpCChar )
		{
			ASSERT_CONTINUE;
		}

		CSelectedValve *pclSelectedDpController = GetSelectProduct<CSelectedValve>( pclDpController, m_pclIndSelDpCParams->m_pclSelectDpCList );

		if( NULL == pclSelectedDpController )
		{
			ASSERT_CONTINUE;
		}

		if( true == bReset )
		{
			pclSSheet->SetStaticText( CD_DpController_Preset, pCDProduct->GetCellPosition().y, _T("-") );
		}
		else
		{
			CDB_RegulatingValve *pclBalancingValveSelected = dynamic_cast<CDB_RegulatingValve *>( GetCurrentBalancingValveSelected() );
			CSelectedValve *pSelectedTAP = GetSelectProduct<CSelectedValve>( pclBalancingValveSelected, m_pclIndSelDpCParams->m_pclSelectDpCList->GetBvSelected() );

			if( NULL == pSelectedTAP )
			{
				return;
			}

			double dOpening = -1.0;

			if( eDpStab::DpStabOnBranch == m_pclIndSelDpCParams->m_eDpStab )
			{
				if( eMvLoc::MvLocPrimary == m_pclIndSelDpCParams->m_eMvLoc )
				{
					dOpening = pDpCChar->GetOpening( m_pclIndSelDpCParams->m_dDpBranch );
				}
				else
				{
					dOpening = pDpCChar->GetOpening( m_pclIndSelDpCParams->m_dDpBranch + pSelectedTAP->GetDp() );
				}
			}
			else
			{
				double dDpToStabilize = CalcDp( m_pclIndSelDpCParams->m_dFlow, m_pclIndSelDpCParams->m_dKv, m_pclIndSelDpCParams->m_WC.GetDens() );

				if( eMvLoc::MvLocPrimary == m_pclIndSelDpCParams->m_eMvLoc )
				{
					dOpening = pDpCChar->GetOpening( dDpToStabilize );
				}
				else
				{
					
					dOpening = pDpCChar->GetOpening( dDpToStabilize + pSelectedTAP->GetDp() );
				}
			}

			CString str = _T("-");

			if( dOpening >= 0.0 )
			{
				str = pDpCChar->GetSettingString( dOpening );
			}

			pclSSheet->SetStaticText( CD_DpController_Preset, pCDProduct->GetCellPosition().y, str );
		}

		// Go to next product.
		pclCellDescription = pclSheetDescriptionDpC->GetNextCellDescription( pclCellDescription, RVSCellDescription::CD_Product );
	}


	// Set the selection.
	// Remark: it's to force internal variables to maintain the right color under selection.
	pclSSheet->SelectOneRow( m_lDpCSelectedRow, pclSheetDescriptionDpC->GetSelectionFrom(), pclSheetDescriptionDpC->GetSelectionTo() );
}

void CRViewSSelDpC::_FillDpCAccessoryRows( )
{
	if( NULL == m_pclIndSelDpCParams || NULL == m_pclIndSelDpCParams->m_pTADB )
	{
		ASSERT_RETURN;
	}

	// Check current control valve selected.
	CDB_DpController *pclCurrentDpControllerSelected = GetCurrentDpControllerSelected();

	if( NULL == pclCurrentDpControllerSelected )
	{
		return;
	}

	CDB_RuledTable *pRuledTable = ( CDB_RuledTable * )( pclCurrentDpControllerSelected->GetAccessoriesGroupIDPtr().MP );

	if( NULL == pRuledTable )
	{
		return;
	}

	CRank rList;
	int iCount = m_pclIndSelDpCParams->m_pTADB->GetAccessories( &rList, pRuledTable, m_pclIndSelDpCParams->m_eFilterSelection );

	if( 0 == iCount )
	{
		return;
	}

	// Call 'MultiSpreadBase' method to get a new SSheet.
	CSheetDescription *pclSheetDescriptionDpCAccessory = CreateSSheet( SD_DpCAccessories );

	if( NULL == pclSheetDescriptionDpCAccessory || NULL == pclSheetDescriptionDpCAccessory->GetSSheetPointer() )
	{
		return;
	}

	CSSheet *pclSSheet = pclSheetDescriptionDpCAccessory->GetSSheetPointer();

	pclSSheet->SetBool( SSB_REDRAW, FALSE );

	pclSSheet->Init();
	pclSSheet->SetBool( SSB_ALLOWDRAGDROP, FALSE );
	pclSSheet->SetBool( SSB_HORZSCROLLBAR, FALSE );
	pclSSheet->SetBool( SSB_VERTSCROLLBAR, FALSE );
	pclSSheet->SetTextTip( SS_TT_STATUS_FLOATING, 250, NULL, _LIGHTYELLOW, _TAH_BLACK );

	// Two lines by accessories, but two accessories by line (reason why we don't multiply iCount by 2).
	pclSSheet->SetMaxRows( RD_DpCAccessory_FirstAvailRow + iCount - 1 + ( iCount % 2 ) );

	// All cells are static by default and filled with _T("").
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	pclSSheet->FormatStaticText( -1, -1, -1, -1, _T( "" ) );

	// Get sheet description of Dp Controller to retrieve width.
	CSheetDescription *pclSheetDescriptionDpC = m_ViewDescription.GetFromSheetDescriptionID( SD_DpController );
	// It's absolutely not normal to have this pointer NULL.
	ASSERT( NULL != pclSheetDescriptionDpC );

	// Take SSheet size and client area size and determine the right width to take in count.
	CRect rectSize = pclSheetDescriptionDpC->GetSSheetPointer()->GetSheetSizeInPixels( false );

	// Prepare first column width (to match Dp controller sheet).
	long lFirstColumnWidth = pclSheetDescriptionDpC->GetSSheetPointer()->GetColWidthInPixelsW( CD_DpController_FirstColumn );

	if( true == m_pclIndSelDpCParams->m_bOnlyForSet )
	{
		lFirstColumnWidth += pclSheetDescriptionDpC->GetSSheetPointer()->GetColWidthInPixelsW( CD_DpController_Box );
	}

	// Do some correction in regards to some columns.
	long lTotalWidth = rectSize.Width() - lFirstColumnWidth;
	lTotalWidth -= pclSheetDescriptionDpC->GetSSheetPointer()->GetColWidthInPixelsW( CD_DpController_Pointer );

	// Try to create 2 columns in just the middle of Dp controller sheet.
	long lLeftWidth = ( long )( lTotalWidth / 2 );
	long lRightWidth = ( ( lTotalWidth % 2 ) > 0 ) ? lLeftWidth + 1 : lLeftWidth;

	// Set columns.
	// 'CSheetDescription::SD_ParameterMode_Multi' to set that this sheet can have more than one column with parameter.
	pclSheetDescriptionDpCAccessory->Init( 1, pclSSheet->GetMaxRows(), 2, CSheetDescription::SD_ParameterMode_Multi );

	pclSheetDescriptionDpCAccessory->AddColumnInPixels( CD_DpCAccessory_FirstColumn, lFirstColumnWidth );
	pclSheetDescriptionDpCAccessory->AddColumnInPixels( CD_DpCAccessory_Left, lLeftWidth );
	pclSheetDescriptionDpCAccessory->AddColumnInPixels( CD_DpCAccessory_Right, lRightWidth );
	pclSheetDescriptionDpCAccessory->AddColumn( CD_DpCAccessory_LastColumn, 0 );

	// These two columns can contain parameter.
	pclSheetDescriptionDpCAccessory->AddParameterColumn( CD_DpCAccessory_Left );
	pclSheetDescriptionDpCAccessory->AddParameterColumn( CD_DpCAccessory_Right );

	// Set the focus column (don't set on Left of Right (on a check box) )
	pclSheetDescriptionDpCAccessory->SetActiveColumn( CD_DpCAccessory_FirstColumn );

	// Set selectable rows.
	pclSheetDescriptionDpCAccessory->SetSelectableRangeRow( RD_DpCAccessory_FirstAvailRow, pclSSheet->GetMaxRows() - 1 );

	// Increase row height.
	double RowHeight = 12.75;
	pclSSheet->SetRowHeight( RD_DpCAccessory_FirstRow, RowHeight * 0.5 );
	pclSSheet->SetRowHeight( RD_DpCAccessory_GroupName, RowHeight * 1.5 );

	// Set title.
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, ( LPARAM )GetTitleBackgroundColor( pclSheetDescriptionDpCAccessory ) );

	pclSheetDescriptionDpCAccessory->SetUserVariable( _SDUV_TITLEFORECOLOR,
			( pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor ) ) );
	pclSheetDescriptionDpCAccessory->SetUserVariable( _SDUV_TITLEBACKCOLOR,
			( pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor ) ) );

	pclSSheet->AddCellSpanW( CD_DpCAccessory_Left, RD_DpCAccessory_GroupName, CD_DpCAccessory_LastColumn - CD_DpCAccessory_Left, 1 );
	pclSSheet->SetStaticText( CD_DpCAccessory_Left, RD_DpCAccessory_GroupName, IDS_SSHEETSSELDPC_DPCACCGROUP );

	// Fill accessories.
	long lRow = RD_DpCAccessory_FirstAvailRow;

	CString str;
	LPARAM lparam;

	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, ( LPARAM )FALSE );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, ( LPARAM )SSS_ALIGN_LEFT );

	// Left - Right.
	long lLeftOrRight = CD_DpCAccessory_Left;
	BOOL bContinue = rList.GetFirst( str, lparam );

	while( TRUE == bContinue )
	{
		CDB_Product *pAccessory = (CDB_Product * )lparam;
		VERIFY( pAccessory );
		//VERIFY( pAccessory->IsClass( CLASS( CDB_Product ) ) );
		
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

		// If accessory has its property 'IsAttached' to true and user is in selection by set, he can't select it.
		bool bEnabled = !pAccessory->IsAttached();

		CCDBCheckboxAccessory *pCheckbox = CreateCheckboxAccessory( lLeftOrRight, lRow, false, bEnabled, strName, pAccessory, 
				pRuledTable, &m_vecDpCAccessoryList, pclSheetDescriptionDpCAccessory );

		if( NULL != pCheckbox )
		{
			pCheckbox->ApplyInternalChange();

			if( false == bEnabled )
			{
				pCheckbox->SetToolTip( TASApp.LoadLocalizedString( AFXMSG_ACCATTACHEDNOTINSET ) );
			}
		}

		// Description.
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleEllipses, ( LPARAM )TRUE );
		pclSSheet->SetStaticText( lLeftOrRight, lRow + 1, pAccessory->GetComment() );
		bContinue = rList.GetNext( str, lparam );

		// Restart left part.
		if( bContinue && lLeftOrRight == CD_DpCAccessory_Right )
		{
			pclSSheet->SetCellBorder( CD_DpCAccessory_Left, lRow + 1, CD_DpCAccessory_LastColumn - 1, lRow + 1, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _GRAY );
			lLeftOrRight = CD_DpCAccessory_Left;
			lRow += 2;
			ASSERT( lRow < pclSSheet->GetMaxRows() );
		}
		else
		{
			lLeftOrRight = CD_DpCAccessory_Right;
		}

		if( FALSE == bContinue )
		{
			lRow++;
		}
	}

	VerifyCheckboxAccessories( NULL, false, &m_vecDpCAccessoryList );

	m_pCDBExpandCollapseGroupDpCAccessory = CreateExpandCollapseGroupButton( CD_DpCAccessory_FirstColumn, RD_DpCAccessory_GroupName, true,
			CCDButtonExpandCollapseGroup::ButtonState::CollapseRow, pclSheetDescriptionDpCAccessory->GetFirstSelectableRow(), 
			pclSheetDescriptionDpCAccessory->GetLastSelectableRow( false ), pclSheetDescriptionDpCAccessory );

	// Show button.
	if( NULL != m_pCDBExpandCollapseGroupDpCAccessory )
	{
		m_pCDBExpandCollapseGroupDpCAccessory->SetShowStatus( true );
	}

	pclSSheet->SetCellBorder( CD_DpCAccessory_Left, lRow, CD_DpCAccessory_LastColumn - 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );

	pclSSheet->SetBool( SSB_REDRAW, TRUE );
}

void CRViewSSelDpC::_ClickOnBalancingValve( CSheetDescription *pclSheetDescriptionBv, CDB_RegulatingValve *pBalancingValve,
		CCellDescriptionProduct *pclCellDescriptionProduct, long lColumn, long lRow, bool bExternalCall )
{
	if( NULL == m_pclIndSelDpCParams || NULL == m_pclIndSelDpCParams->m_pclSelectDpCList )
	{
		ASSERT( 0 );
		return;
	}

	// Check Dp controller selected.
	CDB_DpController *pclCurrentDpControllerSelected = GetCurrentDpControllerSelected();

	if( NULL == pclCurrentDpControllerSelected )
	{
		return;
	}

	CSSheet *pclSSheet = pclSheetDescriptionBv->GetSSheetPointer();

	LPARAM lBvTotalCount;
	pclSheetDescriptionBv->GetUserVariable( _SDUV_TOTALPRODUCT_COUNT, lBvTotalCount );

	// Do nothing if user wants to click on a balancing valve belonging to a set in which there is only ONE balancing valve.
	if( true == bExternalCall && true == m_pclIndSelDpCParams->m_bOnlyForSet && true == pclCurrentDpControllerSelected->IsPartOfaSet() && 1 == lBvTotalCount )
	{
		return;
	}

	// Memorize previous focus row.
	long lCurrentFocusedRow = GetCurrentFocusedRow();

	// To be sure to clean all previous data concerning focus.
	KillCurrentFocus();

	// Unselect row where current selection is set (yellow light).
	// Remark: 'KillCurrentFocus' unselect only rows where focus is on (blue light).
	pclSSheet->UnSelectMultipleRows();

	// By default reset setting of Dp controller if exist.
	_UpdateDpCPresetColumn( true );

	// By default clear accessories list.
	m_vecBvAccessoryList.clear();
	m_pCDBExpandCollapseGroupBvAccessory = NULL;

	LPARAM lBalancingValveCount;
	pclSheetDescriptionBv->GetUserVariable( _SDUV_TOTALPRODUCT_COUNT, lBalancingValveCount );

	// Retrieve the current selected control valve if exist.
	CDB_TAProduct *pclCurrentBalancingValveSelected = GetCurrentBalancingValveSelected();

	// If there is already one balancing valve selected and user click on the current one...
	// Remark: 'm_pCDBExpandCollapseRowsBv' is not created if there is only one balancing valve. Thus we need to check first if there is only one balancing valve.
	//         Otherwise we have to check if user has clicked on the current one.
	if( NULL != pclCurrentBalancingValveSelected 
			&& ( ( 1 == lBalancingValveCount ) || ( NULL != m_pCDBExpandCollapseRowsBv && lRow == m_pCDBExpandCollapseRowsBv->GetCellPosition().y ) ) )
	{
		// Reset current product selected.
		SetCurrentBalancingValveSelected( NULL );

		// Uncheck checkbox.
		pclSSheet->SetCheckBox(CD_BalancingValve_CheckBox, lRow, _T(""), false, true);

		// Delete Expand/Collapse rows button if exist.
		// Remark: 'm_pCDBExpandCollapseRowsBv' is set to NULL in method!
		DeleteExpandCollapseRowsButton( m_pCDBExpandCollapseRowsBv, pclSheetDescriptionBv );

		// Show Show/Hide all priorities button if exist.
		if( NULL != m_pCDBShowAllPrioritiesBv )
		{
			m_pCDBShowAllPrioritiesBv->ApplyInternalChange();
		}

		// Set focus on balancing valve currently selected.
		PrepareAndSetNewFocus( pclSheetDescriptionBv, pclSheetDescriptionBv->GetActiveColumn(), lRow, 0 );

		// Remove all sheets after balancing valve.
		m_ViewDescription.RemoveAllSheetAfter( SD_BalancingValve );

		if( true == m_pclIndSelDpCParams->m_bOnlyForSet && true == pclCurrentDpControllerSelected->IsPartOfaSet() )
		{
			CSheetDescription *pclSheetDescriptionDpC = m_ViewDescription.GetFromSheetDescriptionID( SD_DpController );

			if( NULL != pclSheetDescriptionDpC )
			{
				// Change box button to open state for Dp controller.
				CCDButtonBox *pCDButtonBox = GetCDButtonBox( CD_DpController_Box, m_lDpCSelectedRow, pclSheetDescriptionDpC );

				if( NULL != pCDButtonBox )
				{
					pCDButtonBox->SetButtonState( CCDButtonBox::ButtonState::BoxOpened,  true );
				}
			}

			// Change box button to open state for balancing valve selected.
			CCDButtonBox *pCDButtonBox = GetCDButtonBox( CD_BalancingValve_Box, lRow, pclSheetDescriptionBv );

			if( NULL != pCDButtonBox )
			{
				pCDButtonBox->SetButtonState( CCDButtonBox::ButtonState::BoxOpened,  true );
			}
		}

		// HYS-1382: Auto check or uncheck crossing accessories
		if( true == m_ViewDescription.IsSheetDescriptionExist( SD_DpCAccessories ) )
		{
			_SelectAutoCrossingAccessories( (CDB_RegulatingValve*)GetCurrentBalancingValveSelected(), false );
		}
		// Update DlgInfoSSelDpC.
		UpdateBvInfos();
		m_lBvSelectedRow = 0;

		// Notify a change in the selection.
		SelectionHasChanged( IsSelectionReady() );
	}
	else
	{
		// If a balancing valve is already selected...
		if( NULL != pclCurrentBalancingValveSelected )
		{
			// Uncheck checkbox.
			pclSSheet->SetCheckBox(CD_BalancingValve_CheckBox, m_lBvSelectedRow, _T(""), false, true);

			// Remove all sheets after balancing valve.
			m_ViewDescription.RemoveAllSheetAfter( SD_BalancingValve );

			// Change box button to open state for previous balancing valve.
			if( true == m_pclIndSelDpCParams->m_bOnlyForSet && true == pclCurrentDpControllerSelected->IsPartOfaSet() )
			{
				CCDButtonBox *pCDButtonBox = GetCDButtonBox( CD_BalancingValve_Box, m_lBvSelectedRow, pclSheetDescriptionBv );

				if( NULL != pCDButtonBox )
				{
					pCDButtonBox->SetButtonState( CCDButtonBox::ButtonState::BoxOpened,  true );
				}
			}
		}

		// Save new balancing valve selection.
		SetCurrentBalancingValveSelected( pclCellDescriptionProduct );

		// Check checkbox.
		pclSSheet->SetCheckBox( CD_BalancingValve_CheckBox, lRow, _T(""), true, true );

		// Update DlgInfoSSelDpC.
		UpdateBvInfos();

		// Compute setting of Dp controller if allow.
		_UpdateDpCPresetColumn( false );

		// Delete Expand/Collapse rows button if exist.
		if( NULL != m_pCDBExpandCollapseRowsBv )
		{
			// Remark: 'm_pCDBExpandCollapseRowsBv' is set to NULL in method!
			DeleteExpandCollapseRowsButton( m_pCDBExpandCollapseRowsBv, pclSheetDescriptionBv );
		}

		// Create Expand/Collapse rows button if needed...
		if( lBvTotalCount > 1 )
		{
			m_pCDBExpandCollapseRowsBv = CreateExpandCollapseRowsButton( CD_BalancingValve_FirstColumn, lRow, true, 
					CCDButtonExpandCollapseRows::ButtonState::CollapseRow, pclSheetDescriptionBv->GetFirstSelectableRow(), 
					pclSheetDescriptionBv->GetLastSelectableRow( false ), pclSheetDescriptionBv );

			// Show button.
			if( NULL != m_pCDBExpandCollapseRowsBv )
			{
				m_pCDBExpandCollapseRowsBv->SetShowStatus( true );
			}
		}

		// Select balancing valve (just highlight background).
		pclSSheet->SelectOneRow( lRow, pclSheetDescriptionBv->GetSelectionFrom(), pclSheetDescriptionBv->GetSelectionTo() );

		// Fill accessories available for the current balancing valve.
		_FillBvAccessoryRows();

		// Try to set the focus on the next table but do not lose the focus if no other table exist.
		CSheetDescription *pclNextSheetDescription = NULL;
		long lNewFocusedRow;
		bool fShiftPressed;

		if( DNFERRC_FocusOnNextGroupOK == DefineNextFocusedRow( pclSheetDescriptionBv, CD_BalancingValve_FirstColumn, lRow, false, lNewFocusedRow,
				pclNextSheetDescription, fShiftPressed ) )
		{
			long lNewFocusedCol = pclNextSheetDescription->GetFirstParameterColumn();
			PrepareAndSetNewFocus( pclNextSheetDescription, lNewFocusedCol, lNewFocusedRow );
		}
		else
		{
			// Because we have changed current selection background in yellow, if there is no other sheet following current one, we
			// must set focus on current selection.
			PrepareAndSetNewFocus( pclSheetDescriptionBv, lColumn, lRow );
		}

		m_lBvSelectedRow = lRow;

		// If user has selected DpC-MV set mode and user has directly clicked on balancing valve (otherwise, it's a call
		// coming from '_ClickOnDpController' and this method will check itself if it must close or not the box button)...
		if( true == m_pclIndSelDpCParams->m_bOnlyForSet && true == pclCurrentDpControllerSelected->IsPartOfaSet() && true == bExternalCall )
		{
			CSheetDescription *pclSheetDescriptionDpC = m_ViewDescription.GetFromSheetDescriptionID( SD_DpController );

			if( NULL != pclSheetDescriptionDpC )
			{
				// Change box button to open state for Dp controller.
				CCDButtonBox *pCDButtonBox = GetCDButtonBox( CD_DpController_Box, m_lDpCSelectedRow, pclSheetDescriptionDpC );

				if( NULL != pCDButtonBox )
				{
					pCDButtonBox->SetButtonState( CCDButtonBox::ButtonState::BoxClosed,  true );
				}
			}

			// Change box button to open state for balancing valve selected.
			CCDButtonBox *pCDButtonBox = GetCDButtonBox( CD_BalancingValve_Box, lRow, pclSheetDescriptionBv );

			if( NULL != pCDButtonBox )
			{
				pCDButtonBox->SetButtonState( CCDButtonBox::ButtonState::BoxClosed,  true );
			}
		}

		// HYS-1382: Auto check or uncheck crossing accessories
		if( true == m_ViewDescription.IsSheetDescriptionExist( SD_DpCAccessories ) )
		{
			_SelectAutoCrossingAccessories( (CDB_RegulatingValve*)GetCurrentBalancingValveSelected(), true );
		}
		// Notify a change in the selection.
		SelectionHasChanged( IsSelectionReady() );

		if( eMvLoc::MvLocSecondary == m_pclIndSelDpCParams->m_eMvLoc )
		{
			CSelectedValve *pSelectedTAP = GetSelectProduct<CSelectedValve>( pclCurrentBalancingValveSelected, m_pclIndSelDpCParams->m_pclSelectDpCList );

			if( NULL != pSelectedTAP )
			{
				m_pclIndSelDpCParams->m_pclSelectDpCList->SetDpToStab( m_pclIndSelDpCParams->m_dDpBranch + pSelectedTAP->GetRequiredDpMv() );
			}
		}
	}
}

void CRViewSSelDpC::_FillBalancingValveRows( CSelectedValve *pSelectedTAPDpC )
{
	if( NULL == m_pclIndSelDpCParams || NULL == m_pclIndSelDpCParams->m_pclSelectDpCList || NULL == m_pclIndSelDpCParams->m_pTADB )
	{
		ASSERT( 0 );
		return;
	}

	if( NULL == pSelectedTAPDpC )
	{
		return;
	}

	CDB_DpController *pDpController = dynamic_cast<CDB_DpController *>( pSelectedTAPDpC->GetpData() );

	if( NULL == pDpController )
	{
		return;
	}

	// Try to retrieve Dp Controller selected list.
	CSelectDpCList *pclDpCList = m_pclIndSelDpCParams->m_pclSelectDpCList;

	if( NULL == pclDpCList )
	{
		return;
	}

	CSelectList *pSelectedBvList = NULL;

	// If user doesn't want make a selection in a DpC-MV set...
	if( false == m_pclIndSelDpCParams->m_bOnlyForSet || ( true == m_pclIndSelDpCParams->m_bOnlyForSet && false == pDpController->IsPartOfaSet() ) )
	{
		// Retrieve balancing valve list linked to current Dp controller.
		if( pclDpCList->SelectManBV( m_pclIndSelDpCParams, pSelectedTAPDpC ) > 0 )
		{
			pSelectedBvList = pclDpCList->GetBvSelected();
		}
	}
	else
	{
		// Show only corresponding balancing valve to the Dp controller for the set.
		CTableSet *pTableSet = dynamic_cast<CTableSet *>( m_pclIndSelDpCParams->m_pTADB->Get( _T("DPCMVSET_TAB") ).MP );
		std::set<CDB_Set *> DpCMVSet;

		int iSetCount  = 0;

		if( NULL != pTableSet )
		{
			iSetCount = pTableSet->FindCompatibleSet( &DpCMVSet, pSelectedTAPDpC->GetProductIDPtr().ID, _T( "" ) );
		}

		if( iSetCount > 0 )
		{
			// Retrieve balancing valve list linked to current Dp controller.
			if( m_pclIndSelDpCParams->m_pclSelectDpCList->SelectManBVSet( m_pclIndSelDpCParams, &DpCMVSet, pSelectedTAPDpC ) > 0 )
			{
				pSelectedBvList = m_pclIndSelDpCParams->m_pclSelectDpCList->GetBvSelected( );
			}
		}
	}

	// If no balancing valve exist...
	if( NULL == pSelectedBvList || 0 == pSelectedBvList->GetCount() )
	{
		return;
	}

	// Call 'MultiSpreadBase' method to get a new SSheet.
	CSheetDescription *pclSheetDescriptionBv = CreateSSheet( SD_BalancingValve );

	if( NULL == pclSheetDescriptionBv || NULL == pclSheetDescriptionBv->GetSSheetPointer() )
	{
		return;
	}

	CSSheet *pclSSheet = pclSheetDescriptionBv->GetSSheetPointer();

	pclSSheet->SetBool( SSB_REDRAW, FALSE );

	pclSSheet->Init();
	pclSSheet->SetBool( SSB_ALLOWDRAGDROP, FALSE );
	pclSSheet->SetBool( SSB_HORZSCROLLBAR, FALSE );
	pclSSheet->SetBool( SSB_VERTSCROLLBAR, FALSE );
	pclSSheet->SetBool( SSB_MOVEACTIVEONFOCUS, FALSE );
	pclSSheet->SetTextTip( SS_TT_STATUS_FLOATING, 250, NULL, _LIGHTYELLOW, _TAH_BLACK );

	// Initialize Balancing Valve sheet and fill header.
	_InitAndFillBvHeader( pclSheetDescriptionBv, pclSSheet );

	long lRow = RD_BalancingValve_FirstAvailRow;

	long lBvNumbers = 0;
	long lBvNotPriorityNumbers = 0;
	bool bShowAllPrioritiesShown = false;
	long lShowAllPrioritiesButtonRow = 0;

	pclSheetDescriptionBv->SetUserVariable( _SDUV_TOTALPRODUCT_COUNT, 0 );
	pclSheetDescriptionBv->SetUserVariable( _SDUV_NOPRIORITYPRODUCT_COUNT, 0 );

	bool bAtLeastOneBVPartOfASet = false;
	pclSheetDescriptionBv->RestartRemarkGenerator();

	for( CSelectedValve *pclSelectedBalancingValve = pSelectedBvList->GetFirst<CSelectedValve>(); NULL != pclSelectedBalancingValve;
		 pclSelectedBalancingValve = pSelectedBvList->GetNext<CSelectedValve>() )
	{
		CDB_TAProduct *pTAP = dynamic_cast<CDB_TAProduct *>( pclSelectedBalancingValve->GetpData() );

		if( NULL == pTAP )
		{
			continue;
		}

		if( true == pTAP->IsPartOfaSet() )
		{
			bAtLeastOneBVPartOfASet = true;
		}

		if( true == pclSelectedBalancingValve->IsFlagSet( CSelectedBase::eNotPriority ) )
		{
			lBvNotPriorityNumbers++;

			if( false == bShowAllPrioritiesShown )
			{
				// Add one empty not selectable row.
				pclSheetDescriptionBv->AddRows( 1 );
				pclSheetDescriptionBv->RemoveSelectableRow( lRow );

				pclSSheet->SetCellBorder( CD_BalancingValve_CheckBox, lRow, CD_BalancingValve_Separator - 1, lRow, true, 
						SS_BORDERTYPE_TOP | SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );

				pclSSheet->SetCellBorder( CD_BalancingValve_PipeSize, lRow, CD_BalancingValve_Pointer - 1, lRow, true, 
						SS_BORDERTYPE_TOP | SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );

				lShowAllPrioritiesButtonRow = lRow++;
				bShowAllPrioritiesShown = true;
			}
		}

		// If no error...
		if( 0 == _FillOneBvRow( pclSheetDescriptionBv, pclSSheet, pclSelectedBalancingValve, lRow ) )
		{
			lRow++;
			lBvNumbers++;
		}
	}

	long lLastDataRow = lRow - 1;

	// If we are not in selection by package mode OR if there is no valve that belongs to a set...
	if( false == m_pclIndSelDpCParams->m_bOnlyForSet || false == bAtLeastOneBVPartOfASet )
	{
		pclSSheet->ShowCol( CD_BalancingValve_Box, FALSE );
	}

	pclSheetDescriptionBv->SetUserVariable( _SDUV_TOTALPRODUCT_COUNT, lBvNumbers );
	pclSheetDescriptionBv->SetUserVariable( _SDUV_NOPRIORITYPRODUCT_COUNT, lBvNotPriorityNumbers );

	// Now we can create Show/Hide all priorities if needed.
	// Remarks: 2nd condition to avoid to have button if we have only no priority valves.
	if( true == bShowAllPrioritiesShown && lBvNumbers > lBvNotPriorityNumbers )
	{
		// Create Show/Hide all priorities button in regards to current state!
		CCDButtonShowAllPriorities::ButtonState eState = ( true == GetShowAllPrioritiesFlag( SD_BalancingValve ) ) ? 
				CCDButtonShowAllPriorities::ButtonState::HidePriorities : CCDButtonShowAllPriorities::ButtonState::ShowPriorities;
		
		CCDButtonShowAllPriorities *pclShowAllButton = CreateShowAllPrioritiesButton( CD_BalancingValve_FirstColumn, lShowAllPrioritiesButtonRow, 
				bShowAllPrioritiesShown, eState, lShowAllPrioritiesButtonRow + 1, pclSSheet->GetMaxRows(), pclSheetDescriptionBv );

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

	// Try to merge only if there is more than one balancing valve.
	if( lBvNumbers > 2 || ( 2 == lBvNumbers && lBvNotPriorityNumbers != 1 ) )
	{
		vector<long> vecColumnList;
		vecColumnList.push_back( CD_BalancingValve_PipeSize );
		vecColumnList.push_back( CD_BalancingValve_PipeLinDp );
		vecColumnList.push_back( CD_BalancingValve_PipeV );
		pclSSheet->MergeRowsWithSameValues( RD_BalancingValve_FirstAvailRow, lLastDataRow, vecColumnList );
	}

	pclSSheet->SetCellBorder( CD_BalancingValve_CheckBox, lLastDataRow, CD_BalancingValve_Separator - 1, lLastDataRow, true, 
			SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );

	// Needed by pipe merging
	pclSSheet->SetCellBorder( CD_BalancingValve_PipeSize, lLastDataRow, CD_BalancingValve_Pointer - 1, lLastDataRow, true, 
			SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );

	// Add Remarks.
	// Note: these lines must be placed after creating Show/Hide all priorities button to avoid to take in count these lines when defining
	//       range of row to be Show/Hide.
	lRow = pclSheetDescriptionBv->WriteRemarks( lRow, CD_BalancingValve_CheckBox, CD_BalancingValve_Separator );

	pclSSheet->SetBool( SSB_REDRAW, TRUE );

	// Set that there is no selection at now.
	SetCurrentBalancingValveSelected( NULL );

	// Add possibility to change column size.
	// Remark: place this code after any initialization (like adding, removing, resizing, hiding/showing columns).
	CResizingColumnInfo *pclResizingColumnInfo = pclSheetDescriptionBv->GetResizingColumnInfo();
	pclResizingColumnInfo->ActivateFeature( true, false, RD_BalancingValve_FirstAvailRow, lLastDataRow );
	pclResizingColumnInfo->AddRangeColumn( CD_BalancingValve_Name, CD_BalancingValve_TemperatureRange, RD_BalancingValve_ColName, RD_BalancingValve_Unit );
	pclResizingColumnInfo->AddRangeColumn( CD_BalancingValve_PipeSize, CD_BalancingValve_PipeV, RD_BalancingValve_ColName, RD_BalancingValve_Unit );

	// Add possibility to click on title group to reset column width.
	CreateTitleGroupButtons( CD_BalancingValve_CheckBox, CD_BalancingValve_PipeV, RD_BalancingValve_GroupName, pclSheetDescriptionBv );
}

void CRViewSSelDpC::_InitAndFillBvHeader( CSheetDescription *pclSheetDescriptionBv, CSSheet *pclSSheet )
{
	if( NULL == m_pclIndSelDpCParams || NULL == m_pclIndSelDpCParams->m_pclSelectDpCList || NULL == m_pclIndSelDpCParams->m_pclSelectDpCList->GetBvSelected()
			|| NULL == m_pclIndSelDpCParams->m_pPipeDB )
	{
		ASSERT_RETURN;
	}

	// Flags for column customizing
	CSelectedBase *pclSelectedProduct = m_pclIndSelDpCParams->m_pclSelectDpCList->GetBvSelected()->GetFirst<CSelectedValve>();

	if( NULL == pclSelectedProduct )
	{
		return;
	}

	CDB_TAProduct *pTAP = dynamic_cast<CDB_TAProduct *>( pclSelectedProduct->GetpData() );

	if( NULL == pTAP )
	{
		return;
	}

	bool bKvSignalExist = ( NULL != pTAP->IsKvSignalEquipped() );

	pclSSheet->SetMaxRows( RD_BalancingValve_FirstAvailRow - 1 );

	// All cells are static by default and filled with _T("")
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	pclSSheet->FormatStaticText( -1, -1, -1, -1, _T( "" ) );

	// Increase row height.
	double dRowHeight = 12.75;
	pclSSheet->SetRowHeight( RD_BalancingValve_FirstRow, dRowHeight * 0.5 );
	pclSSheet->SetRowHeight( RD_BalancingValve_GroupName, dRowHeight * 1.5 );
	pclSSheet->SetRowHeight( RD_BalancingValve_ColName, dRowHeight * 2 );
	pclSSheet->SetRowHeight( RD_BalancingValve_Unit, dRowHeight * 1.2 );

	// Set columns.
	pclSheetDescriptionBv->Init( 1, pclSSheet->GetMaxRows(), 1, CSheetDescription::SD_ParameterMode_Mono );
	pclSheetDescriptionBv->AddColumnInPixels( CD_BalancingValve_FirstColumn, m_mapSSheetColumnWidth[SD_BalancingValve][CD_BalancingValve_FirstColumn] );
	pclSheetDescriptionBv->AddColumnInPixels( CD_BalancingValve_Box, m_mapSSheetColumnWidth[SD_BalancingValve][CD_BalancingValve_Box] );
	pclSheetDescriptionBv->AddColumnInPixels( CD_BalancingValve_CheckBox, m_mapSSheetColumnWidth[SD_BalancingValve][CD_BalancingValve_CheckBox] );
	pclSheetDescriptionBv->AddColumnInPixels( CD_BalancingValve_Name, m_mapSSheetColumnWidth[SD_BalancingValve][CD_BalancingValve_Name] );
	pclSheetDescriptionBv->AddColumnInPixels( CD_BalancingValve_Material, m_mapSSheetColumnWidth[SD_BalancingValve][CD_BalancingValve_Material] );
	pclSheetDescriptionBv->AddColumnInPixels( CD_BalancingValve_Connection, m_mapSSheetColumnWidth[SD_BalancingValve][CD_BalancingValve_Connection] );
	pclSheetDescriptionBv->AddColumnInPixels( CD_BalancingValve_Version, m_mapSSheetColumnWidth[SD_BalancingValve][CD_BalancingValve_Version] );
	pclSheetDescriptionBv->AddColumnInPixels( CD_BalancingValve_PN, m_mapSSheetColumnWidth[SD_BalancingValve][CD_BalancingValve_PN] );
	pclSheetDescriptionBv->AddColumnInPixels( CD_BalancingValve_Size, m_mapSSheetColumnWidth[SD_BalancingValve][CD_BalancingValve_Size] );
	pclSheetDescriptionBv->AddColumnInPixels( CD_BalancingValve_Preset, m_mapSSheetColumnWidth[SD_BalancingValve][CD_BalancingValve_Preset] );
	pclSheetDescriptionBv->AddColumnInPixels( CD_BalancingValve_DpSignal, m_mapSSheetColumnWidth[SD_BalancingValve][CD_BalancingValve_DpSignal] );
	pclSheetDescriptionBv->AddColumnInPixels( CD_BalancingValve_Dp, m_mapSSheetColumnWidth[SD_BalancingValve][CD_BalancingValve_Dp] );
	pclSheetDescriptionBv->AddColumnInPixels( CD_BalancingValve_DpFullOpening, m_mapSSheetColumnWidth[SD_BalancingValve][CD_BalancingValve_DpFullOpening] );
	pclSheetDescriptionBv->AddColumnInPixels( CD_BalancingValve_DpHalfOpening, m_mapSSheetColumnWidth[SD_BalancingValve][CD_BalancingValve_DpHalfOpening] );
	pclSheetDescriptionBv->AddColumnInPixels( CD_BalancingValve_TemperatureRange, m_mapSSheetColumnWidth[SD_BalancingValve][CD_BalancingValve_TemperatureRange] );
	pclSheetDescriptionBv->AddColumnInPixels( CD_BalancingValve_Separator, m_mapSSheetColumnWidth[SD_BalancingValve][CD_BalancingValve_Separator] );
	pclSheetDescriptionBv->AddColumnInPixels( CD_BalancingValve_PipeSize, m_mapSSheetColumnWidth[SD_BalancingValve][CD_BalancingValve_PipeSize] );
	pclSheetDescriptionBv->AddColumnInPixels( CD_BalancingValve_PipeLinDp, m_mapSSheetColumnWidth[SD_BalancingValve][CD_BalancingValve_PipeLinDp] );
	pclSheetDescriptionBv->AddColumnInPixels( CD_BalancingValve_PipeV, m_mapSSheetColumnWidth[SD_BalancingValve][CD_BalancingValve_PipeV] );
	pclSheetDescriptionBv->AddColumnInPixels( CD_BalancingValve_Pointer, m_mapSSheetColumnWidth[SD_BalancingValve][CD_BalancingValve_Pointer] );

	// Set in which column parameter must be saved.
	pclSheetDescriptionBv->AddParameterColumn( CD_BalancingValve_Pointer );

	// Set the focus column.
	pclSheetDescriptionBv->SetActiveColumn( CD_BalancingValve_Name );

	// Set range for selection.
	pclSheetDescriptionBv->SetFocusColumnRange( CD_BalancingValve_CheckBox, CD_BalancingValve_TemperatureRange );

	// Hide columns for DpSignal if not needed.
	pclSSheet->ShowCol( CD_BalancingValve_DpSignal, ( true == bKvSignalExist ) ? TRUE : FALSE );

	// Column Full opening is show if the user doesn't input specify pressure drop.
	pclSSheet->ShowCol( CD_BalancingValve_DpFullOpening, ( false == m_pclIndSelDpCParams->m_bIsGroupDpbranchOrKvChecked ) ? TRUE : FALSE );
	pclSSheet->ShowCol( CD_BalancingValve_DpHalfOpening, ( false == m_pclIndSelDpCParams->m_bIsGroupDpbranchOrKvChecked ) ? TRUE : FALSE );

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Row name.

	// Add the main title.
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, ( LPARAM )GetTitleBackgroundColor( pclSheetDescriptionBv ) );
	pclSheetDescriptionBv->SetUserVariable( _SDUV_TITLEFORECOLOR, ( pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor ) ) );
	pclSheetDescriptionBv->SetUserVariable( _SDUV_TITLEBACKCOLOR, ( pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor ) ) );

	pclSSheet->AddCellSpanW( CD_BalancingValve_CheckBox, RD_BalancingValve_GroupName, CD_BalancingValve_Separator - CD_BalancingValve_CheckBox, 1 );
	pclSSheet->SetStaticText( CD_BalancingValve_CheckBox, RD_BalancingValve_GroupName, IDS_SSHEETSSELDPC_BVGROUP );
	pclSSheet->AddCellSpanW( CD_BalancingValve_PipeSize, RD_BalancingValve_GroupName, CD_BalancingValve_Pointer - CD_BalancingValve_PipeSize, 1 );

	CTable *pclTable = dynamic_cast<CTable *>( m_pclIndSelDpCParams->m_pPipeDB->Get( m_pclIndSelDpCParams->m_strPipeSeriesID ).MP );
	ASSERT( NULL != pclTable );

	pclSSheet->SetStaticText( CD_BalancingValve_PipeSize, RD_BalancingValve_GroupName, pclTable->GetName() );

	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, ( LPARAM )FALSE );

	pclSSheet->SetStaticText( CD_BalancingValve_Name, RD_BalancingValve_ColName, IDS_SSHEETSSEL_NAME );
	pclSSheet->SetStaticText( CD_BalancingValve_Material, RD_BalancingValve_ColName, IDS_SSHEETSSEL_MATERIAL );
	pclSSheet->SetStaticText( CD_BalancingValve_Connection, RD_BalancingValve_ColName, IDS_SSHEETSSEL_CONNECT );
	pclSSheet->SetStaticText( CD_BalancingValve_Version, RD_BalancingValve_ColName, IDS_SSHEETSSEL_VERSION );
	pclSSheet->SetStaticText( CD_BalancingValve_PN, RD_BalancingValve_ColName, IDS_SSHEETSSEL_PN );
	pclSSheet->SetStaticText( CD_BalancingValve_Size, RD_BalancingValve_ColName, IDS_SSHEETSSEL_SIZE );
	pclSSheet->SetStaticText( CD_BalancingValve_Preset, RD_BalancingValve_ColName, IDS_SSHEETSSEL_PRESET );
	pclSSheet->SetStaticText( CD_BalancingValve_DpSignal, RD_BalancingValve_ColName, IDS_SHEETHDR_SIGNAL );
	pclSSheet->SetStaticText( CD_BalancingValve_Dp, RD_BalancingValve_ColName, IDS_SSHEETSSEL_DP );
	pclSSheet->SetStaticText( CD_BalancingValve_DpFullOpening, RD_BalancingValve_ColName, IDS_SSHEETSSEL_DPFO );
	pclSSheet->SetStaticText( CD_BalancingValve_DpHalfOpening, RD_BalancingValve_ColName, IDS_SSHEETSSEL_DPHO );
	pclSSheet->SetStaticText( CD_BalancingValve_TemperatureRange, RD_BalancingValve_ColName, IDS_SSHEETSSEL_COLTEMPRANGE );

	pclSSheet->SetStaticText( CD_BalancingValve_Separator, RD_BalancingValve_ColName, _T( "" ) );

	// Pipes
	pclSSheet->SetStaticText( CD_BalancingValve_PipeSize, RD_BalancingValve_ColName, IDS_SSHEETSSEL_PIPESIZE );
	pclSSheet->SetStaticText( CD_BalancingValve_PipeLinDp, RD_BalancingValve_ColName, IDS_SSHEETSSEL_PIPELINDP );
	pclSSheet->SetStaticText( CD_BalancingValve_PipeV, RD_BalancingValve_ColName, IDS_SSHEETSSEL_PIPEV );
	//
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Row units
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();
	pclSSheet->SetStaticText( CD_BalancingValve_Dp, RD_BalancingValve_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ) ).c_str() );
	pclSSheet->SetStaticText( CD_BalancingValve_DpSignal, RD_BalancingValve_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ) ).c_str() );
	pclSSheet->SetStaticText( CD_BalancingValve_DpFullOpening, RD_BalancingValve_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ) ).c_str() );
	pclSSheet->SetStaticText( CD_BalancingValve_DpHalfOpening, RD_BalancingValve_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ) ).c_str() );
	pclSSheet->SetStaticText( CD_BalancingValve_TemperatureRange, RD_BalancingValve_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_TEMPERATURE ) ).c_str() );
	pclSSheet->SetStaticText( CD_BalancingValve_Preset, RD_BalancingValve_Unit, IDS_SHEETHDR_TURNSPOS );

	// Units
	pclSSheet->SetStaticText( CD_BalancingValve_PipeLinDp, RD_BalancingValve_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_LINPRESSDROP ) ).c_str() );
	pclSSheet->SetStaticText( CD_BalancingValve_PipeV, RD_BalancingValve_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_VELOCITY ) ).c_str() );
	//
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Draw black line below column header.
	pclSSheet->SetCellBorder( CD_BalancingValve_CheckBox, RD_BalancingValve_Unit, CD_BalancingValve_Separator - 1, RD_BalancingValve_Unit, true, 
			SS_BORDERTYPE_BOTTOM );

	pclSSheet->SetCellBorder( CD_BalancingValve_PipeSize, RD_BalancingValve_Unit, CD_BalancingValve_Pointer - 1, RD_BalancingValve_Unit, true,
			SS_BORDERTYPE_BOTTOM );
}

long CRViewSSelDpC::_FillOneBvRow( CSheetDescription *pclSheetDescriptionBv, CSSheet *pclSSheet, CSelectedValve *pSelectedTAP, long lRow )
{
	if( NULL == m_pclIndSelDpCParams || NULL == m_pclIndSelDpCParams->m_pclSelectDpCList || NULL == m_pclIndSelDpCParams->m_pclSelectDpCList->GetSelectPipeList() )
	{
		ASSERTA_RETURN( -1 );
	}

	CDS_TechnicalParameter *pTechParam = m_pclIndSelDpCParams->m_pTADS->GetpTechParams();

	CDB_TAProduct *pTAP = dynamic_cast<CDB_TAProduct *>( pSelectedTAP->GetpData() );

	if( NULL == pTAP )
	{
		ASSERTA_RETURN( -1 );
	}

	bool bKvSignalExist = ( NULL != pTAP->IsKvSignalEquipped() );
	bool bAtLeastOneError = false;
	bool bAtLeastOneWarning = false;
	bool bBest = pSelectedTAP->IsFlagSet( CSelectedBase::eBest );

	// Add this row.
	// 'true' to specify that this row can be selected.
	pclSheetDescriptionBv->AddRows( 1, true );

	// If user wants a DpC-Mv set, we add an opened box in front of the line.
	if( true == m_pclIndSelDpCParams->m_bOnlyForSet && true == pTAP->IsPartOfaSet() )
	{
		CCDButtonBox *pCDButtonBoxBv = CreateCellDescriptionBox( CD_BalancingValve_Box, lRow, true, CCDButtonBox::ButtonState::BoxOpened, pclSheetDescriptionBv );

		if( NULL != pCDButtonBoxBv )
		{
			pCDButtonBoxBv->ApplyInternalChange();
		}
	}

	// First columns will be set at the end!

	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, ( LPARAM )_BLACK );

	// Add checkbox.
	pclSSheet->SetCheckBox(CD_BalancingValve_CheckBox, lRow, _T(""), false, true);

	// Set the hand wheel presetting.
	CString str = _T("-");

	if( NULL != pTAP && NULL != pTAP->GetValveCharacteristic() )
	{
		str = pTAP->GetValveCharacteristic()->GetSettingString( pSelectedTAP->GetH() );
	}

	if( true == pSelectedTAP->IsFlagSet( CSelectedBase::eValveMaxSetting ) )
	{
		pclSheetDescriptionBv->WriteTextWithFlags( str, CD_BalancingValve_Preset, lRow, CSheetDescription::RemarkFlags::FullOpening );
	}
	else
	{
		if( true == pSelectedTAP->IsFlagSet( CSelectedBase::eValveSetting ) )
		{
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, ( LPARAM )_ORANGE );
			bAtLeastOneWarning = true;
		}

		pclSSheet->SetStaticText( CD_BalancingValve_Preset, lRow, str );
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, ( LPARAM )_BLACK );
	}

	if( true == pSelectedTAP->IsFlagSet( CSelectedBase::eDp ) )
	{
		// Set in red only error if Dp is below or above min or max technical parameter.
		if( true == m_pclIndSelDpCParams->m_bIsGroupDpbranchOrKvChecked && pSelectedTAP->GetDp() > 0.0 )
		{
			if( pSelectedTAP->GetDp() > pTechParam->VerifyValvMaxDp( pTAP ) )
			{
				pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_RED );
				bAtLeastOneError = true;
			}
			else if( pSelectedTAP->GetDp() < pTechParam->GetValvMinDp( pTAP->GetTypeIDPtr().ID ) )
			{
				pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, ( LPARAM )_ORANGE );
				bAtLeastOneWarning = true;
			}
		}
	}

	pclSSheet->SetStaticText( CD_BalancingValve_Dp, lRow, WriteCUDouble( _U_DIFFPRESS, pSelectedTAP->GetDp() ) );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, ( LPARAM )_BLACK );

	// Set signal.
	if( true == bKvSignalExist )
	{
		pclSSheet->SetStaticText( CD_BalancingValve_DpSignal, lRow, WriteCUDouble( _U_DIFFPRESS, pSelectedTAP->GetDpSignal() ) );
	}

	// If Dp unknown.
	if( false == m_pclIndSelDpCParams->m_bIsGroupDpbranchOrKvChecked || -1 == pSelectedTAP->GetDp() )
	{
		if( true == pSelectedTAP->IsFlagSet( CSelectedBase::eValveFullODp ) )
		{
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, ( LPARAM )_ORANGE );
			bAtLeastOneWarning = true;
		}

		pclSSheet->SetStaticText( CD_BalancingValve_DpFullOpening, lRow, WriteCUDouble( _U_DIFFPRESS, pSelectedTAP->GetDpFullOpen() ) );
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, ( LPARAM )_BLACK );

		if( true == pSelectedTAP->IsFlagSet( CSelectedBase::eValveHalfODp ) )
		{
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, ( LPARAM ) _ORANGE );
			bAtLeastOneWarning = true;
		}

		pclSSheet->SetStaticText( CD_BalancingValve_DpHalfOpening, lRow, WriteCUDouble( _U_DIFFPRESS, pSelectedTAP->GetDpHalfOpen() ) );
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, ( LPARAM )_BLACK );
	}

	// Temperature range.
	if( true == pSelectedTAP->IsFlagSet( CSelectedBase::eTemperature ) )
	{
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, ( LPARAM )_RED );
		bAtLeastOneError = true;
	}

	pclSSheet->SetStaticText( CD_BalancingValve_TemperatureRange, lRow, ( ( CDB_TAProduct * )pSelectedTAP->GetProductIDPtr().MP )->GetTempRange() );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, ( LPARAM )_BLACK );

	// Now we can set first columns in regards to current status (error, best or normal).
	if( true == bAtLeastOneError )
	{
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, ( LPARAM )_RED );
	}
	else if( true == pSelectedTAP->IsFlagSet( CSelectedBase::eBest ) )
	{
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, ( LPARAM )_GREEN );
	}

	// In addition of current flag, we have possibility that valve has the flag 'Not available' or 'Deleted'. In that case, we show valve name in red with "*" around it and
	// symbol '!' or '!!' after.
	if( true == pTAP->IsDeleted() )
	{
		pclSheetDescriptionBv->WriteTextWithFlags( CString( pTAP->GetName() ), CD_BalancingValve_Name, lRow, CSheetDescription::RemarkFlags::Deleted, _T(""), _RED );
	}
	else if( false == pTAP->IsAvailable() )
	{
		pclSheetDescriptionBv->WriteTextWithFlags( CString( pTAP->GetName() ), CD_BalancingValve_Name, lRow, CSheetDescription::RemarkFlags::NotAvailable, _T(""), _RED );
	}
	else
	{
		pclSSheet->SetStaticText( CD_BalancingValve_Name, lRow, pTAP->GetName() );
	}

	if( true == bAtLeastOneError )
	{
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, ( LPARAM )_BLACK );
	}
	else if( true == pSelectedTAP->IsFlagSet( CSelectedBase::eBest ) )
	{
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, ( LPARAM )_GREEN );
	}

	pclSSheet->SetStaticText( CD_BalancingValve_Material, lRow, pTAP->GetBodyMaterial() );
	pclSSheet->SetStaticText( CD_BalancingValve_Connection, lRow, pTAP->GetConnect() );
	pclSSheet->SetStaticText( CD_BalancingValve_Version, lRow, pTAP->GetVersion() );
	pclSSheet->SetStaticText( CD_BalancingValve_PN, lRow, pTAP->GetPN().c_str() );
	pclSSheet->SetStaticText( CD_BalancingValve_Size, lRow, pTAP->GetSize() );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, ( LPARAM )_BLACK );

	// Set pipe size.
	CSelectPipe selPipe( m_pclIndSelDpCParams );
	m_pclIndSelDpCParams->m_pclSelectDpCList->GetSelectPipeList()->GetMatchingPipe( pTAP->GetSizeKey(), selPipe );
	pclSSheet->SetStaticText( CD_BalancingValve_PipeSize, lRow, selPipe.GetpPipe()->GetName() );

	// Set the LinDp to orange if it is above or below the technical parameters limits.
	if( selPipe.GetLinDp() > pTechParam->GetPipeMaxDp() || selPipe.GetLinDp() < pTechParam->GetPipeMinDp() )
	{
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, ( LPARAM )_ORANGE );
	}

	pclSSheet->SetStaticText( CD_BalancingValve_PipeLinDp, lRow, WriteCUDouble( _U_LINPRESSDROP, selPipe.GetLinDp() ) );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, ( LPARAM )_BLACK );

	// Set the Velocity to orange if it is above the technical parameter limit.
	// Orange if it is below the dMinVel.
	if( selPipe.GetU() > pTechParam->GetPipeMaxVel() || selPipe.GetU() < pTechParam->GetPipeMinVel() )
	{
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, ( LPARAM )_ORANGE );
	}

	pclSSheet->SetStaticText( CD_BalancingValve_PipeV, lRow, WriteCUDouble( _U_VELOCITY, selPipe.GetU() ) );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, ( LPARAM )_BLACK );

	// Save parameter.
	CreateCellDescriptionProduct( pclSheetDescriptionBv->GetFirstParameterColumn(), lRow, ( LPARAM )pTAP, pclSheetDescriptionBv );

	pclSSheet->SetCellBorder( CD_BalancingValve_CheckBox, lRow, CD_BalancingValve_Separator - 1, lRow, true, SS_BORDERTYPE_BOTTOM, 
			SS_BORDERSTYLE_SOLID, _GRAY );

	pclSSheet->SetCellBorder( CD_BalancingValve_PipeSize, lRow, CD_BalancingValve_Pointer - 1, lRow, true, SS_BORDERTYPE_BOTTOM, 
			SS_BORDERSTYLE_SOLID, _GRAY );

	return 0;
}

void CRViewSSelDpC::_FillBvAccessoryRows( )
{
	if( NULL == m_pclIndSelDpCParams || NULL == m_pclIndSelDpCParams->m_pTADB )
	{
		ASSERT_RETURN;
	}

	CDB_TAProduct *pclCurrentBalancingValveSelected = GetCurrentBalancingValveSelected();

	if( NULL == pclCurrentBalancingValveSelected )
	{
		return;
	}

	CDB_RuledTable *pRuledTable = ( CDB_RuledTable * )( pclCurrentBalancingValveSelected->GetAccessoriesGroupIDPtr().MP );

	if( NULL == pRuledTable )
	{
		return;
	}

	CRank rList;
	int iCount = m_pclIndSelDpCParams->m_pTADB->GetAccessories( &rList, pRuledTable, m_pclIndSelDpCParams->m_eFilterSelection );

	if( 0 == iCount )
	{
		return;
	}

	// Call 'MultiSpreadBase' method to get a new SSheet.
	CSheetDescription *pclSheetDescriptionBvAccessory = CreateSSheet( SD_BvAccessories );

	if( NULL == pclSheetDescriptionBvAccessory || NULL == pclSheetDescriptionBvAccessory->GetSSheetPointer() )
	{
		return;
	}

	CSSheet *pclSSheet = pclSheetDescriptionBvAccessory->GetSSheetPointer();

	pclSSheet->SetBool( SSB_REDRAW, FALSE );

	pclSSheet->Init();
	pclSSheet->SetBool( SSB_ALLOWDRAGDROP, FALSE );
	pclSSheet->SetBool( SSB_HORZSCROLLBAR, FALSE );
	pclSSheet->SetBool( SSB_VERTSCROLLBAR, FALSE );
	pclSSheet->SetTextTip( SS_TT_STATUS_FLOATING, 250, NULL, _LIGHTYELLOW, _TAH_BLACK );

	// Two lines by accessories, but two accessories by line (reason why we don't multiply iCount by 2).
	pclSSheet->SetMaxRows( RD_BVAccessory_FirstAvailRow + iCount - 1 + ( iCount % 2 ) );

	// All cells are static by default and filled with _T("")
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	pclSSheet->FormatStaticText( -1, -1, -1, -1, _T( "" ) );

	// Get sheet description of Balancing Valve to retrieve width.
	CSheetDescription *pclSheetDescriptionBv = m_ViewDescription.GetFromSheetDescriptionID( SD_BalancingValve );
	// It's absolutely not normal to have this pointer NULL.
	ASSERT( NULL != pclSheetDescriptionBv );

	// Take SSheet size and client area size and determine the right width to take in count.
	CRect rectSize = pclSheetDescriptionBv->GetSSheetPointer()->GetSheetSizeInPixels( false );

	// Prepare first column width (to match balancing valve sheet).
	long lFirstColumnWidth = pclSheetDescriptionBv->GetSSheetPointer()->GetColWidthInPixelsW( CD_BalancingValve_FirstColumn );

	if( true == m_pclIndSelDpCParams->m_bOnlyForSet )
	{
		lFirstColumnWidth += pclSheetDescriptionBv->GetSSheetPointer()->GetColWidthInPixelsW( CD_BalancingValve_Box );
	}

	// Do some correction in regards to some columns.
	long lTotalWidth = rectSize.Width() - lFirstColumnWidth;
	lTotalWidth -= pclSheetDescriptionBv->GetSSheetPointer()->GetColWidthInPixelsW( CD_BalancingValve_Pointer );

	// Try to create 2 columns in just the middle of balancing valve sheet.
	long lLeftWidth = ( long )( lTotalWidth / 2 );
	long lRightWidth = ( ( lTotalWidth % 2 ) > 0 ) ? lLeftWidth + 1 : lLeftWidth;

	// Set columns.
	// 'CSheetDescription::SD_ParameterMode_Multi' to set that this sheet can have more than one column with parameter.
	pclSheetDescriptionBvAccessory->Init( 1, pclSSheet->GetMaxRows(), 2, CSheetDescription::SD_ParameterMode_Multi );

	pclSheetDescriptionBvAccessory->AddColumnInPixels( CD_BVAccessory_FirstColumn, lFirstColumnWidth );
	pclSheetDescriptionBvAccessory->AddColumnInPixels( CD_BVAccessory_Left, lLeftWidth );
	pclSheetDescriptionBvAccessory->AddColumnInPixels( CD_BVAccessory_Right, lRightWidth );
	pclSheetDescriptionBvAccessory->AddColumn( CD_BVAccessory_LastColumn, 0 );

	// These two columns can contain parameter.
	pclSheetDescriptionBvAccessory->AddParameterColumn( CD_BVAccessory_Left );
	pclSheetDescriptionBvAccessory->AddParameterColumn( CD_BVAccessory_Right );

	// Set the focus column (don't set on Left of Right (on a check box) ).
	pclSheetDescriptionBvAccessory->SetActiveColumn( CD_BVAccessory_FirstColumn );

	// Set selectable rows.
	pclSheetDescriptionBvAccessory->SetSelectableRangeRow( RD_BVAccessory_FirstAvailRow, pclSSheet->GetMaxRows() - 1 );

	// Increase row height.
	double dRowHeight = 12.75;
	pclSSheet->SetRowHeight( RD_BVAccessory_FirstRow, dRowHeight * 0.5 );
	pclSSheet->SetRowHeight( RD_BVAccessory_GroupName, dRowHeight * 1.5 );

	// Set title.
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, ( LPARAM )GetTitleBackgroundColor( pclSheetDescriptionBvAccessory ) );

	pclSheetDescriptionBvAccessory->SetUserVariable( _SDUV_TITLEFORECOLOR, ( pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor ) ) );
	pclSheetDescriptionBvAccessory->SetUserVariable( _SDUV_TITLEBACKCOLOR, ( pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor ) ) );

	pclSSheet->AddCellSpanW( CD_BVAccessory_Left, RD_BVAccessory_GroupName, CD_BVAccessory_LastColumn - CD_BVAccessory_Left, 1 );
	pclSSheet->SetStaticText( CD_BVAccessory_Left, RD_BVAccessory_GroupName, IDS_SSHEETSSELDPC_BVACCGROUP );

	// Fill accessories.
	long lRow = RD_BVAccessory_FirstAvailRow;

	CString str;
	LPARAM lparam;

	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, ( LPARAM )FALSE );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, ( LPARAM )SSS_ALIGN_LEFT );

	// Left - Right.
	long lLeftOrRight = CD_BVAccessory_Left;
	BOOL bContinue = rList.GetFirst( str, lparam );

	while( TRUE == bContinue )
	{
		CDB_Product *pAccessory = (CDB_Product * )lparam;
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

		// If accessory has its property 'IsAttached' to true and user is in selection by set, he can't select it.
		bool bEnabled = true;

		if( true == pAccessory->IsAttached() && true == m_pclIndSelDpCParams->m_bOnlyForSet )
		{
			bEnabled = false;
		}

		CCDBCheckboxAccessory *pCheckbox = CreateCheckboxAccessory( lLeftOrRight, lRow, false, bEnabled, strName, 
				pAccessory, pRuledTable, &m_vecBvAccessoryList, pclSheetDescriptionBvAccessory );

		if( NULL != pCheckbox )
		{
			pCheckbox->ApplyInternalChange();

			if( false == bEnabled )
			{
				pCheckbox->SetToolTip( TASApp.LoadLocalizedString( AFXMSG_ACCATTACHEDNOTINSET ) );
			}
		}

		// Description.
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleEllipses, ( LPARAM )TRUE );
		pclSSheet->SetStaticText( lLeftOrRight, lRow + 1, pAccessory->GetComment() );
		bContinue = rList.GetNext( str, lparam );

		// restart left part.
		if( TRUE == bContinue && CD_BVAccessory_Right == lLeftOrRight )
		{
			pclSSheet->SetCellBorder( CD_BVAccessory_Left, lRow + 1, CD_BVAccessory_LastColumn - 1, lRow + 1, true, SS_BORDERTYPE_BOTTOM, 
					SS_BORDERSTYLE_SOLID, _GRAY );

			lLeftOrRight = CD_BVAccessory_Left;
			lRow += 2;
			ASSERT( lRow < pclSSheet->GetMaxRows() );
		}
		else
		{
			lLeftOrRight = CD_BVAccessory_Right;
		}

		if( !bContinue )
		{
			lRow++;
		}
	}

	VerifyCheckboxAccessories( NULL, false, &m_vecBvAccessoryList );

	// Add possibility to collapse/expand the accessory sheet.
	m_pCDBExpandCollapseGroupBvAccessory = CreateExpandCollapseGroupButton( CD_BVAccessory_FirstColumn, RD_BVAccessory_GroupName, true,
			CCDButtonExpandCollapseGroup::ButtonState::CollapseRow, pclSheetDescriptionBvAccessory->GetFirstSelectableRow(), 
			pclSheetDescriptionBvAccessory->GetLastSelectableRow( false ), pclSheetDescriptionBvAccessory );

	// Show button.
	if( NULL != m_pCDBExpandCollapseGroupBvAccessory )
	{
		m_pCDBExpandCollapseGroupBvAccessory->SetShowStatus( true );
	}

	pclSSheet->SetCellBorder( CD_BVAccessory_Left, lRow, CD_BVAccessory_LastColumn - 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );

	pclSSheet->SetBool( SSB_REDRAW, TRUE );
}

long CRViewSSelDpC::_GetRowOfEditedBv( CSheetDescription *pclSheetDescriptionBv, CDB_TAProduct *pEditedTAP )
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

		if( NULL != pCDProduct && 0 != pCDProduct->GetProduct() && pEditedTAP == ( CDB_TAProduct * )pCDProduct->GetProduct() )
		{
			lReturnValue = pCDProduct->GetCellPosition().y;
			break;
		}

		vecIter++;
	}

	return lReturnValue;
}

void CRViewSSelDpC::_ReadAllColumnWidth( CDS_IndSelParameter *pclIndSelParameter )
{
	if( NULL == pclIndSelParameter )
	{
		ASSERT_RETURN;
	}

	std::map<UINT, short> mapSDIDVersion;
	mapSDIDVersion[CW_RVIEWSSELDPC_DPCONTROLLER_SHEETID] = CW_RVIEWSSELDPC_DPCONTROLLER_VERSION;
	mapSDIDVersion[CW_RVIEWSSELDPC_BALANCINGVALVE_SHEETID] = CW_RVIEWSSELDPC_BALANCINGVALVE_VERSION;

	// Container window sheet ID to sheetdescription of this rightview.
	std::map<UINT, short> mapCWtoRW;
	mapCWtoRW[CW_RVIEWSSELDPC_DPCONTROLLER_SHEETID] = SD_DpController;
	mapCWtoRW[CW_RVIEWSSELDPC_BALANCINGVALVE_SHEETID] = SD_BalancingValve;

	// By default and before reading registry saved column width force reset column width for all sheets.
	for( auto &iter : mapCWtoRW )
	{
		ResetColumnWidth( iter.second );
	}

	// Access to the 'RViewSSelDpC' group.
	CDS_IndSelParameter::CCWWindow *pclCWWindow = pclIndSelParameter->GetWindowColumnWidth( CW_WINDOWID_INDSELDPC, true );

	for( auto &iter : mapSDIDVersion )
	{
		CDS_IndSelParameter::CCWSheet *pclCWSheet = pclCWWindow->GetCWSheet( iter.first );

		if( NULL != pclCWSheet && m_mapSSheetColumnWidth[mapCWtoRW[iter.first]].size() == pclCWSheet->GetMap().size() )
		{
			short nVersion = pclCWSheet->GetVersion();

			if( nVersion == iter.second )
			{
				m_mapSSheetColumnWidth[mapCWtoRW[iter.first]] = pclCWSheet->GetMap();
			}
		}
	}
}

void CRViewSSelDpC::_WriteAllColumnWidth( CDS_IndSelParameter *pclIndSelParameter )
{
	if( NULL == pclIndSelParameter )
	{
		ASSERT_RETURN;
	}

	std::map<UINT, short> mapSDIDVersion;
	mapSDIDVersion[CW_RVIEWSSELDPC_DPCONTROLLER_SHEETID] = CW_RVIEWSSELDPC_DPCONTROLLER_VERSION;
	mapSDIDVersion[CW_RVIEWSSELDPC_BALANCINGVALVE_SHEETID] = CW_RVIEWSSELDPC_BALANCINGVALVE_VERSION;

	// Container window sheet ID to sheetdescription of this rightview.
	std::map<UINT, short> mapCWtoRW;
	mapCWtoRW[CW_RVIEWSSELDPC_DPCONTROLLER_SHEETID] = SD_DpController;
	mapCWtoRW[CW_RVIEWSSELDPC_BALANCINGVALVE_SHEETID] = SD_BalancingValve;

	// Access to the 'RViewSSelDpC' group.
	CDS_IndSelParameter::CCWWindow *pclCWWindow = pclIndSelParameter->GetWindowColumnWidth( CW_WINDOWID_INDSELDPC, true );

	for( auto &iter : mapSDIDVersion )
	{
		CDS_IndSelParameter::CCWSheet *pclCWSheet = pclCWWindow->GetCWSheet( iter.first, true );
		pclCWSheet->SetVersion( iter.second );
		pclCWSheet->GetMap() = m_mapSSheetColumnWidth[mapCWtoRW[iter.first]];
	}
}

void CRViewSSelDpC::_SelectAutoCrossingAccessories( CDB_RegulatingValve* pBalancingValve, bool bCheck )
{
	CDB_DpController* pclCurrentDpCSelected = GetCurrentDpControllerSelected();

	if( NULL == pclCurrentDpCSelected )
	{
		return;
	}

	vector<CDB_Product*> VectCrossingAccessories;
	CDB_RuledTable* pclRuledTableMV = NULL;

	if( NULL != pBalancingValve )
	{
		pclRuledTableMV = (CDB_RuledTable*)(pBalancingValve->GetAccessoriesGroupIDPtr().MP);
	}

	CDB_RuledTable* pclRuledTableDpC = (CDB_RuledTable*)(pclCurrentDpCSelected->GetAccessoriesGroupIDPtr().MP);

	// Find crossing accessories.
	if( NULL == pclRuledTableDpC )
	{
		return;
	}

	for( IDPTR IDPtr = pclRuledTableDpC->GetFirst(); NULL != IDPtr.MP; IDPtr = pclRuledTableDpC->GetNext() )
	{
		if( true == pclRuledTableDpC->IsACrossingAccDisplayed( IDPtr.MP ) )
		{
			VectCrossingAccessories.push_back( dynamic_cast<CDB_Product*>(IDPtr.MP) );
		}
	}

	// First step: If we select a MV (bCheck = true), check if we need to check the common accessory if it exists.
	if( NULL != pclRuledTableMV && true == bCheck )
	{
		// Find shared accessories between the two cross tables.
		for( IDPTR IDPtr = pclRuledTableMV->GetFirst(); NULL != IDPtr.MP; IDPtr = pclRuledTableMV->GetNext() )
		{
			// Search if the accessory in the MV accessory table exists in the accessory table of the DpC that is cross-referenced.
			vector<CDB_Product*>::iterator it = find( VectCrossingAccessories.begin(), VectCrossingAccessories.end(), dynamic_cast<CDB_Product*>(IDPtr.MP) );

			// If accessory is common (We have in this case the '+' for the accessory in the MV accessory group which signals us that it's an accessory
			// common with the DpC but that we can't show -> The 'IsACrossingAcc' method returns in this case 'true').
			if( (true == pclRuledTableMV->IsACrossingAcc( IDPtr.MP )) && (VectCrossingAccessories.end() != it) )
			{
				VectCrossingAccessories.erase( it );

				// Search in the available accessories for the current DpC if this common accessory exist.
				for( vecCDCAccessoryListIter vecIter = m_vecDpCAccessoryList.begin(); vecIter != m_vecDpCAccessoryList.end(); vecIter++ )
				{
					CCDBCheckboxAccessory* pCDBCheckboxAccessory = *vecIter;

					if( NULL != pCDBCheckboxAccessory && NULL != pCDBCheckboxAccessory->GetAccessoryPointer() )
					{
						// If the current DpC accessory displayed is the same as the common accessory...
						if( dynamic_cast<CDB_Product*>(pCDBCheckboxAccessory->GetAccessoryPointer()->GetIDPtr().MP) == IDPtr.MP )
						{
							// Change state if the previous one was unchecked.
							if( false == pCDBCheckboxAccessory->GetCheckStatus() )
							{
								pCDBCheckboxAccessory->SetCheckStatus( true );
							}
							pCDBCheckboxAccessory->ApplyInternalChange();
						}
					}
				}
			}
		}
	}

	// bCheck = true  -> Accessories which are not compatible with MV (Those that were common with the MV were removed in the first step just above).
	// bCheck = false -> Accessories that belong to a cross-reference table and are displayed.
	if( VectCrossingAccessories.size() > 0 )
	{
		// Run all the DpC accessories in the right view.
		for( vecCDCAccessoryListIter vecIter = m_vecDpCAccessoryList.begin(); vecIter != m_vecDpCAccessoryList.end(); vecIter++ )
		{
			CCDBCheckboxAccessory* pCDBCheckboxAccessory = *vecIter;

			if( NULL != pCDBCheckboxAccessory && NULL != pCDBCheckboxAccessory->GetAccessoryPointer() )
			{
				// Verify if the current accessory in the right view belongs to a cross-reference table.
				vector<CDB_Product*>::iterator it = find( VectCrossingAccessories.begin(), VectCrossingAccessories.end(), dynamic_cast<CDB_Product*>(pCDBCheckboxAccessory->GetAccessoryPointer()->GetIDPtr().MP) );

				// If it's a common accessory between DpC and MV...
				if( it != VectCrossingAccessories.end() )
				{
					if( true == bCheck )
					{
						// If 'bCheck = true' and there are yet accessories in 'VectCrossingAccessories', it means that these accessories are not common with the
						// the current selected MV (Because if common the 'VectCrossingAccessories.erase( it )' remove this accessory from the vector).

						// If the accessory was selected, we need to unselect it.
						if( true == pCDBCheckboxAccessory->GetCheckStatus() )
						{
							pCDBCheckboxAccessory->SetCheckStatus( false );
						}

						pCDBCheckboxAccessory->ApplyInternalChange();
					}
					else
					{
						// If 'bCheck = false', there is not yet MV selected. So, it's not possible for the moment to set the status of the common accessories.
						// This is why we restore the initial state.
						if( true == pCDBCheckboxAccessory->GetCheckStatus() )
						{
							pCDBCheckboxAccessory->SetCheckStatus( false );
						}

						pCDBCheckboxAccessory->ApplyInternalChange();
					}
				}
			}
		}
	}
}