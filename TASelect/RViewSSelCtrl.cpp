#include "stdafx.h"


#include "TASelect.h"
#include "MainFrm.h"

#include "DlgLeftTabSelManager.h"

#include "ProductSelectionParameters.h"
#include "RViewSSelCtrl.h"

CRViewSSelCtrl::CRViewSSelCtrl( CMainFrame::RightViewList eRightViewID ) : CRViewSSelSS( eRightViewID, false )
{
	m_pclIndSelCtrlParams = NULL;
	m_lCtrlSelectedRow = 0;
	m_lActuatorSelectedRow = 0;
	m_vecAdapterIter = m_vecControlValveAdapter.begin();
	m_pCDBExpandCollapseRowsCtrl = NULL;
	m_pCDBExpandCollapseGroupCtrlAcc = NULL;
	m_pCDBExpandCollapseGroupAdapter = NULL;
	m_pCDBExpandCollapseRowsActuator = NULL;
	m_pCDBExpandCollapseGroupActuatorAcc = NULL;
	m_pCDBExpandCollapseGroupCVSetAcc = NULL;
	m_pCurrentAdapter = NULL;
	m_bDowngradeActuatorFunctionality = false;
}

CDB_ControlValve *CRViewSSelCtrl::GetCurrentControlValveSelected( void )
{
	CDB_ControlValve *pclCurrentControlValveSelected = NULL;
	CSheetDescription *pclSheetDescriptionCtrl = m_ViewDescription.GetFromSheetDescriptionID( SD_ControlValve );

	if( NULL != pclSheetDescriptionCtrl )
	{
		// Retrieve the current selected control valve.
		CCellDescriptionProduct *pclCDCurrentControlValveSelected = NULL;
		LPARAM lpPointer;

		if( true == pclSheetDescriptionCtrl->GetUserVariable( _SDUV_SELECTEDPRODUCT, lpPointer ) )
		{
			pclCDCurrentControlValveSelected = ( CCellDescriptionProduct * )lpPointer;
		}

		if( NULL != pclCDCurrentControlValveSelected && NULL != pclCDCurrentControlValveSelected->GetProduct() )
		{
			pclCurrentControlValveSelected = dynamic_cast<CDB_ControlValve *>( ( CData * )pclCDCurrentControlValveSelected->GetProduct() );
		}
	}

	return pclCurrentControlValveSelected;
}

CSelectedValve *CRViewSSelCtrl::GetCurrentSelectedValveObject()
{
	CSelectedValve *pclCurrentSelectedValveObject = NULL;
	CSheetDescription *pclSheetDescriptionCtrl = m_ViewDescription.GetFromSheetDescriptionID( SD_ControlValve );

	if( NULL != pclSheetDescriptionCtrl )
	{
		// Retrieve the current selected control valve.
		CCellDescriptionProduct *pclCDCurrentControlValveSelected = NULL;
		LPARAM lpPointer;

		if( true == pclSheetDescriptionCtrl->GetUserVariable( _SDUV_SELECTEDPRODUCT, lpPointer ) )
		{
			pclCDCurrentControlValveSelected = (CCellDescriptionProduct *)lpPointer;
		}

		if( NULL != pclCDCurrentControlValveSelected && NULL != pclCDCurrentControlValveSelected->GetUserParam() )
		{
			pclCurrentSelectedValveObject = (CSelectedValve *)( pclCDCurrentControlValveSelected->GetUserParam() );
		}
	}

	return pclCurrentSelectedValveObject;
}

CDB_Actuator *CRViewSSelCtrl::GetCurrentActuatorSelected( void )
{
	CDB_Actuator *pclCurrentActuatorSelected = NULL;
	CSheetDescription *pclSheetDescriptionActuator = m_ViewDescription.GetFromSheetDescriptionID( SD_Actuator );

	if( NULL != pclSheetDescriptionActuator )
	{
		// Retrieve the current selected actuator.
		CCellDescriptionProduct *pclCDCurrentActuatorSelected = NULL;
		LPARAM lpPointer;

		if( true == pclSheetDescriptionActuator->GetUserVariable( _SDUV_SELECTEDPRODUCT, lpPointer ) )
		{
			pclCDCurrentActuatorSelected = ( CCellDescriptionProduct * )lpPointer;
		}

		if( NULL != pclCDCurrentActuatorSelected && NULL != pclCDCurrentActuatorSelected->GetProduct() )
		{
			pclCurrentActuatorSelected = dynamic_cast<CDB_Actuator *>( ( CData * )pclCDCurrentActuatorSelected->GetProduct() );
		}
	}

	return pclCurrentActuatorSelected;
}

void CRViewSSelCtrl::Reset()
{
	m_lCtrlSelectedRow = 0;
	m_lActuatorSelectedRow = 0;
	m_vecControlValveAccessories.clear();
	m_vecControlValveAdapter.clear();
	m_vecActuatorAccessories.clear();
	m_vecCtrlActuatorSetAccessories.clear();
	m_pCDBExpandCollapseRowsCtrl = NULL;
	m_pCDBExpandCollapseGroupCtrlAcc = NULL;
	m_pCDBExpandCollapseGroupAdapter = NULL;
	m_pCDBExpandCollapseRowsActuator = NULL;
	m_pCDBExpandCollapseGroupActuatorAcc = NULL;
	m_pCDBExpandCollapseGroupCVSetAcc = NULL;
	m_pCurrentAdapter = NULL;
	CRViewSSelSS::Reset();
}

