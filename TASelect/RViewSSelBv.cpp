#include "stdafx.h"
#include "afxctl.h"

#include "TASelect.h"
#include "MainFrm.h"

#include "DlgLeftTabSelManager.h"

#include "RViewSSelSS.h"
#include "RViewSSelBv.h"

CRViewSSelBv *pRViewSSelBv = NULL;
CRViewSSelBv::CRViewSSelBv() : CRViewSSelSS( CMainFrame::RightViewList::eRVSSelBv, false )
{
	m_pclIndSelBVParams = NULL;

	m_pCDBExpandCollapseRows = NULL;
	m_pCDBExpandCollapseGroupAccessory = NULL;
	m_pCDBShowAllPriorities = NULL;
	m_lBvSelectedRow = -1;
	
	pRViewSSelBv = this;
}

CRViewSSelBv::~CRViewSSelBv()
{
	pRViewSSelBv = NULL;
}

CDB_TAProduct *CRViewSSelBv::GetCurrentBalancingValveSelected( void )
{
	CDB_TAProduct *pclCurrentBalancingValveSelected = NULL;
	CSheetDescription *pclSheetDescriptionBv = m_ViewDescription.GetFromSheetDescriptionID( SD_Valve );

	if( NULL != pclSheetDescriptionBv )
	{
		// Retrieve the current selected balancing valve.
		CCellDescriptionProduct *pclCDCurrentBalancingValveSelected = NULL;
		LPARAM lpPointer;
		
		if( true == pclSheetDescriptionBv->GetUserVariable( _SDUV_SELECTEDPRODUCT, lpPointer ) )
		{
			pclCDCurrentBalancingValveSelected = (CCellDescriptionProduct*)lpPointer;
		}

		if( NULL != pclCDCurrentBalancingValveSelected && NULL != pclCDCurrentBalancingValveSelected->GetProduct() )
		{
			pclCurrentBalancingValveSelected = dynamic_cast<CDB_TAProduct*>( (CData*)pclCDCurrentBalancingValveSelected->GetProduct() );
		}
	}

	return pclCurrentBalancingValveSelected;
}

void CRViewSSelBv::Reset()
{
	m_pCDBExpandCollapseRows = NULL;
	m_pCDBExpandCollapseGroupAccessory = NULL;
	m_pCDBShowAllPriorities = NULL;
	m_vecAccessoryList.clear();
	m_lBvSelectedRow = -1;
	CRViewSSelSS::Reset();
}