void CRViewSSelCtrl::Suggest( CProductSelelectionParameters *pclProductSelectionParameters, LPARAM lpParam )
{
	CRViewSSelSS::Suggest( pclProductSelectionParameters, lpParam );

	if( NULL == pclProductSelectionParameters || NULL == dynamic_cast<CIndSelCtrlParamsBase*>( pclProductSelectionParameters ) )
	{
		return;
	}

	m_pclIndSelCtrlParams = dynamic_cast<CIndSelCtrlParamsBase*>( pclProductSelectionParameters );

	// To remove all current displayed sheets.
	Reset();

	BeginWaitCursor();
	CWnd::SetRedraw( FALSE );

	CDS_SSelCtrl *pSelectedControlValve = dynamic_cast<CDS_SSelCtrl *>( (CData *)m_pclIndSelCtrlParams->m_SelIDPtr.MP );
	long lRowSelected = -1;

	if( NULL != pSelectedControlValve )
	{
		lRowSelected = FillControlValveRows( pSelectedControlValve->GetProductAs<CDB_ControlValve>() );
	}
	else
	{
		lRowSelected = FillControlValveRows();
	}

	// Verify if sheet description has been well created.
	CSheetDescription *pclSheetDescription = m_ViewDescription.GetFromSheetDescriptionID( SD_ControlValve );

	if( NULL != pclSheetDescription && NULL != pclSheetDescription->GetSSheetPointer() )
	{
		CSSheet *pclSSheet = pclSheetDescription->GetSSheetPointer();

		if( lRowSelected > -1 && NULL != pSelectedControlValve )
		{
			CCellDescriptionProduct *pclCDControlValve = FindCDProduct( lRowSelected, ( LPARAM )( pSelectedControlValve->GetProductAs<CDB_ControlValve>() ),
					pclSheetDescription );

			if( NULL != pclCDControlValve && NULL != pclCDControlValve->GetProduct() )
			{
				// If we are in edition mode we simulate a click on the product.
				OnClickProduct( pclSheetDescription, pclCDControlValve, pclSheetDescription->GetActiveColumn(), lRowSelected );

				// Allow to check if we need to change the 'Show all priorities' button or not.
				CheckShowAllPrioritiesButtonState( pclSheetDescription, lRowSelected );

				// Verify accessories on control valve.
				CAccessoryList *pclCvAccessoryList = pSelectedControlValve->GetCvAccessoryList();
				CAccessoryList::AccessoryItem rAccessoryItem = pclCvAccessoryList->GetFirst( CAccessoryList::_AT_Accessory );

				while( rAccessoryItem.IDPtr.MP != NULL )
				{
					// Remark: 'true' because accessories in the 'pclCvAccessoryList' are those that are selected!
					VerifyCheckboxAccessories( (CDB_Product *)rAccessoryItem.IDPtr.MP, true, &m_vecControlValveAccessories );
					rAccessoryItem = pclCvAccessoryList->GetNext( CAccessoryList::_AT_Accessory );
				}

				// Verify adapter on control valve.
				rAccessoryItem = pclCvAccessoryList->GetFirst( CAccessoryList::_AT_Adapter );

				while( rAccessoryItem.IDPtr.MP != NULL )
				{
					// Remark: 'true' because accessories in the 'pclCvAccessoryList' are those that are selected!
					VerifyCheckboxAccessories( (CDB_Product *)rAccessoryItem.IDPtr.MP, true, &m_vecControlValveAdapter );

					// Gray all incompatible actuators with the selected adapter.
					GrayUncompatibleAccessoriesCheckbox( (CDB_Product *)rAccessoryItem.IDPtr.MP, true );

					rAccessoryItem = pclCvAccessoryList->GetNext( CAccessoryList::_AT_Adapter );
				}

				// Verify if user has selected an actuator.
				IDPTR ActuatorIDPtr = pSelectedControlValve->GetActrIDPtr();
				CSheetDescription *pclSheetDescriptionActuator = m_ViewDescription.GetFromSheetDescriptionID( SD_Actuator );

				if( _NULL_IDPTR != ActuatorIDPtr && NULL != ActuatorIDPtr.MP && NULL != pclSheetDescriptionActuator )
				{
					CDB_Actuator *pEditedActuator = dynamic_cast<CDB_Actuator *>( ActuatorIDPtr.MP );

					if( NULL != pEditedActuator )
					{
						// Find row number where is the	actuator.
						long lActuatorRowSelected = _GetRowOfEditedActuator( pclSheetDescriptionActuator, pEditedActuator );

						if( lActuatorRowSelected != -1 )
						{
							CCellDescriptionProduct *pclCDActuator = FindCDProduct( lActuatorRowSelected, ( LPARAM )pEditedActuator, pclSheetDescriptionActuator );

							if( NULL != pclCDActuator && NULL != pclCDActuator->GetProduct() )
							{
								// Simulate a click on the edited actuator.
								OnClickProduct( pclSheetDescriptionActuator, pclCDActuator, pclSheetDescriptionActuator->GetActiveColumn(), lActuatorRowSelected );

								// Verify accessories on actuator.
								CAccessoryList *pclActuatorAccessoryList = pSelectedControlValve->GetActuatorAccessoryList();
								rAccessoryItem = pclActuatorAccessoryList->GetFirst();

								while( rAccessoryItem.IDPtr.MP != NULL )
								{
									VerifyCheckboxAccessories( (CDB_Product *)rAccessoryItem.IDPtr.MP, true, &m_vecActuatorAccessories );
									rAccessoryItem = pclActuatorAccessoryList->GetNext();
								}

								// HYS-959 : The adapter is not checked we must uncheck it because the ClickOnActuator() automatically checked the adapter
								// Verify adapter on control valve.
								rAccessoryItem = pclCvAccessoryList->GetFirst(CAccessoryList::_AT_Adapter);

								if (rAccessoryItem.IDPtr.MP == NULL)
								{
									// Remark: 'false' no adapter is selected
									// uncheck the checkbox
									VerifyCheckboxAccessories(m_pCurrentAdapter, false, &m_vecControlValveAdapter);
									// Remove gray actuators
									GrayUncompatibleAccessoriesCheckbox(m_pCurrentAdapter, false);
									// Update the selection sheet 
									SelectionHasChanged(IsSelectionReady());
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
			PrepareAndSetNewFocus( pclSheetDescription, CD_ControlValve_Name, RD_ControlValve_FirstAvailRow, 0 );
		}
	}

	// Call inherited class (see .h for description).
	SuggestOtherProducts( pSelectedControlValve );

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

void CRViewSSelCtrl::FillInSelected( CDS_SelProd *pSelectedProductToFill )
{
	if( NULL == m_pclIndSelCtrlParams )
	{
		ASSERT_RETURN;
	}

	CDS_SSelCtrl *pSelectedCtrlToFill = dynamic_cast<CDS_SSelCtrl *>( pSelectedProductToFill );

	if( NULL == pSelectedCtrlToFill )
	{
		ASSERT_RETURN;
	}

	// Clear previous selected actuator.
	pSelectedCtrlToFill->SetActrIDPtr( _NULL_IDPTR );

	// HYS-987: Clear previous selected accessories on control valve if we are not in edition mode.
	CAccessoryList *pclCvAccessoryListToFill = pSelectedCtrlToFill->GetCvAccessoryList();

	// Clear previous selected accessories on actuator if we are not in edition mode.
	CAccessoryList *pclActuatorAccessoryListToFill = pSelectedCtrlToFill->GetActuatorAccessoryList();
	bool bIsEditionMode = false;

	if( false == m_pclIndSelCtrlParams->m_bEditModeRunning )
	{
		pclCvAccessoryListToFill->Clear();
		pclActuatorAccessoryListToFill->Clear();
	}
	else
	{
		bIsEditionMode = true;
	}

	// Fill data for control valve.
	CDB_ControlValve *pclCurrentControlValveSelected = GetCurrentControlValveSelected();

	if( NULL != pclCurrentControlValveSelected && NULL != m_pclIndSelCtrlParams->GetSelectCtrlList() )
	{
		pSelectedCtrlToFill->SetProductIDPtr( pclCurrentControlValveSelected->GetIDPtr() );

		// Search balancing valve in CSelectList to set the correct opening.
		for( CSelectedValve *pclSelectedValve = m_pclIndSelCtrlParams->GetSelectCtrlList()->GetFirst<CSelectedValve>(); NULL != pclSelectedValve;
				pclSelectedValve = m_pclIndSelCtrlParams->GetSelectCtrlList()->GetNext<CSelectedValve>() )
		{
			if( pclSelectedValve->GetpData() == pclCurrentControlValveSelected )	// Product found
			{
				pSelectedCtrlToFill->SetOpening( pclSelectedValve->GetH() );
				break;
			}
		}

		// HYS-987.
		if( false == bIsEditionMode )
		{
			// Retrieve selected accessory and add it.
			for( vecCDCAccessoryListIter vecIter = m_vecControlValveAccessories.begin(); vecIter != m_vecControlValveAccessories.end(); vecIter++ )
			{
				CCDBCheckboxAccessory *pCDBCheckboxAccessory = *vecIter;

				if( NULL != pCDBCheckboxAccessory && true == pCDBCheckboxAccessory->GetCheckStatus() && NULL != pCDBCheckboxAccessory->GetAccessoryPointer() )
				{
					pclCvAccessoryListToFill->Add( pCDBCheckboxAccessory->GetAccessoryPointer()->GetIDPtr(), CAccessoryList::_AT_Accessory,
						pCDBCheckboxAccessory->GetRuledTable() );
				}
			}

			// Retrieve selected adapter and add it.
			for( vecCDCAccessoryListIter vecIter = m_vecControlValveAdapter.begin(); vecIter != m_vecControlValveAdapter.end(); vecIter++ )
			{
				CCDBCheckboxAccessory *pCDBCheckboxAccessory = *vecIter;

				if( NULL != pCDBCheckboxAccessory && true == pCDBCheckboxAccessory->GetCheckStatus() && NULL != pCDBCheckboxAccessory->GetAccessoryPointer() )
				{
					pclCvAccessoryListToFill->Add( pCDBCheckboxAccessory->GetAccessoryPointer()->GetIDPtr(), CAccessoryList::_AT_Adapter,
						pCDBCheckboxAccessory->GetRuledTable() );
				}
			}

			// Retrieve selected control valve actuator set accessories and add it.
			for( vecCDCAccessoryListIter vecIter = m_vecCtrlActuatorSetAccessories.begin(); vecIter != m_vecCtrlActuatorSetAccessories.end(); vecIter++ )
			{
				CCDBCheckboxAccessory *pCDBCheckboxAccessory = *vecIter;

				if( NULL != pCDBCheckboxAccessory && true == pCDBCheckboxAccessory->GetCheckStatus() && NULL != pCDBCheckboxAccessory->GetAccessoryPointer() )
				{
					pclCvAccessoryListToFill->Add( pCDBCheckboxAccessory->GetAccessoryPointer()->GetIDPtr(), CAccessoryList::_AT_SetAccessory,
						pCDBCheckboxAccessory->GetRuledTable() );
				}
			}
		}
		else
		{
			// for valve accessories
			UpdateAccessoryList( m_vecControlValveAccessories, pclCvAccessoryListToFill, CAccessoryList::_AT_Accessory );
			// for valve adapter
			UpdateAccessoryList( m_vecControlValveAdapter, pclCvAccessoryListToFill, CAccessoryList::_AT_Adapter );
			// for set
			UpdateAccessoryList( m_vecCtrlActuatorSetAccessories, pclCvAccessoryListToFill, CAccessoryList::_AT_SetAccessory );
		}

		// Selected pipe informations.
		if( NULL != m_pclIndSelCtrlParams->GetSelectCtrlList()->GetSelectPipeList() )
		{
			m_pclIndSelCtrlParams->GetSelectCtrlList()->GetSelectPipeList()->GetMatchingPipe( pclCurrentControlValveSelected->GetSizeKey(), 
					*pSelectedCtrlToFill->GetpSelPipe() );
		}
	}

	// Fill data for actuator.
	CDB_Actuator *pclCurrentSelectedActuator = GetCurrentActuatorSelected();

	if( NULL != pclCurrentSelectedActuator )
	{
		pSelectedCtrlToFill->SetActrIDPtr( pclCurrentSelectedActuator->GetIDPtr() );
		// HYS-987
		if( false == bIsEditionMode )
		{
			// Retrieve selected accessory and add it.
			for( vecCDCAccessoryListIter vecIter = m_vecActuatorAccessories.begin(); vecIter != m_vecActuatorAccessories.end(); vecIter++ )
			{
				CCDBCheckboxAccessory *pCDBCheckboxAccessory = *vecIter;

				if( NULL != pCDBCheckboxAccessory && true == pCDBCheckboxAccessory->GetCheckStatus() && NULL != pCDBCheckboxAccessory->GetAccessoryPointer() )
				{
					pclActuatorAccessoryListToFill->Add( pCDBCheckboxAccessory->GetAccessoryPointer()->GetIDPtr(), CAccessoryList::_AT_Accessory,
						pCDBCheckboxAccessory->GetRuledTable() );
				}
			}
		}
		else
		{
			// for actuator accessories
			UpdateAccessoryList( m_vecActuatorAccessories, pclActuatorAccessoryListToFill, CAccessoryList::_AT_Accessory );
		}
	}
}

void CRViewSSelCtrl::OnNewDocument( CDS_IndSelParameter *pclIndSelParameter )
{
	_ReadAllColumnWidth( pclIndSelParameter );
}

void CRViewSSelCtrl::SaveSelectionParameters( CDS_IndSelParameter *pclIndSelParameter )
{
	_WriteAllColumnWidth( pclIndSelParameter );
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PROTECTED MEMBERS
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CRViewSSelCtrl::OnTextTipFetch( CSheetDescription *pclSheetDescription, long lColumn, long lRow, WORD *pwMultiLine, SHORT *pnTipWidth, 
		TCHAR *pstrTipText, BOOL *pbShowTip )
{
	if( NULL == m_pclIndSelCtrlParams || NULL == m_pclIndSelCtrlParams->GetSelectCtrlList() || false == m_bInitialised 
			|| NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() )
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

	CDB_TAProduct *pTAP = dynamic_cast<CDB_TAProduct *>( ( CData * )GetCDProduct( lColumn, lRow, pclSheetDescription ) );
	CDB_ControlValve *pCv = dynamic_cast<CDB_ControlValve *>( pTAP );
	CSelectedValve *pclSelectedCtrl = GetSelectProduct<CSelectedValve>( pTAP, m_pclIndSelCtrlParams->GetSelectCtrlList() );
	CSSheet *pclSSheet = pclSheetDescription->GetSSheetPointer();
	CDS_TechnicalParameter *pclTechParam = m_pclIndSelCtrlParams->m_pTADS->GetpTechParams();

	switch( pclSheetDescription->GetSheetDescriptionID() )
	{
		case SD_ControlValve:

			if( NULL != pclSelectedCtrl )
			{
				if( CD_ControlValve_ImgCharacteristic == lColumn )
				{
					if( NULL != pCv )
					{
						CDB_ControlProperties::eCTRLCHAR CtrlChar = ( ( CDB_ControlProperties * )pCv->GetCtrlProp() )->GetCvCtrlChar();

						if( CDB_ControlProperties::NotCharacterized == CtrlChar )
						{
							str = TASApp.LoadLocalizedString( IDS_TOOLTIP_UNDEF );
						}
						else if( CDB_ControlProperties::Linear == CtrlChar )
						{
							str = TASApp.LoadLocalizedString( IDS_TOOLTIP_LINEARCHARACT );
						}
						else if( CDB_ControlProperties::EqualPc == CtrlChar )
						{
							str = TASApp.LoadLocalizedString( IDS_TOOLTIP_EQUPERCHARACT );
						}
					}
				}
				else if( CD_ControlValve_ImgPushClose == lColumn )
				{
					if( NULL != pCv )
					{
						CDB_ControlProperties::ePushOrPullToClose PushClose = ( ( CDB_ControlProperties * )pCv->GetCtrlProp() )->GetCvPushClose();

						if( CDB_ControlProperties::PushToClose == PushClose )
						{
							str = TASApp.LoadLocalizedString( IDS_TOOLTIP_PUSHCLOSE );
						}
						else if( CDB_ControlProperties::PullToClose == PushClose )
						{
							str = TASApp.LoadLocalizedString( IDS_TOOLTIP_PULLCLOSE );
						}
					}
				}
				else if( CD_ControlValve_TemperatureRange == lColumn )
				{
					if( true == pclSelectedCtrl->IsFlagSet( CSelectedBase::eTemperature ) )
					{
						FormatString( str, IDS_SSHEETSSEL_TEMPERROR, pCv->GetTempRange() );
					}
				}
				else if( CD_ControlValve_PipeLinDp == lColumn )
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
				else if( CD_ControlValve_PipeV == lColumn )
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

		case SD_ControlValveAccessory:

			if( lColumn > CD_Accessory_FirstColumn && lColumn < CD_Accessory_LastColumn )
			{
				TextTipFetchEllipsesHelper( lColumn, lRow, pclSheetDescription, pnTipWidth, &str );
			}

			break;

		case SD_Actuator:
		{
			// Try to cast selected object to actuator.
			CDB_ElectroActuator *pclElectroActuator = dynamic_cast<CDB_ElectroActuator *>( ( CData * )GetCDProduct( lColumn, lRow, pclSheetDescription ) );

			if( NULL != pclElectroActuator )
			{
				if( CD_Actuator_CloseOffValue == lColumn )
				{
					// Try to retrieve current selected control valve.
					CDB_ControlValve *pclCurrentCV = GetCurrentControlValveSelected();
					CSelectedValve *pSelectedCV = GetSelectProduct<CSelectedValve>( pclCurrentCV, m_pclIndSelCtrlParams->GetSelectCtrlList() );

					if( NULL != pclCurrentCV && NULL != pSelectedCV )
					{
						CDB_CloseOffChar *pCloseOffChar = (CDB_CloseOffChar *)( pclCurrentCV->GetCloseOffCharIDPtr().MP );

						if( NULL != pCloseOffChar && CDB_CloseOffChar::CloseOffDp == pCloseOffChar->GetLimitType() )
						{
							double dDpmax = pclCurrentCV->GetDpmax();
							double dCloseOffDp = pCloseOffChar->GetCloseOffDp( pclElectroActuator->GetMaxForceTorque() );

							if( -1.0 != dCloseOffDp && -1.0 != dDpmax && dCloseOffDp > dDpmax )
							{
								CString str2 = WriteCUDouble( _U_DIFFPRESS, dCloseOffDp, true );
								CString str3 = WriteCUDouble( _U_DIFFPRESS, dDpmax, true );
								FormatString( str, IDS_SSHEETSSELCV_CLOSEOFFERROR, str2, str3 );
							}
							// HYS-1519 : If the closeOfDp is below the Dp max of the PIBCV valve we authorize to show the 
							// actuator with a tooltip on the CloseOffDp value because at Dp max the actuator can't close the valve.
							if( NULL != dynamic_cast<CDB_PIControlValve*>( pclCurrentCV ) && -1.0 != dCloseOffDp
								&& dCloseOffDp < dDpmax )
							{
								CString str2 = WriteCUDouble( _U_DIFFPRESS, dCloseOffDp, true );
								CString str3 = WriteCUDouble( _U_DIFFPRESS, dDpmax, true );
								FormatString( str, IDS_SSHEETSSELCV_CLOSEOFFERROR, str2, str3 );
							}
						}
					}
				}
				else if( CD_Actuator_InputSig == lColumn && _ORANGE == pclSSheet->GetForeColor( lColumn, lRow ) )
				{
					str = TASApp.LoadLocalizedString( IDS_SSHEETSSELCV_FCTDOWNGRADED );
				}
				//HYS-726
				else if( ( CD_Actuator_Name == lColumn || CD_Actuator_MaxTemp == lColumn) && _RED == pclSSheet->GetForeColor( lColumn, lRow ) )
				{
					str = TASApp.LoadLocalizedString( IDS_SSHEETACT_TT_MAXTEMPERROR );
				}
				else if( CD_Actuator_FailSafe == lColumn && _ORANGE == pclSSheet->GetForeColor( lColumn, lRow ) )
				{
					str = TASApp.LoadLocalizedString( IDS_SSHEETACT_TT_FAILSAFENOTMATCH );
				}
				else if( CD_Actuator_DefaultReturnPos == lColumn && _ORANGE == pclSSheet->GetForeColor( lColumn, lRow ) )
				{
					str = TASApp.LoadLocalizedString( IDS_SSHEETACT_TT_DRPNOTFOUND );
				}
			}
		}
		break;

		case SD_ActuatorAccessory:
			if( lColumn > CD_Accessory_FirstColumn && lColumn < CD_Accessory_LastColumn )
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

void CRViewSSelCtrl::GrayUncompatibleAccessoriesCheckbox( CDB_Product *pclAccessory, bool bCheck )
{
	// Check .h file to have complete description of this method.

	// First, verify if it's well an adapter.
	if( NULL == pclAccessory || false == pclAccessory->IsAnAccessory() )
	{
		return;
	}

	CString strFamilyID;
	if( CDB_Product::eProdUse::eBoth == pclAccessory->GetProductUseFlag() )
	{
		strFamilyID = pclAccessory->GetAccessFamilyIDPtr().ID;
	}
	else
	{
		strFamilyID = pclAccessory->GetFamilyID();
	}

	if( true == strFamilyID.IsEmpty() || -1 == strFamilyID.Find( _T("ADAPTFAM_") ) )
	{
		return;
	}

	CDB_ControlValve *pclCurrentControlValveSelected = GetCurrentControlValveSelected();

	if( NULL == pclCurrentControlValveSelected || NULL == pclCurrentControlValveSelected->GetActuatorGroupIDPtr().MP )
	{
		return;
	}

	// Retrieve sheet description.
	CSheetDescription *pclSheetDescriptionActuator = m_ViewDescription.GetFromSheetDescriptionID( SD_Actuator );

	if( NULL == pclSheetDescriptionActuator || NULL == pclSheetDescriptionActuator->GetSSheetPointer() )
	{
		return;
	}

	CSSheet *pclSSheet = pclSheetDescriptionActuator->GetSSheetPointer();

	long lFirstFocusColumn, lLastFocusColumn;
	pclSheetDescriptionActuator->GetFocusColumnRange( lFirstFocusColumn, lLastFocusColumn );

	// Variables.
	LPARAM lparam = 0;
	CTable *pTab = NULL;
	IDPTR IDPtr = _NULL_IDPTR;
	bool bAtLeastOneActuatorFit = false;
	bool bActuatorGrayed = true;

	m_pCurrentAdapter = pclAccessory;

	// Retrieve current actuator selected if exist.
	CDB_Actuator *pclCurrentActuatorSelected = GetCurrentActuatorSelected();

	// Run all registered products.
	CSheetDescription::vecCellDescription vecProductList;
	pclSheetDescriptionActuator->GetCellDescriptionList( vecProductList, RVSCellDescription::CD_Product );

	for( CSheetDescription::vecCellDescriptionIter vecIter = vecProductList.begin(); vecIter != vecProductList.end(); vecIter++ )
	{
		// Retrieve info on current product.
		CCellDescriptionProduct *pCDProduct = dynamic_cast<CCellDescriptionProduct *>( *vecIter );

		if( NULL == pCDProduct || NULL == pCDProduct->GetProduct() )
		{
			continue;
		}

		// Test if it's well an actuator.
		CDB_Actuator *pclActuator = dynamic_cast<CDB_Actuator *>( ( CData * )pCDProduct->GetProduct() );

		if( NULL == pclActuator )
		{
			continue;
		}

		long lActuatorRow = pCDProduct->GetCellPosition().y;

		// If user has deselected accessory (or adapter).
		// We have 2 cases. When user selects a valve we display a list of compatible actuators following by a list of compatible adapters.
		// If user directly clicks on an adapter, there is not yet actuator selected and in this case we can call 'UnSelectMultipleRows' in case of the
		// actuator was grayed before.
		// If user clicks first on an actuator, the corresponding adapter is selected. If user clicks now on the adapter selected to unselect it, we 
		// can't call 'UnSelectMultipleRows' because the actuator must be let selected.
		if( false == bCheck && ( NULL == pclCurrentActuatorSelected || 0 != _tcscmp( pclCurrentActuatorSelected->GetIDPtr().ID, pclActuator->GetIDPtr().ID ) ) )
		{
			pclSSheet->UnSelectMultipleRows( lActuatorRow );
			// One actuator fit always if no selection have been made.
			bAtLeastOneActuatorFit = true;
			continue;
		}

		// Retrieve table containing list of all adapters compatible with current actuator.
		pTab = ( CTable * )( pclActuator->GetActAdapterGroupIDPtr().MP );

		// If there is no adapter for current actuator we can directly gray it and check the next actuator.
		// Otherwise, check if current control valve adapter is compatible with at least one adapter from the list of current actuator.
		// If it's the case, we can not grayed current actuator.
		bActuatorGrayed = true;

		if( pTab != NULL )
		{
			for( IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; IDPtr = pTab->GetNext() )
			{
				CDB_Product *pclActuatorAdapter = (CDB_Product *)IDPtr.MP;

				if( pclAccessory->GetIDPtr() == pclActuatorAdapter->GetIDPtr() )
				{
					bActuatorGrayed = false;
					break;
				}
			}
		}

		// If current actuator is disabled for adapter selected by user...
		if( true == bActuatorGrayed )
		{
			pclSSheet->UnSelectMultipleRows( lActuatorRow );
			pclSSheet->SelectOneRow( lActuatorRow, lFirstFocusColumn, lLastFocusColumn, SPREAD_COLOR_IGNORE, _DARKGRAY );

			if( pclActuator == pclCurrentActuatorSelected )
			{
				// Delete the current actuator selection.
				SetCurrentActuatorSelected( NULL );

				// Uncheck checkbox.
				pclSSheet->SetCheckBox( CD_Actuator_CheckBox, lActuatorRow, _T(""), false, true );

				// Disable all current accessories for the actuator.
				DisableCheckboxAccessories( _T("ACTACCGROUP_TAB"), &m_vecActuatorAccessories );

				// Delete Expand/Collapse rows button if exist...
				if( NULL != m_pCDBExpandCollapseRowsActuator )
				{
					// Remark: 'm_pCDBExpandCollapseRowsActuator' is set to NULL in method!
					DeleteExpandCollapseRowsButton( m_pCDBExpandCollapseRowsActuator, pclSheetDescriptionActuator );
				}
			}
		}
		else
		{
			bAtLeastOneActuatorFit = true;
		}
	}
	
	// HYS-1108: update the display of crossing accessory depending on the actuators
	if( NULL == GetCurrentActuatorSelected() )
	{
		GrayUncompatibleCrossingAccessories( GetCurrentActuatorSelected(), false );
	}
	
	// If any actuator fit (all are grayed) or there is no more actuator selected...
	if( false == bAtLeastOneActuatorFit || NULL == GetCurrentActuatorSelected() )
	{
		// Remove only sheets that are linked to actuators.
		if( true == m_ViewDescription.IsSheetDescriptionExist( SD_ActuatorAccessory ) )
		{
			m_ViewDescription.RemoveOneSheetDescription( SD_ActuatorAccessory );
		}

		if( true == m_ViewDescription.IsSheetDescriptionExist( SD_CtrlActuatorAccessorySet ) )
		{
			m_ViewDescription.RemoveOneSheetDescription( SD_CtrlActuatorAccessorySet );
		}

		// We force a refresh of the control valve adapter group (If exist) to be sure to have the right color in the
		// main title.
		if( true == m_ViewDescription.IsSheetDescriptionExist( SD_ControlValveAdapter ) )
		{
			// HYS-912: don't remove the sheet and create it again because we lost the fact that user has clicked for example
			// on one adapter.
			UpdateTitleBackgroundColor( m_ViewDescription.GetFromSheetDescriptionID( SD_ControlValveAdapter ) );
		}
	}
}

void CRViewSSelCtrl::GrayUncompatibleCrossingAccessories( CDB_Actuator *pclCurrentActuatorSelected, bool bCheck )
{
	// HYS-1617: This code has been duplicated in the 'CDlgDirSel::_CheckCrossingAccessoriesCVAndActuator' method for the direct selection in the 'DglDirSel.cpp' file.
	// Any modification applied here must be done also in 'CDlgDirSel::_CheckCrossingAccessoriesCVAndActuator' !!

	CDB_ControlValve *pclCurrentControlValveSelected = GetCurrentControlValveSelected();

	if( NULL == pclCurrentControlValveSelected )
	{
		return;
	}

	vector<CDB_Product *> vecCrossingAccessories;
	CDB_RuledTable *pclRuledTableActuator = NULL;

	if( NULL != pclCurrentActuatorSelected )
	{
		pclRuledTableActuator = (CDB_RuledTable *)( pclCurrentActuatorSelected->GetAccessoriesGroupIDPtr().MP );
	}
	
	CDB_RuledTable *pclRuledTableCtrlValve = (CDB_RuledTable *)( pclCurrentControlValveSelected->GetAccessoriesGroupIDPtr().MP );

	// Find crossing accessories.
	if( NULL == pclRuledTableCtrlValve )
	{
		return;
	}

	for( IDPTR IDPtr = pclRuledTableCtrlValve->GetFirst(); NULL != IDPtr.MP; IDPtr = pclRuledTableCtrlValve->GetNext() )
	{
		if( true == pclRuledTableCtrlValve->IsACrossingAccDisplayed( IDPtr.MP ) )
		{
			vecCrossingAccessories.push_back( dynamic_cast<CDB_Product*>( IDPtr.MP ) );
		}
	}

	// First step: If we select an actuator (bCheck = true), check if we need to activate the accessory in common if exist.
	if( NULL != pclRuledTableActuator && true == bCheck )
	{
		// Find shared accessories between the two cross tables.
		for( IDPTR IDPtr = pclRuledTableActuator->GetFirst(); NULL != IDPtr.MP; IDPtr = pclRuledTableActuator->GetNext() )
		{
			// Search if the accessory in the actuator accessory table exists in the accessory table of the valve that is cross-referenced.
			vector<CDB_Product *>::iterator it = find( vecCrossingAccessories.begin(), vecCrossingAccessories.end(), dynamic_cast<CDB_Product *>( IDPtr.MP ) );

			// If accessory is common (We have in this case the '+' for the accessory in the actuator accessory group which signals us that it's an accessory
			// common with the product but that we can't show -> The 'IsACrossingAcc' method returns in this case 'true').
			if( ( true == pclRuledTableActuator->IsACrossingAcc( IDPtr.MP ) ) && ( vecCrossingAccessories.end() != it ) )
			{
				vecCrossingAccessories.erase( it );

				// Search in the available accessories for the current control valve if this common accessory exist.
				for( vecCDCAccessoryListIter vecIter = m_vecControlValveAccessories.begin(); vecIter != m_vecControlValveAccessories.end(); vecIter++ )
				{
					CCDBCheckboxAccessory *pCDBCheckboxAccessory = *vecIter;

					if( NULL != pCDBCheckboxAccessory && NULL != pCDBCheckboxAccessory->GetAccessoryPointer() )
					{
						// If the current control valve accessory displayed is the same as the common accessory...
						if( dynamic_cast<CDB_Product *>( pCDBCheckboxAccessory->GetAccessoryPointer()->GetIDPtr().MP ) == IDPtr.MP )
						{
							// Change state if the previous one was disabled.
							if( false == pCDBCheckboxAccessory->GetEnableStatus() )
							{
								pCDBCheckboxAccessory->SetEnableStatus( true );
							}
							
							pCDBCheckboxAccessory->ApplyInternalChange();
						}
					}
				}
			}
		}
	}

	// bCheck = true  -> Accessories which are not compatible with actuator (Those that were common with the actuator were removed in the first step just above).
	// bCheck = false -> Accessories that belong to a cross-reference table and are displayed.
	if( vecCrossingAccessories.size() > 0 )
	{
		// Run all the control valve accessories in the right view.
		for( vecCDCAccessoryListIter vecIter = m_vecControlValveAccessories.begin(); vecIter != m_vecControlValveAccessories.end(); vecIter++ )
		{
			CCDBCheckboxAccessory *pCDBCheckboxAccessory = *vecIter;
			
			if( NULL != pCDBCheckboxAccessory && NULL != pCDBCheckboxAccessory->GetAccessoryPointer() )
			{
				// Verify if the current accessory in the right view belongs to a cross-reference table.
				vector<CDB_Product *>::iterator it = find( vecCrossingAccessories.begin(), vecCrossingAccessories.end(), dynamic_cast<CDB_Product *>( pCDBCheckboxAccessory->GetAccessoryPointer()->GetIDPtr().MP ) );

				// If it's a common accessory between valve and actuator...
				if( it != vecCrossingAccessories.end() )
				{
					if( true == bCheck )
					{
						// If 'bCheck = true' and there are yet accessories in 'vecCrossingAccessories', it means that these accessories are not common with the
						// the current selected actuator (Because if common the 'vecCrossingAccessories.erase( it )' remove this accessory from the vector).

						// If the accessory wad before selected, we need to unselect it.
						if( true == pCDBCheckboxAccessory->GetCheckStatus() )
						{
							pCDBCheckboxAccessory->SetCheckStatus( false );
						}
				
						// And we must set it as disable. Because it's a common accessory but not available with the current selected actuator.
						pCDBCheckboxAccessory->SetEnableStatus( false );
						pCDBCheckboxAccessory->ApplyInternalChange();
					}
					else
					{
						// If 'bCheck = false', there is not yet actuator selected. So, it's not possible for the moment to set the status of the common accessories.
						// This is why we enable all the common accessories for the moment.
						if( false == pCDBCheckboxAccessory->GetEnableStatus() )
						{
							pCDBCheckboxAccessory->SetEnableStatus( true );
						}
					
						pCDBCheckboxAccessory->ApplyInternalChange();
					}
				}
			}
		}
	}
}

void CRViewSSelCtrl::GrayOtherExcludedOrUncompatibleProduct( CDB_Product *pclAccessory, bool bCheck )
{
	// Current control valve selected which has as accessory the stem heater and the adapter in its adapter list.
	CDB_ControlValve *pclCurrentControlValveSelected = GetCurrentControlValveSelected();

	if( NULL == pclCurrentControlValveSelected )
	{
		return;
	}

	// First, verify if it's well an adapter.
	if( NULL == pclAccessory || false == pclAccessory->IsAnAccessory() )
	{
		return;
	}

	// Verify if the accessory is a crossing accessory (an accessory that must be in two different tables of two different product to be considered).
	// For example the stem heater is displayed only if it is in CV accessory table and Actuator accessory table.
	CDB_RuledTable *pclRuledTableCtrlValve = (CDB_RuledTable *)(pclCurrentControlValveSelected->GetAccessoriesGroupIDPtr().MP);

	if( false == pclRuledTableCtrlValve->IsACrossingAccDisplayed( pclAccessory ) )
	{
		return;
	}

	// A map that listed products to be excluded when the CV accessory is checked. Key is adapter ID and value is actuator ID.
	std::map<LPCTSTR, LPCTSTR> mapExcluded;

	// Get Excluded products for the selected accessory
	std::vector<CData *> vectCtrlAccExcl;
	pclRuledTableCtrlValve->GetExcluded( pclAccessory, &vectCtrlAccExcl );

	// Retrieve sheet description of displayed actuators. We grayed all actuators not compatible with the selected CV accessory.
	CSheetDescription *pclSheetDescriptionActuator = m_ViewDescription.GetFromSheetDescriptionID( SD_Actuator );

	if( NULL == pclSheetDescriptionActuator || NULL == pclSheetDescriptionActuator->GetSSheetPointer() )
	{
		// No actuator selected, do not continue.
		return;
	}

	CSSheet *pclSSheet = pclSheetDescriptionActuator->GetSSheetPointer();

	bool bActuatorFit = false;
	long lFirstFocusColumn, lLastFocusColumn;
	pclSheetDescriptionActuator->GetFocusColumnRange( lFirstFocusColumn, lLastFocusColumn );


	// Retrieve current actuator selected if exist.
	CDB_Actuator *pclCurrentActuatorSelected = GetCurrentActuatorSelected();

	// Run all registered products. Loop on actuators to gray out when it's not compatible with the crossing accessory.
	// Compatible adapter is also checked to see if it will be excluded if the accessory is checked.
	CSheetDescription::vecCellDescription vecProductList;
	pclSheetDescriptionActuator->GetCellDescriptionList( vecProductList, RVSCellDescription::CD_Product );

	for( CSheetDescription::vecCellDescriptionIter vecIter = vecProductList.begin(); vecIter != vecProductList.end(); vecIter++ )
	{
		// Retrieve info on current product.
		CCellDescriptionProduct *pCDProduct = dynamic_cast<CCellDescriptionProduct *>(*vecIter);

		if( NULL == pCDProduct || NULL == pCDProduct->GetProduct() )
		{
			continue;
		}

		// Test if it's well an actuator.
		CDB_Actuator *pclActuator = dynamic_cast<CDB_Actuator *>((CData *)pCDProduct->GetProduct());

		if( NULL == pclActuator )
		{
			continue;
		}

		bActuatorFit = false;
		long lActuatorRow = pCDProduct->GetCellPosition().y;

		// Look if the actuator is compatible by looking for crossing accessory.

		CDB_RuledTable *pclRuledTableActuator = NULL;
		pclRuledTableActuator = (CDB_RuledTable *)(pclActuator->GetAccessoriesGroupIDPtr().MP);

		if( NULL != pclRuledTableActuator )
		{
			// Find shared accessories between the two cross tables.
			for( IDPTR IDPtr = pclRuledTableActuator->GetFirst(); NULL != IDPtr.MP; IDPtr = pclRuledTableActuator->GetNext() )
			{
				if( (false == bCheck) || (true == pclRuledTableActuator->IsACrossingAcc( IDPtr.MP ) && 0 == IDcmp( IDPtr.ID, pclAccessory->GetIDPtr().ID )) )
				{
					bActuatorFit = true;
					break;
				}
			}

			if( false == bActuatorFit )
			{
				if( true == bCheck )
				{
					// Gray out the actuator line 
					pclSSheet->UnSelectMultipleRows( lActuatorRow );
					pclSSheet->SelectOneRow( lActuatorRow, lFirstFocusColumn, lLastFocusColumn, SPREAD_COLOR_IGNORE, _DARKGRAY );
				}
				else
				{
					// Accessory is not selected so enable the actuator line
					if( (NULL == pclCurrentActuatorSelected || 0 != _tcscmp( pclCurrentActuatorSelected->GetIDPtr().ID, pclActuator->GetIDPtr().ID )) )
					{
						pclSSheet->UnSelectMultipleRows( lActuatorRow );
						// One actuator fit always if no selection have been made.
					}
				}
			}
			else
			{
				// Actuator fit so enable the actuator line
				if( (NULL == pclCurrentActuatorSelected || 0 != _tcscmp( pclCurrentActuatorSelected->GetIDPtr().ID, pclActuator->GetIDPtr().ID )) )
				{
					pclSSheet->UnSelectMultipleRows( lActuatorRow );
				}
			}
		}

		// Check the adapter status for the current actuator

		CDB_Product *pclAdapter = pclCurrentControlValveSelected->GetMatchingAdapter( pclActuator, false );

		if( true == bActuatorFit && NULL != pclAdapter )
		{
			// Look if the adapter must be exclude due to the ctrl valve accessory. 
			if( true == bCheck )
			{
				CDB_RuledTable *pRTadaptActr = dynamic_cast<CDB_RuledTable *>(pclActuator->GetActAdapterGroupIDPtr().MP);
				if( NULL != pRTadaptActr )
				{
					// Get Excluded accessory.
					std::vector<CData *> vectAdaptExcl;
					pRTadaptActr->GetExcluded( pclAdapter, &vectAdaptExcl );

					bool bFoudExcluded = false;

					for( auto &itAdpt : vectAdaptExcl )
					{
						CDB_Product * pclExcludedProd = dynamic_cast<CDB_Product *>(itAdpt);

						// Actuator adapter ruled table has exclude the accessory (stem heater)
						if( NULL != pclExcludedProd && 0 == IDcmp( pclExcludedProd->GetIDPtr().ID, pclAccessory->GetIDPtr().ID ) )
						{
							// Look if it is the same for CV accessory rules table. 
							for( auto &itCvAcc : vectCtrlAccExcl )
							{
								CDB_Product * pclExcludedAdap = dynamic_cast<CDB_Product *>(itCvAcc);

								if( NULL != pclExcludedAdap && 0 == IDcmp( pclExcludedAdap->GetIDPtr().ID, pclAdapter->GetIDPtr().ID ) )
								{
									// Keep the adapter to disable in the map.
									mapExcluded.insert( std::pair<LPCTSTR, LPCTSTR>( pclAdapter->GetIDPtr().ID, pclActuator->GetIDPtr().ID ) );
									bFoudExcluded = true;
									break;
								}
							}

						}

						if( true == bFoudExcluded )
						{
							break;
						}
					}
				}
			}
			else
			{
				// Enable adapter
				EnableCheckboxAccessories( _T( "" ), &m_vecControlValveAdapter );

				if( NULL != pclCurrentActuatorSelected && pclCurrentActuatorSelected == pclActuator )
				{
					// Set adapter according to the actuator.
					GrayUncompatibleAccessoriesCheckbox( pclAdapter, false );
					_SelectActuatorAdapter();
				}
			}
		}
	}

	// Retrieve sheet description of displayed apdater.
	CSheetDescription *pclSheetDescriptionAdapter = m_ViewDescription.GetFromSheetDescriptionID( SD_ControlValveAdapter );

	if( NULL == pclSheetDescriptionAdapter || NULL == pclSheetDescriptionAdapter->GetSSheetPointer() )
	{
		return;
	}

	vecCDCAccessoryList vecAdapterList;
	CCellDescription *pclLoopCellDescription = pclSheetDescriptionAdapter->GetFirstCellDescription( RVSCellDescription::CDBC_Accessories );

	while( NULL != pclLoopCellDescription )
	{
		vecAdapterList.push_back( (CCDBCheckboxAccessory*)pclLoopCellDescription );
		pclLoopCellDescription = pclSheetDescriptionAdapter->GetNextCellDescription( pclLoopCellDescription, RVSCellDescription::CDBC_Accessories );
	}

	// There is adapter.
	if( 0 < vecAdapterList.size() )
	{
		for( auto &vecIter : vecAdapterList )
		{
			CCDBCheckboxAccessory *pCheckboxAccessoryInList = vecIter;

			if( NULL == pCheckboxAccessoryInList )
			{
				continue;
			}

			CDB_Product *pclProductInSheet = pCheckboxAccessoryInList->GetAccessoryPointer();


			if( NULL == pclProductInSheet )
			{
				continue;
			}

			// Verify if the displayed adapter is in the map for excluded products.

			for( std::map<LPCTSTR, LPCTSTR>::iterator iterMap = mapExcluded.begin(); iterMap != mapExcluded.end(); ++iterMap )
			{
				CDB_Product *pclAdapter = dynamic_cast<CDB_Product *>((CData *)TASApp.GetpTADB()->Get( iterMap->first ).MP);

				if( NULL == pclAdapter )
				{
					continue;
				}

				// CV accessory is checked, uncheck the adapter and disable it.
				if( 0 == IDcmp( pclAdapter->GetIDPtr().ID, pclProductInSheet->GetIDPtr().ID ) )
				{
					VerifyCheckboxAccessories( pclAdapter, false, &vecAdapterList, true );
					DisableCheckboxAccessories( _T( "" ), &vecAdapterList );
				}
			}
		}
	}
}

bool CRViewSSelCtrl::ResetColumnWidth( short nSheetDescriptionID )
{
	TSpread clTSpread;

	if( FALSE == clTSpread.Create( WS_CHILD, CRect( 0, 0, 0, 0 ), this, 0 ) )
	{
		ASSERT( 0 );
		return false;
	}

	switch( nSheetDescriptionID )
	{
		case SD_ControlValve:
			m_mapSSheetColumnWidth[SD_ControlValve][CD_ControlValve_FirstColumn] = clTSpread.ColWidthToLogUnits( 4 );
			m_mapSSheetColumnWidth[SD_ControlValve][CD_ControlValve_Box] = clTSpread.ColWidthToLogUnits( 2 );
			m_mapSSheetColumnWidth[SD_ControlValve][CD_ControlValve_CheckBox] = clTSpread.ColWidthToLogUnits( 2 );
			m_mapSSheetColumnWidth[SD_ControlValve][CD_ControlValve_Name] = clTSpread.ColWidthToLogUnits( 16 );
			m_mapSSheetColumnWidth[SD_ControlValve][CD_ControlValve_Material] = clTSpread.ColWidthToLogUnits( 12 );
			m_mapSSheetColumnWidth[SD_ControlValve][CD_ControlValve_Connection] = clTSpread.ColWidthToLogUnits( 14 );
			m_mapSSheetColumnWidth[SD_ControlValve][CD_ControlValve_Version] = clTSpread.ColWidthToLogUnits( 12 );
			m_mapSSheetColumnWidth[SD_ControlValve][CD_ControlValve_Size] = clTSpread.ColWidthToLogUnits( 10 );
			m_mapSSheetColumnWidth[SD_ControlValve][CD_ControlValve_PN] = clTSpread.ColWidthToLogUnits( 6 );
			m_mapSSheetColumnWidth[SD_ControlValve][CD_ControlValve_Rangeability] = clTSpread.ColWidthToLogUnits( 8 );
			m_mapSSheetColumnWidth[SD_ControlValve][CD_ControlValve_LeakageRate] = clTSpread.ColWidthToLogUnits( 6 );
			m_mapSSheetColumnWidth[SD_ControlValve][CD_ControlValve_Stroke] = clTSpread.ColWidthToLogUnits( 4 );
			m_mapSSheetColumnWidth[SD_ControlValve][CD_ControlValve_ImgCharacteristic] = clTSpread.ColWidthToLogUnits( 2 );
			m_mapSSheetColumnWidth[SD_ControlValve][CD_ControlValve_ImgSeparator] = clTSpread.ColWidthToLogUnits( 2 );
			m_mapSSheetColumnWidth[SD_ControlValve][CD_ControlValve_ImgPushClose] = clTSpread.ColWidthToLogUnits( 2 );
			m_mapSSheetColumnWidth[SD_ControlValve][CD_ControlValve_Kvs] = clTSpread.ColWidthToLogUnits( 6 );
			m_mapSSheetColumnWidth[SD_ControlValve][CD_ControlValve_Preset] = clTSpread.ColWidthToLogUnits( 6 );
			m_mapSSheetColumnWidth[SD_ControlValve][CD_ControlValve_Dp] = clTSpread.ColWidthToLogUnits( 6 );
			m_mapSSheetColumnWidth[SD_ControlValve][CD_ControlValve_DpFullOpening] = clTSpread.ColWidthToLogUnits( 6 );
			m_mapSSheetColumnWidth[SD_ControlValve][CD_ControlValve_DpHalfOpening] = clTSpread.ColWidthToLogUnits( 6 );
			m_mapSSheetColumnWidth[SD_ControlValve][CD_ControlValve_DplRange] = clTSpread.ColWidthToLogUnits( 10 );
			m_mapSSheetColumnWidth[SD_ControlValve][CD_ControlValve_DpMax] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_ControlValve][CD_ControlValve_TemperatureRange] = clTSpread.ColWidthToLogUnits( 8 );
			m_mapSSheetColumnWidth[SD_ControlValve][CD_ControlValve_Separator] = clTSpread.ColWidthToLogUnits( 1 );
			m_mapSSheetColumnWidth[SD_ControlValve][CD_ControlValve_PipeSize] = clTSpread.ColWidthToLogUnits( 8 );
			m_mapSSheetColumnWidth[SD_ControlValve][CD_ControlValve_PipeLinDp] = clTSpread.ColWidthToLogUnits( 8 );
			m_mapSSheetColumnWidth[SD_ControlValve][CD_ControlValve_PipeV] = clTSpread.ColWidthToLogUnits( 8 );
			m_mapSSheetColumnWidth[SD_ControlValve][CD_ControlValve_Pointer] = clTSpread.ColWidthToLogUnits( 1 );
			break;

		case SD_Actuator:
			m_mapSSheetColumnWidth[SD_Actuator][CD_Actuator_FirstColumn] = clTSpread.ColWidthToLogUnits( 4 );
			m_mapSSheetColumnWidth[SD_Actuator][CD_Actuator_Box] = clTSpread.ColWidthToLogUnits( 2 );
			m_mapSSheetColumnWidth[SD_Actuator][CD_Actuator_CheckBox] = clTSpread.ColWidthToLogUnits( 2 );
			m_mapSSheetColumnWidth[SD_Actuator][CD_Actuator_Name] = clTSpread.ColWidthToLogUnits( 30 );
			m_mapSSheetColumnWidth[SD_Actuator][CD_Actuator_CloseOffValue] = clTSpread.ColWidthToLogUnits( 10 );
			m_mapSSheetColumnWidth[SD_Actuator][CD_Actuator_ActuatingTime] = clTSpread.ColWidthToLogUnits( 10 );
			m_mapSSheetColumnWidth[SD_Actuator][CD_Actuator_IP] = clTSpread.ColWidthToLogUnits( 10 );
			m_mapSSheetColumnWidth[SD_Actuator][CD_Actuator_PowSupply] = clTSpread.ColWidthToLogUnits( 20 );
			m_mapSSheetColumnWidth[SD_Actuator][CD_Actuator_InputSig] = clTSpread.ColWidthToLogUnits( 20 );
			m_mapSSheetColumnWidth[SD_Actuator][CD_Actuator_OutputSig] = clTSpread.ColWidthToLogUnits( 20 );
			m_mapSSheetColumnWidth[SD_Actuator][CD_Actuator_RelayType] = clTSpread.ColWidthToLogUnits( 15 );
			m_mapSSheetColumnWidth[SD_Actuator][CD_Actuator_FailSafe] = clTSpread.ColWidthToLogUnits( 10 );
			m_mapSSheetColumnWidth[SD_Actuator][CD_Actuator_DefaultReturnPos] = clTSpread.ColWidthToLogUnits( 15 );
			m_mapSSheetColumnWidth[SD_Actuator][CD_Actuator_MaxTemp] = clTSpread.ColWidthToLogUnits( 8 );
			m_mapSSheetColumnWidth[SD_Actuator][CD_Actuator_Pointer] = clTSpread.ColWidthToLogUnits( 1 );
			break;
	}

	if( clTSpread.GetSafeHwnd() != NULL )
	{
		clTSpread.DestroyWindow();
	}

	return true;
}

void CRViewSSelCtrl::UpdateTitleBackgroundColor( CSheetDescription *pclSheetDescription )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() )
	{
		ASSERT_RETURN;
	}

	if( SD_ControlValve == pclSheetDescription->GetSheetDescriptionID() )
	{
		COLORREF backgroundColor = GetTitleBackgroundColor( pclSheetDescription );
		pclSheetDescription->GetSSheetPointer()->SetBackColor( CD_ControlValve_CheckBox, RD_ControlValve_GroupName, backgroundColor );
		pclSheetDescription->SetUserVariable( _SDUV_TITLEBACKCOLOR, backgroundColor );
	}
	else if( SD_ControlValveAccessory == pclSheetDescription->GetSheetDescriptionID()
			|| SD_ControlValveAdapter == pclSheetDescription->GetSheetDescriptionID()
			|| SD_ActuatorAccessory == pclSheetDescription->GetSheetDescriptionID()
			|| SD_CtrlActuatorAccessorySet == pclSheetDescription->GetSheetDescriptionID() )
	{
		COLORREF backgroundColor = GetTitleBackgroundColor( pclSheetDescription );
		pclSheetDescription->GetSSheetPointer()->SetBackColor( CD_Accessory_Left, RD_Accessory_GroupName, backgroundColor );
		pclSheetDescription->SetUserVariable( _SDUV_TITLEBACKCOLOR, backgroundColor );
	}
	else if( SD_Actuator == pclSheetDescription->GetSheetDescriptionID() )
	{
		COLORREF backgroundColor = GetTitleBackgroundColor( pclSheetDescription );
		pclSheetDescription->GetSSheetPointer()->SetBackColor( CD_Actuator_CheckBox, RD_Actuator_GroupName, backgroundColor );
		pclSheetDescription->SetUserVariable( _SDUV_TITLEBACKCOLOR, backgroundColor );
	}
	else
	{
		CRViewSSelSS::UpdateTitleBackgroundColor( pclSheetDescription );
	}
}

bool CRViewSSelCtrl::OnClickProduct( CSheetDescription *pclSheetDescription, CCellDescriptionProduct *pclCellDescriptionProduct, long lColumn, long lRow )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() || NULL == pclCellDescriptionProduct )
	{
		ASSERTA_RETURN( false );
	}

	bool bNeedRefresh = true;
	CSSheet *pclSSheet = pclSheetDescription->GetSSheetPointer();
	pclSSheet->SetBool( SSB_REDRAW, FALSE );

	CDB_ControlValve *pControlValve = dynamic_cast<CDB_ControlValve *>( ( CData * )pclCellDescriptionProduct->GetProduct() );
	CDB_Actuator *pActuator = dynamic_cast<CDB_Actuator *>( ( CData * )pclCellDescriptionProduct->GetProduct() );

	if( NULL != pControlValve )
	{
		// User has clicked on a control valve.
		_ClickOnControlValve( pclSheetDescription, pControlValve, pclCellDescriptionProduct, lColumn, lRow );
	}
	else if( NULL != pActuator )
	{
		// User has clicked on an actuator.
		_ClickOnActuator( pclSheetDescription, pActuator, pclCellDescriptionProduct, lColumn, lRow );
	}
	else
	{
		bNeedRefresh = false;
	}

	pclSSheet->SetBool( SSB_REDRAW, TRUE );
	return bNeedRefresh;
}

long CRViewSSelCtrl::FillControlValveRows( CDB_ControlValve *pclControlValve )
{
	// Here we do only header initialization that is common for 'CRViewSSelBCV', 'CRViewSSelCv' and 'CRViewSSelPICv' inherited classes.

	if( NULL == m_pclIndSelCtrlParams || NULL == m_pclIndSelCtrlParams->GetSelectCtrlList() )
	{
		ASSERTA_RETURN( -1 );
	}

	// Call 'MultiSpreadBase' method to get a new SSheet.
	CSheetDescription *pclSheetDescriptionCv = CreateSSheet( SD_ControlValve );

	if( NULL == pclSheetDescriptionCv || NULL == pclSheetDescriptionCv->GetSSheetPointer() )
	{
		ASSERTA_RETURN( -1 );
	}

	CSSheet *pclSSheet = pclSheetDescriptionCv->GetSSheetPointer();

	pclSSheet->SetBool( SSB_REDRAW, FALSE );

	pclSSheet->Init();
	pclSSheet->SetBool( SSB_ALLOWDRAGDROP, FALSE );
	pclSSheet->SetBool( SSB_HORZSCROLLBAR, FALSE );
	pclSSheet->SetBool( SSB_VERTSCROLLBAR, FALSE );
	pclSSheet->SetBool( SSB_MOVEACTIVEONFOCUS, FALSE );
	pclSSheet->SetTextTip( SS_TT_STATUS_FLOATING, 250, NULL, _LIGHTYELLOW, _TAH_BLACK );

	// Set title.
	pclSSheet->SetMaxRows( RD_ControlValve_FirstAvailRow - 1 );

	// All cells are static by default and filled with _T("").
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	pclSSheet->FormatStaticText( -1, -1, -1, -1, _T( "" ) );

	// Increase row height.
	double dRowHeight = 12.75;
	pclSSheet->SetRowHeight( RD_ControlValve_FirstRow, dRowHeight * 0.5 );
	pclSSheet->SetRowHeight( RD_ControlValve_GroupName, dRowHeight * 1.5 );
	pclSSheet->SetRowHeight( RD_ControlValve_ColName, dRowHeight * 2 );
	pclSSheet->SetRowHeight( RD_ControlValve_Unit, dRowHeight * 1.2 );

	// Set columns.
	pclSheetDescriptionCv->Init( 1, pclSSheet->GetMaxRows(), 1, CSheetDescription::SD_ParameterMode_Mono );
	pclSheetDescriptionCv->AddColumnInPixels( CD_ControlValve_FirstColumn, m_mapSSheetColumnWidth[SD_ControlValve][CD_ControlValve_FirstColumn] );
	pclSheetDescriptionCv->AddColumnInPixels( CD_ControlValve_Box, m_mapSSheetColumnWidth[SD_ControlValve][CD_ControlValve_Box] );
	pclSheetDescriptionCv->AddColumnInPixels( CD_ControlValve_CheckBox, m_mapSSheetColumnWidth[SD_ControlValve][CD_ControlValve_CheckBox] );
	pclSheetDescriptionCv->AddColumnInPixels( CD_ControlValve_Name, m_mapSSheetColumnWidth[SD_ControlValve][CD_ControlValve_Name] );
	pclSheetDescriptionCv->AddColumnInPixels( CD_ControlValve_Material, m_mapSSheetColumnWidth[SD_ControlValve][CD_ControlValve_Material] );
	pclSheetDescriptionCv->AddColumnInPixels( CD_ControlValve_Connection, m_mapSSheetColumnWidth[SD_ControlValve][CD_ControlValve_Connection] );
	pclSheetDescriptionCv->AddColumnInPixels( CD_ControlValve_Version, m_mapSSheetColumnWidth[SD_ControlValve][CD_ControlValve_Version] );
	pclSheetDescriptionCv->AddColumnInPixels( CD_ControlValve_Size, m_mapSSheetColumnWidth[SD_ControlValve][CD_ControlValve_Size] );
	pclSheetDescriptionCv->AddColumnInPixels( CD_ControlValve_PN, m_mapSSheetColumnWidth[SD_ControlValve][CD_ControlValve_PN] );
	pclSheetDescriptionCv->AddColumnInPixels( CD_ControlValve_Rangeability, m_mapSSheetColumnWidth[SD_ControlValve][CD_ControlValve_Rangeability] );
	pclSheetDescriptionCv->AddColumnInPixels( CD_ControlValve_LeakageRate, m_mapSSheetColumnWidth[SD_ControlValve][CD_ControlValve_LeakageRate] );
	pclSheetDescriptionCv->AddColumnInPixels( CD_ControlValve_Stroke, m_mapSSheetColumnWidth[SD_ControlValve][CD_ControlValve_Stroke] );
	pclSheetDescriptionCv->AddColumnInPixels( CD_ControlValve_ImgCharacteristic, m_mapSSheetColumnWidth[SD_ControlValve][CD_ControlValve_ImgCharacteristic] );
	pclSheetDescriptionCv->AddColumnInPixels( CD_ControlValve_ImgSeparator, m_mapSSheetColumnWidth[SD_ControlValve][CD_ControlValve_ImgSeparator] );
	pclSheetDescriptionCv->AddColumnInPixels( CD_ControlValve_ImgPushClose, m_mapSSheetColumnWidth[SD_ControlValve][CD_ControlValve_ImgPushClose] );
	pclSheetDescriptionCv->AddColumnInPixels( CD_ControlValve_Kvs, m_mapSSheetColumnWidth[SD_ControlValve][CD_ControlValve_Kvs] );
	pclSheetDescriptionCv->AddColumnInPixels( CD_ControlValve_Preset, m_mapSSheetColumnWidth[SD_ControlValve][CD_ControlValve_Preset] );
	pclSheetDescriptionCv->AddColumnInPixels( CD_ControlValve_Dp, m_mapSSheetColumnWidth[SD_ControlValve][CD_ControlValve_Dp] );
	pclSheetDescriptionCv->AddColumnInPixels( CD_ControlValve_DpFullOpening, m_mapSSheetColumnWidth[SD_ControlValve][CD_ControlValve_DpFullOpening] );
	pclSheetDescriptionCv->AddColumnInPixels( CD_ControlValve_DpHalfOpening, m_mapSSheetColumnWidth[SD_ControlValve][CD_ControlValve_DpHalfOpening] );
	pclSheetDescriptionCv->AddColumnInPixels( CD_ControlValve_DplRange, m_mapSSheetColumnWidth[SD_ControlValve][CD_ControlValve_DplRange] );
	pclSheetDescriptionCv->AddColumnInPixels( CD_ControlValve_DpMax, m_mapSSheetColumnWidth[SD_ControlValve][CD_ControlValve_DpMax] );
	pclSheetDescriptionCv->AddColumnInPixels( CD_ControlValve_TemperatureRange, m_mapSSheetColumnWidth[SD_ControlValve][CD_ControlValve_TemperatureRange] );
	pclSheetDescriptionCv->AddColumnInPixels( CD_ControlValve_Separator, m_mapSSheetColumnWidth[SD_ControlValve][CD_ControlValve_Separator] );
	pclSheetDescriptionCv->AddColumnInPixels( CD_ControlValve_PipeSize, m_mapSSheetColumnWidth[SD_ControlValve][CD_ControlValve_PipeSize] );
	pclSheetDescriptionCv->AddColumnInPixels( CD_ControlValve_PipeLinDp, m_mapSSheetColumnWidth[SD_ControlValve][CD_ControlValve_PipeLinDp] );
	pclSheetDescriptionCv->AddColumnInPixels( CD_ControlValve_PipeV, m_mapSSheetColumnWidth[SD_ControlValve][CD_ControlValve_PipeV] );
	pclSheetDescriptionCv->AddColumnInPixels( CD_ControlValve_Pointer, m_mapSSheetColumnWidth[SD_ControlValve][CD_ControlValve_Pointer] );

	// Set in which column parameter must be saved.
	pclSheetDescriptionCv->AddParameterColumn( CD_ControlValve_Pointer );

	// Set the focus column.
	pclSheetDescriptionCv->SetActiveColumn( CD_ControlValve_Name );

	// Set range for selection.
	pclSheetDescriptionCv->SetFocusColumnRange( CD_ControlValve_CheckBox, CD_ControlValve_TemperatureRange );

	// Columns Title.
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, ( LPARAM )FALSE );

	pclSSheet->SetStaticText( CD_ControlValve_Name, RD_ControlValve_ColName, IDS_SSHEETSSEL_NAME );
	pclSSheet->SetStaticText( CD_ControlValve_Material, RD_ControlValve_ColName, IDS_SSHEETSSEL_MATERIAL );
	pclSSheet->SetStaticText( CD_ControlValve_Connection, RD_ControlValve_ColName, IDS_SSHEETSSEL_CONNECT );
	pclSSheet->SetStaticText( CD_ControlValve_Version, RD_ControlValve_ColName, IDS_SSHEETSSEL_VERSION );
	pclSSheet->SetStaticText( CD_ControlValve_Size, RD_ControlValve_ColName, IDS_SSHEETSSEL_SIZE );
	pclSSheet->SetStaticText( CD_ControlValve_PN, RD_ControlValve_ColName, IDS_SSHEETSSEL_PN );

	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();

	if( 0 == pUnitDB->GetDefaultUnitIndex( _C_KVCVCOEFF ) )
	{
		pclSSheet->SetStaticText( CD_ControlValve_Kvs, RD_ControlValve_ColName, IDS_SSHEETSSELCV_CVKVS );
	}
	else
	{
		pclSSheet->SetStaticText( CD_ControlValve_Kvs, RD_ControlValve_ColName, IDS_SSHEETSSELCV_CVCV );
	}

	pclSSheet->SetStaticText( CD_ControlValve_Preset, RD_ControlValve_ColName, IDS_SSHEETSSEL_PRESET );
	pclSSheet->SetStaticText( CD_ControlValve_Rangeability, RD_ControlValve_ColName, IDS_SSHEETSSELCV_RANGEABILITY );
	pclSSheet->SetStaticText( CD_ControlValve_LeakageRate, RD_ControlValve_ColName, IDS_SSHEETSSELCV_LEAKAGERATE );
	pclSSheet->SetStaticText( CD_ControlValve_Dp, RD_ControlValve_ColName, IDS_SSHEETSSEL_DP );
	pclSSheet->SetStaticText( CD_ControlValve_DpFullOpening, RD_ControlValve_ColName, IDS_SSHEETSSEL_DPFO );
	pclSSheet->SetStaticText( CD_ControlValve_DpHalfOpening, RD_ControlValve_ColName, IDS_SSHEETSSEL_DPHO );
	pclSSheet->SetStaticText( CD_ControlValve_DplRange, RD_ControlValve_ColName, IDS_SHEETHDR_DPLRANGE );
	pclSSheet->SetStaticText( CD_ControlValve_DpMax, RD_ControlValve_ColName, IDS_RVIEWSSELCTRL_DPMAX );
	pclSSheet->SetStaticText( CD_ControlValve_TemperatureRange, RD_ControlValve_ColName, IDS_SSHEETSSEL_COLTEMPRANGE );
	pclSSheet->SetStaticText( CD_ControlValve_Stroke, RD_ControlValve_ColName, IDS_SSHEETSSELCV_CVSTROKE );
	pclSSheet->SetStaticText( CD_ControlValve_Separator, RD_ControlValve_ColName, _T( "" ) );

	// Pipes
	pclSSheet->SetStaticText( CD_ControlValve_PipeSize, RD_ControlValve_ColName, IDS_SSHEETSSEL_PIPESIZE );
	pclSSheet->SetStaticText( CD_ControlValve_PipeLinDp, RD_ControlValve_ColName, IDS_SSHEETSSEL_PIPELINDP );
	pclSSheet->SetStaticText( CD_ControlValve_PipeV, RD_ControlValve_ColName, IDS_SSHEETSSEL_PIPEV );

	// Units
	pclSSheet->SetStaticText( CD_ControlValve_LeakageRate, RD_ControlValve_Unit, IDS_SSHEETSSELCV_LEAKAGERATEUNIT );
	pclSSheet->SetStaticText( CD_ControlValve_Dp, RD_ControlValve_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ) ).c_str() );
	pclSSheet->SetStaticText( CD_ControlValve_DpFullOpening, RD_ControlValve_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ) ).c_str() );
	pclSSheet->SetStaticText( CD_ControlValve_DpHalfOpening, RD_ControlValve_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ) ).c_str() );
	pclSSheet->SetStaticText( CD_ControlValve_TemperatureRange, RD_ControlValve_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_TEMPERATURE ) ).c_str() );
	pclSSheet->SetStaticText( CD_ControlValve_Preset, RD_ControlValve_Unit, IDS_SHEETHDR_TURNSPOS );

	// Units
	CSelectedValve *pclSelectedControlValve = m_pclIndSelCtrlParams->GetSelectCtrlList()->GetFirst<CSelectedValve>();

	if( NULL != pclSelectedControlValve )
	{
		CDB_ControlValve *pclControlValve = dynamic_cast<CDB_ControlValve *>( pclSelectedControlValve->GetpData() );

		if( NULL != pclControlValve )
		{
			CDB_CloseOffChar *pclCloseOffChar = ( CDB_CloseOffChar * )pclControlValve->GetCloseOffCharIDPtr().MP;

			if( NULL != pclCloseOffChar )
			{
				if( CDB_CloseOffChar::eOpenType::Linear == pclCloseOffChar->GetOpenType() )
				{
					pclSSheet->SetStaticText( CD_ControlValve_Stroke, RD_ControlValve_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_DIAMETER ) ).c_str() );
				}
				else	//Circular
				{
					pclSSheet->SetStaticText( CD_ControlValve_Stroke, RD_ControlValve_Unit, IDS_ANGULARDEGRE );
				}
			}
			else
			{
				pclSSheet->SetStaticText( CD_ControlValve_Stroke, RD_ControlValve_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_DIAMETER ) ).c_str() );
			}
		}
	}

	pclSSheet->SetStaticText( CD_ControlValve_DplRange, RD_ControlValve_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ) ).c_str() );
	pclSSheet->SetStaticText( CD_ControlValve_DpMax, RD_ControlValve_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ) ).c_str() );
	pclSSheet->SetStaticText( CD_ControlValve_PipeLinDp, RD_ControlValve_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_LINPRESSDROP ) ).c_str() );
	pclSSheet->SetStaticText( CD_ControlValve_PipeV, RD_ControlValve_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_VELOCITY ) ).c_str() );

	// Draw black line below column header.
	pclSSheet->SetCellBorder( CD_ControlValve_CheckBox, RD_ControlValve_Unit, CD_ControlValve_Separator - 1, RD_ControlValve_Unit, true, SS_BORDERTYPE_BOTTOM );
	pclSSheet->SetCellBorder( CD_ControlValve_PipeSize, RD_ControlValve_Unit, CD_ControlValve_Pointer - 1, RD_ControlValve_Unit, true, SS_BORDERTYPE_BOTTOM );

	// Set that there is no selection at now.
	SetCurrentControlValveSelected( NULL );

	// Returns 0 to tell that all is OK.
	return 0;
}

void CRViewSSelCtrl::SetCurrentControlValveSelected( CCellDescriptionProduct *pclCDCurrentControlValveSelected )
{
	// Try to retrieve sheet description linked to control valve.
	CSheetDescription *pclSheetDescriptionCv = m_ViewDescription.GetFromSheetDescriptionID( SD_ControlValve );

	if( NULL != pclSheetDescriptionCv )
	{
		pclSheetDescriptionCv->SetUserVariable( _SDUV_SELECTEDPRODUCT, ( LPARAM )pclCDCurrentControlValveSelected );
	}
}

CDB_Product *CRViewSSelCtrl::GetFirstAdapterSelected( void )
{
	CDB_Product *pclFirstAdapter = NULL;

	// Retrieve selected adapter and add it.
	m_vecAdapterIter = m_vecControlValveAdapter.begin();
	bool fFound = false;

	while( m_vecAdapterIter != m_vecControlValveAdapter.end() && false == fFound )
	{
		CCDBCheckboxAccessory *pCDBCheckboxAccessory = *m_vecAdapterIter;

		if( NULL != pCDBCheckboxAccessory && true == pCDBCheckboxAccessory->GetCheckStatus() && NULL != pCDBCheckboxAccessory->GetAccessoryPointer() )
		{
			pclFirstAdapter = pCDBCheckboxAccessory->GetAccessoryPointer();
			fFound = true;
		}

		m_vecAdapterIter++;
	}

	return pclFirstAdapter;
}