void CRViewSSelBv::Suggest( CProductSelelectionParameters *pclProductSelectionParameters, LPARAM lpParam )
{
	CRViewSSelSS::Suggest( pclProductSelectionParameters, lpParam );

	if( NULL == pclProductSelectionParameters || NULL == dynamic_cast<CIndSelBVParams *>( pclProductSelectionParameters ) )
	{
		ASSERT_RETURN;
	}

	m_pclIndSelBVParams = dynamic_cast<CIndSelBVParams *>( pclProductSelectionParameters );

	// To remove all current displayed sheets.
	Reset();

	BeginWaitCursor();
	CWnd::SetRedraw( FALSE );

	long lRowSelected = -1;

	CDS_SSelBv *pSelectedBv = NULL;

	// If we are in edition mode...
	if( NULL != m_pclIndSelBVParams->m_SelIDPtr.MP )
	{
		pSelectedBv = reinterpret_cast<CDS_SSelBv*>( (CData*)( m_pclIndSelBVParams->m_SelIDPtr.MP ) );

		if( NULL == pSelectedBv )
		{
			ASSERT_RETURN;
		}

		lRowSelected = _FillValveRows( pSelectedBv );
	}
	else
	{
		lRowSelected = _FillValveRows();
	}
	
	// Verify if sheet description has been well created.
	CSheetDescription *pclSheetDescription = m_ViewDescription.GetFromSheetDescriptionID( SD_Valve );

	if( NULL != pclSheetDescription && NULL != pclSheetDescription->GetSSheetPointer() )
	{
		CSSheet *pclSSheet = pclSheetDescription->GetSSheetPointer();

		if( lRowSelected > -1 && NULL != pSelectedBv )
		{
			CCellDescriptionProduct *pclCDProduct = FindCDProduct( lRowSelected, (LPARAM)( pSelectedBv->GetProductAs<CDB_TAProduct>() ), pclSheetDescription );

			if( NULL != pclCDProduct && NULL != pclCDProduct->GetProduct() )
			{
				// If we are in edition mode we simulate a click on the product.
				OnClickProduct( pclSheetDescription, pclCDProduct, pclSheetDescription->GetActiveColumn(), lRowSelected );

				// Allow to check if we need to change the 'Show all priorities' button or not.
				CheckShowAllPrioritiesButtonState( pclSheetDescription, lRowSelected );

				// Verify accessories.
				CAccessoryList *pclAccessoryList = pSelectedBv->GetAccessoryList();

				if( pclAccessoryList->GetCount() > 0 )
				{
					CAccessoryList::AccessoryItem rAccessoryItem = pclAccessoryList->GetFirst();
					
					while( rAccessoryItem.IDPtr.MP != NULL )
					{
						VerifyCheckboxAccessories( (CDB_Product *)rAccessoryItem.IDPtr.MP, true, &m_vecAccessoryList );
						rAccessoryItem = pclAccessoryList->GetNext();
					}
				}
			}
		}
		else
		{
			// Define the first product row as the active cell and set a visual focus.
			PrepareAndSetNewFocus( pclSheetDescription, CD_BalancingValve_Name, RD_BalancingValve_FirstAvailRow, 0 );
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

void CRViewSSelBv::FillInSelected( CDS_SelProd *pSelectedProductToFill )
{
	if( NULL == m_pclIndSelBVParams )
	{
		ASSERT_RETURN;
	}

	CDS_SSelBv *pSelectedBvToFill = dynamic_cast<CDS_SSelBv *>( pSelectedProductToFill );

	if( NULL == pSelectedBvToFill )
	{
		ASSERT_RETURN;
	}

	// HYS-987: Clear previous selected accessories if wa are not in edition mode.
	CAccessoryList *pclBvAccessoryListToFill = pSelectedBvToFill->GetAccessoryList();
	bool bIsEditionMode = false;
	if( false == m_pclIndSelBVParams->m_bEditModeRunning )
	{
		pclBvAccessoryListToFill->Clear();
	}
	else
	{
		bIsEditionMode = true;
	}

	// Retrieve the current selected balancing valve if exist.
	CDB_TAProduct *pclCurrentBalancingValveSelected = GetCurrentBalancingValveSelected();

	if( NULL != pclCurrentBalancingValveSelected && NULL != m_pclIndSelBVParams->m_pclSelectBvList )
	{
		pSelectedBvToFill->SetProductIDPtr( pclCurrentBalancingValveSelected->GetIDPtr() );

		// Search balancing valve in CSelectList to set the correct opening.
		for( CSelectedValve *pclSelectedValve = m_pclIndSelBVParams->m_pclSelectBvList->GetFirst<CSelectedValve>(); NULL != pclSelectedValve; 
				pclSelectedValve = m_pclIndSelBVParams->m_pclSelectBvList->GetNext<CSelectedValve>() )
		{
			if( pclSelectedValve->GetpData() == pclCurrentBalancingValveSelected )	// Product found
			{
				pSelectedBvToFill->SetOpening( pclSelectedValve->GetH() );	
				break;
			}
		}

		// HYS-987
		if( false == bIsEditionMode )
		{
			// Retrieve selected accessory and add it.
			for( vecCDCAccessoryListIter vecIter = m_vecAccessoryList.begin(); vecIter != m_vecAccessoryList.end(); vecIter++ )
			{
				CCDBCheckboxAccessory *pCDBCheckboxAccessory = *vecIter;

				if( NULL != pCDBCheckboxAccessory && true == pCDBCheckboxAccessory->GetCheckStatus() && NULL != pCDBCheckboxAccessory->GetAccessoryPointer() )
				{
					pclBvAccessoryListToFill->Add( pCDBCheckboxAccessory->GetAccessoryPointer()->GetIDPtr(), CAccessoryList::_AT_Accessory,
						pCDBCheckboxAccessory->GetRuledTable() );
				}
			}
		}
		else
		{
			UpdateAccessoryList( m_vecAccessoryList, pclBvAccessoryListToFill, CAccessoryList::_AT_Accessory );
		}

		// Selected Pipe informations.
		if( NULL != m_pclIndSelBVParams->m_pclSelectBvList->GetSelectPipeList() )
		{
			m_pclIndSelBVParams->m_pclSelectBvList->GetSelectPipeList()->GetMatchingPipe( pclCurrentBalancingValveSelected->GetSizeKey(), 
					*pSelectedBvToFill->GetpSelPipe() );
		}
	}
}

void CRViewSSelBv::OnNewDocument( CDS_IndSelParameter *pclIndSelParameter )
{
	_ReadAllColumnWidth( pclIndSelParameter );
}

void CRViewSSelBv::SaveSelectionParameters( CDS_IndSelParameter *pclIndSelParameter )
{
	_WriteAllColumnWidth( pclIndSelParameter );
}

bool CRViewSSelBv::OnTextTipFetch( CSheetDescription *pclSheetDescription, long lColumn, long lRow, WORD *pwMultiLine, SHORT *pnTipWidth, 
	TCHAR *pstrTipText, BOOL *pfShowTip )
{
	if( NULL == m_pclIndSelBVParams || NULL == m_pclIndSelBVParams->m_pclSelectBvList || false == m_bInitialised || NULL == pclSheetDescription )
	{
		ASSERTA_RETURN( false );
	}

	// First, we must call base class.
	// Remark: if base class has displayed a tooltip, it's not necessary to go further in this method.
	if( true == CRViewSSelSS::OnTextTipFetch( pclSheetDescription, lColumn, lRow, pwMultiLine, pnTipWidth, pstrTipText, pfShowTip ) )
	{
		return true;
	}

	CString str;
	bool bReturnValue = false;
	*pfShowTip = false;
	*pwMultiLine = SS_TT_MULTILINE_AUTO;

	CDB_TAProduct *pTAP = dynamic_cast<CDB_TAProduct *>( (CData *)GetCDProduct( lColumn, lRow, pclSheetDescription ) );
	CDB_RegulatingValve *pclRegulatingValve = dynamic_cast<CDB_RegulatingValve *>( pTAP );
	CSelectedValve *pclSelectedBv = GetSelectProduct<CSelectedValve>( pTAP, m_pclIndSelBVParams->m_pclSelectBvList );
	CSSheet *pclSSheet = pclSheetDescription->GetSSheetPointer();
	CDS_TechnicalParameter *pclTechParam = m_pclIndSelBVParams->m_pTADS->GetpTechParams();
	
	switch( pclSheetDescription->GetSheetDescriptionID() )
	{
		// Check if mouse cursor has passed over a valve.
		case SD_Valve:

			if( NULL != pclSelectedBv )
			{
				if( CD_BalancingValve_Preset == lColumn && NULL != pclRegulatingValve )
				{
					// Check if flag is set.
					CDB_ValveCharacteristic *pValveCharacteristic = pclRegulatingValve->GetValveCharacteristic();

					if( true == pclSelectedBv->IsFlagSet( CSelectedBase::eValveSetting ) && NULL != pValveCharacteristic )
					{
						CString str2 = pValveCharacteristic->GetSettingString( pclSelectedBv->GetH() );
						
						double dMinRecommendedSetting = pValveCharacteristic->GetMinRecSetting();
						str2 += _T(" < ") + pValveCharacteristic->GetSettingString( dMinRecommendedSetting );
						FormatString( str, IDS_SSHEETSSEL_SETTINGERROR, str2 );
					}
				}
				else if( CD_BalancingValve_DpSignal == lColumn )
				{
					double dDpValveMinDp = -1.0;
					double dDpValveMaxDp = -1.0;

					if( NULL != dynamic_cast<CDB_FixedOrifice*>( (CData*)pclSelectedBv->GetProductIDPtr().MP ) )
					{
						dDpValveMinDp = pclTechParam->GetFoMinDp();
						dDpValveMaxDp = pclTechParam->GetFoMaxDp();
					}
					else if( NULL != dynamic_cast<CDB_VenturiValve*>( (CData*)pclSelectedBv->GetProductIDPtr().MP ) )
					{
						dDpValveMinDp = pclTechParam->GetVtriMinDp();
						dDpValveMaxDp = pclTechParam->GetVtriMaxDp();
					}

					if( -1.0 != dDpValveMinDp && -1.0 != dDpValveMaxDp )
					{
						CString str2 = WriteCUDouble( _U_DIFFPRESS, pclSelectedBv->GetDpSignal() );
						
						if( pclSelectedBv->GetDpSignal() < dDpValveMinDp )
						{
							str2 += CString( _T(" < ") ) + WriteCUDouble( _U_DIFFPRESS, dDpValveMinDp, true );
							FormatString( str, IDS_SSHEETSSELBV_DPSIGNALERRORL, str2 );
						}
						else if( pclSelectedBv->GetDpSignal() > dDpValveMaxDp )
						{
							str2 += CString( _T(" > ") ) + WriteCUDouble( _U_DIFFPRESS, dDpValveMaxDp, true );
							FormatString( str, IDS_SSHEETSSELBV_DPSIGNALERRORH, str2 );
						}
					}
				}
				else if( CD_BalancingValve_Dp == lColumn )
				{
					// Check if flag is set.
					if( true == pclSelectedBv->IsFlagSet( CSelectedBase::eDp ) )
					{
						// See 'Select.h' for description of errors.
						CString str2 = WriteCUDouble( _U_DIFFPRESS, pclSelectedBv->GetDp() );
						int iFlagError = pclSelectedBv->GetFlagError( CSelectedBase::eDp );
						
						switch( iFlagError )
						{
							case CSelectList::BvFlagError::DpBelowMinDp:
								str2 += CString( _T(" < ") ) + WriteCUDouble( _U_DIFFPRESS, pclTechParam->GetValvMinDp(), true );

								// "Dp is below the minimum value defined in technical parameters ( %1 )"
								FormatString( str, IDS_SSHEETSSEL_DPERRORL, str2 );
								break;

							case CSelectList::BvFlagError::DpAboveMaxDp:
							{
								// Either the max Dp is defined for the valve or we take the max Dp defined in the technical parameters.
								// This is why the message is different in regards to these both cases.
								double dDpMax = pTAP->GetDpmax();

								// IDS_SSHEETSSEL_DPERRORH: "Dp is above the maximum value defined in technical parameters ( %1 )"
								// IDS_SSHEETSSEL_DPERRORH2: "Dp is above the maximum value defined for this valve ( %1 )"
								int iMsgID = ( dDpMax <= 0.0 ) ? IDS_SSHEETSSEL_DPERRORH : IDS_SSHEETSSEL_DPERRORH2;

								str2 += CString( _T(" > ") ) + WriteCUDouble( _U_DIFFPRESS, pclTechParam->VerifyValvMaxDp( pTAP ), true );
								FormatString( str, iMsgID, str2 );
								break;
							}

							case CSelectList::BvFlagError::DpToReachTooLow:
								str2 += CString( _T(" > ") ) + WriteCUDouble( _U_DIFFPRESS, m_pclIndSelBVParams->m_dDp, true );

								// "Pressure drop on valve fully open is already higher than requested Dp value ( %1 )"
								FormatString( str, IDS_SSHEETSSEL_DPERRORNOTF, str2 );
								break;

							case CSelectList::BvFlagError::DpQOTooLow:
								str2 = WriteCUDouble( _U_DIFFPRESS, pclSelectedBv->GetDpQuarterOpen() );
								str2 += CString( _T(" < ") ) + WriteCUDouble( _U_DIFFPRESS, pclTechParam->GetValvMinDp(), true );

								// "Pressure drop on valve at quarter opening is below the minimum valve pressure drop defined in technical parameters ( %1 )"
								FormatString( str, IDS_SSHEETSSELBV_DPQOTOOLOWERROR, str2 );
								break;
						
							case CSelectList::BvFlagError::DpFOTooHigh:
								str2 = WriteCUDouble( _U_DIFFPRESS, pclSelectedBv->GetDpFullOpen() );
								str2 += CString( _T(" > ") ) + WriteCUDouble( _U_DIFFPRESS, pclTechParam->GetValvMinDp(), true );

								// "Pressure drop on valve fully open is above the minimum valve pressure drop defined in technical parameters ( %1 )"
								FormatString( str, IDS_SSHEETSSELBV_DPFOTOOLOWERROR, str2 );
								break;
						}
					}
				}
				else if( CD_BalancingValve_DpFullOpening == lColumn )
				{
					// Check if flag is set.
					if( true == pclSelectedBv->IsFlagSet( CSelectedBase::eValveFullODp ) )
					{
						// HYS-1468: Tooltip is not the same if it's a regulating valve or venturi.
						if( NULL != pclRegulatingValve )
						{
							if( pclSelectedBv->GetDpFullOpen() < pclTechParam->GetValvMinDp() )
							{
								CString str2 = WriteCUDouble( _U_DIFFPRESS, pclSelectedBv->GetDpFullOpen() );
								str2 += CString( _T(" < ") ) + WriteCUDouble( _U_DIFFPRESS, pclTechParam->GetValvMinDp(), true );

								// "Pressure drop on valve fully open is below the minimum value defined in technical parameters ( %1 )"
								FormatString( str, IDS_SSHEETSSEL_DPFOERROR, str2 );
							}
						}
						else if( NULL != dynamic_cast<CDB_VenturiValve*>( (CData*)pclSelectedBv->GetProductIDPtr().MP ) )
						{
							CString str2 = WriteCUDouble( _U_DIFFPRESS, pclSelectedBv->GetDpFullOpen() );
							str2 += CString( _T(" > ") ) + WriteCUDouble( _U_DIFFPRESS, m_pclIndSelBVParams->m_dDp, true );

							// "Pressure drop on valve fully open is already higher than requested Dp value ( %1 )"
							FormatString( str, IDS_SSHEETSSEL_DPERRORNOTF, str2 );
						}
					}
				}
				else if( CD_BalancingValve_DpHalfOpening == lColumn )
				{
					// Check if flag is set.
					if( true == pclSelectedBv->IsFlagSet( CSelectedBase::eValveHalfODp ) )
					{
						CString str2 = WriteCUDouble( _U_DIFFPRESS, pclSelectedBv->GetDpHalfOpen() );
						
						if( pclSelectedBv->GetDpHalfOpen() < pclTechParam->GetValvMinDp() )
						{
							str2 += CString( _T(" < ") ) + WriteCUDouble( _U_DIFFPRESS, pclTechParam->GetValvMinDp(), true );
							FormatString( str, IDS_SSHEETSSEL_DPHOERRORL, str2 );
						}
						else if( pclSelectedBv->GetDpHalfOpen() > pclTechParam->VerifyValvMaxDp( pTAP ) )
						{
							str2 += CString( _T(" > ") ) + WriteCUDouble( _U_DIFFPRESS, pclTechParam->VerifyValvMaxDp( pTAP ), true );
							FormatString( str, IDS_SSHEETSSEL_DPHOERRORH, str2 );
						}
					}
				}
				else if( CD_BalancingValve_TemperatureRange == lColumn )
				{
					if( true == pclSelectedBv->IsFlagSet( CSelectedBase::eTemperature ) )
					{
						FormatString( str, IDS_SSHEETSSEL_TEMPERROR, pclRegulatingValve->GetTempRange() );
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

				if( true == str.IsEmpty() && NULL != pTAP )
				{
					if( 0 != _tcslen( pTAP->GetComment() ) )
					{
						str = pTAP->GetComment();
					}
				}

			}
			break;

		case SD_Accessory:
			if( lColumn > CD_Accessory_FirstColumn && lColumn < CD_Accessory_LastColumn )
			{
				TextTipFetchEllipsesHelper( lColumn, lRow, pclSheetDescription, pnTipWidth, &str );
			}
			break;
	}

	if( false == str.IsEmpty() )
	{
		*pnTipWidth = (SHORT)pclSSheet->GetTipTextWidth( str );
		wcsncpy_s( pstrTipText, SS_TT_TEXTMAX, (LPCTSTR)str, SS_TT_TEXTMAX );
		*pfShowTip = true;
		bReturnValue = true;
	}
	return bReturnValue;
}

bool CRViewSSelBv::OnClickProduct( CSheetDescription *pclSheetDescriptionBv, CCellDescriptionProduct *pclCellDescriptionProduct, long lColumn, long lRow )
{
	if( NULL == pclSheetDescriptionBv || NULL == pclSheetDescriptionBv->GetSSheetPointer() || NULL == pclCellDescriptionProduct )
	{
		ASSERTA_RETURN( false );
	}

	CDB_TAProduct *pBalancingValve = dynamic_cast<CDB_TAProduct*>( (CData*)pclCellDescriptionProduct->GetProduct() );

	if( NULL == pBalancingValve )
	{
		ASSERTA_RETURN( false );
	}

	CSSheet *pclSSheet = pclSheetDescriptionBv->GetSSheetPointer();
		
	// Memorize previous focus row.
	long lCurrentFocusedRow = GetCurrentFocusedRow();

	// To be sure to clean all previous data concerning focus.
	KillCurrentFocus();

	// Unselect row where current selection is set (yellow light).
	// Remark: 'KillCurrentFocus' unselect only rows where focus is on (blue light).
	pclSSheet->UnSelectMultipleRows();

	// By default clear accessories list.
	m_vecAccessoryList.clear();

	LPARAM lBalancingValveCount;
	pclSheetDescriptionBv->GetUserVariable( _SDUV_TOTALPRODUCT_COUNT, lBalancingValveCount );

	// Retrieve the current selected balancing valve if exist.
	CDB_TAProduct *pclCurrentBalancingValveSelected = GetCurrentBalancingValveSelected();

	// If there is already one balancing valve selected and user clicks on the current one...
	// Remark: 'm_pCDBExpandCollapseRows' is not created if there is only one balancing valve. Thus we need to check first if there is only one balancing valve.
	//         Otherwise we have to check if user has clicked on the current one.
	if( NULL != pclCurrentBalancingValveSelected && ( ( 1 == lBalancingValveCount) || ( NULL != m_pCDBExpandCollapseRows && lRow == m_pCDBExpandCollapseRows->GetCellPosition().y ) ) )
	{
		// Reset current product selected.
		SetCurrentBalancingValveSelected( NULL );

		// uncheck checkbox
		pclSSheet->SetCheckBox(CD_BalancingValve_CheckBox, lRow, _T(""), false, true);
		m_lBvSelectedRow = -1;

		// Delete Expand/Collapse rows button if exist.
		// Remark: 'm_pCDBExpandCollapseRows' is set to NULL in method!
		DeleteExpandCollapseRowsButton( m_pCDBExpandCollapseRows, pclSheetDescriptionBv );

		// Show Show/Hide all priorities button if exist.
		if( NULL != m_pCDBShowAllPriorities )
		{
			m_pCDBShowAllPriorities->ApplyInternalChange();
		}

		// Set focus on balancing valve currently selected.
		PrepareAndSetNewFocus( pclSheetDescriptionBv, pclSheetDescriptionBv->GetActiveColumn(), lRow, 0 );

		// Remove all sheets after balancing valve.
		m_pCDBExpandCollapseGroupAccessory = NULL;
		m_ViewDescription.RemoveAllSheetAfter( SD_Valve );

		// Notify a change in the selection.
		SelectionHasChanged( IsSelectionReady() );
	}
	else
	{
		// If a balancing valve is already selected...
		if( NULL != pclCurrentBalancingValveSelected )
		{
			// Remove all sheets after balancing valve.
			m_pCDBExpandCollapseGroupAccessory = NULL;
			m_ViewDescription.RemoveAllSheetAfter( SD_Valve );
		}

		// Save new balancing valve selection.
		SetCurrentBalancingValveSelected( pclCellDescriptionProduct );

		// unselect previous selection
		if( m_lBvSelectedRow > -1 )
		{
			pclSSheet->SetCheckBox( CD_BalancingValve_CheckBox, m_lBvSelectedRow, _T(""), false, true );
		}

		// check checkbox
		pclSSheet->SetCheckBox( CD_BalancingValve_CheckBox, lRow, _T(""), true, true );
		m_lBvSelectedRow = lRow;

		// Delete Expand/Collapse rows button if exist.
		if( NULL != m_pCDBExpandCollapseRows )
		{
			// Remark: 'm_pCDBExpandCollapseRows' is set to NULL in method!
			DeleteExpandCollapseRowsButton( m_pCDBExpandCollapseRows, pclSheetDescriptionBv );
		}

		// Create Expand/Collapse rows button if needed...
		LPARAM lValveTotalCount;
		pclSheetDescriptionBv->GetUserVariable( _SDUV_TOTALPRODUCT_COUNT, lValveTotalCount );
		
		if( lValveTotalCount > 1 )
		{
			m_pCDBExpandCollapseRows = CreateExpandCollapseRowsButton( CD_BalancingValve_FirstColumn, lRow, true, 
					CCDButtonExpandCollapseRows::ButtonState::CollapseRow, pclSheetDescriptionBv->GetFirstSelectableRow(), 
					pclSheetDescriptionBv->GetLastSelectableRow( false ), pclSheetDescriptionBv );

			// Show button.
			if( NULL != m_pCDBExpandCollapseRows )
			{
				m_pCDBExpandCollapseRows->SetShowStatus( true );
			}
		}

		// Select balancing valve (just highlight background).
		pclSSheet->SelectOneRow( lRow, pclSheetDescriptionBv->GetSelectionFrom(), pclSheetDescriptionBv->GetSelectionTo() );

		// Fill corresponding accessories.
		_FillAccessoryRows();

		// Try to set the focus on the next table but do not lose the focus if no other table exist.
		CSheetDescription *pclNextSheetDescription = NULL;
		long lNewFocusedRow;
		bool fShiftPressed;
		
		if( DNFERRC_FocusOnNextGroupOK == DefineNextFocusedRow( pclSheetDescriptionBv, CD_BalancingValve_FirstColumn, lRow, false, 
				lNewFocusedRow, pclNextSheetDescription, fShiftPressed ) )
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

		// Notify a change in the selection.
		SelectionHasChanged( IsSelectionReady() );
	}

	return true;
}

bool CRViewSSelBv::ResetColumnWidth( short nSheetDescriptionID )
{
	TSpread clTSpread;

	if( FALSE == clTSpread.Create( WS_CHILD, CRect( 0, 0, 0, 0 ), this, 0 ) )
	{
		ASSERT( 0 );
		return false;
	}
	
	switch( nSheetDescriptionID )
	{
		case SD_Valve:
			m_mapSSheetColumnWidth[SD_Valve][CD_BalancingValve_FirstColumn] = clTSpread.ColWidthToLogUnits( 4 );
			m_mapSSheetColumnWidth[SD_Valve][CD_BalancingValve_CheckBox] = clTSpread.ColWidthToLogUnits( 2 );
			m_mapSSheetColumnWidth[SD_Valve][CD_BalancingValve_Name] = clTSpread.ColWidthToLogUnits( 14 );
			m_mapSSheetColumnWidth[SD_Valve][CD_BalancingValve_Material] = clTSpread.ColWidthToLogUnits( 12 );
			m_mapSSheetColumnWidth[SD_Valve][CD_BalancingValve_Connection] = clTSpread.ColWidthToLogUnits( 14 );
			m_mapSSheetColumnWidth[SD_Valve][CD_BalancingValve_Version] = clTSpread.ColWidthToLogUnits( 12 );
			m_mapSSheetColumnWidth[SD_Valve][CD_BalancingValve_PN] = clTSpread.ColWidthToLogUnits( 6 );
			m_mapSSheetColumnWidth[SD_Valve][CD_BalancingValve_Size] = clTSpread.ColWidthToLogUnits( 10 );
			m_mapSSheetColumnWidth[SD_Valve][CD_BalancingValve_Preset] = clTSpread.ColWidthToLogUnits( 6 );
			m_mapSSheetColumnWidth[SD_Valve][CD_BalancingValve_DpSignal] = clTSpread.ColWidthToLogUnits( 6 );
			m_mapSSheetColumnWidth[SD_Valve][CD_BalancingValve_Dp] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_Valve][CD_BalancingValve_DpFullOpening] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_Valve][CD_BalancingValve_DpHalfOpening] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_Valve][CD_BalancingValve_TemperatureRange] = clTSpread.ColWidthToLogUnits( 8 );
			m_mapSSheetColumnWidth[SD_Valve][CD_BalancingValve_Separator] = clTSpread.ColWidthToLogUnits( 1 );
			m_mapSSheetColumnWidth[SD_Valve][CD_BalancingValve_PipeSize] = clTSpread.ColWidthToLogUnits( 8 );
			m_mapSSheetColumnWidth[SD_Valve][CD_BalancingValve_PipeLinDp] = clTSpread.ColWidthToLogUnits( 8 );
			m_mapSSheetColumnWidth[SD_Valve][CD_BalancingValve_PipeV] = clTSpread.ColWidthToLogUnits( 8 );
			m_mapSSheetColumnWidth[SD_Valve][CD_BalancingValve_Pointer] = clTSpread.ColWidthToLogUnits( 1 );
			break;

		case SD_Accessory:
			// No yet!
			break;
	}

	if( NULL != clTSpread.GetSafeHwnd() )
	{
		clTSpread.DestroyWindow();
	}

	return true;
}

bool CRViewSSelBv::IsSelectionReady( void )
{
	CDB_TAProduct *pSelection = GetCurrentBalancingValveSelected();

	if( NULL != pSelection )
	{
		return ( false == pSelection->IsDeleted() );
	}

	return false; // No selection available
}

void CRViewSSelBv::SetCurrentBalancingValveSelected( CCellDescriptionProduct *pclCDCurrentBalancingValveSelected )
{
	// Try to retrieve sheet description linked to balancing valve.
	CSheetDescription *pclSheetDescriptionBv = m_ViewDescription.GetFromSheetDescriptionID( SD_Valve );

	if( NULL != pclSheetDescriptionBv )
	{
		pclSheetDescriptionBv->SetUserVariable( _SDUV_SELECTEDPRODUCT, (LPARAM)pclCDCurrentBalancingValveSelected );
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PRIVATE MEMBERS
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

long CRViewSSelBv::_FillValveRows( CDS_SSelBv *pEditedProduct )
{
	if( NULL == m_pclIndSelBVParams || NULL == m_pclIndSelBVParams->m_pclSelectBvList 
			|| NULL == m_pclIndSelBVParams->m_pclSelectBvList->GetSelectPipeList() || NULL == m_pclIndSelBVParams->m_pTADS
			|| NULL == m_pclIndSelBVParams->m_pPipeDB )
	{
		ASSERTA_RETURN( 0 );
	}

	// Call 'MultiSpreadBase' method to get a new SSheet.
	CSheetDescription *pclSheetDescriptionBv = CreateSSheet( SD_Valve );

	if( NULL == pclSheetDescriptionBv || NULL == pclSheetDescriptionBv->GetSSheetPointer() )
	{
		return 0;
	}

	CSSheet *pclSSheet = pclSheetDescriptionBv->GetSSheetPointer();

	pclSSheet->SetBool( SSB_REDRAW, FALSE );

	pclSSheet->Init();
	pclSSheet->SetBool( SSB_ALLOWDRAGDROP, FALSE );
	pclSSheet->SetBool( SSB_HORZSCROLLBAR, FALSE );
	pclSSheet->SetBool( SSB_VERTSCROLLBAR, FALSE );
	pclSSheet->SetBool( SSB_MOVEACTIVEONFOCUS, FALSE );
	pclSSheet->SetTextTip( SS_TT_STATUS_FLOATING, 250, NULL, _LIGHTYELLOW, _TAH_BLACK );
	
	CDB_TAProduct *pEditedTAP = NULL;

	if( NULL != pEditedProduct )
	{
		pEditedTAP = pEditedProduct->GetProductAs<CDB_TAProduct>();
	}
	
	// Flags for column customizing.
	CSelectedValve *pclSelectedValve = m_pclIndSelBVParams->m_pclSelectBvList->GetFirst<CSelectedValve>();

	if( NULL == pclSelectedValve )
	{
		return 0;
	}

	CDB_TAProduct *pTAP = dynamic_cast<CDB_TAProduct *>( pclSelectedValve->GetpData() );

	if( NULL == pTAP )
	{
		return 0;
	}

	bool bValveSettingExist = ( NULL != pTAP->GetValveCharacteristic() );
	bool bKvSignalExist = pTAP->IsKvSignalEquipped();

	bool bIsUserPressureDrop = m_pclIndSelBVParams->m_bDpEnabled;

	// Set max rows.
	pclSSheet->SetMaxRows( RD_BalancingValve_FirstAvailRow - 1 );
	
	// All cells are static by default and filled with _T("").
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	pclSSheet->FormatStaticText( -1, -1, -1, -1, _T("") );
	
	// Increase row height.
	double RowHeight = 12.75;
	pclSSheet->SetRowHeight( RD_BalancingValve_FirstRow, RowHeight * 0.5 );
	pclSSheet->SetRowHeight( RD_BalancingValve_GroupName, RowHeight * 1.5 );
	pclSSheet->SetRowHeight( RD_BalancingValve_ColName, RowHeight * 2 );
	pclSSheet->SetRowHeight( RD_BalancingValve_Unit, RowHeight * 1.2 );
	
	// Initialize.
	pclSheetDescriptionBv->Init( 1, pclSSheet->GetMaxRows(), 1, CSheetDescription::SD_ParameterMode_Mono );
	pclSheetDescriptionBv->AddColumnInPixels( CD_BalancingValve_FirstColumn, m_mapSSheetColumnWidth[SD_Valve][CD_BalancingValve_FirstColumn] );
	pclSheetDescriptionBv->AddColumnInPixels( CD_BalancingValve_CheckBox, m_mapSSheetColumnWidth[SD_Valve][CD_BalancingValve_CheckBox] );
	pclSheetDescriptionBv->AddColumnInPixels( CD_BalancingValve_Name, m_mapSSheetColumnWidth[SD_Valve][CD_BalancingValve_Name] );
	pclSheetDescriptionBv->AddColumnInPixels( CD_BalancingValve_Material, m_mapSSheetColumnWidth[SD_Valve][CD_BalancingValve_Material] );
	pclSheetDescriptionBv->AddColumnInPixels( CD_BalancingValve_Connection, m_mapSSheetColumnWidth[SD_Valve][CD_BalancingValve_Connection] );
	pclSheetDescriptionBv->AddColumnInPixels( CD_BalancingValve_Version, m_mapSSheetColumnWidth[SD_Valve][CD_BalancingValve_Version] );
	pclSheetDescriptionBv->AddColumnInPixels( CD_BalancingValve_PN, m_mapSSheetColumnWidth[SD_Valve][CD_BalancingValve_PN] );
	pclSheetDescriptionBv->AddColumnInPixels( CD_BalancingValve_Size, m_mapSSheetColumnWidth[SD_Valve][CD_BalancingValve_Size] );
	pclSheetDescriptionBv->AddColumnInPixels( CD_BalancingValve_Preset, m_mapSSheetColumnWidth[SD_Valve][CD_BalancingValve_Preset] );
	pclSheetDescriptionBv->AddColumnInPixels( CD_BalancingValve_DpSignal, m_mapSSheetColumnWidth[SD_Valve][CD_BalancingValve_DpSignal] );
	pclSheetDescriptionBv->AddColumnInPixels( CD_BalancingValve_Dp, m_mapSSheetColumnWidth[SD_Valve][CD_BalancingValve_Dp] );
	pclSheetDescriptionBv->AddColumnInPixels( CD_BalancingValve_DpFullOpening, m_mapSSheetColumnWidth[SD_Valve][CD_BalancingValve_DpFullOpening]);
	pclSheetDescriptionBv->AddColumnInPixels( CD_BalancingValve_DpHalfOpening, m_mapSSheetColumnWidth[SD_Valve][CD_BalancingValve_DpHalfOpening] );
	pclSheetDescriptionBv->AddColumnInPixels( CD_BalancingValve_TemperatureRange, m_mapSSheetColumnWidth[SD_Valve][CD_BalancingValve_TemperatureRange] );
	pclSheetDescriptionBv->AddColumnInPixels( CD_BalancingValve_Separator, m_mapSSheetColumnWidth[SD_Valve][CD_BalancingValve_Separator] );
	pclSheetDescriptionBv->AddColumnInPixels( CD_BalancingValve_PipeSize, m_mapSSheetColumnWidth[SD_Valve][CD_BalancingValve_PipeSize] );
	pclSheetDescriptionBv->AddColumnInPixels( CD_BalancingValve_PipeLinDp, m_mapSSheetColumnWidth[SD_Valve][CD_BalancingValve_PipeLinDp] );
	pclSheetDescriptionBv->AddColumnInPixels( CD_BalancingValve_PipeV, m_mapSSheetColumnWidth[SD_Valve][CD_BalancingValve_PipeV] );
	pclSheetDescriptionBv->AddColumnInPixels( CD_BalancingValve_Pointer, m_mapSSheetColumnWidth[SD_Valve][CD_BalancingValve_Pointer] );

	// Set in which column parameter must be saved.
	pclSheetDescriptionBv->AddParameterColumn( CD_BalancingValve_Pointer );
	
	// Set the focus column.
	pclSheetDescriptionBv->SetActiveColumn( CD_BalancingValve_Name );

	// Set range for selection.
	pclSheetDescriptionBv->SetFocusColumnRange( CD_BalancingValve_CheckBox, CD_BalancingValve_TemperatureRange );

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Row name.
	if( false == pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn ) )
	{
		return -1;
	}

	if( false == pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE ) )
	{
		return -1;
	}

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

	pclSSheet->SetStaticText( CD_BalancingValve_Separator, RD_BalancingValve_ColName, _T("") );

	// Pipes.
	pclSSheet->SetStaticText( CD_BalancingValve_PipeSize, RD_BalancingValve_ColName, IDS_SSHEETSSEL_PIPESIZE );
	pclSSheet->SetStaticText( CD_BalancingValve_PipeLinDp, RD_BalancingValve_ColName, IDS_SSHEETSSEL_PIPELINDP );
	pclSSheet->SetStaticText( CD_BalancingValve_PipeV, RD_BalancingValve_ColName, IDS_SSHEETSSEL_PIPEV );
	//
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Row units.
	CUnitDatabase* pUnitDB = CDimValue::AccessUDB();
	pclSSheet->SetStaticText( CD_BalancingValve_Dp, RD_BalancingValve_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ) ).c_str() );
	pclSSheet->SetStaticText( CD_BalancingValve_DpSignal, RD_BalancingValve_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ) ).c_str() );
	pclSSheet->SetStaticText( CD_BalancingValve_DpFullOpening, RD_BalancingValve_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ) ).c_str() );
	pclSSheet->SetStaticText( CD_BalancingValve_DpHalfOpening, RD_BalancingValve_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ) ).c_str() );
	pclSSheet->SetStaticText( CD_BalancingValve_TemperatureRange, RD_BalancingValve_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_TEMPERATURE ) ).c_str() );
	pclSSheet->SetStaticText( CD_BalancingValve_Preset, RD_BalancingValve_Unit, IDS_SHEETHDR_TURNSPOS );

	// Units.
	pclSSheet->SetStaticText( CD_BalancingValve_PipeLinDp, RD_BalancingValve_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_LINPRESSDROP ) ).c_str() );
	pclSSheet->SetStaticText( CD_BalancingValve_PipeV, RD_BalancingValve_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_VELOCITY ) ).c_str() );
	//
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Draw black line below column header.
	pclSSheet->SetCellBorder( CD_BalancingValve_CheckBox, RD_BalancingValve_Unit, CD_BalancingValve_Separator - 1, RD_BalancingValve_Unit, true, SS_BORDERTYPE_BOTTOM );
	pclSSheet->SetCellBorder( CD_BalancingValve_PipeSize, RD_BalancingValve_Unit, CD_BalancingValve_Pointer - 1, RD_BalancingValve_Unit, true, SS_BORDERTYPE_BOTTOM );

	CSelectPipe selPipe( m_pclIndSelBVParams );
	long lRetRow = -1;

	double dRho = m_pclIndSelBVParams->m_WC.GetDens();
	double dKinVisc = m_pclIndSelBVParams->m_WC.GetKinVisc();
	double dFlow = m_pclIndSelBVParams->m_dFlow;

	CDS_TechnicalParameter *pTechParam = m_pclIndSelBVParams->m_pTADS->GetpTechParams();
	
	long lRow = RD_BalancingValve_FirstAvailRow;

	long lValveTotalCount = 0;
	long lValveNotPriorityCount = 0;
	bool bShowAllPrioritiesShown = false;
	long lShowAllPrioritiesButtonRow = 0;
	bool bAtLeastOneError;
	bool bAtLeastOneWarning;

	pclSheetDescriptionBv->SetUserVariable( _SDUV_TOTALPRODUCT_COUNT, 0 );
	pclSheetDescriptionBv->SetUserVariable( _SDUV_NOPRIORITYPRODUCT_COUNT, 0 );

	pclSheetDescriptionBv->RestartRemarkGenerator();
	
	for( pclSelectedValve = m_pclIndSelBVParams->m_pclSelectBvList->GetFirst<CSelectedValve>(); NULL != pclSelectedValve; 
			pclSelectedValve = m_pclIndSelBVParams->m_pclSelectBvList->GetNext<CSelectedValve>() )
	{
		pTAP = dynamic_cast<CDB_TAProduct *>( pclSelectedValve->GetpData() );

		if( NULL == pTAP ) 
		{
			continue;
		}

		bAtLeastOneError = false;
		bAtLeastOneWarning = false;
		bool bBest = pclSelectedValve->IsFlagSet(CSelectedBase::eBest );
		
		if( true == pclSelectedValve->IsFlagSet(CSelectedBase::eNotPriority) )
		{
			lValveNotPriorityCount++;
			
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

		// Add this row.
		// 'true' to specify that this row can be selected.
		pclSheetDescriptionBv->AddRows( 1, true );

		// First columns will be set at the end!
		
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

		// Set the hand wheel presetting.
		CString str = _T("-");

		if( NULL != pTAP && NULL != pTAP->GetValveCharacteristic() )
		{
			str = pTAP->GetValveCharacteristic()->GetSettingString( pclSelectedValve->GetH() );
		}

		if( true == pclSelectedValve->IsFlagSet( CSelectedBase::eValveMaxSetting ) )
		{
			pclSheetDescriptionBv->WriteTextWithFlags( str, CD_BalancingValve_Preset, lRow, CSheetDescription::RemarkFlags::FullOpening );
		}
		else
		{
			if( true == pclSelectedValve->IsFlagSet( CSelectedBase::eValveSetting ) )
			{
				pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_ORANGE );
				bAtLeastOneWarning = true;
			}

			pclSSheet->SetStaticText( CD_BalancingValve_Preset, lRow, str );
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );
		}

		// Set Dp.
		if( true == pclSelectedValve->IsFlagSet( CSelectedBase::eDp ) )
		{
			if( pclSelectedValve->GetDp() > pTechParam->VerifyValvMaxDp( pTAP ) )
			{
				pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_RED );
				bAtLeastOneError = true;
			}
			else
			{
				pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_ORANGE );
				bAtLeastOneWarning = true;
			}
		}

		pclSSheet->SetStaticText( CD_BalancingValve_Dp, lRow, WriteCUDouble( _U_DIFFPRESS, pclSelectedValve->GetDp() ) );
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

		// Set Signal.
		if( true == bKvSignalExist )
		{
			if( true == pclSelectedValve->IsFlagSet( CSelectedBase::eDpSignal ) )
			{
				pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_ORANGE );
				bAtLeastOneWarning = true;
			}		

			pclSSheet->SetStaticText( CD_BalancingValve_DpSignal, lRow, WriteCUDouble( _U_DIFFPRESS, pclSelectedValve->GetDpSignal() ) );
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );
		}

		if( true == pclSelectedValve->IsFlagSet( CSelectedBase::eValveFullODp ) )
		{
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_ORANGE );
			bAtLeastOneWarning = true;
		}

		pclSSheet->SetStaticText( CD_BalancingValve_DpFullOpening, lRow, WriteCUDouble( _U_DIFFPRESS, pclSelectedValve->GetDpFullOpen() ) );
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );
			
		// If Dp unknown
		if( false == bIsUserPressureDrop || -1 == pclSelectedValve->GetDp() )
		{
			if( true == pclSelectedValve->IsFlagSet( CSelectedBase::eValveHalfODp ) )
			{
				pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_ORANGE );
				bAtLeastOneWarning = true;
			}

			pclSSheet->SetStaticText( CD_BalancingValve_DpHalfOpening, lRow, WriteCUDouble( _U_DIFFPRESS, pclSelectedValve->GetDpHalfOpen() ) );
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );
		}

		// Temperature range.
		if( true == pclSelectedValve->IsFlagSet( CSelectedBase::eTemperature ) )
		{
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_RED );
			bAtLeastOneError = true;
		}

		pclSSheet->SetStaticText( CD_BalancingValve_TemperatureRange, lRow, ((CDB_TAProduct *)pclSelectedValve->GetProductIDPtr().MP)->GetTempRange() );
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

		// Now we can set first columns in regards to current status (error, best or normal).
		if( true == bAtLeastOneError )
		{
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_RED );
		}
		else if( true == pclSelectedValve->IsFlagSet( CSelectedBase::eBest ) )
		{
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_GREEN );
		}
		
		// Add checkbox.
		pclSSheet->SetCheckBox(CD_BalancingValve_CheckBox, lRow, _T(""), false, true);

		// In addition of current flag, we have possibility that valve has the flag 'Not available' or 'Deleted'. In that case, we show valve name in red with "*" around it and
		// symbol '!' or '!!' after.
		if( true == pTAP->IsDeleted() )
		{
			pclSheetDescriptionBv->WriteTextWithFlags( CString( pTAP->GetName() ), CD_BalancingValve_Name, lRow, 
					CSheetDescription::RemarkFlags::Deleted, _T(""), _RED );
		}
		else if( false == pTAP->IsAvailable() )
		{
			pclSheetDescriptionBv->WriteTextWithFlags( CString( pTAP->GetName() ), CD_BalancingValve_Name, lRow, 
					CSheetDescription::RemarkFlags::NotAvailable, _T(""), _RED );
		}
		else
		{
			pclSSheet->SetStaticText( CD_BalancingValve_Name, lRow, pTAP->GetName() );
		}

		if( true == bAtLeastOneError )
		{
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );
		}
		else if( true == bBest )
		{
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_GREEN );
		}

		pclSSheet->SetStaticText( CD_BalancingValve_Material, lRow, pTAP->GetBodyMaterial() );
		pclSSheet->SetStaticText( CD_BalancingValve_Connection, lRow, pTAP->GetConnect() );
		pclSSheet->SetStaticText( CD_BalancingValve_Version, lRow, pTAP->GetVersion() );
		pclSSheet->SetStaticText( CD_BalancingValve_PN, lRow, pTAP->GetPN().c_str() );
		pclSSheet->SetStaticText( CD_BalancingValve_Size, lRow, pTAP->GetSize() );
		
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );
		
		// Set pipe size.
		m_pclIndSelBVParams->m_pclSelectBvList->GetSelectPipeList()->GetMatchingPipe( pTAP->GetSizeKey(), selPipe );
		pclSSheet->SetStaticText( CD_BalancingValve_PipeSize, lRow, selPipe.GetpPipe()->GetName() );
		
		// Set the LinDp to orange if it is above or below the technical parameters limits.
		if( selPipe.GetLinDp() > pTechParam->GetPipeMaxDp() || selPipe.GetLinDp() < pTechParam->GetPipeMinDp())
		{
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_ORANGE );
		}

		pclSSheet->SetStaticText( CD_BalancingValve_PipeLinDp, lRow, WriteCUDouble( _U_LINPRESSDROP, selPipe.GetLinDp() ) );
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

		// Set the Velocity to orange if it is above the technical parameter limit.
		// Orange if it is below the dMinVel.
		if( selPipe.GetU() > pTechParam->GetPipeMaxVel() || selPipe.GetU() < pTechParam->GetPipeMinVel() )
		{
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_ORANGE );
		}

		pclSSheet->SetStaticText( CD_BalancingValve_PipeV, lRow, WriteCUDouble( _U_VELOCITY, selPipe.GetU() ) );
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

		// Save parameter.
		CreateCellDescriptionProduct( pclSheetDescriptionBv->GetFirstParameterColumn(), lRow, (LPARAM)pTAP, pclSheetDescriptionBv );

		if( NULL != pEditedTAP && pEditedTAP == pTAP )
		{
			lRetRow = lRow;
		}

		pclSSheet->SetCellBorder( CD_BalancingValve_CheckBox, lRow, CD_BalancingValve_Separator - 1, lRow, true, SS_BORDERTYPE_BOTTOM, 
				SS_BORDERSTYLE_SOLID, _GRAY );

		pclSSheet->SetCellBorder( CD_BalancingValve_PipeSize, lRow, CD_BalancingValve_Pointer - 1, lRow, true, SS_BORDERTYPE_BOTTOM,
				SS_BORDERSTYLE_SOLID, _GRAY );

		lRow++;
		lValveTotalCount++;
	}

	long lLastDataRow = lRow - 1;

	// Hide columns for DpSignal if not needed.
	pclSSheet->ShowCol( CD_BalancingValve_DpSignal, ( true == bKvSignalExist ) ? TRUE : FALSE );

	// ShowDp column when valve setting exist (BV) or for FO, not for VV.
	bool bShowDpColumn = !bKvSignalExist || !bValveSettingExist || bIsUserPressureDrop;
	pclSSheet->ShowCol( CD_BalancingValve_Dp, bShowDpColumn );

	// Column Full opening is show if the user doesn't input specify pressure drop; or in case of VV
	bool bShowDpFOColumn = ( !bKvSignalExist || bValveSettingExist );
	pclSSheet->ShowCol( CD_BalancingValve_DpFullOpening, ( true == bShowDpFOColumn ) ? TRUE : FALSE );

	bool bShowDpHalfOpen = !bIsUserPressureDrop && !bKvSignalExist;
	pclSSheet->ShowCol( CD_BalancingValve_DpHalfOpening, ( true == bShowDpHalfOpen ) ? TRUE : FALSE );

	// Column Presetting is hidden when setting doesn't exist.
	pclSSheet->ShowCol( CD_BalancingValve_Preset, ( true == bValveSettingExist ) ? TRUE : FALSE );

	// Hide columns corresponding to user selected combos.
	pclSSheet->ShowCol( CD_BalancingValve_Material, ( true == m_pclIndSelBVParams->m_strComboMaterialID.IsEmpty() ) ? TRUE : FALSE );
	pclSSheet->ShowCol( CD_BalancingValve_Connection, ( true == m_pclIndSelBVParams->m_strComboConnectID.IsEmpty() ) ? TRUE : FALSE );	
	pclSSheet->ShowCol( CD_BalancingValve_Version, ( true == m_pclIndSelBVParams->m_strComboVersionID.IsEmpty() ) ? TRUE : FALSE );
	pclSSheet->ShowCol( CD_BalancingValve_PN, ( true == m_pclIndSelBVParams->m_strComboPNID.IsEmpty() ) ? TRUE : FALSE );

	pclSheetDescriptionBv->SetUserVariable( _SDUV_TOTALPRODUCT_COUNT, lValveTotalCount );
	pclSheetDescriptionBv->SetUserVariable( _SDUV_NOPRIORITYPRODUCT_COUNT, lValveNotPriorityCount );

	// Now we can create Show/Hide all priorities if needed.
	// Remarks: 2nd condition to avoid to have button if we have only no priority valves.
	if( true == bShowAllPrioritiesShown && lValveTotalCount > lValveNotPriorityCount )
	{
		// Create Show/Hide all priorities button in regards to current state!
		CCDButtonShowAllPriorities::ButtonState eState = ( true == GetShowAllPrioritiesFlag( SD_Valve ) ) ? 
				CCDButtonShowAllPriorities::ButtonState::HidePriorities : CCDButtonShowAllPriorities::ButtonState::ShowPriorities;

		CCDButtonShowAllPriorities *pclShowAllButton = CreateShowAllPrioritiesButton( CD_BalancingValve_FirstColumn, lShowAllPrioritiesButtonRow, 
				bShowAllPrioritiesShown,  eState, lShowAllPrioritiesButtonRow + 1, pclSSheet->GetMaxRows(), pclSheetDescriptionBv );

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
	if( lValveTotalCount > 2 || ( 2 == lValveTotalCount && lValveNotPriorityCount != 1 ) )
	{
		vector<long> vecColumnList;
		vecColumnList.push_back( CD_BalancingValve_PipeSize );
		vecColumnList.push_back( CD_BalancingValve_PipeLinDp );
		vecColumnList.push_back( CD_BalancingValve_PipeV );
		pclSSheet->MergeRowsWithSameValues( RD_BalancingValve_FirstAvailRow, lLastDataRow, vecColumnList );
	}

	pclSSheet->SetCellBorder( CD_BalancingValve_CheckBox, lLastDataRow, CD_BalancingValve_Separator - 1, lLastDataRow, true, 
			SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );

	// Needed by pipe merging.
	pclSSheet->SetCellBorder( CD_BalancingValve_PipeSize, lLastDataRow, CD_BalancingValve_Pointer - 1, lLastDataRow, true, 
			SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );

	// Add Remarks.
	// Note: these lines must be placed after creating Show/Hide all priorities button to avoid to take in count these lines when defining
	//       range of row to be Show/Hide.
	pclSheetDescriptionBv->WriteRemarks( lRow, CD_BalancingValve_CheckBox, CD_BalancingValve_Separator );

	// Add the main title.
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)GetTitleBackgroundColor( pclSheetDescriptionBv ) );
	pclSheetDescriptionBv->SetUserVariable( _SDUV_TITLEFORECOLOR, ( pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor ) ) );
	pclSheetDescriptionBv->SetUserVariable( _SDUV_TITLEBACKCOLOR, ( pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor ) ) );

	pclSSheet->AddCellSpanW( CD_BalancingValve_FirstColumn + 1, RD_BalancingValve_GroupName, CD_BalancingValve_Separator - CD_BalancingValve_FirstColumn - 1, 1 );
	pclSSheet->SetStaticText( CD_BalancingValve_FirstColumn + 1, RD_BalancingValve_GroupName, IDS_SSHEETSSELBV_VALVEGROUP );
	
	pclSSheet->AddCellSpanW( CD_BalancingValve_PipeSize, RD_BalancingValve_GroupName, CD_BalancingValve_Pointer - CD_BalancingValve_PipeSize, 1 );
	
	CTable *pclTable = dynamic_cast<CTable *>( m_pclIndSelBVParams->m_pPipeDB->Get( m_pclIndSelBVParams->m_strPipeSeriesID ).MP );
	ASSERT( NULL != pclTable );

	pclSSheet->SetStaticText( CD_BalancingValve_PipeSize, RD_BalancingValve_GroupName, pclTable->GetName() );
	
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

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

	return lRetRow;
}