CDB_Product *CRViewSSelCtrl::GetNextAdapterSelected( void )
{
	CDB_Product *pclNextAdapter = NULL;

	// Retrieve selected adapter and add it.
	if( m_vecAdapterIter != m_vecControlValveAdapter.end() )
	{
		m_vecAdapterIter++;
	}

	bool fFound = false;

	while( m_vecAdapterIter != m_vecControlValveAdapter.end() && false == fFound )
	{
		CCDBCheckboxAccessory *pCDBCheckboxAccessory = *m_vecAdapterIter;

		if( NULL != pCDBCheckboxAccessory && true == pCDBCheckboxAccessory->GetCheckStatus() && NULL != pCDBCheckboxAccessory->GetAccessoryPointer() )
		{
			pclNextAdapter = pCDBCheckboxAccessory->GetAccessoryPointer();
			fFound = true;
		}

		m_vecAdapterIter++;
	}

	return pclNextAdapter;
}

void CRViewSSelCtrl::SetCurrentActuatorSelected( CCellDescriptionProduct *pclCDCurrentActuatorSelected )
{
	// Try to retrieve sheet description linked to actuator.
	CSheetDescription *pclSheetDescriptionActuator = m_ViewDescription.GetFromSheetDescriptionID( SD_Actuator );

	if( NULL != pclSheetDescriptionActuator )
	{
		pclSheetDescriptionActuator->SetUserVariable( _SDUV_SELECTEDPRODUCT, ( LPARAM )pclCDCurrentActuatorSelected );
	}
}

bool CRViewSSelCtrl::IsSelectionReady( void )
{
	bool bReady = false;
	CDB_ControlValve *pclSelectedControlValve = GetCurrentControlValveSelected();

	if( NULL != pclSelectedControlValve )
	{
		bReady = ( false == pclSelectedControlValve->IsDeleted() );

		if( NULL != m_pclIndSelCtrlParams && true == m_pclIndSelCtrlParams->m_bOnlyForSet && true == pclSelectedControlValve->IsPartOfaSet()
				&& NULL == GetCurrentActuatorSelected() )
		{
			bReady = false;
		}
	}

	return bReady;
}

void CRViewSSelCtrl::GetAdapterList( CDB_ControlValve *pclSelectedControlValve, CRank *pclAdapterList, CDB_RuledTableBase **ppclRuledTable )
{
	if( NULL == m_pclIndSelCtrlParams || NULL == m_pclIndSelCtrlParams->m_pTADB )
	{
		ASSERT_RETURN;
	}

	m_pclIndSelCtrlParams->m_pTADB->GetAdapterList( pclSelectedControlValve, pclAdapterList, ppclRuledTable, true, _T(""), _T(""), -1, CDB_ControlValve::drpfUndefined,
			CDB_ControlProperties::eCvNU, false, m_pclIndSelCtrlParams->m_eFilterSelection );
}

void CRViewSSelCtrl::GetActuatorAccessoryList( CDB_Actuator *pclSeletedActuator, CRank *pclActuatorAccessoryList, CDB_RuledTableBase **ppclRuledTable )
{
	if( NULL == m_pclIndSelCtrlParams || NULL == m_pclIndSelCtrlParams->m_pTADB )
	{
		ASSERT_RETURN;
	}

	if( NULL == pclSeletedActuator || NULL == pclActuatorAccessoryList || NULL == ppclRuledTable )
	{
		return;
	}

	pclActuatorAccessoryList->PurgeAll();
	*ppclRuledTable = ( CDB_RuledTable * )( pclSeletedActuator->GetAccessoriesGroupIDPtr().MP );

	if( NULL == *ppclRuledTable )
	{
		return;
	}

	m_pclIndSelCtrlParams->m_pTADB->GetAccessories( pclActuatorAccessoryList, *ppclRuledTable, m_pclIndSelCtrlParams->m_eFilterSelection );
}

void CRViewSSelCtrl::GetSetAccessoryList( CDB_ControlValve *pclSelectedControlValve, CDB_Actuator *pclSeletedActuator, CRank *pclSetAccessoryList,
		CDB_RuledTableBase **ppclRuledTable )
{
	if( NULL == m_pclIndSelCtrlParams || NULL == m_pclIndSelCtrlParams->m_pTADB )
	{
		ASSERT_RETURN;
	}

	if( NULL == pclSelectedControlValve || NULL == pclSeletedActuator || NULL == pclSetAccessoryList || NULL == ppclRuledTable )
	{
		return;
	}

	pclSetAccessoryList->PurgeAll();
	*ppclRuledTable = NULL;

	if( false == pclSelectedControlValve->IsPartOfaSet() )
	{
		return;
	}

	CTableSet *pActSetTab = pclSelectedControlValve->GetTableSet();
	ASSERT( NULL != pActSetTab );
	CDB_Set *pCvActSet = pActSetTab->FindCompatibleSet( pclSelectedControlValve->GetIDPtr().ID, pclSeletedActuator->GetIDPtr().ID );

	if( NULL != pCvActSet )
	{
		*ppclRuledTable = dynamic_cast<CDB_RuledTable *>( pCvActSet->GetAccGroupIDPtr().MP );

		if( NULL != *ppclRuledTable )
		{
			m_pclIndSelCtrlParams->m_pTADB->GetAccessories( pclSetAccessoryList, *ppclRuledTable, m_pclIndSelCtrlParams->m_eFilterSelection );
		}
	}
}