void CRViewSSelBv::_FillAccessoryRows( )
{
	if( NULL == m_pclIndSelBVParams || NULL == m_pclIndSelBVParams->m_pTADB )
	{
		ASSERT_RETURN;
	}

	// Retrieve the current selected balancing valve.
	CDB_TAProduct *pclCurrentBalancingValveSelected = GetCurrentBalancingValveSelected();

	if( NULL == pclCurrentBalancingValveSelected )
	{
		return;
	}

	// Sanity check.
	CDB_RuledTable *pRuledTable = (CDB_RuledTable *)( pclCurrentBalancingValveSelected->GetAccessoriesGroupIDPtr().MP );

	if( NULL == pRuledTable )
	{
		return;
	}

	CRank rList;
	int iCount = m_pclIndSelBVParams->m_pTADB->GetAccessories( &rList, pRuledTable, m_pclIndSelBVParams->m_eFilterSelection );
	
	if( 0 == iCount )
	{
		return;
	}

	// Call 'MultiSpreadBase' method to get a new SSheet.
	CSheetDescription *pclSDAccessory = CreateSSheet( SD_Accessory );
	
	if( NULL == pclSDAccessory || NULL == pclSDAccessory->GetSSheetPointer() )
	{
		return;
	}

	CSSheet *pclSSheet = pclSDAccessory->GetSSheetPointer();

	pclSSheet->SetBool( SSB_REDRAW, FALSE );

	pclSSheet->Init();
	pclSSheet->SetBool( SSB_ALLOWDRAGDROP, FALSE );
	pclSSheet->SetBool( SSB_HORZSCROLLBAR, FALSE );
	pclSSheet->SetBool( SSB_VERTSCROLLBAR, FALSE );
	pclSSheet->SetTextTip( SS_TT_STATUS_FLOATING, 250, NULL, _LIGHTYELLOW, _TAH_BLACK );

	// Two lines by accessories, but two accessories by line (reason why we don't multiply iCount by 2).
	pclSSheet->SetMaxRows( RD_Accessory_FirstAvailRow + iCount );

	// All cells are static by default and filled with _T("")
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	pclSSheet->FormatStaticText( -1, -1, -1, -1, _T("") );

	// Get sheet description of balancing valve to retrieve width.
	CSheetDescription *pclSDBalancingValve = m_ViewDescription.GetFromSheetDescriptionID( SD_Valve );
	// It's absolutely not normal to have this pointer NULL.
	ASSERT( NULL != pclSDBalancingValve );
	
	// Take SSheet size and client area size and determine the right width to take in count.
	CRect rectSize = pclSDBalancingValve->GetSSheetPointer()->GetSheetSizeInPixels();

	// Prepare first column width (to match balancing valve sheet).
	long lFirstColumnWidth = pclSDBalancingValve->GetSSheetPointer()->GetColWidthInPixelsW( CD_BalancingValve_FirstColumn );

	// Do some correction in regards to some columns.
	long lTotalWidth = rectSize.Width() - lFirstColumnWidth;
	lTotalWidth -= pclSDBalancingValve->GetSSheetPointer()->GetColWidthInPixelsW( CD_BalancingValve_Pointer );

	// Try to create 2 columns in just the middle of balancing valve sheet.
	long lLeftWidth = (long)( lTotalWidth / 2 );
	long lRightWidth = ( ( lTotalWidth % 2 ) > 0 ) ? lLeftWidth + 1 : lLeftWidth;

	// Set columns.
	// 'CSheetDescription::SD_ParameterMode_Multi' to set that this sheet can have more than one column with parameter.
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
	double dRowHeight = 12.75;
	pclSSheet->SetRowHeight( RD_Accessory_FirstRow, dRowHeight * 0.5 );
	pclSSheet->SetRowHeight( RD_Accessory_GroupName, dRowHeight * 1.5 );

	// Set title.
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)GetTitleBackgroundColor( pclSDAccessory ) );
	
	pclSDAccessory->SetUserVariable( _SDUV_TITLEFORECOLOR, ( pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor ) ) );
	pclSDAccessory->SetUserVariable( _SDUV_TITLEBACKCOLOR, ( pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor ) ) );

	pclSSheet->AddCellSpanW( CD_Accessory_Left, RD_Accessory_GroupName, CD_Accessory_LastColumn - CD_Accessory_Left, 1 );
	pclSSheet->SetStaticText( CD_Accessory_Left, RD_Accessory_GroupName, IDS_SSHEETSSELBV_VALVEACCGROUP );

	// Fill accessories.
	long lRow = RD_Accessory_FirstAvailRow;
	
	CString str;
	LPARAM lparam;

	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_LEFT );

	// Left - Right.
	long lLeftOrRight = CD_Accessory_Left;
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

		CString strName = _T("");

		if( true == pRuledTable->IsByPair( pAccessory->GetIDPtr().ID ) )
		{
			strName += _T("2x ");
		}

		strName += pAccessory->GetName();

		// Create checkbox accessory.
		CCDBCheckboxAccessory *pCheckbox = CreateCheckboxAccessory( lLeftOrRight, lRow, false, true, strName, pAccessory, pRuledTable, &m_vecAccessoryList, pclSDAccessory );

		if( NULL != pCheckbox )
		{
			pCheckbox->ApplyInternalChange();
		}
		
		// Description.
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleEllipses, (LPARAM)TRUE );
		pclSSheet->SetStaticText( lLeftOrRight, lRow + 1, pAccessory->GetComment() );
		bContinue = rList.GetNext( str, lparam );

		// Restart left part.
		if( TRUE == bContinue && CD_Accessory_Right == lLeftOrRight )
		{
			pclSSheet->SetCellBorder( CD_Accessory_Left, lRow + 1, CD_Accessory_LastColumn - 1, lRow + 1, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _GRAY );
			lLeftOrRight = CD_Accessory_Left;
			lRow += 2;
			ASSERT( lRow < pclSSheet->GetMaxRows() );
		}
		else
		{
			lLeftOrRight = CD_Accessory_Right;
		}
		
		if( false == bContinue )
		{
			lRow++;
		}
	}

	VerifyCheckboxAccessories( NULL, false, &m_vecAccessoryList );

	m_pCDBExpandCollapseGroupAccessory = CreateExpandCollapseGroupButton( CD_Accessory_FirstColumn, RD_Accessory_GroupName, true, CCDButtonExpandCollapseGroup::ButtonState::CollapseRow, 
				pclSDAccessory->GetFirstSelectableRow(), pclSDAccessory->GetLastSelectableRow( false ), pclSDAccessory );

	// Show button.
	if( NULL != m_pCDBExpandCollapseGroupAccessory )
	{
		m_pCDBExpandCollapseGroupAccessory->SetShowStatus( true );
	}

	pclSSheet->SetCellBorder( CD_Accessory_Left, lRow, CD_Accessory_LastColumn - 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );

	pclSSheet->SetBool( SSB_REDRAW, TRUE );
}