void CRViewSSelCtrl::RemoveSheetDescriptions( UINT uiFromSheetDescriptionID )
{
	CSheetDescription *pBottomSheetDescription;

	switch( uiFromSheetDescriptionID )
	{
		case SD_ControlValve:

			// Can remove all sheets when user remove selection on a control valve.
			m_ViewDescription.RemoveAllSheetAfter( SD_ControlValve );
			break;

		case SD_Actuator:

			// Remove all sheet after the current except if it's the sheet for adapter.
			pBottomSheetDescription = m_ViewDescription.GetBottomSheetDescription();

			while( NULL != pBottomSheetDescription && SD_Actuator != pBottomSheetDescription->GetSheetDescriptionID() )
			{
				if( pBottomSheetDescription->GetSheetDescriptionID() >= SD_ControlValveLast ||
					SD_ControlValveAdapter == pBottomSheetDescription->GetSheetDescriptionID() )
				{
					// Don't touch to sheets that are managed by inherited classes ( ID >= SD_ControlValveLast) and
					// don't remove to the adapters.
					pBottomSheetDescription = m_ViewDescription.GetPrevSheetDescription( pBottomSheetDescription );
				}
				else
				{
					m_ViewDescription.RemoveOneSheetDescription( pBottomSheetDescription->GetSheetDescriptionID() );
					pBottomSheetDescription = m_ViewDescription.GetBottomSheetDescription();
				}
			}

			break;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PRIVATE MEMBERS
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CRViewSSelCtrl::_ClickOnControlValve( CSheetDescription *pclSheetDescriptionCtrl, CDB_ControlValve *pControlValveClicked,
		CCellDescriptionProduct *pclCellDescriptionProduct, long lColumn, long lRow )
{
	if( NULL == m_pclIndSelCtrlParams || NULL == m_pclIndSelCtrlParams->GetSelectCtrlList() )
	{
		ASSERT_RETURN;
	}

	CSSheet *pclSSheet = pclSheetDescriptionCtrl->GetSSheetPointer();

	m_lActuatorSelectedRow = 0;
	m_pCDBExpandCollapseGroupCtrlAcc = NULL;
	m_pCDBExpandCollapseGroupAdapter = NULL;
	m_pCDBExpandCollapseRowsActuator = NULL;
	m_pCDBExpandCollapseGroupActuatorAcc = NULL;
	m_pCDBExpandCollapseGroupCVSetAcc = NULL;
	m_vecControlValveAccessories.clear();
	m_vecControlValveAdapter.clear();
	m_vecActuatorAccessories.clear();
	m_vecCtrlActuatorSetAccessories.clear();

	SetCurrentActuatorSelected( NULL );
	m_pCurrentAdapter = NULL;

	// Memorize previous focus row.
	long lCurrentFocusedRow = GetCurrentFocusedRow();

	// To be sure to clean all previous data concerning focus.
	KillCurrentFocus();

	// Unselect row where current selection is set (yellow light).
	// Remark: 'KillCurrentFocus' unselect only rows where focus is on (blue light).
	pclSSheet->UnSelectMultipleRows();

	LPARAM lControlValveCount;
	pclSheetDescriptionCtrl->GetUserVariable( _SDUV_TOTALPRODUCT_COUNT, lControlValveCount );

	// Retrieve the current selected control valve if exist.
	CDB_ControlValve *pclCurrentControlValveSelected = GetCurrentControlValveSelected();

	// If there is already one control valve selected and user click on the current one...
	// Remark: 'm_pCDBExpandCollapseRowsCtrl' is not created if there is only one control valve. Thus we need to check first if there is only one control valve.
	//         Otherwise we have to check if user has clicked on the current one.
	if( NULL != pclCurrentControlValveSelected && ( ( 1 == lControlValveCount ) 
			|| ( NULL != m_pCDBExpandCollapseRowsCtrl && lRow == m_pCDBExpandCollapseRowsCtrl->GetCellPosition().y ) ) )
	{
		// Change focus state (selected to normal) and delete Expand/Collapse rows button.
		// If control valve is part of a set, change box button to open state.
		if( true == pclCurrentControlValveSelected->IsPartOfaSet() )
		{
			CCDButtonBox *pCDButtonBox = GetCDButtonBox( CD_ControlValve_Box, lRow, pclSheetDescriptionCtrl );

			if( NULL != pCDButtonBox )
			{
				pCDButtonBox->SetButtonState( CCDButtonBox::ButtonState::BoxOpened,  true );
			}
		}

		// Reset current product selected.
		SetCurrentControlValveSelected( NULL );

		// Uncheck checkbox.
		pclSSheet->SetCheckBox( CD_ControlValve_CheckBox, lRow, _T(""), false, true );

		// Delete Expand/Collapse rows button if exist.
		// Remark: 'm_pCDBExpandCollapseRowsCtrl' is set to NULL in method!
		DeleteExpandCollapseRowsButton( m_pCDBExpandCollapseRowsCtrl, pclSheetDescriptionCtrl );

		// Set focus on control valve currently selected.
		PrepareAndSetNewFocus( pclSheetDescriptionCtrl, pclSheetDescriptionCtrl->GetActiveColumn(), lRow, 0 );

		// Remove all sheets after control valve.
		RemoveSheetDescriptions( SD_ControlValve );

		m_lCtrlSelectedRow = 0;

		// Notify a change in the selection.
		SelectionHasChanged( IsSelectionReady() );
	}
	else
	{
		// If a control valve is already selected...
		if( NULL != pclCurrentControlValveSelected )
		{
			// Change box button to open state for previous control valve.
			if( true == pclCurrentControlValveSelected->IsPartOfaSet() )
			{
				CCDButtonBox *pCDButtonBox = GetCDButtonBox( CD_ControlValve_Box, m_lCtrlSelectedRow, pclSheetDescriptionCtrl );

				if( NULL != pCDButtonBox )
				{
					pCDButtonBox->SetButtonState( CCDButtonBox::ButtonState::BoxOpened, true );
				}
			}

			// Uncheck checkbox.
			pclSSheet->SetCheckBox(CD_ControlValve_CheckBox, m_lCtrlSelectedRow, _T(""), false, true);

			// Remove all sheets after control valve.
			RemoveSheetDescriptions( SD_ControlValve);
		}

		// Save new control valve selection.
		SetCurrentControlValveSelected( pclCellDescriptionProduct );

		// Delete Expand/Collapse rows button if exist.
		if( NULL != m_pCDBExpandCollapseRowsCtrl )
		{
			// Remark: 'm_pCDBExpandCollapseRows' is set to NULL in method!
			DeleteExpandCollapseRowsButton( m_pCDBExpandCollapseRowsCtrl, pclSheetDescriptionCtrl );
		}

		// Create Expand/Collapse rows button if needed...
		LPARAM lControlValveTotalCount;
		pclSheetDescriptionCtrl->GetUserVariable( _SDUV_TOTALPRODUCT_COUNT, lControlValveTotalCount );

		if( lControlValveTotalCount > 1 )
		{
			m_pCDBExpandCollapseRowsCtrl = CreateExpandCollapseRowsButton( CD_ControlValve_FirstColumn, lRow, true, 
					CCDButtonExpandCollapseRows::ButtonState::CollapseRow, pclSheetDescriptionCtrl->GetFirstSelectableRow(), 
					pclSheetDescriptionCtrl->GetLastSelectableRow( false ), pclSheetDescriptionCtrl );

			// Show button.
			if( NULL != m_pCDBExpandCollapseRowsCtrl )
			{
				m_pCDBExpandCollapseRowsCtrl->SetShowStatus( true );
			}
		}

		// Check the checkbox.
		pclSSheet->SetCheckBox( CD_ControlValve_CheckBox, lRow, _T(""), true, true );

		// Select control valve (just highlight background).
		pclSSheet->SelectOneRow( lRow, pclSheetDescriptionCtrl->GetSelectionFrom(), pclSheetDescriptionCtrl->GetSelectionTo() );

		// Fill accessories available for the current control valve.
		_FillCtrlAccessoryRows();

		// Fill actuators available for the current control valve.
		// Remark: this method MUST be called before '_FillCtrlAdapterRows' because this last uses 'm_fDowngradeActuatorFunctionality' that is
		//         set in '_FillCtrlActuatorRows'.
		_FillCtrlActuatorRows();

		// Fill adapters available for the current control valve.
		_FillCtrlAdapterRows();

		// Call inherited class to check if it has something to show.
		CSelectedValve *pclSelectedCtrl = GetSelectProduct<CSelectedValve>( pControlValveClicked, m_pclIndSelCtrlParams->GetSelectCtrlList() );
		FillOtherProducts( pclSelectedCtrl );

		bool bSetFocus = true;

		// Memorize control valve row selected.
		// Remark: needed if call to '_ClickOnActuator' is done below.
		m_lCtrlSelectedRow = lRow;

		// If user has selected Cv-Actuator set and we have only one actuator...
		CSheetDescription *pSheetDescriptionActuator = m_ViewDescription.GetFromSheetDescriptionID( SD_Actuator );

		if( true == m_pclIndSelCtrlParams->m_bOnlyForSet && true == pControlValveClicked->IsPartOfaSet() && NULL != pSheetDescriptionActuator )
		{
			// Verify first if we have only one actuator.
			LPARAM lActuatorTotalCount;
			pSheetDescriptionActuator->GetUserVariable( _SDUV_TOTALACTUATOR_COUNT, lActuatorTotalCount );

			if( 1 == lActuatorTotalCount )
			{
				// ... we can than automatically select actuator and close boxes in front of control valve.

				// Retrieve actuator.
				CCellDescriptionProduct *pclCDProduct = NULL;
				long lActuatorRow = pSheetDescriptionActuator->GetFirstSelectableRow();
				long lColParam = pSheetDescriptionActuator->GetFirstParameterColumn();
				LPARAM lProduct = GetCDProduct( lColParam, lActuatorRow, pSheetDescriptionActuator, &pclCDProduct );

				if( NULL != pclCDProduct )
				{
					// Simulate a click on the edited balancing valve.
					// Remark: the change of box state is done in this method.
					_ClickOnActuator( pSheetDescriptionActuator, dynamic_cast<CDB_Actuator *>( ( CData * )lProduct ), pclCDProduct, pSheetDescriptionActuator->GetSelectionFrom(),
									  lActuatorRow, false );
					bSetFocus = false;
				}
			}
		}

		if( true == bSetFocus )
		{
			// Try to set the focus on the next table but do not lose the focus if no other table exist.
			CSheetDescription *pclNextSheetDescription = NULL;
			long lNewFocusedRow;
			bool fShiftPressed;

			if( DNFERRC_FocusOnNextGroupOK == DefineNextFocusedRow( pclSheetDescriptionCtrl, CD_ControlValve_FirstColumn, lRow, false, lNewFocusedRow,
					pclNextSheetDescription, fShiftPressed ) )
			{
				long lNewFocusedCol = pclNextSheetDescription->GetFirstParameterColumn();
				PrepareAndSetNewFocus( pclNextSheetDescription, lNewFocusedCol, lNewFocusedRow );
			}
			else
			{
				// Because we have changed current selection background in yellow, if there is no other sheet following current one, we
				// must set focus on current selection.
				PrepareAndSetNewFocus( pclSheetDescriptionCtrl, lColumn, lRow );
			}
		}

		// Notify a change in the selection.
		SelectionHasChanged( IsSelectionReady() );
	}
}

void CRViewSSelCtrl::_FillCtrlActuatorRows()
{
	if( NULL == m_pclIndSelCtrlParams || NULL == m_pclIndSelCtrlParams->GetSelectCtrlList() )
	{
		ASSERT_RETURN;
	}

	CSelectedValve *pclSelectedValveObject = GetCurrentSelectedValveObject();

	if( NULL == pclSelectedValveObject )
	{
		return;
	}

	CDB_ControlValve *pclSelectedControlValve = dynamic_cast<CDB_ControlValve *>( pclSelectedValveObject->GetProductIDPtr().MP );

	if( NULL == pclSelectedControlValve )
	{
		return;
	}

	CRank clActuatorList;
	_GetActuatorList( pclSelectedValveObject, &clActuatorList );

	if( 0 == clActuatorList.GetCount() )
	{
		return;
	}

	CDB_CloseOffChar *pCloseOffChar = ( CDB_CloseOffChar * )( pclSelectedControlValve->GetCloseOffCharIDPtr().MP );

	// Call 'MultiSpreadBase' method to get a new SSheet.
	CSheetDescription *pclSheetDescriptionActuator = CreateSSheet( SD_Actuator );

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
	pclSSheet->SetMaxRows( RD_Actuator_FirstAvailRow - 1 );

	// All cells are static by default and filled with _T("")
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	pclSSheet->FormatStaticText( -1, -1, -1, -1, _T( "" ) );

	// Increase ROW height
	double dRowHeight = 12.75;

	pclSSheet->SetRowHeight( RD_Actuator_FirstRow, dRowHeight * 0.5 );
	pclSSheet->SetRowHeight( RD_Actuator_GroupName, dRowHeight * 1.5 );
	pclSSheet->SetRowHeight( RD_Actuator_ColName, dRowHeight * 1.2 );
	pclSSheet->SetRowHeight( RD_Actuator_Unit, dRowHeight * 1.2 );

	// Initialize.
	pclSheetDescriptionActuator->Init( 1, pclSSheet->GetMaxRows(), 1, CSheetDescription::SD_ParameterMode_Mono );
	pclSheetDescriptionActuator->AddColumnInPixels( CD_Actuator_FirstColumn, m_mapSSheetColumnWidth[SD_Actuator][CD_Actuator_FirstColumn] );
	pclSheetDescriptionActuator->AddColumnInPixels( CD_Actuator_Box, m_mapSSheetColumnWidth[SD_Actuator][CD_Actuator_Box] );
	pclSheetDescriptionActuator->AddColumnInPixels( CD_Actuator_CheckBox, m_mapSSheetColumnWidth[SD_Actuator][CD_Actuator_CheckBox] );
	pclSheetDescriptionActuator->AddColumnInPixels( CD_Actuator_Name, m_mapSSheetColumnWidth[SD_Actuator][CD_Actuator_Name] );
	pclSheetDescriptionActuator->AddColumnInPixels( CD_Actuator_CloseOffValue, m_mapSSheetColumnWidth[SD_Actuator][CD_Actuator_CloseOffValue] );
	pclSheetDescriptionActuator->AddColumnInPixels( CD_Actuator_ActuatingTime, m_mapSSheetColumnWidth[SD_Actuator][CD_Actuator_ActuatingTime] );
	pclSheetDescriptionActuator->AddColumnInPixels( CD_Actuator_IP, m_mapSSheetColumnWidth[SD_Actuator][CD_Actuator_IP] );
	pclSheetDescriptionActuator->AddColumnInPixels( CD_Actuator_PowSupply, m_mapSSheetColumnWidth[SD_Actuator][CD_Actuator_PowSupply] );
	pclSheetDescriptionActuator->AddColumnInPixels( CD_Actuator_InputSig, m_mapSSheetColumnWidth[SD_Actuator][CD_Actuator_InputSig] );
	pclSheetDescriptionActuator->AddColumnInPixels( CD_Actuator_OutputSig, m_mapSSheetColumnWidth[SD_Actuator][CD_Actuator_OutputSig] );
	pclSheetDescriptionActuator->AddColumnInPixels( CD_Actuator_RelayType, m_mapSSheetColumnWidth[SD_Actuator][CD_Actuator_RelayType] );
	pclSheetDescriptionActuator->AddColumnInPixels( CD_Actuator_FailSafe, m_mapSSheetColumnWidth[SD_Actuator][CD_Actuator_FailSafe] );
	pclSheetDescriptionActuator->AddColumnInPixels( CD_Actuator_DefaultReturnPos, m_mapSSheetColumnWidth[SD_Actuator][CD_Actuator_DefaultReturnPos] );
	pclSheetDescriptionActuator->AddColumnInPixels( CD_Actuator_MaxTemp, m_mapSSheetColumnWidth[SD_Actuator][CD_Actuator_MaxTemp] );
	pclSheetDescriptionActuator->AddColumnInPixels( CD_Actuator_Pointer, m_mapSSheetColumnWidth[SD_Actuator][CD_Actuator_Pointer] );

	// Show or hide box column.
	pclSSheet->ShowCol( CD_Actuator_Box, ( false == m_pclIndSelCtrlParams->m_bOnlyForSet || false == pclSelectedControlValve->IsPartOfaSet() ) ? FALSE : TRUE );

	// Set in which column parameter must be saved.
	pclSheetDescriptionActuator->AddParameterColumn( CD_Actuator_Pointer );

	// Set the focus column.
	pclSheetDescriptionActuator->SetActiveColumn( CD_Actuator_Name );

	// Set selectable rows.
	pclSheetDescriptionActuator->SetSelectableRangeRow( RD_Actuator_FirstAvailRow, pclSSheet->GetMaxRows() );

	// Set range for selection.
	pclSheetDescriptionActuator->SetFocusColumnRange( CD_Actuator_CheckBox, CD_Actuator_MaxTemp );

	// Add the main title.
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, ( LPARAM )GetTitleBackgroundColor( pclSheetDescriptionActuator ) );
	
	pclSheetDescriptionActuator->SetUserVariable( _SDUV_TITLEFORECOLOR,
			( pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor ) ) );
	
	pclSheetDescriptionActuator->SetUserVariable( _SDUV_TITLEBACKCOLOR,
			( pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor ) ) );

	pclSSheet->AddCellSpanW( CD_Actuator_CheckBox, RD_Actuator_GroupName, CD_Actuator_Pointer - CD_Actuator_CheckBox, 1 );
	pclSSheet->SetStaticText( CD_Actuator_CheckBox, RD_Actuator_GroupName, IDS_SSHEETSSELCV_ACTRGROUP );

	// Columns title.
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, ( LPARAM )FALSE );

	pclSSheet->SetStaticText( CD_Actuator_Name, RD_Actuator_ColName, IDS_SSHEETSSELCV_ACTRNAME );

	if( NULL != pCloseOffChar )
	{
		if( CDB_CloseOffChar::CloseOffDp == pCloseOffChar->GetLimitType() )
		{
			pclSSheet->SetStaticText( CD_Actuator_CloseOffValue, RD_Actuator_ColName, IDS_SSHEETSSELCV_ACTRCLOSEOFFDP );
		}
		else
		{
			pclSSheet->SetStaticText( CD_Actuator_CloseOffValue, RD_Actuator_ColName, IDS_SSHEETSSELCV_ACTRMAXINLETPRESS );
		}
	}
	else
	{
		pclSSheet->ShowCol( CD_Actuator_CloseOffValue, FALSE );
	}
	
	pclSSheet->SetStaticText( CD_Actuator_ActuatingTime, RD_Actuator_ColName, IDS_SSHEETSSELCV_ACTUATINGTIME );
	pclSSheet->SetStaticText( CD_Actuator_IP, RD_Actuator_ColName, IDS_SSHEETSSELCV_ACTRIP );
	pclSSheet->SetStaticText( CD_Actuator_PowSupply, RD_Actuator_ColName, IDS_SSHEETSSELCV_ACTRPOWERSUP );
	pclSSheet->SetStaticText( CD_Actuator_InputSig, RD_Actuator_ColName, IDS_SSHEETSSELCV_ACTRINPUTSIG );
	pclSSheet->SetStaticText( CD_Actuator_OutputSig, RD_Actuator_ColName, IDS_SSHEETSSELCV_ACTROUTPUTSIG );
	pclSSheet->SetStaticText( CD_Actuator_RelayType, RD_Actuator_ColName, IDS_SSHEETSSELCV_ACTRRELAYTYPE );
	pclSSheet->SetStaticText( CD_Actuator_FailSafe, RD_Actuator_ColName, IDS_SSHEETSSELCV_ACTRFAILSAFE );
	if( m_pclIndSelCtrlParams->m_iActuatorFailSafeFunction == 1 )
	{
		pclSSheet->SetStaticText( CD_Actuator_DefaultReturnPos, RD_Actuator_ColName, IDS_SSHEETSSEL_ACTRFSP );
	}
	else
	{
		pclSSheet->SetStaticText( CD_Actuator_DefaultReturnPos, RD_Actuator_ColName, IDS_SSHEETSSEL_ACTRDRP );
	}
	//HYS-726: To display max. temp. info for actuator
	pclSSheet->SetStaticText( CD_Actuator_MaxTemp, RD_Actuator_ColName, IDS_SSHEETSSELCV_ACTRMAXTEMP );
	pclSSheet->SetStaticText( CD_Actuator_MaxTemp, RD_Actuator_Unit, IDS_UNITDEGREECELSIUS );

	// Unit.
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();

	if( NULL != pCloseOffChar )
	{
		if( CDB_CloseOffChar::CloseOffDp == pCloseOffChar->GetLimitType() )
		{
			pclSSheet->SetStaticText( CD_Actuator_CloseOffValue, RD_Actuator_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ) ).c_str() );
		}
		else
		{
			pclSSheet->SetStaticText( CD_Actuator_CloseOffValue, RD_Actuator_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_PRESSURE ) ).c_str() );
		}
	}

	pclSSheet->SetStaticText( CD_Actuator_ActuatingTime, RD_Actuator_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_TIME ) ).c_str() );
	pclSSheet->SetCellBorder( CD_Actuator_CheckBox, RD_Actuator_Unit, CD_Actuator_Pointer - 1, RD_Actuator_Unit,
							  true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );

	// Retrieve Dp on current valve.
	double dDpOnCV = -1.0;
	CSelectedValve *pSelectedTAP = GetSelectProduct<CSelectedValve>( pclSelectedControlValve, m_pclIndSelCtrlParams->GetSelectCtrlList() );

	if( NULL != pSelectedTAP )
	{
		dDpOnCV = pSelectedTAP->GetDp();
	}

	// Fill actuator list.
	long lRow = RD_Actuator_FirstAvailRow;
	long lActuatorCount = 0;
	pclSheetDescriptionActuator->SetUserVariable( _SDUV_TOTALACTUATOR_COUNT, 0 );
	CString str;
	LPARAM lParam;

	// HYS-1519
	CIndSelPIBCVParams* pclPIBCVParam = dynamic_cast<CIndSelPIBCVParams*>( m_pclIndSelCtrlParams );
	// A map of compatible actuators with a closeOfDp >= Dpmax of the valve
	std::map<int, CDB_ElectroActuator*> mapCompatibleList;
	// A map of actuators which have a closeOfDp < Dpmax of the valve
	std::multimap<double, CDB_ElectroActuator*> mapCODpMaxIncompatibleList;
	// A map of all elements displayed int the right view
	std::map<int, CDB_ElectroActuator*> mapListDisplayed;
	int iNbelem = 0;
	// A vector of displayed CloseOffDp for displayed actuators
	std::vector<double> vectCloseOffDp;

	for( BOOL bContinue = clActuatorList.GetFirst( str, lParam ); TRUE == bContinue; bContinue = clActuatorList.GetNext( str, lParam ) )
	{
		CDB_ElectroActuator *pElectroActuator = dynamic_cast<CDB_ElectroActuator *>( ( CData * )lParam );

		if( NULL == pElectroActuator )
		{
			continue;
		}

		// Do not show actuator if max. inlet pressure is below 5 bar (Eric Bernadou 31-08-2015)
		if( NULL != pCloseOffChar && pCloseOffChar->GetLimitType() == CDB_CloseOffChar::InletPressure
				&& pCloseOffChar->GetMaxInletPressure( pElectroActuator->GetMaxForceTorque() ) < 500000 )
		{
			continue;
		}

		double dCloseOffDp = -1.0;

		// Check if the actuator force/torque is enough for the current Dp through the valve.
		if( NULL != pCloseOffChar )
		{
			if( CDB_CloseOffChar::CloseOffDp == pCloseOffChar->GetLimitType() )
			{
				bool bActuatorMatch = false;

				for( int iLoopMaxForceTorque = 0; iLoopMaxForceTorque < pElectroActuator->GetMaxForceTorqueNumber(); iLoopMaxForceTorque++ )
				{
					dCloseOffDp = pCloseOffChar->GetCloseOffDp( pElectroActuator->GetMaxForceTorque( iLoopMaxForceTorque ) );

					if( -1.0 == dCloseOffDp )
					{
						// If no close Dp available with this force, we continue.
						continue;
					}
					
					// HYS-1519 : We look with GetDp() if there is no DpMax in the left tab.
					if( NULL == pclPIBCVParam )
					{
						if( pSelectedTAP->GetDp() > 0.0 && dCloseOffDp < pSelectedTAP->GetDp() )
						{
							// If there is a Dp defined on the valve but actuator can't close it, we continue.
							continue;
						}

						if( pSelectedTAP->GetDp() <= 0.0 && pclSelectedControlValve->GetDpmax() > 0.0 && dCloseOffDp < pclSelectedControlValve->GetDpmax() )
						{
							// If there is no Dp on the valve but Dp max exist but actuator can't close at Dp max, we continue.
							continue;
						}
					}
					else
					{
						// HYS-1519 : We let the possibility to display the TA-Slider for the case where the Dp accros the valve is under
						// the DpMax and when the actuator could close it.
						if( pclPIBCVParam->m_dDpMax > 0.0 && dCloseOffDp < pclPIBCVParam->m_dDpMax )
						{
							// If there is a DpMax defined on the valve but actuator can't close it, we continue.
							continue;
						}
						if( NULL != pclPIBCVParam )
						{
							if( pclPIBCVParam->m_dDpMax <= 0.0 && pclSelectedControlValve->GetDpmax() > 0.0 && dCloseOffDp < pclSelectedControlValve->GetDpmax() )
							{
								if( iLoopMaxForceTorque == pElectroActuator->GetMaxForceTorqueNumber() - 1 )
								{
									mapCODpMaxIncompatibleList.insert( pair<double, CDB_ElectroActuator*>( dCloseOffDp, pElectroActuator ) );
								}
								continue;
							}
						}
					}
					bActuatorMatch = true;
					break;
				}

				if( false == bActuatorMatch )
				{
					continue;
				}
			}
			// HYS-1519 : We insert actuators to display at the top
			mapCompatibleList.insert( pair<int, CDB_ElectroActuator*>( iNbelem, pElectroActuator ) );
			mapListDisplayed.insert( pair<int, CDB_ElectroActuator*>( iNbelem, pElectroActuator ) );
			vectCloseOffDp.push_back( dCloseOffDp );
			iNbelem++;
		}
	}
	
	// HYS-1519 : We insert actuators to display at the end of the list
	map<double, CDB_ElectroActuator*>::iterator it;
	for( it = mapCODpMaxIncompatibleList.begin(); it != mapCODpMaxIncompatibleList.end(); it++ )
	{
		mapListDisplayed.insert( pair<int, CDB_ElectroActuator*>( iNbelem, it->second ) );
		vectCloseOffDp.push_back( it->first );
		iNbelem++;
	}

	map<int, CDB_ElectroActuator*>::iterator itmap;
	for( itmap = mapListDisplayed.begin(); itmap != mapListDisplayed.end(); itmap++ )
	{
		CDB_ElectroActuator* pElectroActuator = itmap->second;
		double dCloseOffDp = vectCloseOffDp.at( itmap->first );

		// Add this row.
		// 'true' to specify that this row can be selected.
		pclSheetDescriptionActuator->AddRows( 1, true );

		// If control valve is part of a set, actuator automatically belongs to the set (because 'GetActuator' above).
		if( true == m_pclIndSelCtrlParams->m_bOnlyForSet && true == pclSelectedControlValve->IsPartOfaSet() )
		{
			CCDButtonBox *pCDButtonBox = CreateCellDescriptionBox( CD_Actuator_Box, lRow, true, CCDButtonBox::ButtonState::BoxOpened, 
					pclSheetDescriptionActuator );

			if( NULL != pCDButtonBox )
			{
				pCDButtonBox->ApplyInternalChange();
			}
		}
		
		// Add checkbox.
		pclSSheet->SetCheckBox( CD_Actuator_CheckBox, lRow, _T(""), false, true );
		
		// HYS-726.
		if( m_pclIndSelCtrlParams->m_WC.GetTemp() > pElectroActuator->GetTmax() )
		{
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_RED );

			pclSSheet->SetStaticText( CD_Actuator_Name, lRow, pElectroActuator->GetName() );
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );
		}
		else
		{
			pclSSheet->SetStaticText( CD_Actuator_Name, lRow, pElectroActuator->GetName() );
		}

		// Set the close off value (Dp or inlet pressure depending of the selected control valve).
		if( NULL != pCloseOffChar )
		{
			if( CDB_CloseOffChar::CloseOffDp == pCloseOffChar->GetLimitType() )
			{
				if( -1.0 == dCloseOffDp )
				{
					str = _T("-");
				}
				else
				{
					// If there is a Dp on the control valve BUT Dp is above the close-off Dp of the actuator...
					if( pclSelectedControlValve->GetDpmax() > 0.0 && dCloseOffDp > pclSelectedControlValve->GetDpmax() )
					{
						pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_ORANGE );
					}
					
					// HYS-1519 : 
					if( NULL != pclPIBCVParam )
					{
						if( pclPIBCVParam->m_dDpMax <= 0.0 && pclSelectedControlValve->GetDpmax() > 0.0 && dCloseOffDp < pclSelectedControlValve->GetDpmax() )
						{
							pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_ORANGE );
						}
					}
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

			pclSSheet->SetStaticText( CD_Actuator_CloseOffValue, lRow, str );
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );
		}

		// Compute Actuating time.
		// Remark: actuating Time is given in s/m or in s/deg; stroke is given in m or in deg.
		pclSSheet->SetStaticText( CD_Actuator_ActuatingTime, lRow, pElectroActuator->GetActuatingTimesStr( pclSelectedControlValve->GetStroke() ) );

		CString str = pElectroActuator->GetIPxxFull();
		pclSSheet->SetStaticText( CD_Actuator_IP, lRow, str );

		CDB_StringID *pclPowerSupply = (CDB_StringID *)( m_pclIndSelCtrlParams->m_pTADB->Get( (LPCTSTR)m_pclIndSelCtrlParams->m_strActuatorPowerSupplyID ).MP );
		if( ( false == m_pclIndSelCtrlParams->m_strActuatorPowerSupplyID.IsEmpty() )
			&& ( false == pElectroActuator->IsPowerSupplyAvailable( pclPowerSupply->GetIDPtr() ) ) )
		{
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_ORANGE );
		}
		pclSSheet->SetStaticText( CD_Actuator_PowSupply, lRow, pElectroActuator->GetPowerSupplyStr() );
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

		str.Empty();
		CDB_StringID *pclInputSignal = (CDB_StringID *)( m_pclIndSelCtrlParams->m_pTADB->Get( (LPCTSTR)m_pclIndSelCtrlParams->m_strActuatorInputSignalID ).MP );
		if( true == m_bDowngradeActuatorFunctionality && ( false == m_pclIndSelCtrlParams->m_strActuatorInputSignalID.IsEmpty()
			&& ( false ==  pElectroActuator->IsInputSignalAvailable( pclInputSignal->GetIDPtr() ) ) ) )
		{
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, ( LPARAM )_ORANGE );
		}

		pclSSheet->SetStaticText( CD_Actuator_InputSig, lRow, pElectroActuator->GetInOutSignalsStr( true ) );
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, ( LPARAM )_BLACK );

		pclSSheet->SetStaticText( CD_Actuator_OutputSig, lRow, pElectroActuator->GetInOutSignalsStr( false ) );

		pclSSheet->SetStaticText( CD_Actuator_RelayType, lRow, pElectroActuator->GetRelayStr() );

		// Fail safe
		// HYS-1458 : If fail-safe checkbox is checked we have to get a fail-safe type. If the checkbox is not checked we have to get No fail-safe. 
		// if these conditions are not satisfied, fail-safe function is not fit.
		if( -1 != m_pclIndSelCtrlParams->m_iActuatorFailSafeFunction 
			&& ( ( pElectroActuator->GetFailSafe() >= CDB_ElectroActuator::FailSafeType::eFSTypeElectronic && pElectroActuator->GetFailSafe() < CDB_ElectroActuator::FailSafeType::eFSTypeLast
			&& m_pclIndSelCtrlParams->m_iActuatorFailSafeFunction != 1 )
			|| ( (int)pElectroActuator->GetFailSafe() == 0 && m_pclIndSelCtrlParams->m_iActuatorFailSafeFunction != 0 ) ) )
		{
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_ORANGE );
		}

		if( pElectroActuator->GetFailSafe() == CDB_ElectroActuator::FailSafeType::eFSTypeElectronic )
		{
			str = TASApp.LoadLocalizedString( IDS_STRELECFAILSAFE );
		}
		else if( pElectroActuator->GetFailSafe() == CDB_ElectroActuator::FailSafeType::eFSTypeSpringReturn )
		{
			str = TASApp.LoadLocalizedString( IDS_STRSPRINGFAILSAFE );
		}
		else
		{
			str = TASApp.LoadLocalizedString( IDS_NO );
		}
		pclSSheet->SetStaticText( CD_Actuator_FailSafe, lRow, str );
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );
		str.Empty();

		// Default return position.
		if( ( CDB_ControlValve::DRPFunction::drpfAll != m_pclIndSelCtrlParams->m_eActuatorDRPFunction )
			&& ( pclSelectedControlValve->GetCompatibleDRPFunction((int) pElectroActuator->GetDefaultReturnPos())
			     != m_pclIndSelCtrlParams->m_eActuatorDRPFunction ) )
		{
			if( CDB_ElectroActuator::DefaultReturnPosition::Configurable != pElectroActuator->GetDefaultReturnPos()
				|| ( CDB_ElectroActuator::DefaultReturnPosition::Configurable == pElectroActuator->GetDefaultReturnPos()
				&& CDB_ElectroActuator::DefaultReturnPosition::Extending != m_pclIndSelCtrlParams->m_eActuatorDRPFunction
				&& CDB_ElectroActuator::DefaultReturnPosition::Retracting != m_pclIndSelCtrlParams->m_eActuatorDRPFunction ) )
			{
				pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_ORANGE );
			}
		}
		pclSSheet->SetStaticText( CD_Actuator_DefaultReturnPos, lRow, CString( pElectroActuator->GetDefaultReturnPosStr( pElectroActuator->GetDefaultReturnPos() ).c_str() ) );
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );
		
		// HYS-726.
		CString sTmaxMedia = ( DBL_MAX == pElectroActuator->GetTmax() ) ? _T("-") : WriteDouble( pElectroActuator->GetTmax(), 3 );

		if( m_pclIndSelCtrlParams->m_WC.GetTemp() > pElectroActuator->GetTmax() )
		{
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_RED );
			pclSSheet->SetStaticText( CD_Actuator_MaxTemp, lRow, sTmaxMedia );
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );
		}
		else
		{
			pclSSheet->SetStaticText( CD_Actuator_MaxTemp, lRow, sTmaxMedia );
		}

		pclSSheet->SetCellBorder( CD_Actuator_CheckBox, lRow, CD_Actuator_Pointer - 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _GRAY );

		// Save parameter.
		CreateCellDescriptionProduct( pclSheetDescriptionActuator->GetFirstParameterColumn(), lRow, ( LPARAM )pElectroActuator, pclSheetDescriptionActuator );

		lRow++;
		lActuatorCount++;
	}

	long lLastDataRow = lRow - 1;

	// Save the total actuator number.
	pclSheetDescriptionActuator->SetUserVariable( _SDUV_TOTALACTUATOR_COUNT, lActuatorCount );

	// Set that there is no selection at now.
	SetCurrentActuatorSelected( NULL );
	
	// Add possibility to collapse / expand Actuator sheet
	CCDButtonExpandCollapseGroup **ppclGroupButton = NULL;
	ppclGroupButton = &m_pCDBExpandCollapseGroupActuator;
	
	if (NULL != ppclGroupButton)
	{
		*ppclGroupButton = CreateExpandCollapseGroupButton(CD_Actuator_FirstColumn, RD_Actuator_GroupName, true,
			CCDButtonExpandCollapseGroup::ButtonState::CollapseRow, pclSheetDescriptionActuator->GetFirstSelectableRow() - 2,
			pclSheetDescriptionActuator->GetLastSelectableRow(false), pclSheetDescriptionActuator );

		// Show button.
		if (NULL != *ppclGroupButton)
		{
			(*ppclGroupButton)->SetShowStatus( true );
		}
	}
	
	pclSSheet->AddCellSpan(CD_Actuator_FirstColumn, RD_Actuator_GroupName, 2, 1);

	pclSSheet->SetCellBorder( CD_Actuator_CheckBox, lLastDataRow, CD_Actuator_Pointer - 1, lLastDataRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );

	// Add possibility to change column size.
	// Remark: place this code after any initialization (like adding, removing, resizing, hiding/showing columns).
	CResizingColumnInfo *pclResizingColumnInfo = pclSheetDescriptionActuator->GetResizingColumnInfo();
	pclResizingColumnInfo->ActivateFeature( true, false, RD_Actuator_FirstAvailRow, lLastDataRow );
	pclResizingColumnInfo->AddRangeColumn( CD_Actuator_Name, CD_Actuator_MaxTemp, RD_Actuator_ColName, RD_Actuator_Unit );

	// Add possibility to click on title group to reset column width.
	CreateTitleGroupButtons( CD_Actuator_CheckBox, CD_Actuator_MaxTemp, RD_Actuator_GroupName, pclSheetDescriptionActuator );
}

void CRViewSSelCtrl::OnExpandCollapeGroupButtonClicked(CCDButtonExpandCollapseGroup *pCDBExpandCollapseGroup, CSheetDescription *pclSSheetDescription)
{
	if (NULL == pclSSheetDescription)
	{
		return;
	}

	UINT uiSheetDescriptionID = pclSSheetDescription->GetSheetDescriptionID();

	if (uiSheetDescriptionID != SD_Actuator)
	{
		CRViewSSelSS::OnExpandCollapeGroupButtonClicked(pCDBExpandCollapseGroup, pclSSheetDescription);
	}
	else
	{
		for (UINT uiLoopSheetID = SheetDescription::SD_ControlValveFirst; uiLoopSheetID < SheetDescription::SD_ControlValveLast; uiLoopSheetID++)
		{
			if (uiLoopSheetID == uiSheetDescriptionID)
			{
				continue;
			}
			if ((true == m_ViewDescription.IsSheetDescriptionExist(uiLoopSheetID) &&
				((uiLoopSheetID == SD_ActuatorAccessory) || (uiLoopSheetID == SD_CtrlActuatorAccessorySet))))
			{
				if (CCDButtonExpandCollapseGroup::ButtonState::ExpandRow == pCDBExpandCollapseGroup->GetButtonState())
				{
					HideSSheet(uiLoopSheetID);
				}
				else
				{
					ShowSSheet(uiLoopSheetID);
				}
			}
		}
		CRViewSSelSS::OnExpandCollapeGroupButtonClicked(pCDBExpandCollapseGroup, pclSSheetDescription);
	}
}