void CRViewSSelBv::_ReadAllColumnWidth( CDS_IndSelParameter *pclIndSelParameter )
{
	// Access to the 'RViewSSelBv' group.
	CDS_IndSelParameter::CCWWindow *pclCWWindow = pclIndSelParameter->GetWindowColumnWidth( CW_WINDOWID_INDSELBV, true );

	// Access to the 'valve' group.
	ResetColumnWidth( SD_Valve );
	CDS_IndSelParameter::CCWSheet *pclCWSheet = pclCWWindow->GetCWSheet( CW_RVIEWSSELBV_BALANCINGVALVE_SHEETID );
	
	// 2019-12-13: Verify also that we have the same number of columns. Because during a development (Between two releases) it can happen
	// that we add columns without incrementing at each time the version. And it's generating bad dimensioning of the column width.
	if( NULL != pclCWSheet && m_mapSSheetColumnWidth[SD_Valve].size() == pclCWSheet->GetMap().size() )
	{
		short nVersion = pclCWSheet->GetVersion();

		if( CW_RVIEWSSELBV_BALANCINGVALVE_VERSION == nVersion )
		{
			m_mapSSheetColumnWidth[SD_Valve] = pclCWSheet->GetMap();
		}
	}
}

void CRViewSSelBv::_WriteAllColumnWidth( CDS_IndSelParameter *pclIndSelParameter )
{
	// Access to the 'RViewSSelBv' group.
	CDS_IndSelParameter::CCWWindow *pclCWWindow = pclIndSelParameter->GetWindowColumnWidth( CW_WINDOWID_INDSELBV, true );

	// Write the 'valve' group.
	CDS_IndSelParameter::CCWSheet *pclCWSheet = pclCWWindow->GetCWSheet( CW_RVIEWSSELBV_BALANCINGVALVE_SHEETID, true );
	pclCWSheet->SetVersion( CW_RVIEWSSELBV_BALANCINGVALVE_VERSION );
	pclCWSheet->GetMap() = m_mapSSheetColumnWidth[SD_Valve];
}