void CRViewSSelCtrl::_FillCtrlAccessoryRows()
{
	if( NULL == m_pclIndSelCtrlParams || NULL == m_pclIndSelCtrlParams->m_pTADB )
	{
		ASSERT_RETURN;
	}

	CDB_ControlValve *pclSelectedControlValve = GetCurrentControlValveSelected();

	if( NULL != pclSelectedControlValve )
	{
		CDB_RuledTable *pclRuledTable = ( CDB_RuledTable * )( pclSelectedControlValve->GetAccessoriesGroupIDPtr().MP );

		if( pclRuledTable != NULL )
		{
			CRank rList;
			m_pclIndSelCtrlParams->m_pTADB->GetAccessories( &rList, pclRuledTable, m_pclIndSelCtrlParams->m_eFilterSelection );
			_FillRowsAcc( IDS_SSHEETSSELCTRL_VALVEACCGROUP, true, &rList, pclRuledTable );
		}
	}
}

void CRViewSSelCtrl::_FillCtrlAdapterRows()
{
	CDB_ControlValve *pclSelectedControlValve = GetCurrentControlValveSelected();

	if( NULL != pclSelectedControlValve )
	{
		CRank rList;
		CDB_RuledTableBase *pclRuledTable = NULL;
		GetAdapterList( pclSelectedControlValve, &rList, &pclRuledTable );
		_FillRowsAcc( IDS_SSHEETSSELCTRL_VALVEADAPTERGROUP, true, &rList, pclRuledTable );
	}
}

void CRViewSSelCtrl::_ClickOnActuator( CSheetDescription *pclSheetDescriptionActuator, CDB_Actuator *pActuatorClicked,
		CCellDescriptionProduct *pclCellDescriptionProduct, long lColumn, long lRow, bool bExternalCall )
{
	if( NULL == m_pclIndSelCtrlParams )
	{
		ASSERT_RETURN;
	}

	// Check control valve selected.
	CDB_ControlValve *pclControlValveSelected = GetCurrentControlValveSelected();

	if( NULL == pclControlValveSelected )
	{
		ASSERT_RETURN;
	}

	CSSheet *pclSSheet = pclSheetDescriptionActuator->GetSSheetPointer();

	LPARAM lActuatorCount;
	pclSheetDescriptionActuator->GetUserVariable( _SDUV_TOTALACTUATOR_COUNT, lActuatorCount );

	// Do nothing if user wants to click on an actuator belonging to a set in which there is only ONE control valve.
	if( true == bExternalCall && true == m_pclIndSelCtrlParams->m_bOnlyForSet && true == pclControlValveSelected->IsPartOfaSet() && 1 ==  lActuatorCount )
	{
		return;
	}

	m_vecActuatorAccessories.clear();
	m_vecCtrlActuatorSetAccessories.clear();
	m_pCDBExpandCollapseGroupActuatorAcc = NULL;
	m_pCDBExpandCollapseGroupCVSetAcc = NULL;

	// Memorize previous focus row.
	long lCurrentFocusedRow = GetCurrentFocusedRow();

	// To be sure to clean all previous data concerning focus.
	KillCurrentFocus();

	// Unselect row where current selection is set (yellow light).
	// Remark: 'KillCurrentFocus' unselect only rows where focus is on (blue light).
	pclSSheet->UnSelectMultipleRows();

	// Retrieve the current selected actuator if exist.
	CDB_Actuator *pclCurrentActuatorSelected = GetCurrentActuatorSelected();

	// If there is already one actuator selected and user click on the current one...
	// Remark: 'm_pCDBExpandCollapseRowsActuator' is not created if there is only one actuator. Thus we need to check first if there is only one actuator.
	//         Otherwise we have to check if user has clicked on the current one.
	if( NULL != pclCurrentActuatorSelected && ( ( 1 == lActuatorCount ) || ( NULL != m_pCDBExpandCollapseRowsActuator
			&& lRow == m_pCDBExpandCollapseRowsActuator->GetCellPosition().y ) ) )
	{
		// Reset current product selected.
		SetCurrentActuatorSelected( NULL );

		// Delete Expand/Collapse rows button if exist.
		// Remark: 'm_pCDBExpandCollapseRowsActuator' is set to NULL in method!
		DeleteExpandCollapseRowsButton( m_pCDBExpandCollapseRowsActuator, pclSheetDescriptionActuator );

		// Remove all sheet after the current except if it's the sheet for adapter.
		RemoveSheetDescriptions( SD_Actuator );

		if( true == pclControlValveSelected->IsPartOfaSet() && true == m_pclIndSelCtrlParams->m_bOnlyForSet )
		{
			CSheetDescription *pclSheetDescriptionCtrl = m_ViewDescription.GetFromSheetDescriptionID( SD_ControlValve );

			if( NULL != pclSheetDescriptionCtrl )
			{
				// Change box button to open state for control valve.
				CCDButtonBox *pCDButtonBox = GetCDButtonBox( CD_ControlValve_Box, m_lCtrlSelectedRow, pclSheetDescriptionCtrl );

				if( NULL != pCDButtonBox )
				{
					pCDButtonBox->SetButtonState( CCDButtonBox::ButtonState::BoxOpened,  true );
				}
			}

			// Change box button to open state for actuator selected.
			CCDButtonBox *pCDButtonBox = GetCDButtonBox( CD_Actuator_Box, lRow, pclSheetDescriptionActuator );

			if( NULL != pCDButtonBox )
			{
				pCDButtonBox->SetButtonState( CCDButtonBox::ButtonState::BoxOpened,  true );
			}
		}

		// We force a refresh of the control valve adapter group (If exist) to be sure to have the right color in the
		// main title.
		if( true == m_ViewDescription.IsSheetDescriptionExist( SD_ControlValveAdapter ) )
		{
			KillCurrentFocus();
			m_pCDBExpandCollapseGroupAdapter = NULL;
			m_vecControlValveAdapter.clear();
			m_pCurrentAdapter = NULL;
			m_ViewDescription.RemoveOneSheetDescription( SD_ControlValveAdapter );
			_FillCtrlAdapterRows();
		}

		// Uncheck checkbox.
		pclSSheet->SetCheckBox(CD_Actuator_CheckBox, lRow, _T(""), false, true);
		
		// HYS-1108: Verify crossing accessory to enable it if disabled.
		if( true == m_ViewDescription.IsSheetDescriptionExist( SD_ControlValveAccessory ) )
		{
			GrayUncompatibleCrossingAccessories( GetCurrentActuatorSelected(), false );

			// HYS-2031: If a selected accessory has an adapter to exclude that was selected
			for( auto &itSelectedAcc : m_vecControlValveAccessories )
			{
				CCDBCheckboxAccessory *pCDBCheckboxAccessory = itSelectedAcc;

				if( NULL != pCDBCheckboxAccessory && true == pCDBCheckboxAccessory->GetCheckStatus() )
				{
					GrayOtherExcludedOrUncompatibleProduct( pCDBCheckboxAccessory->GetAccessoryPointer(), pCDBCheckboxAccessory->GetCheckStatus() );
				}
			}
		}

		// Set focus on actuator currently selected.
		PrepareAndSetNewFocus( pclSheetDescriptionActuator, pclSheetDescriptionActuator->GetActiveColumn(), lRow, 0 );

		m_lActuatorSelectedRow = 0;
		m_pCurrentAdapter = NULL;

		// Notify a change in the selection.
		SelectionHasChanged( IsSelectionReady() );
	}
	else
	{
		// If an actuator is already selected...
		if( NULL != pclCurrentActuatorSelected )
		{
			// Remove all sheet after the current except if it's the sheet for adapter.
			RemoveSheetDescriptions( SD_Actuator );

			// Change box button to open state for previous actuator.
			if( true == pclControlValveSelected->IsPartOfaSet() )
			{
				CCDButtonBox *pCDButtonBox = GetCDButtonBox( CD_Actuator_Box, m_lActuatorSelectedRow, pclSheetDescriptionActuator );

				if( NULL != pCDButtonBox )
				{
					pCDButtonBox->SetButtonState( CCDButtonBox::ButtonState::BoxOpened,  true );
				}
			}

			if( true == m_ViewDescription.IsSheetDescriptionExist( SD_ControlValveAdapter ) )
			{
				if( NULL != m_pCurrentAdapter )
				{
					// Unselect the previous adapter if exist.
					VerifyCheckboxAccessories( m_pCurrentAdapter, false, &m_vecControlValveAdapter, true );
					m_pCurrentAdapter = NULL;
				}
				else
				{
					// Particular case: user has selected an actuator but there is no compatible adapter. In this case, all adapter were disabled.
					//                  If user unselects the same actuator, we need to enable back all adapters.
					EnableCheckboxAccessories( _T( "" ), &m_vecControlValveAdapter );
				}
			}

			// Uncheck checkbox.
			pclSSheet->SetCheckBox(CD_Actuator_CheckBox, m_lActuatorSelectedRow, _T(""), false, true);
		}

		// Save new actuator selection.
		SetCurrentActuatorSelected( pclCellDescriptionProduct );

		// Delete Expand/Collapse rows button if exist.
		if( NULL != m_pCDBExpandCollapseRowsActuator )
		{
			// Remark: 'm_pCDBExpandCollapseRowsActuator' is set to NULL in method!
			DeleteExpandCollapseRowsButton( m_pCDBExpandCollapseRowsActuator, pclSheetDescriptionActuator );
		}

		// Create Expand/Collapse rows button if needed...
		if( lActuatorCount > 1 )
		{
			m_pCDBExpandCollapseRowsActuator = CreateExpandCollapseRowsButton( CD_Actuator_FirstColumn, lRow, true, 
					CCDButtonExpandCollapseRows::ButtonState::CollapseRow, pclSheetDescriptionActuator->GetFirstSelectableRow(), 
					pclSheetDescriptionActuator->GetLastSelectableRow( false ), pclSheetDescriptionActuator );

			// Show button.
			if( NULL != m_pCDBExpandCollapseRowsActuator )
			{
				m_pCDBExpandCollapseRowsActuator->SetShowStatus( true );
			}
		}

		// Check checkbox.
		pclSSheet->SetCheckBox( CD_Actuator_CheckBox, lRow, _T(""), true, true );

		// Fill accessories on actuator if exist.
		_FillActuatorAccessoryRows();

		// We force a refresh of the control valve adapter group (If exist) to be sure to have the right color in the
		// main title.
		if( true == m_ViewDescription.IsSheetDescriptionExist( SD_ControlValveAdapter ) )
		{
			KillCurrentFocus();
			m_pCDBExpandCollapseGroupAdapter = NULL;
			m_vecControlValveAdapter.clear();
			m_pCurrentAdapter = NULL;
			m_ViewDescription.RemoveOneSheetDescription( SD_ControlValveAdapter );
			_FillCtrlAdapterRows();
		}

		// HYS-1108: Verify crossing accessory to disable it if the selected actuator is not compatible.
		if( true == m_ViewDescription.IsSheetDescriptionExist( SD_ControlValveAccessory ) )
		{
			GrayUncompatibleCrossingAccessories( GetCurrentActuatorSelected(), true );
		}
		
		// Select the correct adapter that match with the current actuator.
		_SelectActuatorAdapter();

		// HYS-2031: Verify if a selected accessory has to exclude the adapter. In this case
		// the adapter must be unchecked and disabled.
		if( true == m_ViewDescription.IsSheetDescriptionExist( SD_ControlValveAccessory ) )
		{
			for( auto &itSelectedAcc : m_vecControlValveAccessories )
			{
				CCDBCheckboxAccessory *pCDBCheckboxAccessory = itSelectedAcc;
				if( NULL != pCDBCheckboxAccessory && true == pCDBCheckboxAccessory->GetCheckStatus() )
				{
					GrayOtherExcludedOrUncompatibleProduct( pCDBCheckboxAccessory->GetAccessoryPointer(), pCDBCheckboxAccessory->GetCheckStatus() );
				}
			}
		}

		// Select actuator (just highlight background).
		// Remark: this line must be placed after '_selectActuatorAdapter'. Why? In 'SelectOneRow' we save the back and fore color of the row before
		//         applying on it the highlighting. If current actuator was grayed (because current adapter selected doesn't match with this new actuator
		//         selected), 'SelectOneRow' saved gray for fore color of the text. When calling '_SelectActuatorAdapted', we verify the new adapter that will
		//         match with this new actuator selection. And then, some actuators in the list can become grayed and others can become black. If current actuator
		//         becomes black, the saved color are no more correct.
		pclSSheet->SelectOneRow( lRow, pclSheetDescriptionActuator->GetSelectionFrom(), pclSheetDescriptionActuator->GetSelectionTo() );

		// Try to set the focus on the next table but do not lose the focus if no other table exist.
		CSheetDescription *pclNextSheetDescription = NULL;
		long lNewFocusedRow;
		bool bShiftPressed;

		if( DNFERRC_FocusOnNextGroupOK == DefineNextFocusedRow( pclSheetDescriptionActuator, CD_Actuator_FirstColumn, lRow, false, lNewFocusedRow,
				pclNextSheetDescription, bShiftPressed ) )
		{
			long lNewFocusedCol = pclNextSheetDescription->GetFirstParameterColumn();
			PrepareAndSetNewFocus( pclNextSheetDescription, lNewFocusedCol, lNewFocusedRow );
		}
		else
		{
			// Because we have changed current selection background in yellow, if there is no other sheet following current one, we
			// must set focus on current selection.
			PrepareAndSetNewFocus( pclSheetDescriptionActuator, lColumn, lRow );
		}

		m_lActuatorSelectedRow = lRow;

		// If we are in selection by package mode and the current control valve and actuator belong to a set...
		if( true == m_pclIndSelCtrlParams->m_bOnlyForSet && true == pclControlValveSelected->IsPartOfaSet() )
		{
			CSheetDescription *pclSheetDescriptionCtrl = m_ViewDescription.GetFromSheetDescriptionID( SD_ControlValve );

			if( NULL != pclSheetDescriptionCtrl )
			{
				// Change box button to close state for control valve.
				CCDButtonBox *pCDButtonBox = GetCDButtonBox( CD_ControlValve_Box, m_lCtrlSelectedRow, pclSheetDescriptionCtrl );

				if( NULL != pCDButtonBox )
				{
					pCDButtonBox->SetButtonState( CCDButtonBox::ButtonState::BoxClosed,  true );
				}
			}

			// Change box button to close state for actuator selected.
			CCDButtonBox *pCDButtonBox = GetCDButtonBox( CD_Actuator_Box, lRow, pclSheetDescriptionActuator );

			if( NULL != pCDButtonBox )
			{
				pCDButtonBox->SetButtonState( CCDButtonBox::ButtonState::BoxClosed,  true );
			}
		}

		// Notify a change in the selection.
		SelectionHasChanged( IsSelectionReady() );
	}
}

void CRViewSSelCtrl::_FillActuatorAccessoryRows()
{
	if( NULL == m_pclIndSelCtrlParams )
	{
		ASSERT_RETURN;
	}

	// Check current control valve selected.
	CDB_ControlValve *pclCurrentControlValveSelected = GetCurrentControlValveSelected();

	if( NULL == pclCurrentControlValveSelected )
	{
		ASSERT_RETURN;
	}

	// Check current actuator selected.
	CDB_Actuator *pclCurrentActuatorSelected = GetCurrentActuatorSelected();

	if( NULL == pclCurrentActuatorSelected )
	{
		ASSERT_RETURN;
	}

	// Retrieve accessories belonging to the selected actuator.
	CRank rActuatorAccessoryList;
	CDB_RuledTableBase *pclRuledTable = NULL;
	GetActuatorAccessoryList( pclCurrentActuatorSelected, &rActuatorAccessoryList, &pclRuledTable );

	if( rActuatorAccessoryList.GetCount() > 0 )
	{
		_FillRowsAcc( IDS_SSHEETSSELCV_ACTRACCGROUP, false, &rActuatorAccessoryList, pclRuledTable );
	}

	// Add accessories for CvActrSet if needed.
	if( true == m_pclIndSelCtrlParams->m_bOnlyForSet )
	{
		CRank rSetAccessoryList;
		pclRuledTable = NULL;
		GetSetAccessoryList( pclCurrentControlValveSelected, pclCurrentActuatorSelected, &rSetAccessoryList, &pclRuledTable );

		if( rSetAccessoryList.GetCount() > 0 )
		{
			_FillRowsAcc( IDS_SSHEETSSELCV_ACCCVACTRSET, false, &rSetAccessoryList, pclRuledTable );
		}
	}
}

void CRViewSSelCtrl::_GetActuatorList( CSelectedValve *pclSelectedValveObject, CRank *pclActuatorList )
{
	if( NULL == m_pclIndSelCtrlParams || NULL == m_pclIndSelCtrlParams->m_pTADB || NULL == pclSelectedValveObject 
			|| NULL == dynamic_cast<CDB_ControlValve *>( pclSelectedValveObject->GetProductIDPtr().MP ) || NULL == pclActuatorList )
	{
		ASSERT_RETURN;
	}

	CDB_ControlValve *pclSelectedControlValve = (CDB_ControlValve *)( pclSelectedValveObject->GetProductIDPtr().MP );
	CDB_ValveCharacteristic *pclValveCharacteristic = dynamic_cast<CDB_ValveCharacteristic *>( pclSelectedControlValve->GetValveCharacteristic() );

	// HYS-1355 & HYS-1389: Need to filter actuators that are not TA-Slider when setting of the valve is below the min. measurable setting.
	CDB_PICVCharacteristic *pclPIBCValveCharacteristic = NULL;
	bool bIsStrokeCurveDefined = false;

	if( NULL != dynamic_cast<CDB_PIControlValve *>( pclSelectedControlValve ) )
	{
		CDB_PIControlValve *pclPIBCValve = (CDB_PIControlValve *)pclSelectedControlValve;
		pclPIBCValveCharacteristic = pclPIBCValve->GetPICVCharacteristic();

		if( NULL != pclPIBCValveCharacteristic && pclPIBCValveCharacteristic->IsStrokeCurveDefined() )
		{
			bIsStrokeCurveDefined = true;
		}
	}

	pclActuatorList->PurgeAll();

	if( false == m_pclIndSelCtrlParams->m_bOnlyForSet || false == pclSelectedControlValve->IsPartOfaSet() )
	{
		// Retrieve the actuator group on the selected control valve.
		CTable *pclActuatorGroup = ( CTable * )( pclSelectedControlValve->GetActuatorGroupIDPtr().MP );

		if( NULL == pclActuatorGroup )
		{
			return;
		}

		// Retrieve list of all actuators in this group.
		CRank rList( false );
		m_pclIndSelCtrlParams->m_pTADB->GetActuator( &rList, pclActuatorGroup, L"", L"", -1, CDB_ElectroActuator::DefaultReturnPosition::Undefined, m_pclIndSelCtrlParams->m_eFilterSelection );

		CString str;
		LPARAM lparam;
		int iPass = 0;

		do
		{
			// 0 - Do a first pass to select actuators that has the same input signal type as the control type that user wants.
			//     (On/Off with On/Off, 3 points with 3 points and proportional with proportional).
			// 1 - If not found, we accept functionality of the actuator to be downgraded AND we accept all fail safe mode.
			// 2 - If not found, we accept functionality of '3 points' and 'Proportional' actuators to work in the 'On/Off' and '3 points' modes.
			m_bDowngradeActuatorFunctionality = ( iPass < 2 ) ? false : true;
			bool bAcceptAllDefaultReturnPos = ( iPass > 0 ) ? true : false;

			for( BOOL bContinue = rList.GetFirst( str, lparam ); TRUE == bContinue; bContinue = rList.GetNext( str, lparam ) )
			{
				CDB_ElectroActuator *pclElectroActuator = dynamic_cast<CDB_ElectroActuator *>( ( CData * )lparam );

				if( NULL == pclElectroActuator )
				{
					continue;
				}

				// HYS-1355: First of all verify if setting of the valve is below the minimum measurable setting. 
				// If the current actuator has not the possibility to set a minimum limited stroke, we don't accept this actuator.
				// Remark: Below this setting, there is too much error with the TA-Scope to read flow. So, it’s no recommended to work 
				// with it to apply a perfect balancing. Except if we can put an actuator for which its lift is controlled by electronic 
				// (Like TA-Slider).
				if( NULL != pclValveCharacteristic && pclSelectedValveObject->GetH() < pclValveCharacteristic->GetMinMeasSetting()
						&& CDB_Actuator::LimitedStrokeMode::LSM_Electronic != pclElectroActuator->GetLimitedStrokeMode() )
				{
					continue;
				}

				// HYS-1389: Second test: if valve has stroke characteristic in function of the setting AND if the actuator has
				// a mechanical limited stroke, we verify if the minimal limited stroke of the actuator is enough for the setting
				// of the valve. Example of EMO-TM (1mm limited stroke) with TA-Compact-P DN 10 setting 3 (lift < 1mm).
				if( NULL != pclValveCharacteristic && true == bIsStrokeCurveDefined 
						&& CDB_Actuator::LimitedStrokeMode::LSM_No != pclElectroActuator->GetLimitedStrokeMode()
						&& -1.0 != pclElectroActuator->GetMinLimitedStroke()
						&& true == pclPIBCValveCharacteristic->StrokeCharGiven()
						&& pclSelectedValveObject->GetH() < pclPIBCValveCharacteristic->GetMinSettingWithActuator( pclElectroActuator ) )
				{
					continue;
				}

				if( true == IsActuatorFit( pclElectroActuator, pclSelectedControlValve, m_bDowngradeActuatorFunctionality, bAcceptAllDefaultReturnPos ) )
				{
					pclActuatorList->Add( pclElectroActuator->GetName(), pclElectroActuator->GetOrderKey(), ( LPARAM )pclElectroActuator, false );
				}
			}

			iPass++;
		}
		while( 0 == pclActuatorList->GetCount() && iPass < 3 );
	}
	else
	{
		// User has chosen selection by package. We have to list only actuators that belong to a package.
		// Remark: it is possible to have more than one package with the same BCV.
		std::set<CDB_Set *> BCVActSetArray;
		CTableSet *pBCVActTableSet = pclSelectedControlValve->GetTableSet();
		ASSERT( NULL != pBCVActTableSet );
		int iCDBSetCount = pBCVActTableSet->FindCompatibleSet( &BCVActSetArray, pclSelectedControlValve->GetIDPtr().ID, _T( "" ) );

		if( 0 == iCDBSetCount )
		{
			return;
		}

		int iPass = 0;

		do
		{
			// 1 - Do a first pass to select actuators that has the same input signal type as the control type that user wants.
			//     (On/Off with On/Off, 3 points with 3 points and proportional with proportional).
			// 2 - If not found, we accept functionality of the actuator to be downgraded AND we accept all fail safe mode.
			// 3 - If not found, we accept functionality of '3 points' and 'Proportional' actuators to work in the 'On/Off' and '3 points' modes.
			m_bDowngradeActuatorFunctionality = ( 2 > iPass ) ? false : true;
			bool bAcceptAllDefaultReturnPos = ( iPass > 0 ) ? true : false;

			for( std::set<CDB_Set *>::iterator iterCDBSet = BCVActSetArray.begin(); iterCDBSet != BCVActSetArray.end(); iterCDBSet++ )
			{
				CDB_Set *pBCVActSet = *iterCDBSet;
				IDPTR ActuatorIDPtr = pBCVActSet->GetSecondIDPtr();

				if( *ActuatorIDPtr.ID != _T('\0') )
				{
					CDB_ElectroActuator *pclElectroActuator = dynamic_cast<CDB_ElectroActuator *>( ActuatorIDPtr.MP );

					if( NULL == pclElectroActuator )
					{
						continue;
					}

					if( true == IsActuatorFit( pclElectroActuator, pclSelectedControlValve, m_bDowngradeActuatorFunctionality, bAcceptAllDefaultReturnPos ) )
					{
						pclActuatorList->Add( pclElectroActuator->GetName(), pclElectroActuator->GetOrderKey(), ( LPARAM )pclElectroActuator, false );
					}
				}
			}

			iPass++;
		}
		while( 0 == pclActuatorList->GetCount() && iPass < 3 );
	}
}

void CRViewSSelCtrl::_FillRowsAcc( int iTitleID, bool bForCtrl, CRank *pclList, CDB_RuledTableBase *pclRuledTable )
{
	if( NULL == m_pclIndSelCtrlParams )
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

	switch( iTitleID )
	{
		case IDS_SSHEETSSELCTRL_VALVEACCGROUP:
			iSSheetID = SD_ControlValveAccessory;
			pvecAccessories = &m_vecControlValveAccessories;
			ppclGroupButton = &m_pCDBExpandCollapseGroupCtrlAcc;
			break;

		case IDS_SSHEETSSELCTRL_VALVEADAPTERGROUP:
			iSSheetID = SD_ControlValveAdapter;
			pvecAccessories = &m_vecControlValveAdapter;
			ppclGroupButton = &m_pCDBExpandCollapseGroupAdapter;
			break;

		case IDS_SSHEETSSELCV_ACTRACCGROUP:
			iSSheetID = SD_ActuatorAccessory;
			pvecAccessories = &m_vecActuatorAccessories;
			ppclGroupButton = &m_pCDBExpandCollapseGroupActuatorAcc;
			break;

		case IDS_SSHEETSSELCV_ACCCVACTRSET:
			iSSheetID = SD_CtrlActuatorAccessorySet;
			pvecAccessories = &m_vecCtrlActuatorSetAccessories;
			ppclGroupButton = &m_pCDBExpandCollapseGroupCVSetAcc;
			break;
	}

	// Call 'MultiSpreadBase' method to get a new SSheet.
	CSheetDescription *pclSheetDescription = NULL;
	
	if ((iSSheetID == SD_ActuatorAccessory) || (iSSheetID == SD_CtrlActuatorAccessorySet))
	{
		pclSheetDescription = CreateSSheet( iSSheetID, SD_Actuator );
	}
	else
	{
		pclSheetDescription = CreateSSheet(iSSheetID);
	}

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
		case SD_ControlValve:
			lFirstColumnWidth = pclPreviousSheetDescription->GetSSheetPointer()->GetColWidthInPixelsW( CD_ControlValve_FirstColumn );
			lFirstColumnWidth += pclPreviousSheetDescription->GetSSheetPointer()->GetColWidthInPixelsW( CD_ControlValve_Box );
			lLastColumnWidth = pclPreviousSheetDescription->GetSSheetPointer()->GetColWidthInPixelsW( CD_ControlValve_Pointer );
			break;

		case SD_ControlValveAccessory:
		case SD_ControlValveAdapter:
		case SD_ActuatorAccessory:
			lFirstColumnWidth = pclPreviousSheetDescription->GetSSheetPointer()->GetColWidthInPixelsW( CD_Accessory_FirstColumn );
			lLastColumnWidth = pclPreviousSheetDescription->GetSSheetPointer()->GetColWidthInPixelsW( CD_Accessory_LastColumn );
			break;

		case SD_Actuator:
			lFirstColumnWidth = pclPreviousSheetDescription->GetSSheetPointer()->GetColWidthInPixelsW( CD_Actuator_FirstColumn );
			lFirstColumnWidth += pclPreviousSheetDescription->GetSSheetPointer()->GetColWidthInPixelsW( CD_Actuator_Box );
			lLastColumnWidth = pclPreviousSheetDescription->GetSSheetPointer()->GetColWidthInPixelsW( CD_Actuator_Pointer );
			break;
	}

	// Do some correction in regards to some columns.
	long lTotalWidth = rectSize.Width() - lFirstColumnWidth;
	lTotalWidth -= lLastColumnWidth;

	// Try to create 2 columns in just the middle of previous sheet.
	long lLeftWidth = ( long )( lTotalWidth / 2 );
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
	pclSheetDescription->SetSelectableRangeRow( RD_Accessory_FirstAvailRow,  pclSSheet->GetMaxRows() - 1 );

	// Two lines by accessory, but two accessories by line
	if( 0 != iTitleID )
	{
		// Increase row height.
		double dRowHeight = 12.75;
		pclSSheet->SetRowHeight( RD_Accessory_FirstRow, dRowHeight * 0.5 );
		pclSSheet->SetRowHeight( RD_Accessory_GroupName, dRowHeight * 1.5 );

		// Set title.
		pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, ( LPARAM )GetTitleBackgroundColor( pclSheetDescription ) );
		pclSheetDescription->SetUserVariable( _SDUV_TITLEFORECOLOR, ( pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor ) ) );
		pclSheetDescription->SetUserVariable( _SDUV_TITLEBACKCOLOR, ( pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor ) ) );

		pclSSheet->AddCellSpanW( CD_Accessory_Left, RD_Accessory_GroupName, CD_Accessory_LastColumn - CD_Accessory_Left, 1 );
		pclSSheet->SetStaticText( CD_Accessory_Left, RD_Accessory_GroupName, iTitleID );
	}

	// Fill accessories.
	long lRow = RD_Accessory_FirstAvailRow;

	CString str;
	LPARAM lparam;

	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, ( LPARAM )FALSE );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, ( LPARAM )SSS_ALIGN_LEFT );

	// Left - Right.
	long lLeftOrRight = CD_Accessory_Left;
	BOOL bContinue = pclList->GetFirst( str, lparam );

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

		if( true == pclRuledTable->IsByPair( pAccessory->GetIDPtr().ID ) )
		{
			strName += _T("2x ");
		}

		strName += pAccessory->GetName();

		// If accessory has its property 'IsAttached' to true and user is in selection by set, he can't select it.
		bool bEnabled = true;

		if( iSSheetID != SD_ControlValveAdapter && true == pAccessory->IsAttached() && true == m_pclIndSelCtrlParams->m_bOnlyForSet )
		{
			bEnabled = false;
		}

		if( true == bEnabled && SD_ControlValveAdapter == iSSheetID && true == m_pclIndSelCtrlParams->m_bOnlyForSet )
		{
			bEnabled = false;
		}

		CCDBCheckboxAccessory *pCheckbox = CreateCheckboxAccessory( lLeftOrRight, lRow, false, bEnabled, strName, pAccessory, pclRuledTable,
				pvecAccessories, pclSheetDescription );

		if( NULL != pCheckbox )
		{
			pCheckbox->ApplyInternalChange();

			if( false == bEnabled && iSSheetID != SD_ControlValveAdapter )
			{
				pCheckbox->SetToolTip( TASApp.LoadLocalizedString( AFXMSG_ACCATTACHEDNOTINSET ) );
			}
		}

		// Description.
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleEllipses, ( LPARAM )TRUE );
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

long CRViewSSelCtrl::_GetRowOfEditedActuator( CSheetDescription *pclSheetDescriptionActuator, CDB_Actuator *pActuator )
{
	if( NULL == pclSheetDescriptionActuator || NULL == pActuator )
	{
		return -1;
	}

	// Retrieve list of all products in 'pclSheetDescriptionActuator'.
	CSheetDescription::vecCellDescription vecCellDescriptionList;
	pclSheetDescriptionActuator->GetCellDescriptionList( vecCellDescriptionList, RVSCellDescription::CD_Product );

	// Run all objects.
	long lReturnValue = -1;
	CSheetDescription::vecCellDescriptionIter vecIter = vecCellDescriptionList.begin();

	while( vecIter != vecCellDescriptionList.end() )
	{
		CCellDescriptionProduct *pCDProduct = dynamic_cast<CCellDescriptionProduct *>( *vecIter );

		if( NULL != pCDProduct && 0 != pCDProduct->GetProduct() && pActuator == ( CDB_Actuator * )pCDProduct->GetProduct() )
		{
			lReturnValue = pCDProduct->GetCellPosition().y;
			break;
		}

		vecIter++;
	}

	return lReturnValue;
}

void CRViewSSelCtrl::_SelectActuatorAdapter( void )
{
	// This method is called only from '_ClickOnActuator' when user has selected an actuator.

	if( NULL == m_pclIndSelCtrlParams )
	{
		ASSERT_RETURN;
	}

	CDB_ControlValve *pclCurrentControlValveSelected = GetCurrentControlValveSelected();

	if( NULL == pclCurrentControlValveSelected )
	{
		ASSERT_RETURN;
	}

	CDB_Actuator *pclCurrentActuatorSelected = GetCurrentActuatorSelected();

	if( NULL == pclCurrentActuatorSelected )
	{
		ASSERT_RETURN;
	}

	// Variables
	CTable *pActuatorAdapterTable = NULL;		// Table of adapter that works on actuator.
	CTable *pCvAdapterTable = NULL;				// Table of adapter that works on the control valve.
	IDPTR CVIDPtr = _NULL_IDPTR;
	IDPTR ACTIDPtr = _NULL_IDPTR;

	// Get the tables with all adapters that fit the actuator and the control valve.
	if( NULL != pclCurrentControlValveSelected )
	{
		pActuatorAdapterTable = ( CTable * )( pclCurrentActuatorSelected->GetActAdapterGroupIDPtr().MP );
		pCvAdapterTable = ( CTable * )( pclCurrentControlValveSelected->GetAdapterGroupIDPtr().MP );

		if( NULL == pActuatorAdapterTable || NULL == pCvAdapterTable )
		{
			// Disable all adapters that belongs to the control valve.
			// Remark: by directly using 'CVADAPTERGROUP_TAB', we disable all series.
			DisableCheckboxAccessories( _T("CVADAPTERGROUP_TAB"), &m_vecControlValveAdapter );
			return;
		}
	}

	// Unselect the previous adapter if exist.
	VerifyCheckboxAccessories( m_pCurrentAdapter, false, &m_vecControlValveAdapter, true );
	m_pCurrentAdapter = NULL;

	// If one adapter is found, do the process that will gray incompatible actuators and set the correct check box.
	CDB_Product *pclAdapter = pclCurrentControlValveSelected->GetMatchingAdapter( pclCurrentActuatorSelected, m_pclIndSelCtrlParams->m_bOnlyForSet );

	if( NULL != pclAdapter )
	{
		// Select the new adapter.
		VerifyCheckboxAccessories( pclAdapter, true, &m_vecControlValveAdapter, true );

		// Gray all incompatible actuators with the selected adapter.
		GrayUncompatibleAccessoriesCheckbox( pclAdapter, true );

		// Particular case if we are in selection by package mode we disable all adapters.
		if( true == m_pclIndSelCtrlParams->m_bOnlyForSet && true == pclCurrentControlValveSelected->IsPartOfaSet() )
		{
			DisableCheckboxAccessories( _T( "" ), &m_vecControlValveAdapter );
		}
	}
	else
	{
		// If any adapter is found, we must gray all adapters.
		DisableCheckboxAccessories( _T( "" ), &m_vecControlValveAdapter );
	}
}

void CRViewSSelCtrl::_ReadAllColumnWidth( CDS_IndSelParameter *pclIndSelParameter )
{
	if( NULL == pclIndSelParameter )
	{
		ASSERT_RETURN;
	}

	std::map<UINT, short> mapSDIDVersion;
	mapSDIDVersion[CW_RVIEWSSELCTRL_SHEETID_CTRLVALVE] = CW_RVIEWSSELCTRL_CTRLVALVE_VERSION;
	mapSDIDVersion[CW_RVIEWSSELCTRL_SHEETID_ACTUATOR] = CW_RVIEWSSELCTRL_ACTUATOR_VERSION;

	// Container window sheet ID to sheetdescription of this rightview.
	std::map<UINT, short> mapCWtoRW;
	mapCWtoRW[CW_RVIEWSSELCTRL_SHEETID_CTRLVALVE] = SD_ControlValve;
	mapCWtoRW[CW_RVIEWSSELCTRL_SHEETID_ACTUATOR] = SD_Actuator;

	// By default and before reading registry saved column width force reset column width for all sheets.
	for( auto &iter : mapCWtoRW )
	{
		ResetColumnWidth( iter.second );
	}

	// At now we write/read separately in the Windows registry for the 3 inherited classes.
	short nWindowID = CMainFrame::RightViewList::eRVSSelCv;

	switch( m_eRViewID )
	{
		case CMainFrame::RightViewList::eRVSSelCv:
			nWindowID = CW_WINDOWID_INDSELCV;
			break;

		case CMainFrame::RightViewList::eRVSSelBCv:
			nWindowID = CW_WINDOWID_INDSELBCV;
			break;

		case CMainFrame::RightViewList::eRVSSelPICv:
			nWindowID = CW_WINDOWID_INDSELPICV;
			break;

		case CMainFrame::RightViewList::eRVSSelDpCBCV:
			nWindowID = CW_WINDOWID_INDSELDPCBCV;
			break;
	}

	// Access to the 'RViewSSelCtrl' group.
	CDS_IndSelParameter::CCWWindow *pclCWWindow = pclIndSelParameter->GetWindowColumnWidth( nWindowID, true );

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

void CRViewSSelCtrl::_WriteAllColumnWidth( CDS_IndSelParameter *pclIndSelParameter )
{
	if( NULL == pclIndSelParameter )
	{
		ASSERT_RETURN;
	}

	std::map<UINT, short> mapSDIDVersion;
	mapSDIDVersion[CW_RVIEWSSELCTRL_SHEETID_CTRLVALVE] = CW_RVIEWSSELCTRL_CTRLVALVE_VERSION;
	mapSDIDVersion[CW_RVIEWSSELCTRL_SHEETID_ACTUATOR] = CW_RVIEWSSELCTRL_ACTUATOR_VERSION;

	// Container window sheet ID to sheet description of this right view.
	std::map<UINT, short> mapCWtoRW;
	mapCWtoRW[CW_RVIEWSSELCTRL_SHEETID_CTRLVALVE] = SD_ControlValve;
	mapCWtoRW[CW_RVIEWSSELCTRL_SHEETID_ACTUATOR] = SD_Actuator;

	// At now we write/read separately in the Windows registry for the 4 inherited classes.
	short nWindowID = CMainFrame::RightViewList::eRVSSelCv;

	switch( m_eRViewID )
	{
		case CMainFrame::RightViewList::eRVSSelCv:
			nWindowID = CW_WINDOWID_INDSELCV;
			break;

		case CMainFrame::RightViewList::eRVSSelBCv:
			nWindowID = CW_WINDOWID_INDSELBCV;
			break;

		case CMainFrame::RightViewList::eRVSSelPICv:
			nWindowID = CW_WINDOWID_INDSELPICV;
			break;

		case CMainFrame::RightViewList::eRVSSelDpCBCV:
			nWindowID = CW_WINDOWID_INDSELDPCBCV;
			break;
	}

	// Access to the 'RViewSSelXXX' group.
	CDS_IndSelParameter::CCWWindow *pclCWWindow = pclIndSelParameter->GetWindowColumnWidth( nWindowID, true );

	for( std::map<UINT, short>::iterator iter = mapSDIDVersion.begin(); iter != mapSDIDVersion.end(); iter++ )
	{
		CDS_IndSelParameter::CCWSheet *pclCWSheet = pclCWWindow->GetCWSheet( iter->first, true );
		pclCWSheet->SetVersion( iter->second );
		pclCWSheet->GetMap() = m_mapSSheetColumnWidth[mapCWtoRW[iter->first]];
	}
}
