#include "stdafx.h"
#include "afxctl.h"

#include "TASelect.h"
#include "MainFrm.h"
#include "Hydronic.h"

#include "DlgLeftTabSelManager.h"

#include "RViewSSelSS.h"
#include "RViewSSelSmartDpC.h"

CRViewSSelSmartDpC *pRViewSSelSmartDpC = NULL;
CRViewSSelSmartDpC::CRViewSSelSmartDpC() : CRViewSSelSS( CMainFrame::RightViewList::eRVSSelSmartDpController, false )
{
	m_pclIndSelSmartDpCParams = NULL;

	m_pCDBExpandCollapseRowsSmartDpC = NULL;
	m_pCDBExpandCollapseRowsDpSensor = NULL;
	m_pCDBExpandCollapseRowsSets = NULL;
		
	m_pCDBExpandCollapseGroupSmartDpCAccessory = NULL;
	m_pCDBExpandCollapseGroupDpSensor = NULL;
	m_pCDBExpandCollapseGroupDpSensorRowAccessory = NULL;
	m_pCDBExpandCollapseGroupSets = NULL;
	m_pCDBExpandCollapseGroupConnectionSetAcc = NULL;

	m_pCDBShowAllPriorities = NULL;
	m_lSmartDpCSelectedRow = -1;
	m_lDpSensorSelectedRow = -1;
	m_lSetSelectedRow = -1;

	pRViewSSelSmartDpC = this;
}

CRViewSSelSmartDpC::~CRViewSSelSmartDpC()
{
	pRViewSSelSmartDpC = NULL;
}

CDB_TAProduct *CRViewSSelSmartDpC::GetCurrentSmartDpCSelected( void )
{
	CDB_TAProduct *pclCurrentSmartDpCSelected = NULL;
	CSheetDescription *pclSheetDescriptionSmartDpC = m_ViewDescription.GetFromSheetDescriptionID( SD_SmartDpC );

	if( NULL != pclSheetDescriptionSmartDpC )
	{
		// Retrieve the current selected smart differental pressure controller.
		CCellDescriptionProduct *pclCDCurrentSmartDpCSelected = NULL;
		LPARAM lpPointer;

		if( true == pclSheetDescriptionSmartDpC->GetUserVariable( _SDUV_SELECTEDPRODUCT, lpPointer ) )
		{
			pclCDCurrentSmartDpCSelected = (CCellDescriptionProduct *)lpPointer;
		}

		if( NULL != pclCDCurrentSmartDpCSelected && NULL != pclCDCurrentSmartDpCSelected->GetProduct() )
		{
			pclCurrentSmartDpCSelected = dynamic_cast<CDB_TAProduct *>( (CData *)pclCDCurrentSmartDpCSelected->GetProduct() );
		}
	}

	return pclCurrentSmartDpCSelected;
}

CDB_DpSensor *CRViewSSelSmartDpC::GetCurrentDpSensorSelected( void )
{
	CDB_DpSensor *pclCurrentDpSensorSelected = NULL;
	CSheetDescription *pclSheetDescriptionDpSensor = m_ViewDescription.GetFromSheetDescriptionID( SD_DpSensor );

	if( NULL != pclSheetDescriptionDpSensor )
	{
		// Retrieve the current selected Dp sensor.
		CCellDescriptionProduct *pclCDCurrentDpSensorSelected = NULL;
		LPARAM lpPointer;

		if( true == pclSheetDescriptionDpSensor->GetUserVariable( _SDUV_SELECTEDPRODUCT, lpPointer ) )
		{
			pclCDCurrentDpSensorSelected = (CCellDescriptionProduct *)lpPointer;
		}

		if( NULL != pclCDCurrentDpSensorSelected && NULL != pclCDCurrentDpSensorSelected->GetProduct() )
		{
			pclCurrentDpSensorSelected = dynamic_cast<CDB_DpSensor *>( (CData *)pclCDCurrentDpSensorSelected->GetProduct() );
		}
	}

	return pclCurrentDpSensorSelected;
}

CDB_Product *CRViewSSelSmartDpC::GetCurrentProductSetSelected()
{
	CDB_Product *pclCurrentSetSelected = NULL;
	CSheetDescription *pclSheetDescriptionSet = m_ViewDescription.GetFromSheetDescriptionID( SD_Sets );

	if( NULL != pclSheetDescriptionSet )
	{
		// Retrieve the current selected Dp sensor.
		CCellDescriptionProduct *pclCDCurrentDpSensorSelected = NULL;
		LPARAM lpPointer;

		if( true == pclSheetDescriptionSet->GetUserVariable( _SDUV_SELECTEDPRODUCT, lpPointer ) )
		{
			pclCDCurrentDpSensorSelected = (CCellDescriptionProduct *)lpPointer;
		}

		if( NULL != pclCDCurrentDpSensorSelected && NULL != pclCDCurrentDpSensorSelected->GetProduct() )
		{
			pclCurrentSetSelected = dynamic_cast<CDB_Product *>( (CData *)pclCDCurrentDpSensorSelected->GetProduct() );
		}
	}

	return pclCurrentSetSelected;
}

void CRViewSSelSmartDpC::OnExpandCollapeGroupButtonClicked( CCDButtonExpandCollapseGroup *pCDBExpandCollapseGroup, CSheetDescription *pclSSheetDescription )
{
	if( NULL == pclSSheetDescription )
	{
		return;
	}

	UINT uiSheetDescriptionID = pclSSheetDescription->GetSheetDescriptionID();

	if( uiSheetDescriptionID != SD_DpSensor )
	{
		CRViewSSelSS::OnExpandCollapeGroupButtonClicked( pCDBExpandCollapseGroup, pclSSheetDescription );
	}
	else
	{
		for( UINT uiLoopSheetID = SheetDescription::SD_SmartDpC; uiLoopSheetID < SheetDescription::SD_SmartDpCLast; uiLoopSheetID++ )
		{
			if( uiLoopSheetID == uiSheetDescriptionID )
			{
				continue;
			}
			
			if( true == m_ViewDescription.IsSheetDescriptionExist( uiLoopSheetID ) && uiLoopSheetID == SD_SetContent )
			{
				if( CCDButtonExpandCollapseGroup::ButtonState::ExpandRow == pCDBExpandCollapseGroup->GetButtonState() )
				{
					HideSSheet( uiLoopSheetID );
				}
				else
				{
					ShowSSheet( uiLoopSheetID );
				}
			}
		}
		
		CRViewSSelSS::OnExpandCollapeGroupButtonClicked( pCDBExpandCollapseGroup, pclSSheetDescription );
	}
}

void CRViewSSelSmartDpC::Reset()
{
	m_lSmartDpCSelectedRow = -1;
	m_lDpSensorSelectedRow = -1;
	m_lSetSelectedRow = -1;
	
	m_vecSmartDpCAccessoryList.clear();
	m_vecDpSensorAccessoryList.clear();
	m_vecSetContentAccessories.clear();
	
	m_pCDBExpandCollapseRowsSmartDpC = NULL;
	m_pCDBExpandCollapseRowsDpSensor = NULL;
	m_pCDBExpandCollapseRowsSets = NULL;

	m_pCDBExpandCollapseGroupSmartDpCAccessory = NULL;
	m_pCDBExpandCollapseGroupDpSensor = NULL;
	m_pCDBExpandCollapseGroupDpSensorRowAccessory = NULL;
	m_pCDBExpandCollapseGroupSets = NULL;
	m_pCDBExpandCollapseGroupConnectionSetAcc = NULL;

	m_pCDBShowAllPriorities = NULL;
	CRViewSSelSS::Reset();
}

void CRViewSSelSmartDpC::Suggest( CProductSelelectionParameters *pclProductSelectionParameters, LPARAM lpParam )
{
	CRViewSSelSS::Suggest( pclProductSelectionParameters, lpParam );

	if( NULL == pclProductSelectionParameters || NULL == dynamic_cast<CIndSelSmartDpCParams *>( pclProductSelectionParameters ) )
	{
		ASSERT_RETURN;
	}

	m_pclIndSelSmartDpCParams = dynamic_cast<CIndSelSmartDpCParams *>(pclProductSelectionParameters);

	// To remove all current displayed sheets.
	Reset();

	BeginWaitCursor();
	CWnd::SetRedraw( FALSE );

	long lRowSelected = -1;

	CDS_SSelSmartDpC *pSelectedSmartDpC = NULL;

	// If we are in edition mode...
	if( NULL != m_pclIndSelSmartDpCParams->m_SelIDPtr.MP )
	{
		pSelectedSmartDpC = reinterpret_cast<CDS_SSelSmartDpC *>( (CData *)( m_pclIndSelSmartDpCParams->m_SelIDPtr.MP ) );

		if( NULL == pSelectedSmartDpC )
		{
			ASSERT_RETURN;
		}

		lRowSelected = _FillSmartDpCRows( pSelectedSmartDpC );
	}
	else
	{
		lRowSelected = _FillSmartDpCRows();
	}

	// Verify if sheet description has been well created.
	CSheetDescription *pclSheetDescription = m_ViewDescription.GetFromSheetDescriptionID( SD_SmartDpC );

	if( NULL != pclSheetDescription && NULL != pclSheetDescription->GetSSheetPointer() )
	{
		CSSheet *pclSSheet = pclSheetDescription->GetSSheetPointer();

		if( lRowSelected > -1 && NULL != pSelectedSmartDpC )
		{
			CCellDescriptionProduct *pclCDProduct = FindCDProduct( lRowSelected, (LPARAM)( pSelectedSmartDpC->GetProductAs<CDB_TAProduct>() ), pclSheetDescription );

			if( NULL != pclCDProduct && NULL != pclCDProduct->GetProduct() )
			{
				// If we are in edition mode we simulate a click on the product.
				OnClickProduct( pclSheetDescription, pclCDProduct, pclSheetDescription->GetActiveColumn(), lRowSelected );

				// Allow to check if we need to change the 'Show all priorities' button or not.
				CheckShowAllPrioritiesButtonState( pclSheetDescription, lRowSelected );

				// Verify smart differential pressure controller accessories.
				CAccessoryList *pclAccessoryList = pSelectedSmartDpC->GetAccessoryList();

				if( pclAccessoryList->GetCount() > 0 )
				{
					CAccessoryList::AccessoryItem rAccessoryItem = pclAccessoryList->GetFirst();

					while( rAccessoryItem.IDPtr.MP != NULL )
					{
						VerifyCheckboxAccessories( (CDB_Product *)rAccessoryItem.IDPtr.MP, true, &m_vecSmartDpCAccessoryList );
						rAccessoryItem = pclAccessoryList->GetNext();
					}
				}

				// Selected as a fictif set. No more set managed
				IDPTR SetIDPtr = pSelectedSmartDpC->GetSetIDPtr();
				CSheetDescription *pclSheetDescriptionSets = m_ViewDescription.GetFromSheetDescriptionID( SD_Sets );

				if( _NULL_IDPTR != SetIDPtr && NULL != SetIDPtr.MP && NULL != pclSheetDescriptionSets )
				{
					CDB_Product * pProductSet = dynamic_cast<CDB_Product *>( SetIDPtr.MP );

					if( NULL != pProductSet )
					{
						// Find row number where is the set.
						long lSetRowSelected = _GetRowOfEditedSet( pclSheetDescriptionSets, pProductSet );

						if( lSetRowSelected != -1 )
						{
							CCellDescriptionProduct *pclCDProductSet = FindCDProduct( lSetRowSelected, ( LPARAM )pProductSet, pclSheetDescriptionSets );

							if( NULL != pclCDProductSet && NULL != pclCDProductSet->GetProduct() )
							{
								// Simulate a click on the edited set.
								OnClickProduct( pclSheetDescriptionSets, pclCDProductSet, pclSheetDescriptionSets->GetActiveColumn(), lSetRowSelected );

								// Allow to check if we need to change the 'Show all priorities' button or not.
								CheckShowAllPrioritiesButtonState( pclSheetDescriptionSets, lSetRowSelected );

								// Verify connection set accessories.
								CAccessoryList *pclConnectionSetAccessoryList = pSelectedSmartDpC->GetSetContentAccessoryList();

								if( pclConnectionSetAccessoryList->GetCount() > 0 )
								{
									CAccessoryList::AccessoryItem rAccessoryItem = pclConnectionSetAccessoryList->GetFirst();

									while( rAccessoryItem.IDPtr.MP != NULL )
									{
										VerifyCheckboxAccessories( (CDB_Product *)rAccessoryItem.IDPtr.MP, true, &m_vecSetContentAccessories );
										rAccessoryItem = pclConnectionSetAccessoryList->GetNext();
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
			PrepareAndSetNewFocus( pclSheetDescription, CD_SmartDpC_Name, RD_SmartDpC_FirstAvailRow, 0 );
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

void CRViewSSelSmartDpC::FillInSelected( CDS_SelProd *pSelectedProductToFill )
{
	if( NULL == m_pclIndSelSmartDpCParams )
	{
		ASSERT_RETURN;
	}

	CDS_SSelSmartDpC *pSelectedSmartDpCToFill = dynamic_cast<CDS_SSelSmartDpC *>( pSelectedProductToFill );

	if( NULL == pSelectedSmartDpCToFill )
	{
		ASSERT_RETURN;
	}

	pSelectedSmartDpCToFill->SetDpSensorIDPtr( _NULL_IDPTR );
	pSelectedSmartDpCToFill->SetSetIDPtr( _NULL_IDPTR );

	// Clear previous selected accessories if wa are not in edition mode.
	CAccessoryList *pclSmartDpCAccessoryListToFill = pSelectedSmartDpCToFill->GetAccessoryList();
	CAccessoryList *pclDpSensorAccessoryListToFill = pSelectedSmartDpCToFill->GetDpSensorAccessoryList();
	CAccessoryList *pclConnectionSetAccessoryListToFill = pSelectedSmartDpCToFill->GetSetContentAccessoryList();

	bool bIsEditionMode = false;
	if( false == m_pclIndSelSmartDpCParams->m_bEditModeRunning )
	{
		pclSmartDpCAccessoryListToFill->Clear( CAccessoryList::AccessoryType::_AT_Accessory );
		pclDpSensorAccessoryListToFill->Clear( CAccessoryList::AccessoryType::_AT_Accessory );
		pclConnectionSetAccessoryListToFill->Clear( CAccessoryList::AccessoryType::_AT_SetAccessory );
	}
	else
	{
		bIsEditionMode = true;
	}

	// Retrieve the current selected smart differential pressure controller if exists.
	CDB_TAProduct *pclCurrentSmartDpCSelected = GetCurrentSmartDpCSelected();

	if( NULL != pclCurrentSmartDpCSelected && NULL != m_pclIndSelSmartDpCParams->m_pclSelectSmartDpCList )
	{
		pSelectedSmartDpCToFill->SetProductIDPtr( pclCurrentSmartDpCSelected->GetIDPtr() );

		if( false == m_pclIndSelSmartDpCParams->m_bOnlyForSet )
		{
			// Fill data for Dp sensor.
			CDB_DpSensor *pclCurrentSelectedDpSensor = GetCurrentDpSensorSelected();

			if( NULL != pclCurrentSelectedDpSensor )
			{
				pSelectedSmartDpCToFill->SetDpSensorIDPtr( pclCurrentSelectedDpSensor->GetIDPtr() );
			}
			else if( NULL != GetCurrentProductSetSelected() )
			{
				pSelectedSmartDpCToFill->SetSetIDPtr( GetCurrentProductSetSelected()->GetIDPtr() );
			}
		}
		else
		{
			CTableSet *pclSmartDpCSets = dynamic_cast<CTableSet *>( TASApp.GetpTADB()->Get( _T("SMARTDPCSET_TAB") ).MP );
			ASSERT( NULL != pclSmartDpCSets );

			CDB_Product *pclCurrentSelectedSet = GetCurrentProductSetSelected();

			if( NULL != pclCurrentSelectedSet )
			{
				CDB_Set *pSet = pclSmartDpCSets->FindCompatibleSet( pclCurrentSmartDpCSelected->GetIDPtr().ID, pclCurrentSelectedSet->GetIDPtr().ID );

				if( NULL != pSet )
				{
					pSelectedSmartDpCToFill->SetSetIDPtr( pSet->GetIDPtr() );
				}
			}
		}

		if( false == bIsEditionMode )
		{
			// Retrieve selected smart differential pressure controller accessory if exist and add it.
			for( vecCDCAccessoryListIter vecIter = m_vecSmartDpCAccessoryList.begin(); vecIter != m_vecSmartDpCAccessoryList.end(); vecIter++ )
			{
				CCDBCheckboxAccessory *pCDBCheckboxAccessory = *vecIter;

				if( NULL != pCDBCheckboxAccessory && true == pCDBCheckboxAccessory->GetCheckStatus() && NULL != pCDBCheckboxAccessory->GetAccessoryPointer() )
				{
					pclSmartDpCAccessoryListToFill->Add( pCDBCheckboxAccessory->GetAccessoryPointer()->GetIDPtr(), CAccessoryList::_AT_Accessory, 
							pCDBCheckboxAccessory->GetRuledTable() );
				}
			}

			// Retrieve selected Dp sensor accessory if exist and add it.
			for( vecCDCAccessoryListIter vecIter = m_vecDpSensorAccessoryList.begin(); vecIter != m_vecDpSensorAccessoryList.end(); vecIter++ )
			{
				CCDBCheckboxAccessory *pCDBCheckboxAccessory = *vecIter;

				if( NULL != pCDBCheckboxAccessory && true == pCDBCheckboxAccessory->GetCheckStatus() && NULL != pCDBCheckboxAccessory->GetAccessoryPointer() )
				{
					pclDpSensorAccessoryListToFill->Add( pCDBCheckboxAccessory->GetAccessoryPointer()->GetIDPtr(), CAccessoryList::_AT_Accessory, 
							pCDBCheckboxAccessory->GetRuledTable() );
				}
			}

			// Retrieve selected connection set accessory if exist and add it.
			for( vecCDCAccessoryListIter vecIter = m_vecSetContentAccessories.begin(); vecIter != m_vecSetContentAccessories.end(); vecIter++ )
			{
				CCDBCheckboxAccessory *pCDBCheckboxAccessory = *vecIter;

				if( NULL != pCDBCheckboxAccessory && true == pCDBCheckboxAccessory->GetCheckStatus() && NULL != pCDBCheckboxAccessory->GetAccessoryPointer() )
				{
					pclConnectionSetAccessoryListToFill->Add( pCDBCheckboxAccessory->GetAccessoryPointer()->GetIDPtr(), CAccessoryList::_AT_SetAccessory,
							pCDBCheckboxAccessory->GetRuledTable() );
				}
			}
		}
		else
		{
			UpdateAccessoryList( m_vecSmartDpCAccessoryList, pclSmartDpCAccessoryListToFill, CAccessoryList::_AT_Accessory );
			UpdateAccessoryList( m_vecDpSensorAccessoryList, pclDpSensorAccessoryListToFill, CAccessoryList::_AT_Accessory );
			UpdateAccessoryList( m_vecSetContentAccessories, pclConnectionSetAccessoryListToFill, CAccessoryList::_AT_SetAccessory );
		}

		// Selected pipe informations.
		if( NULL != m_pclIndSelSmartDpCParams->m_pclSelectSmartDpCList->GetSelectPipeList() )
		{
			m_pclIndSelSmartDpCParams->m_pclSelectSmartDpCList->GetSelectPipeList()->GetMatchingPipe( pclCurrentSmartDpCSelected->GetSizeKey(),	*pSelectedSmartDpCToFill->GetpSelPipe() );
		}
	}
}

void CRViewSSelSmartDpC::OnNewDocument( CDS_IndSelParameter *pclIndSelParameter )
{
	_ReadAllColumnWidth( pclIndSelParameter );
}

void CRViewSSelSmartDpC::SaveSelectionParameters( CDS_IndSelParameter *pclIndSelParameter )
{
	_WriteAllColumnWidth( pclIndSelParameter );
}

bool CRViewSSelSmartDpC::OnTextTipFetch( CSheetDescription *pclSheetDescription, long lColumn, long lRow, WORD *pwMultiLine, SHORT *pnTipWidth,
		TCHAR *pstrTipText, BOOL *pfShowTip )
{
	if( NULL == m_pclIndSelSmartDpCParams || NULL == m_pclIndSelSmartDpCParams->m_pclSelectSmartDpCList || false == m_bInitialised || NULL == pclSheetDescription )
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
	CDB_SmartControlValve *pclSmartDpC = dynamic_cast<CDB_SmartControlValve *>( pTAP );
	CDB_DpSensor *pclDpSensor = dynamic_cast<CDB_DpSensor *>( (CData *)GetCDProduct( lColumn, lRow, pclSheetDescription ) );
	CSelectedValve *pclSelectedSmartDpC = GetSelectProduct<CSelectedValve>( pTAP, m_pclIndSelSmartDpCParams->m_pclSelectSmartDpCList );
	CSSheet *pclSSheet = pclSheetDescription->GetSSheetPointer();
	CDS_TechnicalParameter *pclTechParam = m_pclIndSelSmartDpCParams->m_pTADS->GetpTechParams();

	switch( pclSheetDescription->GetSheetDescriptionID() )
	{
		// Check if mouse cursor has passed over a valve.
		case SD_SmartDpC:

			if( NULL != pclSelectedSmartDpC )
			{
				if( CD_SmartDpC_Name == lColumn )
				{
					CString strMessage;

					if( true == pclSelectedSmartDpC->IsFlagSet( CSelectedBase::eTemperature ) )
					{
						FormatString( strMessage, IDS_SSHEETSSEL_TEMPERROR, pclSmartDpC->GetTempRange() );
					}

					str = strMessage;
					strMessage = _T("");

					if( true == pclSelectedSmartDpC->IsFlagSet( CSelectedBase::eDp ) )
					{
						CString str2 = WriteCUDouble( _U_DIFFPRESS, m_pclIndSelSmartDpCParams->m_dDpMax, true, 3, 0 );
						CString str3 = WriteCUDouble( _U_DIFFPRESS, pclSmartDpC->GetDpmax(), true, 3, 0 );
						FormatString( strMessage, IDS_PRODUCTSELECTION_ERROR_DPMAX, str2, str3 );
					}

					if( false == str.IsEmpty() && false == strMessage.IsEmpty() )
					{
						str.Insert( 0, _T("- ") );
						str += _T("\r\n- ");
					}

					str += strMessage;
				}
				else if( CD_SmartDpC_DpMax == lColumn )
				{
					if( true == pclSelectedSmartDpC->IsFlagSet( CSelectedBase::eDp ) )
					{
						CString str2 = WriteCUDouble( _U_DIFFPRESS, m_pclIndSelSmartDpCParams->m_dDpMax, true, 3, 0 );
						CString str3 = WriteCUDouble( _U_DIFFPRESS, pclSmartDpC->GetDpmax(), true, 3, 0 );
						FormatString( str, IDS_PRODUCTSELECTION_ERROR_DPMAX, str2, str3 );
					}
				}
				else if( CD_SmartDpC_TemperatureRange == lColumn )
				{
					if( true == pclSelectedSmartDpC->IsFlagSet( CSelectedBase::eTemperature ) )
					{
						FormatString( str, IDS_SSHEETSSEL_TEMPERROR, pclSmartDpC->GetTempRange() );
					}
				}
				else if( CD_SmartDpC_MoreInfo == lColumn && NULL != pclSmartDpC )
				{
					str = TASApp.LoadLocalizedString( IDS_SSHEETSSELCV_ACTRPOWERSUP ) + _T(": ") + pclSmartDpC->GetPowerSupplyStr();
					str += _T("\n") + TASApp.LoadLocalizedString( IDS_SSHEETSSELCV_ACTROUTPUTSIG ) + _T(": ") + pclSmartDpC->GetInOutSignalsStr( false );
				}
				else if( CD_SmartDpC_PipeLinDp == lColumn )
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
				else if( CD_SmartDpC_PipeV == lColumn )
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

		case SD_SmartDpCAccessory:

			if( lColumn > CD_Accessory_FirstColumn && lColumn < CD_Accessory_LastColumn )
			{
				TextTipFetchEllipsesHelper( lColumn, lRow, pclSheetDescription, pnTipWidth, &str );
			}
		
			break;

		case SD_DpSensor:
		
			if( NULL != pclDpSensor )
			{
				if( CD_DpSensor_Name == lColumn || CD_DpSensor_TemperatureRange == lColumn )
				{
					if( m_pclIndSelSmartDpCParams->m_WC.GetTemp() > pclDpSensor->GetTmax()
							|| m_pclIndSelSmartDpCParams->m_WC.GetTemp() < pclDpSensor->GetTmin() )
					{
						FormatString( str, IDS_SSHEETSSEL_TEMPERROR, pclDpSensor->GetTempRange() );
					}
				}
			}

		case SD_Sets:
		case SD_SetContent:

			if( NULL != pclDpSensor )
			{
				int iErrorCount = 0;

				if( true == m_pclIndSelSmartDpCParams->m_bIsGroupDpbranchChecked && m_pclIndSelSmartDpCParams->m_dDpBranch > 0.0 )
				{
					if( m_pclIndSelSmartDpCParams->m_dDpBranch < pclDpSensor->GetMinMeasurableDp() )
					{
						// Dp to stabilize is below the minimum measurable pressure drop of the currenct selected Dp sensor (%1 < %2).
						CString strDpl = WriteCUDouble( _U_DIFFPRESS, m_pclIndSelSmartDpCParams->m_dDpBranch );
						CString strMinMeasurableDpl = WriteCUDouble( _U_DIFFPRESS, pclDpSensor->GetMinMeasurableDp(), true );
						FormatString( str, IDS_HYDROMOD_SMARTDPCDPLTOOLOW, strDpl, strMinMeasurableDpl );
						iErrorCount++;
					}
					else if( m_pclIndSelSmartDpCParams->m_dDpBranch > pclDpSensor->GetMaxMeasurableDp() )
					{
						// Dp to stabilize is above the maximum measurable pressure drop of the current selected Dp sensor (%1 > %2).
						CString strDpl = WriteCUDouble( _U_DIFFPRESS, m_pclIndSelSmartDpCParams->m_dDpBranch );
						CString strMaxMeasurableDpl = WriteCUDouble( _U_DIFFPRESS, pclDpSensor->GetMaxMeasurableDp(), true );
						FormatString( str, IDS_HYDROMOD_SMARTDPCDPLTOOHIGH, strDpl, strMaxMeasurableDpl );
						iErrorCount++;
					}

					// HYS-2007: Only if burst pressure != 0.
					if( 0 != pclDpSensor->GetBurstPressure() && m_pclIndSelSmartDpCParams->m_dDpBranch > pclDpSensor->GetBurstPressure() )
					{
						if( 1 == iErrorCount )
						{
							str.Insert( 0, _T("- ") );
						}
					
						if( iErrorCount >= 1 )
						{
							str += _T("\r\n- ");
						}

						// The maximum differential pressure is above the limit of the burst pressure (%1 > %2).
						CString str2 = WriteCUDouble( _U_DIFFPRESS, m_pclIndSelSmartDpCParams->m_dDpBranch, true, 3, 0 );
						CString str3 = WriteCUDouble( _U_DIFFPRESS, pclDpSensor->GetBurstPressure(), true, 3, 0 );
						CString str4;
						FormatString( str4, IDS_PRODUCTSELECTION_ERROR_BURSTPRESSURE, str2, str3 );
						str += str4;
						iErrorCount++;
					}
				}

				double dTemp = m_pclIndSelSmartDpCParams->m_WC.GetTemp();

				if( dTemp < pclDpSensor->GetTmin() )
				{
					if( 1 == iErrorCount )
					{
						str.Insert( 0, _T("- ") );
					}
					
					if( iErrorCount >= 1 )
					{
						str += _T("\r\n- ");
					}

					// The fluid temperature is lower that the minimum allowed for %1 (%2 < %3).
					CString strProductName = pclDpSensor->GetName();
					CString strTemp = WriteCUDouble( _U_TEMPERATURE, dTemp );
					CString strMinTemp = WriteCUDouble( _U_TEMPERATURE, pclDpSensor->GetTmin(), true );
					CString str4;
					FormatString( str4, IDS_HYDROMOD_TEMPTOOLOWFORPRODUCT, strProductName, strTemp, strMinTemp );
					str += str4;
					iErrorCount++;
				}

				if( dTemp > pclDpSensor->GetTmax() )
				{
					if( 1 == iErrorCount )
					{
						str.Insert( 0, _T("- ") );
					}
					
					if( iErrorCount >= 1 )
					{
						str += _T("\r\n- ");
					}

					// The fluid temperature is higher that the maximum allowed for %1 (%2 > %3).
					CString strProductName = pclDpSensor->GetName();
					CString strTemp = WriteCUDouble( _U_TEMPERATURE, dTemp );
					CString strMaxTemp = WriteCUDouble( _U_TEMPERATURE, pclDpSensor->GetTmax(), true );
					CString str4;
					FormatString( str4, IDS_HYDROMOD_TEMPTOOHIGHFORPRODUCT, strProductName, strTemp, strMaxTemp );
					str += str4;
					iErrorCount++;
				}
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

bool CRViewSSelSmartDpC::OnClickProduct( CSheetDescription *pclSheetDescriptionClicked, CCellDescriptionProduct *pclCellDescriptionProduct, long lColumn, long lRow )
{
	if( NULL == pclSheetDescriptionClicked || NULL == pclSheetDescriptionClicked->GetSSheetPointer() || NULL == pclCellDescriptionProduct )
	{
		ASSERTA_RETURN( false );
	}

	if( SD_SmartDpC == pclSheetDescriptionClicked->GetSheetDescriptionID() )
	{
		CDB_SmartControlValve *pclSmartDpC = dynamic_cast<CDB_SmartControlValve *>( (CData *)pclCellDescriptionProduct->GetProduct() );
		_ClickOnSmartDpC( pclSheetDescriptionClicked, pclSmartDpC, pclCellDescriptionProduct, lColumn, lRow );
	}
	else if( SD_DpSensor == pclSheetDescriptionClicked->GetSheetDescriptionID() )
	{
		CDB_DpSensor *pclDpSensor = dynamic_cast<CDB_DpSensor *>( (CData *)pclCellDescriptionProduct->GetProduct() );
		_ClickOnDpSensor( pclSheetDescriptionClicked, pclDpSensor, pclCellDescriptionProduct, lColumn, lRow );
	}
	else if( SD_Sets == pclSheetDescriptionClicked->GetSheetDescriptionID() )
	{
		CDB_Product *pclSet = dynamic_cast<CDB_Product *>( (CData *)pclCellDescriptionProduct->GetProduct() );
		_ClickOnSets( pclSheetDescriptionClicked, pclSet, pclCellDescriptionProduct, lColumn, lRow );
	}

	return true;
}

bool CRViewSSelSmartDpC::ResetColumnWidth( short nSheetDescriptionID )
{
	TSpread clTSpread;

	if( FALSE == clTSpread.Create( WS_CHILD, CRect( 0, 0, 0, 0 ), this, 0 ) )
	{
		ASSERT( 0 );
		return false;
	}

	switch( nSheetDescriptionID )
	{
		case SD_SmartDpC:
			m_mapSSheetColumnWidth[SD_SmartDpC][CD_SmartDpC_FirstColumn] = clTSpread.ColWidthToLogUnits( 4 );
			//m_mapSSheetColumnWidth[SD_SmartDpC][CD_SmartDpC_Box] = clTSpread.ColWidthToLogUnits( 2 );
			m_mapSSheetColumnWidth[SD_SmartDpC][CD_SmartDpC_CheckBox] = clTSpread.ColWidthToLogUnits( 2 );
			m_mapSSheetColumnWidth[SD_SmartDpC][CD_SmartDpC_Name] = clTSpread.ColWidthToLogUnits( 14 );
			m_mapSSheetColumnWidth[SD_SmartDpC][CD_SmartDpC_Material] = clTSpread.ColWidthToLogUnits( 12 );
			m_mapSSheetColumnWidth[SD_SmartDpC][CD_SmartDpC_Connection] = clTSpread.ColWidthToLogUnits( 14 );
			m_mapSSheetColumnWidth[SD_SmartDpC][CD_SmartDpC_PN] = clTSpread.ColWidthToLogUnits( 6 );
			m_mapSSheetColumnWidth[SD_SmartDpC][CD_SmartDpC_Size] = clTSpread.ColWidthToLogUnits( 10 );
			m_mapSSheetColumnWidth[SD_SmartDpC][CD_SmartDpC_Kvs] = clTSpread.ColWidthToLogUnits( 6 );
			m_mapSSheetColumnWidth[SD_SmartDpC][CD_SmartDpC_Qnom] = clTSpread.ColWidthToLogUnits( 8 );
			m_mapSSheetColumnWidth[SD_SmartDpC][CD_SmartDpC_DpMin] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_SmartDpC][CD_SmartDpC_DpMax] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_SmartDpC][CD_SmartDpC_TemperatureRange] = clTSpread.ColWidthToLogUnits( 8 );
			m_mapSSheetColumnWidth[SD_SmartDpC][CD_SmartDpC_MoreInfo] = clTSpread.ColWidthToLogUnits( 6 );
			m_mapSSheetColumnWidth[SD_SmartDpC][CD_SmartDpC_Separator] = clTSpread.ColWidthToLogUnits( 1 );
			m_mapSSheetColumnWidth[SD_SmartDpC][CD_SmartDpC_PipeSize] = clTSpread.ColWidthToLogUnits( 8 );
			m_mapSSheetColumnWidth[SD_SmartDpC][CD_SmartDpC_PipeLinDp] = clTSpread.ColWidthToLogUnits( 8 );
			m_mapSSheetColumnWidth[SD_SmartDpC][CD_SmartDpC_PipeV] = clTSpread.ColWidthToLogUnits( 8 );
			m_mapSSheetColumnWidth[SD_SmartDpC][CD_SmartDpC_Pointer] = clTSpread.ColWidthToLogUnits( 1 );
			break;

	
		case SD_DpSensor:
			m_mapSSheetColumnWidth[SD_DpSensor][CD_DpSensor_FirstColumn] = clTSpread.ColWidthToLogUnits( 4 );
			m_mapSSheetColumnWidth[SD_DpSensor][CD_DpSensor_Box] = clTSpread.ColWidthToLogUnits( 2 );
			m_mapSSheetColumnWidth[SD_DpSensor][CD_DpSensor_CheckBox] = clTSpread.ColWidthToLogUnits( 2 );
			m_mapSSheetColumnWidth[SD_DpSensor][CD_DpSensor_Name] = clTSpread.ColWidthToLogUnits( 18 );
			m_mapSSheetColumnWidth[SD_DpSensor][CD_DpSensor_Dpl] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_DpSensor][CD_DpSensor_TemperatureRange] = clTSpread.ColWidthToLogUnits( 8 );
			m_mapSSheetColumnWidth[SD_DpSensor][CD_DpSensor_Pointer] = clTSpread.ColWidthToLogUnits( 1 );
			break;

		case SD_Sets:
			m_mapSSheetColumnWidth[SD_Sets][CD_Sets_FirstColumn] = clTSpread.ColWidthToLogUnits( 4 );
			m_mapSSheetColumnWidth[SD_Sets][CD_Sets_Box] = clTSpread.ColWidthToLogUnits( 2 );
			m_mapSSheetColumnWidth[SD_Sets][CD_Sets_CheckBox] = clTSpread.ColWidthToLogUnits( 2 );
			m_mapSSheetColumnWidth[SD_Sets][CD_Sets_Name] = clTSpread.ColWidthToLogUnits( 18 );
			m_mapSSheetColumnWidth[SD_Sets][CD_Sets_Description] = clTSpread.ColWidthToLogUnits( 36 );
			m_mapSSheetColumnWidth[SD_Sets][CD_Sets_Pointer] = clTSpread.ColWidthToLogUnits( 1 );
			break;
	}

	if( NULL != clTSpread.GetSafeHwnd() )
	{
		clTSpread.DestroyWindow();
	}

	return true;
}

bool CRViewSSelSmartDpC::IsSelectionReady( void )
{
	bool bReady = false;
	CDB_TAProduct *pclSelectedSmartDpC = GetCurrentSmartDpCSelected();

	if( NULL != pclSelectedSmartDpC )
	{
		bReady = ( false == pclSelectedSmartDpC->IsDeleted() );

		CDB_DpSensor *pclSelectedDpSensor = GetCurrentDpSensorSelected();

		if( bReady && NULL != pclSelectedDpSensor )
		{
			bReady = ( false == pclSelectedDpSensor->IsDeleted() );
		}

		// If we are in the package selection mode AND the smart differential pressure controller is part of a set BUT there is not yet a set selected...
		if( NULL != m_pclIndSelSmartDpCParams && true == m_pclIndSelSmartDpCParams->m_bOnlyForSet && true == pclSelectedSmartDpC->IsPartOfaSet() )
		{
			CDB_Product *pclSelectedSet = GetCurrentProductSetSelected();

			if( NULL == pclSelectedSet )
			{
				bReady = false;
			}
		}
	}

	return bReady;
}

void CRViewSSelSmartDpC::SetCurrentSmartDpCSelected( CCellDescriptionProduct *pclCDCurrentSmartDpCSelected )
{
	// Try to retrieve sheet description linked to smart differential pressure controller.
	CSheetDescription *pclSheetDescriptionSmartDpC = m_ViewDescription.GetFromSheetDescriptionID( SD_SmartDpC );

	if( NULL != pclSheetDescriptionSmartDpC )
	{
		pclSheetDescriptionSmartDpC->SetUserVariable( _SDUV_SELECTEDPRODUCT, (LPARAM)pclCDCurrentSmartDpCSelected );
	}
}

void CRViewSSelSmartDpC::SetCurrentDpSensorSelected( CCellDescriptionProduct *pclCDCurrentDpSensorSelected )
{
	// Try to retrieve sheet description linked to the dp sensor.
	CSheetDescription *pclSheetDescriptionDpSensor = m_ViewDescription.GetFromSheetDescriptionID( SD_DpSensor );

	if( NULL != pclSheetDescriptionDpSensor )
	{
		pclSheetDescriptionDpSensor->SetUserVariable( _SDUV_SELECTEDPRODUCT, (LPARAM)pclCDCurrentDpSensorSelected );
	}
}

void CRViewSSelSmartDpC::SetCurrentSetSelected( CCellDescriptionProduct *pclCDCurrentSetSelected )
{
	// Try to retrieve sheet description linked to set.
	CSheetDescription *pclSheetDescriptionSet = m_ViewDescription.GetFromSheetDescriptionID( SD_Sets );

	if( NULL != pclSheetDescriptionSet )
	{
		pclSheetDescriptionSet->SetUserVariable( _SDUV_SELECTEDPRODUCT, (LPARAM)pclCDCurrentSetSelected );
	}
}

void CRViewSSelSmartDpC::_ClickOnSmartDpC( CSheetDescription *pclSheetDescriptionSmartDpC, CDB_SmartControlValve *pclSmartDpC, 
		CCellDescriptionProduct *pclCellDescriptionProduct, long lColumn, long lRow )
{
	CSSheet *pclSSheet = pclSheetDescriptionSmartDpC->GetSSheetPointer();

	// Memorize previous focus row.
	long lCurrentFocusedRow = GetCurrentFocusedRow();

	// To be sure to clean all previous data concerning focus.
	KillCurrentFocus();

	// Unselect row where current selection is set (yellow light).
	// Remark: 'KillCurrentFocus' unselect only rows where focus is on (blue light).
	pclSSheet->UnSelectMultipleRows();

	// By default clear accessories list.
	m_vecSmartDpCAccessoryList.clear();
	m_vecDpSensorAccessoryList.clear();
	m_vecSetContentAccessories.clear();

	m_pCDBExpandCollapseRowsDpSensor = NULL;

	m_pCDBExpandCollapseGroupSmartDpCAccessory = NULL;
	m_pCDBExpandCollapseGroupDpSensorRowAccessory = NULL;
	m_pCDBExpandCollapseGroupSets = NULL;
	m_pCDBExpandCollapseGroupConnectionSetAcc = NULL;
	m_pCDBExpandCollapseRowsSets = NULL;

	LPARAM lSmartDpCCount;
	pclSheetDescriptionSmartDpC->GetUserVariable( _SDUV_TOTALPRODUCT_COUNT, lSmartDpCCount );

	// Retrieve the current selected smart differential pressure controller if exists.
	CDB_TAProduct *pclCurrentSmartDpCSelected = GetCurrentSmartDpCSelected();

	// If there is already one smart differential pressure controller selected and user clicks on the current one...
	// Remark: 'm_pCDBExpandCollapseRowsSmartDp' is not created if there is only one smart differential pressure controller. Thus we need to check first if there is only one smart differential pressure controller.
	//         Otherwise we have to check if user has clicked on the current one.
	if( NULL != pclCurrentSmartDpCSelected && ( ( 1 == lSmartDpCCount) || ( NULL != m_pCDBExpandCollapseRowsSmartDpC && lRow == m_pCDBExpandCollapseRowsSmartDpC->GetCellPosition().y ) ) )
	{
		// Reset current product selected.
		SetCurrentSmartDpCSelected( NULL );

		// Uncheck checkbox.
		pclSSheet->SetCheckBox( CD_SmartDpC_CheckBox, lRow, _T(""), false, true );
		m_lSmartDpCSelectedRow = -1;

		// Delete Expand/Collapse rows button if exist.
		// Remark: 'm_pCDBExpandCollapseRows' is set to NULL in method!
		DeleteExpandCollapseRowsButton( m_pCDBExpandCollapseRowsSmartDpC, pclSheetDescriptionSmartDpC );

		// Show Show/Hide all priorities button if exist.
		if( NULL != m_pCDBShowAllPriorities )
		{
			m_pCDBShowAllPriorities->ApplyInternalChange();
		}

		// Set focus on smart differential pressure controller currently selected.
		PrepareAndSetNewFocus( pclSheetDescriptionSmartDpC, pclSheetDescriptionSmartDpC->GetActiveColumn(), lRow, 0 );

		// Remove all sheets after smart differential pressure controller.
		m_pCDBExpandCollapseGroupSmartDpCAccessory = NULL;
		m_ViewDescription.RemoveAllSheetAfter( SD_SmartDpC );

		// Notify a change in the selection.
		SelectionHasChanged( IsSelectionReady() );
	}
	else
	{
		// If a smart differential pressure controller is already selected...
		if( NULL != pclCurrentSmartDpCSelected )
		{
			// Remove all sheets after smart differential pressure controller.
			m_pCDBExpandCollapseGroupSmartDpCAccessory = NULL;


			// Unselect previous selection.
			pclSSheet->SetCheckBox( CD_SmartDpC_CheckBox, m_lSmartDpCSelectedRow, _T(""), false, true );

			m_ViewDescription.RemoveAllSheetAfter( SD_SmartDpC );
		}

		// Save new smart differential pressure controller selection.
		SetCurrentSmartDpCSelected( pclCellDescriptionProduct );

		// Check checkbox.
		pclSSheet->SetCheckBox( CD_SmartDpC_CheckBox, lRow, _T(""), true, true );
		m_lSmartDpCSelectedRow = lRow;

		// Delete Expand/Collapse rows button if exist.
		if( NULL != m_pCDBExpandCollapseRowsSmartDpC )
		{
			// Remark: 'm_pCDBExpandCollapseRows' is set to NULL in method!
			DeleteExpandCollapseRowsButton( m_pCDBExpandCollapseRowsSmartDpC, pclSheetDescriptionSmartDpC );
		}

		// Create Expand/Collapse rows button if needed...
		LPARAM lValveTotalCount;
		pclSheetDescriptionSmartDpC->GetUserVariable( _SDUV_TOTALPRODUCT_COUNT, lValveTotalCount );

		if( lValveTotalCount > 1 )
		{
			m_pCDBExpandCollapseRowsSmartDpC = CreateExpandCollapseRowsButton( CD_SmartDpC_FirstColumn, lRow, true, CCDButtonExpandCollapseRows::ButtonState::CollapseRow, 
					pclSheetDescriptionSmartDpC->GetFirstSelectableRow(), pclSheetDescriptionSmartDpC->GetLastSelectableRow( false ), pclSheetDescriptionSmartDpC );

			// Show button.
			if( NULL != m_pCDBExpandCollapseRowsSmartDpC )
			{
				m_pCDBExpandCollapseRowsSmartDpC->SetShowStatus( true );
			}
		}

		// Select smart differential pressure controller (just highlight background).
		pclSSheet->SelectOneRow( lRow, pclSheetDescriptionSmartDpC->GetSelectionFrom(), pclSheetDescriptionSmartDpC->GetSelectionTo() );

		// Fill corresponding accessories.
		_FillAccessoryRows();

		// Fill corresponding dp sensor. 
		CSelectedValve *pSelectedTAP = GetSelectProduct<CSelectedValve>( pclSmartDpC, m_pclIndSelSmartDpCParams->m_pclSelectSmartDpCList );

		_FillSetsRows( pSelectedTAP );

		// Try to set the focus on the next table but do not lose the focus if no other table exist.
		CSheetDescription *pclNextSheetDescription = NULL;
		long lNewFocusedRow;
		bool bShiftPressed;

		if( DNFERRC_FocusOnNextGroupOK == DefineNextFocusedRow( pclSheetDescriptionSmartDpC, CD_SmartDpC_FirstColumn, lRow, false,
				lNewFocusedRow, pclNextSheetDescription, bShiftPressed ) )
		{
			long lNewFocusedCol = pclNextSheetDescription->GetFirstParameterColumn();
			PrepareAndSetNewFocus( pclNextSheetDescription, lNewFocusedCol, lNewFocusedRow );
		}
		else
		{
			// Because we have changed current selection background in yellow, if there is no other sheet following current one, we
			// must set focus on current selection.
			PrepareAndSetNewFocus( pclSheetDescriptionSmartDpC, lColumn, lRow );
		}

		// Notify a change in the selection.
		SelectionHasChanged( IsSelectionReady() );
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PRIVATE MEMBERS
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
long CRViewSSelSmartDpC::_FillSmartDpCRows( CDS_SSelSmartDpC *pEditedProduct )
{
	if( NULL == m_pclIndSelSmartDpCParams || NULL == m_pclIndSelSmartDpCParams->m_pclSelectSmartDpCList
			|| NULL == m_pclIndSelSmartDpCParams->m_pclSelectSmartDpCList->GetSelectPipeList() || NULL == m_pclIndSelSmartDpCParams->m_pTADS
			|| NULL == m_pclIndSelSmartDpCParams->m_pPipeDB )
	{
		ASSERTA_RETURN( 0 );
	}

	// Call 'MultiSpreadBase' method to get a new SSheet.
	CSheetDescription *pclSheetDescriptionSmartDpC = CreateSSheet( SD_SmartDpC );

	if( NULL == pclSheetDescriptionSmartDpC || NULL == pclSheetDescriptionSmartDpC->GetSSheetPointer() )
	{
		return 0;
	}

	CSSheet *pclSSheet = pclSheetDescriptionSmartDpC->GetSSheetPointer();

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
	CSelectedValve *pclSelectedValve = m_pclIndSelSmartDpCParams->m_pclSelectSmartDpCList->GetFirst<CSelectedValve>();

	if( NULL == pclSelectedValve )
	{
		return 0;
	}

	CDB_SmartControlValve *pclSmartDpC = dynamic_cast<CDB_SmartControlValve *>(pclSelectedValve->GetpData());

	if( NULL == pclSmartDpC )
	{
		return 0;
	}

	bool bValveSettingExist = (NULL != pclSmartDpC->GetValveCharacteristic());
	bool bKvSignalExist = pclSmartDpC->IsKvSignalEquipped();

	// Set max rows.
	pclSSheet->SetMaxRows( RD_SmartDpC_FirstAvailRow - 1 );

	// All cells are static by default and filled with _T("").
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	pclSSheet->FormatStaticText( -1, -1, -1, -1, _T("") );

	// Increase row height.
	double RowHeight = 12.75;
	pclSSheet->SetRowHeight( RD_SmartDpC_FirstRow, RowHeight * 0.5 );
	pclSSheet->SetRowHeight( RD_SmartDpC_GroupName, RowHeight * 1.5 );
	pclSSheet->SetRowHeight( RD_SmartDpC_ColName, RowHeight * 2 );
	pclSSheet->SetRowHeight( RD_SmartDpC_Unit, RowHeight * 1.2 );

	// Initialize.
	pclSheetDescriptionSmartDpC->Init( 1, pclSSheet->GetMaxRows(), 1, CSheetDescription::SD_ParameterMode_Mono );
	pclSheetDescriptionSmartDpC->AddColumnInPixels( CD_SmartDpC_FirstColumn, m_mapSSheetColumnWidth[SD_SmartDpC][CD_SmartDpC_FirstColumn] );
	//pclSheetDescriptionSmartDpC->AddColumnInPixels( CD_SmartDpC_Box, m_mapSSheetColumnWidth[SD_SmartDpC][CD_SmartDpC_Box] );
	pclSheetDescriptionSmartDpC->AddColumnInPixels( CD_SmartDpC_CheckBox, m_mapSSheetColumnWidth[SD_SmartDpC][CD_SmartDpC_CheckBox] );
	pclSheetDescriptionSmartDpC->AddColumnInPixels( CD_SmartDpC_Name, m_mapSSheetColumnWidth[SD_SmartDpC][CD_SmartDpC_Name] );
	pclSheetDescriptionSmartDpC->AddColumnInPixels( CD_SmartDpC_Material, m_mapSSheetColumnWidth[SD_SmartDpC][CD_SmartDpC_Material] );
	pclSheetDescriptionSmartDpC->AddColumnInPixels( CD_SmartDpC_Connection, m_mapSSheetColumnWidth[SD_SmartDpC][CD_SmartDpC_Connection] );
	pclSheetDescriptionSmartDpC->AddColumnInPixels( CD_SmartDpC_PN, m_mapSSheetColumnWidth[SD_SmartDpC][CD_SmartDpC_PN] );
	pclSheetDescriptionSmartDpC->AddColumnInPixels( CD_SmartDpC_Size, m_mapSSheetColumnWidth[SD_SmartDpC][CD_SmartDpC_Size] );
	pclSheetDescriptionSmartDpC->AddColumnInPixels( CD_SmartDpC_Kvs, m_mapSSheetColumnWidth[SD_SmartDpC][CD_SmartDpC_Kvs] );
	pclSheetDescriptionSmartDpC->AddColumnInPixels( CD_SmartDpC_Qnom, m_mapSSheetColumnWidth[SD_SmartDpC][CD_SmartDpC_Qnom] );
	pclSheetDescriptionSmartDpC->AddColumnInPixels( CD_SmartDpC_DpMin, m_mapSSheetColumnWidth[SD_SmartDpC][CD_SmartDpC_DpMin] );
	pclSheetDescriptionSmartDpC->AddColumnInPixels( CD_SmartDpC_DpMax, m_mapSSheetColumnWidth[SD_SmartDpC][CD_SmartDpC_DpMax] );
	pclSheetDescriptionSmartDpC->AddColumnInPixels( CD_SmartDpC_TemperatureRange, m_mapSSheetColumnWidth[SD_SmartDpC][CD_SmartDpC_TemperatureRange] );
	pclSheetDescriptionSmartDpC->AddColumnInPixels( CD_SmartDpC_MoreInfo, m_mapSSheetColumnWidth[SD_SmartDpC][CD_SmartDpC_MoreInfo] );
	pclSheetDescriptionSmartDpC->AddColumnInPixels( CD_SmartDpC_Separator, m_mapSSheetColumnWidth[SD_SmartDpC][CD_SmartDpC_Separator] );
	pclSheetDescriptionSmartDpC->AddColumnInPixels( CD_SmartDpC_PipeSize, m_mapSSheetColumnWidth[SD_SmartDpC][CD_SmartDpC_PipeSize] );
	pclSheetDescriptionSmartDpC->AddColumnInPixels( CD_SmartDpC_PipeLinDp, m_mapSSheetColumnWidth[SD_SmartDpC][CD_SmartDpC_PipeLinDp] );
	pclSheetDescriptionSmartDpC->AddColumnInPixels( CD_SmartDpC_PipeV, m_mapSSheetColumnWidth[SD_SmartDpC][CD_SmartDpC_PipeV] );
	pclSheetDescriptionSmartDpC->AddColumnInPixels( CD_SmartDpC_Pointer, m_mapSSheetColumnWidth[SD_SmartDpC][CD_SmartDpC_Pointer] );

	// Show or hide box column.
	//pclSSheet->ShowCol( CD_SmartDpC_Box, ( false == m_pclIndSelSmartDpCParams->m_bOnlyForSet || false == pclSmartDpC->IsPartOfaSet() ) ? FALSE : TRUE );

	// Set in which column parameter must be saved.
	pclSheetDescriptionSmartDpC->AddParameterColumn( CD_SmartDpC_Pointer );

	// Set the focus column.
	pclSheetDescriptionSmartDpC->SetActiveColumn( CD_SmartDpC_Name );

	// Set range for selection.
	pclSheetDescriptionSmartDpC->SetFocusColumnRange( CD_SmartDpC_CheckBox, CD_SmartDpC_MoreInfo );

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

	pclSSheet->SetStaticText( CD_SmartDpC_Name, RD_SmartDpC_ColName, IDS_SSHEETSSEL_NAME );
	pclSSheet->SetStaticText( CD_SmartDpC_Material, RD_SmartDpC_ColName, IDS_SSHEETSSEL_MATERIAL );
	pclSSheet->SetStaticText( CD_SmartDpC_Connection, RD_SmartDpC_ColName, IDS_SSHEETSSEL_CONNECT );
	pclSSheet->SetStaticText( CD_SmartDpC_PN, RD_SmartDpC_ColName, IDS_SSHEETSSEL_PN );
	pclSSheet->SetStaticText( CD_SmartDpC_Size, RD_SmartDpC_ColName, IDS_SSHEETSSEL_SIZE );

	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();

	if( 0 == pUnitDB->GetDefaultUnitIndex( _C_KVCVCOEFF ) )
	{
		pclSSheet->SetStaticText( CD_SmartDpC_Kvs, RD_SmartDpC_ColName, IDS_SSHEETSSELCV_CVKVS );
	}
	else
	{
		pclSSheet->SetStaticText( CD_SmartDpC_Kvs, RD_SmartDpC_ColName, IDS_SSHEETSSELCV_CVCV );
	}

	pclSSheet->SetStaticText( CD_SmartDpC_Qnom, RD_SmartDpC_ColName, IDS_SSHEETSSEL_QNOM );
	pclSSheet->SetStaticText( CD_SmartDpC_DpMin, RD_SmartDpC_ColName, IDS_SSHEETSSEL_DPMIN );
	pclSSheet->SetStaticText( CD_SmartDpC_DpMax, RD_SmartDpC_ColName, IDS_RVIEWSSELCTRL_DPMAX );
	pclSSheet->SetStaticText( CD_SmartDpC_TemperatureRange, RD_SmartDpC_ColName, IDS_SSHEETSSEL_COLTEMPRANGE );
	pclSSheet->SetStaticText( CD_SmartDpC_MoreInfo, RD_SmartDpC_ColName, IDS_SSHEETSSEL_MORE );

	pclSSheet->SetStaticText( CD_SmartDpC_Separator, RD_SmartDpC_ColName, _T("") );

	// Pipes.
	pclSSheet->SetStaticText( CD_SmartDpC_PipeSize, RD_SmartDpC_ColName, IDS_SSHEETSSEL_PIPESIZE );
	pclSSheet->SetStaticText( CD_SmartDpC_PipeLinDp, RD_SmartDpC_ColName, IDS_SSHEETSSEL_PIPELINDP );
	pclSSheet->SetStaticText( CD_SmartDpC_PipeV, RD_SmartDpC_ColName, IDS_SSHEETSSEL_PIPEV );
	//
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Row units.
	pclSSheet->SetStaticText( CD_SmartDpC_TemperatureRange, RD_SmartDpC_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_TEMPERATURE ) ).c_str() );
	pclSSheet->SetStaticText( CD_SmartDpC_PipeLinDp, RD_SmartDpC_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_LINPRESSDROP ) ).c_str() );
	pclSSheet->SetStaticText( CD_SmartDpC_PipeV, RD_SmartDpC_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_VELOCITY ) ).c_str() );
	//
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Draw black line below column header.
	pclSSheet->SetCellBorder( CD_SmartDpC_CheckBox, RD_SmartDpC_Unit, CD_SmartDpC_Separator - 1, RD_SmartDpC_Unit, true, SS_BORDERTYPE_BOTTOM );
	pclSSheet->SetCellBorder( CD_SmartDpC_PipeSize, RD_SmartDpC_Unit, CD_SmartDpC_Pointer - 1, RD_SmartDpC_Unit, true, SS_BORDERTYPE_BOTTOM );

	CSelectPipe selPipe( m_pclIndSelSmartDpCParams );
	long lRetRow = -1;

	double dRho = m_pclIndSelSmartDpCParams->m_WC.GetDens();
	double dKinVisc = m_pclIndSelSmartDpCParams->m_WC.GetKinVisc();
	double dFlow = m_pclIndSelSmartDpCParams->m_dFlow;

	CDS_TechnicalParameter *pTechParam = m_pclIndSelSmartDpCParams->m_pTADS->GetpTechParams();

	long lRow = RD_SmartDpC_FirstAvailRow;

	long lSmartDpCTotalCount = 0;
	long lSmartDpCNotPriorityCount = 0;
	bool bShowAllPrioritiesShown = false;
	long lShowAllPrioritiesButtonRow = 0;
	bool bAtLeastOneError;
	bool bAtLeastOneWarning;

	pclSheetDescriptionSmartDpC->SetUserVariable( _SDUV_TOTALPRODUCT_COUNT, 0 );
	pclSheetDescriptionSmartDpC->SetUserVariable( _SDUV_NOPRIORITYPRODUCT_COUNT, 0 );

	pclSheetDescriptionSmartDpC->RestartRemarkGenerator();
	bool bAtLeastOneSmartDpCPartOfASet = false;

	for( pclSelectedValve = m_pclIndSelSmartDpCParams->m_pclSelectSmartDpCList->GetFirst<CSelectedValve>(); NULL != pclSelectedValve;
			pclSelectedValve = m_pclIndSelSmartDpCParams->m_pclSelectSmartDpCList->GetNext<CSelectedValve>() )
	{
		pclSmartDpC = dynamic_cast<CDB_SmartControlValve *>( pclSelectedValve->GetpData() );

		if( NULL == pclSmartDpC || NULL == pclSmartDpC->GetSmartValveCharacteristic() )
		{
			continue;
		}

		bAtLeastOneError = false;
		bAtLeastOneWarning = false;
		bool bBest = pclSelectedValve->IsFlagSet( CSelectedBase::eBest );

		if( true == pclSelectedValve->IsFlagSet( CSelectedBase::eNotPriority ) )
		{
			lSmartDpCNotPriorityCount++;

			if( false == bShowAllPrioritiesShown )
			{
				// Add one empty not selectable row.
				pclSheetDescriptionSmartDpC->AddRows( 1 );
				pclSheetDescriptionSmartDpC->RemoveSelectableRow( lRow );

				pclSSheet->SetCellBorder( CD_SmartDpC_CheckBox, lRow, CD_SmartDpC_Separator - 1, lRow, true, SS_BORDERTYPE_TOP | SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );
				pclSSheet->SetCellBorder( CD_SmartDpC_PipeSize, lRow, CD_SmartDpC_Pointer - 1, lRow, true, SS_BORDERTYPE_TOP | SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );

				lShowAllPrioritiesButtonRow = lRow++;
				bShowAllPrioritiesShown = true;
			}
		}

		// Add this row.
		// 'true' to specify that this row can be selected.
		pclSheetDescriptionSmartDpC->AddRows( 1, true );

		// Name column will be set at the end.
		if( true == bBest )
		{
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_GREEN );
		}

		pclSSheet->SetStaticText( CD_SmartDpC_Material, lRow, pclSmartDpC->GetBodyMaterial() );
		pclSSheet->SetStaticText( CD_SmartDpC_Connection, lRow, pclSmartDpC->GetConnect() );
		pclSSheet->SetStaticText( CD_SmartDpC_PN, lRow, pclSmartDpC->GetPN().c_str() );
		pclSSheet->SetStaticText( CD_SmartDpC_Size, lRow, pclSmartDpC->GetSize() );
		pclSSheet->SetStaticText( CD_SmartDpC_Kvs, lRow, WriteCUDouble( _C_KVCVCOEFF, pclSmartDpC->GetSmartValveCharacteristic()->GetKvs() ) );
		pclSSheet->SetStaticText( CD_SmartDpC_Qnom, lRow, WriteCUDouble( _U_FLOW, pclSmartDpC->GetSmartValveCharacteristic()->GetQnom(), true ) );

		// HYS-1914: It's here well the dp min (Kvs is the Kv at full opening).
		double dDpMin = CalcDp( m_pclIndSelSmartDpCParams->m_dFlow, pclSmartDpC->GetSmartValveCharacteristic()->GetKvs(), m_pclIndSelSmartDpCParams->m_WC.GetDens() );
		pclSSheet->SetStaticText( CD_SmartDpC_DpMin, lRow, WriteCUDouble( _U_DIFFPRESS, dDpMin, true ) );

		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

		// Set Dp max.
		if( true == pclSelectedValve->IsFlagSet( CSelectedBase::eDp ) )
		{
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_RED );
			bAtLeastOneError = true;
		}

		pclSSheet->SetStaticText( CD_SmartDpC_DpMax, lRow, WriteCUDouble( _U_DIFFPRESS, pclSmartDpC->GetDpmax(), true ) );
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

		// Temperature range.
		if( true == pclSelectedValve->IsFlagSet( CSelectedBase::eTemperature ) )
		{
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_RED );
			bAtLeastOneError = true;
		}

		pclSSheet->SetStaticText( CD_SmartDpC_TemperatureRange, lRow, ( (CDB_TAProduct *)pclSelectedValve->GetProductIDPtr().MP)->GetTempRange() );
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

		// Now we can set name column in regards to current status (error, best or normal).
		if( true == bAtLeastOneError )
		{
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_RED );
		}
		else if( true == pclSelectedValve->IsFlagSet( CSelectedBase::eBest ) )
		{
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_GREEN );
		}

		// Add checkbox.
		pclSSheet->SetCheckBox( CD_SmartDpC_CheckBox, lRow, _T(""), false, true );

		// In addition of current flag, we have possibility that valve has the flag 'Not available' or 'Deleted'. In that case, we show valve name in red with "*" around it and
		// symbol '!' or '!!' after.
		if( true == pclSmartDpC->IsDeleted() )
		{
			pclSheetDescriptionSmartDpC->WriteTextWithFlags( CString( pclSmartDpC->GetName() ), CD_SmartDpC_Name, lRow,
					CSheetDescription::RemarkFlags::Deleted, _T(""), _RED );
		}
		else if( false == pclSmartDpC->IsAvailable() )
		{
			pclSheetDescriptionSmartDpC->WriteTextWithFlags( CString( pclSmartDpC->GetName() ), CD_SmartDpC_Name, lRow,
					CSheetDescription::RemarkFlags::NotAvailable, _T(""), _RED );
		}
		else
		{
			pclSSheet->SetStaticText( CD_SmartDpC_Name, lRow, pclSmartDpC->GetName() );
		}

		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

		// HYS-1660
		pclSSheet->SetPictureCellWithID( IDI_SMV_MOREINFO, CD_SmartDpC_MoreInfo, lRow, CSSheet::PictureCellType::Icon );

		// Set pipe size.
		m_pclIndSelSmartDpCParams->m_pclSelectSmartDpCList->GetSelectPipeList()->GetMatchingPipe( pclSmartDpC->GetSizeKey(), selPipe );
		pclSSheet->SetStaticText( CD_SmartDpC_PipeSize, lRow, selPipe.GetpPipe()->GetName() );

		// Set the LinDp to orange if it is above or below the technical parameters limits.
		if( selPipe.GetLinDp() > pTechParam->GetPipeMaxDp() || selPipe.GetLinDp() < pTechParam->GetPipeMinDp() )
		{
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_ORANGE );
		}
		
		pclSSheet->SetStaticText( CD_SmartDpC_PipeLinDp, lRow, WriteCUDouble( _U_LINPRESSDROP, selPipe.GetLinDp() ) );

		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

		// Set the Velocity to orange if it is above the technical parameter limit.
		// Orange if it is below the dMinVel.
		if( selPipe.GetU() > pTechParam->GetPipeMaxVel() || selPipe.GetU() < pTechParam->GetPipeMinVel() )
		{
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_ORANGE );
		}

		pclSSheet->SetStaticText( CD_SmartDpC_PipeV, lRow, WriteCUDouble( _U_VELOCITY, selPipe.GetU() ) );
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

		// Save parameter.
		CreateCellDescriptionProduct( pclSheetDescriptionSmartDpC->GetFirstParameterColumn(), lRow, (LPARAM)pclSmartDpC, pclSheetDescriptionSmartDpC );

		if( NULL != pEditedTAP && pEditedTAP == pclSmartDpC )
		{
			lRetRow = lRow;
		}
		pclSSheet->SetCellBorder( CD_SmartDpC_CheckBox, lRow, CD_SmartDpC_Separator - 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _GRAY );
		pclSSheet->SetCellBorder( CD_SmartDpC_PipeSize, lRow, CD_SmartDpC_Pointer - 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _GRAY );

		lRow++;
		lSmartDpCTotalCount++;
	}

	long lLastDataRow = lRow - 1;

	pclSheetDescriptionSmartDpC->SetUserVariable( _SDUV_TOTALPRODUCT_COUNT, lSmartDpCTotalCount );
	pclSheetDescriptionSmartDpC->SetUserVariable( _SDUV_NOPRIORITYPRODUCT_COUNT, lSmartDpCNotPriorityCount );

	// Now we can create Show/Hide all priorities if needed.
	// Remarks: 2nd condition to avoid to have button if we have only no priority valves.
	if( true == bShowAllPrioritiesShown && lSmartDpCTotalCount > lSmartDpCNotPriorityCount )
	{
		// Create Show/Hide all priorities button in regards to current state!
		CCDButtonShowAllPriorities::ButtonState eState = ( true == GetShowAllPrioritiesFlag( SD_SmartDpC ) ) ?
				CCDButtonShowAllPriorities::ButtonState::HidePriorities : CCDButtonShowAllPriorities::ButtonState::ShowPriorities;

		CCDButtonShowAllPriorities *pclShowAllButton = CreateShowAllPrioritiesButton( CD_SmartDpC_FirstColumn, lShowAllPrioritiesButtonRow,
				bShowAllPrioritiesShown, eState, lShowAllPrioritiesButtonRow + 1, pclSSheet->GetMaxRows(), pclSheetDescriptionSmartDpC );

		// Here we must call 'ApplyInternalChange' to show or hide rows in regards to the current state!
		if( NULL != pclShowAllButton )
		{
			pclShowAllButton->ApplyInternalChange();
		}
	}
	else
	{
		if( lShowAllPrioritiesButtonRow != 0 )
		{
			pclSSheet->ShowRow( lShowAllPrioritiesButtonRow, false );
		}
	}

	// Try to merge only if there is more than one valve.
	if( lSmartDpCTotalCount > 2 || (2 == lSmartDpCTotalCount && lSmartDpCNotPriorityCount != 1) )
	{
		vector<long> vecColumnList;
		vecColumnList.push_back( CD_SmartDpC_PipeSize );
		vecColumnList.push_back( CD_SmartDpC_PipeLinDp );
		vecColumnList.push_back( CD_SmartDpC_PipeV );
		pclSSheet->MergeRowsWithSameValues( RD_SmartDpC_FirstAvailRow, lLastDataRow, vecColumnList );
	}

	pclSSheet->SetCellBorder( CD_SmartDpC_CheckBox, lLastDataRow, CD_SmartDpC_Separator - 1, lLastDataRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );

	// Needed by pipe merging.
	pclSSheet->SetCellBorder( CD_SmartDpC_PipeSize, lLastDataRow, CD_SmartDpC_Pointer - 1, lLastDataRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );

	// Add Remarks.
	// Note: these lines must be placed after creating Show/Hide all priorities button to avoid to take in count these lines when defining
	//       range of row to be Show/Hide.
	pclSheetDescriptionSmartDpC->WriteRemarks( lRow, CD_SmartDpC_CheckBox, CD_SmartDpC_Separator );

	// Add the main title.
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)GetTitleBackgroundColor( pclSheetDescriptionSmartDpC ) );
	pclSheetDescriptionSmartDpC->SetUserVariable( _SDUV_TITLEFORECOLOR, (pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor )) );
	pclSheetDescriptionSmartDpC->SetUserVariable( _SDUV_TITLEBACKCOLOR, (pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor )) );

	pclSSheet->AddCellSpanW( CD_SmartDpC_FirstColumn + 1, RD_SmartDpC_GroupName, CD_SmartDpC_Separator - CD_SmartDpC_FirstColumn - 1, 1 );
	pclSSheet->SetStaticText( CD_SmartDpC_FirstColumn + 1, RD_SmartDpC_GroupName, IDS_RVIEWSSELSMARTDPC_TITLE );

	pclSSheet->AddCellSpanW( CD_SmartDpC_PipeSize, RD_SmartDpC_GroupName, CD_SmartDpC_Pointer - CD_SmartDpC_PipeSize, 1 );

	CTable *pclTable = dynamic_cast<CTable *>(m_pclIndSelSmartDpCParams->m_pPipeDB->Get( m_pclIndSelSmartDpCParams->m_strPipeSeriesID ).MP);
	ASSERT( NULL != pclTable );

	pclSSheet->SetStaticText( CD_SmartDpC_PipeSize, RD_SmartDpC_GroupName, pclTable->GetName() );

	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

	pclSSheet->SetBool( SSB_REDRAW, TRUE );

	// Set that there is no selection at now.
	SetCurrentSmartDpCSelected( NULL );

	// Add possibility to change column size.
	// Remark: place this code after any initialization (like adding, removing, resizing, hiding/showing columns).
	CResizingColumnInfo *pclResizingColumnInfo = pclSheetDescriptionSmartDpC->GetResizingColumnInfo();
	pclResizingColumnInfo->ActivateFeature( true, false, RD_SmartDpC_FirstAvailRow, lLastDataRow );
	pclResizingColumnInfo->AddRangeColumn( CD_SmartDpC_Name, CD_SmartDpC_TemperatureRange, RD_SmartDpC_ColName, RD_SmartDpC_Unit );
	pclResizingColumnInfo->AddRangeColumn( CD_SmartDpC_PipeSize, CD_SmartDpC_PipeV, RD_SmartDpC_ColName, RD_SmartDpC_Unit );

	// Add possibility to click on title group to reset column width.
	CreateTitleGroupButtons( CD_SmartDpC_CheckBox, CD_SmartDpC_PipeV, RD_SmartDpC_GroupName, pclSheetDescriptionSmartDpC );

	return lRetRow;
}

void CRViewSSelSmartDpC::_FillAccessoryRows()
{
	if( NULL == m_pclIndSelSmartDpCParams || NULL == m_pclIndSelSmartDpCParams->m_pTADB )
	{
		ASSERT_RETURN;
	}

	// Retrieve the current selected smart differential pressure controller.
	CDB_TAProduct *pclCurrentSmartDpCSelected = GetCurrentSmartDpCSelected();

	if( NULL == pclCurrentSmartDpCSelected )
	{
		return;
	}

	// Sanity check.
	CDB_RuledTable *pRuledTable = (CDB_RuledTable *)( pclCurrentSmartDpCSelected->GetAccessoriesGroupIDPtr().MP );

	if( NULL == pRuledTable )
	{
		return;
	}

	CRank rList;
	int iCount = m_pclIndSelSmartDpCParams->m_pTADB->GetAccessories( &rList, pRuledTable, m_pclIndSelSmartDpCParams->m_eFilterSelection );

	if( 0 == iCount )
	{
		return;
	}

	// Call 'MultiSpreadBase' method to get a new SSheet.
	CSheetDescription *pclSDAccessory = CreateSSheet( SD_SmartDpCAccessory );

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

	// Get sheet description of smart differential pressure controller to retrieve width.
	CSheetDescription *pclSDSmartDpController = m_ViewDescription.GetFromSheetDescriptionID( SD_SmartDpC );
	// It's absolutely not normal to have this pointer NULL.
	ASSERT( NULL != pclSDSmartDpController );

	// Take SSheet size and client area size and determine the right width to take in count.
	CRect rectSize = pclSDSmartDpController->GetSSheetPointer()->GetSheetSizeInPixels();

	// Prepare first column width (to match smart differential pressure controller sheet).
	long lFirstColumnWidth = pclSDSmartDpController->GetSSheetPointer()->GetColWidthInPixelsW( CD_SmartDpC_FirstColumn );

	// Do some correction in regards to some columns.
	long lTotalWidth = rectSize.Width() - lFirstColumnWidth;
	lTotalWidth -= pclSDSmartDpController->GetSSheetPointer()->GetColWidthInPixelsW( CD_SmartDpC_Pointer );

	// Try to create 2 columns in just the middle of smart differential pressure controller sheet.
	long lLeftWidth = (long)(lTotalWidth / 2);
	long lRightWidth = ((lTotalWidth % 2) > 0) ? lLeftWidth + 1 : lLeftWidth;

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

	pclSDAccessory->SetUserVariable( _SDUV_TITLEFORECOLOR, (pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor )) );
	pclSDAccessory->SetUserVariable( _SDUV_TITLEBACKCOLOR, (pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor )) );

	pclSSheet->AddCellSpanW( CD_Accessory_Left, RD_Accessory_GroupName, CD_Accessory_LastColumn - CD_Accessory_Left, 1 );
	pclSSheet->SetStaticText( CD_Accessory_Left, RD_Accessory_GroupName, IDS_SSHEETSSELSMARTDPC_VALVEACCGROUP );

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

		CCDBCheckboxAccessory *pCheckbox = CreateCheckboxAccessory( lLeftOrRight, lRow, false, true, strName, pAccessory, pRuledTable, &m_vecSmartDpCAccessoryList, pclSDAccessory );

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

	VerifyCheckboxAccessories( NULL, false, &m_vecSmartDpCAccessoryList );

	m_pCDBExpandCollapseGroupSmartDpCAccessory = CreateExpandCollapseGroupButton( CD_Accessory_FirstColumn, RD_Accessory_GroupName, true, CCDButtonExpandCollapseGroup::ButtonState::CollapseRow,
			pclSDAccessory->GetFirstSelectableRow(), pclSDAccessory->GetLastSelectableRow( false ), pclSDAccessory );

	// Show button.
	if( NULL != m_pCDBExpandCollapseGroupSmartDpCAccessory )
	{
		m_pCDBExpandCollapseGroupSmartDpCAccessory->SetShowStatus( true );
	}

	pclSSheet->SetCellBorder( CD_Accessory_Left, lRow, CD_Accessory_LastColumn - 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );

	pclSSheet->SetBool( SSB_REDRAW, TRUE );
}

void CRViewSSelSmartDpC::_ClickOnDpSensor( CSheetDescription *pclSheetDescriptionDpSensor, CDB_DpSensor *pDpSensorClicked, 
		CCellDescriptionProduct *pclCellDescriptionProduct, long lColumn, long lRow )
{
	CSSheet *pclSSheet = pclSheetDescriptionDpSensor->GetSSheetPointer();

	// Memorize previous focus row.
	long lCurrentFocusedRow = GetCurrentFocusedRow();

	// To be sure to clean all previous data concerning focus.
	KillCurrentFocus();

	// Unselect row where current selection is set (yellow light).
	// Remark: 'KillCurrentFocus' unselect only rows where focus is on (blue light).
	pclSSheet->UnSelectMultipleRows();

	// By default clear accessories list.
	m_vecDpSensorAccessoryList.clear();
	m_pCDBExpandCollapseGroupDpSensorRowAccessory = NULL;

	LPARAM lDpSensorCount;
	pclSheetDescriptionDpSensor->GetUserVariable( _SDUV_TOTALPRODUCT_COUNT, lDpSensorCount );

	// Retrieve the current selected Dp sensor if exist.
	CDB_Product *pclCurrentDpSensorSelected = GetCurrentDpSensorSelected();

	// If there is already one Dp sensor selected and user clicks on the current one...
	// Remark: 'm_pCDBExpandCollapseRowsDpSensor' is not created if there is only one Dp sensor. Thus we need to check first if there is only one Dp sensor.
	//         Otherwise we have to check if user has clicked on the current one.
	if( NULL != pclCurrentDpSensorSelected && ( ( 1 == lDpSensorCount ) || (NULL != m_pCDBExpandCollapseRowsDpSensor && lRow == m_pCDBExpandCollapseRowsDpSensor->GetCellPosition().y ) ) )
	{
		// Reset current product selected.
		SetCurrentDpSensorSelected( NULL );

		// Uncheck checkbox.
		pclSSheet->SetCheckBox( CD_DpSensor_CheckBox, lRow, _T(""), false, true );
		m_lDpSensorSelectedRow = -1;

		// Delete Expand/Collapse rows button if exist.
		// Remark: 'm_pCDBExpandCollapseRows' is set to NULL in method!
		DeleteExpandCollapseRowsButton( m_pCDBExpandCollapseRowsDpSensor, pclSheetDescriptionDpSensor );

		// Show Show/Hide all priorities button if exist.
		if( NULL != m_pCDBShowAllPriorities )
		{
			m_pCDBShowAllPriorities->ApplyInternalChange();
		}

		// Set focus on Dp sensor currently selected.
		PrepareAndSetNewFocus( pclSheetDescriptionDpSensor, pclSheetDescriptionDpSensor->GetActiveColumn(), lRow, 0 );

		// Remove all sheets after the dp sensor.
		m_ViewDescription.RemoveAllSheetAfter( SD_DpSensor );

		// Notify a change in the selection.
		SelectionHasChanged( IsSelectionReady() );
	}
	else
	{
		// If a Dp sensor is already selected...
		if( NULL != pclCurrentDpSensorSelected )
		{
			// Unselect previous selection.
			pclSSheet->SetCheckBox( CD_DpSensor_CheckBox, m_lDpSensorSelectedRow, _T(""), false, true );

			// Remove all sheets after Dp sensor.
			m_ViewDescription.RemoveAllSheetAfter( SD_DpSensor );
		}

		// Save new Dp sensor selection.
		SetCurrentDpSensorSelected( pclCellDescriptionProduct );

		// Check checkbox.
		pclSSheet->SetCheckBox( CD_DpSensor_CheckBox, lRow, _T(""), true, true );
		m_lDpSensorSelectedRow = lRow;

		// Delete Expand/Collapse rows button if exist.
		if( NULL != m_pCDBExpandCollapseRowsDpSensor )
		{
			// Remark: 'm_pCDBExpandCollapseRows' is set to NULL in method!
			DeleteExpandCollapseRowsButton( m_pCDBExpandCollapseRowsDpSensor, pclSheetDescriptionDpSensor );
		}

		// Create Expand/Collapse rows button if needed...
		LPARAM lValveTotalCount;
		pclSheetDescriptionDpSensor->GetUserVariable( _SDUV_TOTALPRODUCT_COUNT, lValveTotalCount );

		if( lValveTotalCount > 1 )
		{
			m_pCDBExpandCollapseRowsDpSensor = CreateExpandCollapseRowsButton( CD_DpSensor_FirstColumn, lRow, true, CCDButtonExpandCollapseRows::ButtonState::CollapseRow, 
					pclSheetDescriptionDpSensor->GetFirstSelectableRow(), pclSheetDescriptionDpSensor->GetLastSelectableRow( false ), pclSheetDescriptionDpSensor );

			// Show button.
			if( NULL != m_pCDBExpandCollapseRowsDpSensor )
			{
				m_pCDBExpandCollapseRowsDpSensor->SetShowStatus( true );
			}
		}

		// Select Dp sensor (just highlight background).
		pclSSheet->SelectOneRow( lRow, pclSheetDescriptionDpSensor->GetSelectionFrom(), pclSheetDescriptionDpSensor->GetSelectionTo() );

		// Fill Dp sensor accessories.
		_FillDpSensorAccessoryRows();

		// Try to set the focus on the next table but do not lose the focus if no other table exist.
		CSheetDescription *pclNextSheetDescription = NULL;
		long lNewFocusedRow;
		bool bShiftPressed;

		if( DNFERRC_FocusOnNextGroupOK == DefineNextFocusedRow( pclSheetDescriptionDpSensor, CD_DpSensor_FirstColumn, lRow, false, lNewFocusedRow, pclNextSheetDescription, bShiftPressed ) )
		{
			long lNewFocusedCol = pclNextSheetDescription->GetFirstParameterColumn();
			PrepareAndSetNewFocus( pclNextSheetDescription, lNewFocusedCol, lNewFocusedRow );
		}
		else
		{
			// Because we have changed current selection background in yellow, if there is no other sheet following current one, we
			// must set focus on current selection.
			PrepareAndSetNewFocus( pclSheetDescriptionDpSensor, lColumn, lRow );
		}

		// Notify a change in the selection.
		SelectionHasChanged( IsSelectionReady() );
	}
}

void CRViewSSelSmartDpC::_FillDpSensorRows( CSelectedValve *pSelectedTAP )
{
	if( NULL == m_pclIndSelSmartDpCParams || NULL == m_pclIndSelSmartDpCParams->m_pclSelectSmartDpCList || NULL == m_pclIndSelSmartDpCParams->m_pTADB )
	{
		ASSERT_RETURN;
	}

	if( NULL == pSelectedTAP )
	{
		return;
	}

	CDB_SmartControlValve *pclSmartDpC = dynamic_cast<CDB_SmartControlValve *>( pSelectedTAP->GetpData() );

	if( NULL == pclSmartDpC )
	{
		return;
	}

	// Try to retrieve the smart differential pressure controller selected list.
	CSelectSmartDpCList *pclSmartDpCList = m_pclIndSelSmartDpCParams->m_pclSelectSmartDpCList;

	if( NULL == pclSmartDpCList )
	{
		return;
	}

	// Try to retrieve the Dp sensor table.
	CTable *pclDpSensorTable = dynamic_cast<CTable *>( pclSmartDpC->GetDpSensorGroupIDPtr().MP );

	// If no Dp sensor exist...
	if( NULL == pclDpSensorTable || 0 == pclDpSensorTable->GetItemCount() )
	{
		return;
	}

	// Call 'MultiSpreadBase' method to get a new SSheet.
	CSheetDescription *pclSheetDescriptionDpSensor = CreateSSheet( SD_DpSensor );

	if( NULL == pclSheetDescriptionDpSensor || NULL == pclSheetDescriptionDpSensor->GetSSheetPointer() )
	{
		return;
	}

	CSSheet *pclSSheet = pclSheetDescriptionDpSensor->GetSSheetPointer();

	pclSSheet->SetBool( SSB_REDRAW, FALSE );

	pclSSheet->Init();
	pclSSheet->SetBool( SSB_ALLOWDRAGDROP, FALSE );
	pclSSheet->SetBool( SSB_HORZSCROLLBAR, FALSE );
	pclSSheet->SetBool( SSB_VERTSCROLLBAR, FALSE );
	pclSSheet->SetBool( SSB_MOVEACTIVEONFOCUS, FALSE );
	pclSSheet->SetTextTip( SS_TT_STATUS_FLOATING, 250, NULL, _LIGHTYELLOW, _TAH_BLACK );

	// Set max rows.
	pclSSheet->SetMaxRows( RD_DpSensor_FirstAvailRow - 1 );

	// All cells are static by default and filled with _T("")
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	pclSSheet->FormatStaticText( -1, -1, -1, -1, _T("") );

	// Increase ROW height
	double dRowHeight = 12.75;

	pclSSheet->SetRowHeight( RD_DpSensor_FirstRow, dRowHeight * 0.5 );
	pclSSheet->SetRowHeight( RD_DpSensor_GroupName, dRowHeight * 1.5 );
	pclSSheet->SetRowHeight( RD_DpSensor_ColName, dRowHeight * 1.2 );
	pclSSheet->SetRowHeight( RD_DpSensor_Unit, dRowHeight * 1.2 );

	// Initialize.
	pclSheetDescriptionDpSensor->Init( 1, pclSSheet->GetMaxRows(), 1, CSheetDescription::SD_ParameterMode_Mono );
	pclSheetDescriptionDpSensor->AddColumnInPixels( CD_DpSensor_FirstColumn, m_mapSSheetColumnWidth[SD_DpSensor][CD_DpSensor_FirstColumn] );
	pclSheetDescriptionDpSensor->AddColumnInPixels( CD_DpSensor_Box, m_mapSSheetColumnWidth[SD_DpSensor][CD_DpSensor_Box] );
	pclSheetDescriptionDpSensor->AddColumnInPixels( CD_DpSensor_CheckBox, m_mapSSheetColumnWidth[SD_DpSensor][CD_DpSensor_CheckBox] );
	pclSheetDescriptionDpSensor->AddColumnInPixels( CD_DpSensor_Name, m_mapSSheetColumnWidth[SD_DpSensor][CD_DpSensor_Name] );
	pclSheetDescriptionDpSensor->AddColumnInPixels( CD_DpSensor_Dpl, m_mapSSheetColumnWidth[SD_DpSensor][CD_DpSensor_Dpl] );
	pclSheetDescriptionDpSensor->AddColumnInPixels( CD_DpSensor_TemperatureRange, m_mapSSheetColumnWidth[SD_DpSensor][CD_DpSensor_TemperatureRange] );
	pclSheetDescriptionDpSensor->AddColumnInPixels( CD_DpSensor_Pointer, m_mapSSheetColumnWidth[SD_DpSensor][CD_DpSensor_Pointer] );

	// Show or hide box column.
	pclSSheet->ShowCol( CD_DpSensor_Box, ( false == m_pclIndSelSmartDpCParams->m_bOnlyForSet || false == pclSmartDpC->IsPartOfaSet() ) ? FALSE : TRUE );

	// Set in which column parameter must be saved.
	pclSheetDescriptionDpSensor->AddParameterColumn( CD_DpSensor_Pointer );

	// Set the focus column.
	pclSheetDescriptionDpSensor->SetActiveColumn( CD_DpSensor_Name );

	// Set selectable rows.
	pclSheetDescriptionDpSensor->SetSelectableRangeRow( RD_DpSensor_FirstAvailRow, pclSSheet->GetMaxRows() );

	// Set range for selection.
	pclSheetDescriptionDpSensor->SetFocusColumnRange( CD_DpSensor_CheckBox, CD_DpSensor_TemperatureRange );

	// Add the main title.
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)GetTitleBackgroundColor( pclSheetDescriptionDpSensor ) );

	pclSheetDescriptionDpSensor->SetUserVariable( _SDUV_TITLEFORECOLOR, ( pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor ) ) );
	pclSheetDescriptionDpSensor->SetUserVariable( _SDUV_TITLEBACKCOLOR, ( pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor ) ) );

	pclSSheet->AddCellSpanW( CD_DpSensor_Box, RD_DpSensor_GroupName, CD_DpSensor_Pointer - CD_DpSensor_Box, 1 );
	pclSSheet->SetStaticText( CD_DpSensor_Box, RD_DpSensor_GroupName, IDS_SSHEETSSELSMARTDP_DPSENSORGROUP );

	// Columns title.
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

	pclSSheet->SetStaticText( CD_DpSensor_Name, RD_DpSensor_ColName, IDS_SSHEETSSELCV_ACTRNAME );
	pclSSheet->SetStaticText( CD_DpSensor_Dpl, RD_DpSensor_ColName, IDS_DPL );
	pclSSheet->SetStaticText( CD_DpSensor_TemperatureRange, RD_DpSensor_ColName, IDS_SSHEETSSEL_COLTEMPRANGE );

	// Row units.
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();
	pclSSheet->SetStaticText( CD_DpSensor_Dpl, RD_DpSensor_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ) ).c_str() );
	pclSSheet->SetStaticText( CD_DpSensor_TemperatureRange, RD_DpSensor_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_TEMPERATURE ) ).c_str() );

	if( false == m_pclIndSelSmartDpCParams->m_bIsGroupDpbranchChecked )
	{
		pclSSheet->ShowCol( CD_DpSensor_Dpl, FALSE );
	}
	
	// Fill Dp sensor list.
	long lRow = RD_DpSensor_FirstAvailRow;
	long lDpSensorCount = 0;
	pclSheetDescriptionDpSensor->SetUserVariable( _SDUV_TOTALPRODUCT_COUNT, 0 );

	for( IDPTR DpSensorIDPtr = pclDpSensorTable->GetFirst(); _T('\0') != DpSensorIDPtr.ID; DpSensorIDPtr = pclDpSensorTable->GetNext() ) 
	{
		CDB_DpSensor *pclDpSensor = dynamic_cast<CDB_DpSensor *>( (CData *)DpSensorIDPtr.MP );

		if( NULL == pclDpSensor )
		{
			ASSERT_CONTINUE;
		}

		// Add this row.
		// 'true' to specify that this row can be selected.
		pclSheetDescriptionDpSensor->AddRows( 1, true );

		// If Dp sensor is part of a set, add a box.
		if( true == m_pclIndSelSmartDpCParams->m_bOnlyForSet && true == pclSmartDpC->IsPartOfaSet() )
		{
			CCDButtonBox *pCDButtonBox = CreateCellDescriptionBox( CD_DpSensor_Box, lRow, true, CCDButtonBox::ButtonState::BoxOpened, pclSheetDescriptionDpSensor );

			if( NULL != pCDButtonBox )
			{
				pCDButtonBox->ApplyInternalChange();
			}
		}

		// Add checkbox.
		pclSSheet->SetCheckBox( CD_DpSensor_CheckBox, lRow, _T(""), false, true );

		// Add the name.
		if( m_pclIndSelSmartDpCParams->m_WC.GetTemp() > pclDpSensor->GetTmax()
				|| m_pclIndSelSmartDpCParams->m_WC.GetTemp() < pclDpSensor->GetTmin() )
		{
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_RED );
		}

		pclSSheet->SetStaticText( CD_DpSensor_Name, lRow, pclDpSensor->GetFullName() );
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

		// Set dp to stabilize.
		if( true == pclSmartDpCList->IsGroupDpbranchChecked() )
		{
			pclSSheet->SetStaticText( CD_DpSensor_Dpl, lRow, WriteCUDouble( _U_DIFFPRESS, pclSmartDpCList->GetDpToStab() ) );
		}

		pclSSheet->SetCellBorder( CD_DpSensor_CheckBox, lRow, CD_DpSensor_Pointer - 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _GRAY );

		// Temperature range.
		if( m_pclIndSelSmartDpCParams->m_WC.GetTemp() > pclDpSensor->GetTmax()
				|| m_pclIndSelSmartDpCParams->m_WC.GetTemp() < pclDpSensor->GetTmin() )
		{
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_RED );
		}

		pclSSheet->SetStaticText( CD_DpSensor_TemperatureRange, lRow, pclDpSensor->GetTempRange() );
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

		// Save parameter.
		CreateCellDescriptionProduct( pclSheetDescriptionDpSensor->GetFirstParameterColumn(), lRow, (LPARAM)pclDpSensor, pclSheetDescriptionDpSensor );

		lRow++;
		lDpSensorCount++;
	}

	long lLastDataRow = lRow - 1;

	// Save the total Dp sensor number.
	pclSheetDescriptionDpSensor->SetUserVariable( _SDUV_TOTALPRODUCT_COUNT, lDpSensorCount );

	// Set that there is no selection at now.
	SetCurrentDpSensorSelected( NULL );

	// Add possibility to collapse/expand Dp sensor sheet.
	m_pCDBExpandCollapseGroupDpSensor = CreateExpandCollapseGroupButton( CD_DpSensor_FirstColumn, RD_DpSensor_GroupName, true, CCDButtonExpandCollapseGroup::ButtonState::CollapseRow, 
			pclSheetDescriptionDpSensor->GetFirstSelectableRow() - 2, pclSheetDescriptionDpSensor->GetLastSelectableRow( false ), pclSheetDescriptionDpSensor );

	// Show button.
	if( NULL != m_pCDBExpandCollapseGroupDpSensor )
	{
		m_pCDBExpandCollapseGroupDpSensor->SetShowStatus( true );
	}

	pclSSheet->SetCellBorder( CD_DpSensor_CheckBox, lLastDataRow, CD_DpSensor_Pointer - 1, lLastDataRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );

	// Add possibility to change column size.
	// Remark: place this code after any initialization (like adding, removing, resizing, hiding/showing columns).
	CResizingColumnInfo *pclResizingColumnInfo = pclSheetDescriptionDpSensor->GetResizingColumnInfo();
	pclResizingColumnInfo->ActivateFeature( true, false, RD_DpSensor_FirstAvailRow, lLastDataRow );
	pclResizingColumnInfo->AddRangeColumn( CD_DpSensor_Name, CD_DpSensor_TemperatureRange, RD_DpSensor_ColName, RD_DpSensor_Unit );

	// Add possibility to click on title group to reset column width.
	CreateTitleGroupButtons( CD_DpSensor_CheckBox, CD_DpSensor_TemperatureRange, RD_DpSensor_GroupName, pclSheetDescriptionDpSensor );
}

void CRViewSSelSmartDpC::_FillDpSensorAccessoryRows( )
{
	if( NULL == m_pclIndSelSmartDpCParams || NULL == m_pclIndSelSmartDpCParams->m_pTADB )
	{
		ASSERT_RETURN;
	}

	CDB_DpSensor *pclCurrentDpSensorSelected = GetCurrentDpSensorSelected();

	if( NULL == pclCurrentDpSensorSelected )
	{
		return;
	}

	CDB_RuledTable *pRuledTable = (CDB_RuledTable *)( pclCurrentDpSensorSelected->GetAccessoriesGroupIDPtr().MP );

	if( NULL == pRuledTable )
	{
		return;
	}

	CRank rList;
	int iCount = m_pclIndSelSmartDpCParams->m_pTADB->GetAccessories( &rList, pRuledTable, m_pclIndSelSmartDpCParams->m_eFilterSelection );

	if( 0 == iCount )
	{
		return;
	}

	// Call 'MultiSpreadBase' method to get a new SSheet.
	CSheetDescription *pclSheetDescriptionDpSensorAccessory = CreateSSheet( SD_DpSensorAccessory );

	if( NULL == pclSheetDescriptionDpSensorAccessory || NULL == pclSheetDescriptionDpSensorAccessory->GetSSheetPointer() )
	{
		return;
	}

	CSSheet *pclSSheet = pclSheetDescriptionDpSensorAccessory->GetSSheetPointer();

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

	// Get sheet description of the smart differential pressure controller to retrieve width.
	// Remark: Dp sensor sheet is too narrow.
	CSheetDescription *pclSheetDescriptionSmartDpC = m_ViewDescription.GetFromSheetDescriptionID( SD_SmartDpC );
	
	// It's absolutely not normal to have this pointer NULL.
	ASSERT( NULL != pclSheetDescriptionSmartDpC );

	// Take SSheet size and client area size and determine the right width to take in count.
	CRect rectSize = pclSheetDescriptionSmartDpC->GetSSheetPointer()->GetSheetSizeInPixels( false );

	// Prepare first column width (to match Dp sensor sheet).
	long lFirstColumnWidth = pclSheetDescriptionSmartDpC->GetSSheetPointer()->GetColWidthInPixelsW( CD_SmartDpC_FirstColumn );


	// Do some correction in regards to some columns.
	long lTotalWidth = rectSize.Width() - lFirstColumnWidth;
	lTotalWidth -= pclSheetDescriptionSmartDpC->GetSSheetPointer()->GetColWidthInPixelsW( CD_SmartDpC_Pointer );

	// Try to create 2 columns in just the middle of the Dp sensor sheet.
	long lLeftWidth = ( long )( lTotalWidth / 2 );
	long lRightWidth = ( ( lTotalWidth % 2 ) > 0 ) ? lLeftWidth + 1 : lLeftWidth;

	// Set columns.
	// 'CSheetDescription::SD_ParameterMode_Multi' to set that this sheet can have more than one column with parameter.
	pclSheetDescriptionDpSensorAccessory->Init( 1, pclSSheet->GetMaxRows(), 2, CSheetDescription::SD_ParameterMode_Multi );

	pclSheetDescriptionDpSensorAccessory->AddColumnInPixels( CD_Accessory_FirstColumn, lFirstColumnWidth );
	pclSheetDescriptionDpSensorAccessory->AddColumnInPixels( CD_Accessory_Left, lLeftWidth );
	pclSheetDescriptionDpSensorAccessory->AddColumnInPixels( CD_Accessory_Right, lRightWidth );
	pclSheetDescriptionDpSensorAccessory->AddColumn( CD_Accessory_LastColumn, 0 );

	// These two columns can contain parameter.
	pclSheetDescriptionDpSensorAccessory->AddParameterColumn( CD_Accessory_Left );
	pclSheetDescriptionDpSensorAccessory->AddParameterColumn( CD_Accessory_Right );

	// Set the focus column (don't set on Left of Right (on a check box) ).
	pclSheetDescriptionDpSensorAccessory->SetActiveColumn( CD_Accessory_FirstColumn );

	// Set selectable rows.
	pclSheetDescriptionDpSensorAccessory->SetSelectableRangeRow( RD_Accessory_FirstAvailRow, pclSSheet->GetMaxRows() - 1 );

	// Increase row height.
	double dRowHeight = 12.75;
	pclSSheet->SetRowHeight( RD_Accessory_FirstRow, dRowHeight * 0.5 );
	pclSSheet->SetRowHeight( RD_Accessory_GroupName, dRowHeight * 1.5 );

	// Set title.
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)GetTitleBackgroundColor( pclSheetDescriptionDpSensorAccessory ) );

	pclSheetDescriptionDpSensorAccessory->SetUserVariable( _SDUV_TITLEFORECOLOR, ( pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor ) ) );
	pclSheetDescriptionDpSensorAccessory->SetUserVariable( _SDUV_TITLEBACKCOLOR, ( pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor ) ) );

	pclSSheet->AddCellSpanW( CD_Accessory_Left, RD_Accessory_GroupName, CD_Accessory_LastColumn - CD_Accessory_Left, 1 );
	pclSSheet->SetStaticText( CD_Accessory_Left, RD_Accessory_GroupName, IDS_SSHEETSSELSMARTDPC_DPSENSORACCGROUP );

	// Fill accessories.
	long lRow = RD_Accessory_FirstAvailRow;

	CString str;
	LPARAM lparam;

	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, ( LPARAM )FALSE );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, ( LPARAM )SSS_ALIGN_LEFT );

	// Left - Right.
	long lLeftOrRight = CD_Accessory_Left;
	BOOL bContinue = rList.GetFirst( str, lparam );

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

		if( true == pAccessory->IsAttached() && true == m_pclIndSelSmartDpCParams->m_bOnlyForSet )
		{
			bEnabled = false;
		}

		CCDBCheckboxAccessory *pCheckbox = CreateCheckboxAccessory( lLeftOrRight, lRow, false, bEnabled, pAccessory->GetName(), 
				pAccessory, pRuledTable, &m_vecDpSensorAccessoryList, pclSheetDescriptionDpSensorAccessory );

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

		if( !bContinue )
		{
			lRow++;
		}
	}

	VerifyCheckboxAccessories( NULL, false, &m_vecDpSensorAccessoryList );

	m_pCDBExpandCollapseGroupDpSensorRowAccessory = CreateExpandCollapseGroupButton( CD_Accessory_FirstColumn, RD_Accessory_GroupName, true,
			CCDButtonExpandCollapseGroup::ButtonState::CollapseRow, pclSheetDescriptionDpSensorAccessory->GetFirstSelectableRow(), 
			pclSheetDescriptionDpSensorAccessory->GetLastSelectableRow( false ), pclSheetDescriptionDpSensorAccessory );

	// Show button.
	if( NULL != m_pCDBExpandCollapseGroupDpSensorRowAccessory )
	{
		m_pCDBExpandCollapseGroupDpSensorRowAccessory->SetShowStatus( true );
	}

	pclSSheet->SetCellBorder( CD_Accessory_Left, lRow, CD_Accessory_LastColumn - 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );

	pclSSheet->SetBool( SSB_REDRAW, TRUE );
}

void CRViewSSelSmartDpC::_ClickOnSets( CSheetDescription *pclSheetDescriptionSets, CDB_Product *pSetsClicked, CCellDescriptionProduct *pclCellDescriptionProduct, long lColumn, long lRow )
{
	CSSheet *pclSSheet = pclSheetDescriptionSets->GetSSheetPointer();

	// Memorize previous focus row.
	long lCurrentFocusedRow = GetCurrentFocusedRow();

	// To be sure to clean all previous data concerning focus.
	KillCurrentFocus();

	// Unselect row where current selection is set (yellow light).
	// Remark: 'KillCurrentFocus' unselect only rows where focus is on (blue light).
	pclSSheet->UnSelectMultipleRows();

	// By default clear accessories list.
	m_vecSetContentAccessories.clear();
	m_pCDBExpandCollapseGroupConnectionSetAcc = NULL;

	LPARAM lSetCount;
	pclSheetDescriptionSets->GetUserVariable( _SDUV_TOTALPRODUCT_COUNT, lSetCount );

	// Retrieve the current selected set if exist.
	CDB_Product *pclCurrentSetSelected = GetCurrentProductSetSelected();

	// If there is already one set selected and user clicks on the current one...
	// Remark: 'm_pCDBExpandCollapseRowsSets' is not created if there is only one set. Thus we need to check first if there is only one set.
	//         Otherwise we have to check if user has clicked on the current one.
	if( NULL != pclCurrentSetSelected && ( ( 1 == lSetCount ) || (NULL != m_pCDBExpandCollapseRowsSets && lRow == m_pCDBExpandCollapseRowsSets->GetCellPosition().y ) ) )
	{
		
		if( NULL != GetCurrentProductSetSelected() )
		{
			// Reopen the box for the smart differential pressure controller.
			CSheetDescription *pclSheetDescriptionSmartDpC = m_ViewDescription.GetFromSheetDescriptionID( SD_SmartDpC );


			// Reopen the box for the set.
			CCDButtonBox *pCDButtonBox = GetCDButtonBox( CD_Sets_Box, lRow, pclSheetDescriptionSets );

			if( NULL != pCDButtonBox )
			{
				pCDButtonBox->SetButtonState( CCDButtonBox::ButtonState::BoxOpened, true );
			}
		}

		// Uncheck checkbox.
		pclSSheet->SetCheckBox( CD_Sets_CheckBox, lRow, _T(""), false, true );
		m_lSetSelectedRow = -1;

		// Reset current product selected.
		SetCurrentSetSelected( NULL );
		// Delete Expand/Collapse rows button if exist.
		// Remark: 'm_pCDBExpandCollapseRowsSets' is set to NULL in method!
		DeleteExpandCollapseRowsButton( m_pCDBExpandCollapseRowsSets, pclSheetDescriptionSets );

		// Show Show/Hide all priorities button if exist.
		if( NULL != m_pCDBShowAllPriorities )
		{
			m_pCDBShowAllPriorities->ApplyInternalChange();
		}

		// Set focus on set currently selected.
		PrepareAndSetNewFocus( pclSheetDescriptionSets, pclSheetDescriptionSets->GetActiveColumn(), lRow, 0 );

		// Remove all sheets after the sets.
		m_ViewDescription.RemoveAllSheetAfter( SD_Sets );

		// Notify a change in the selection.
		SelectionHasChanged( IsSelectionReady() );
	}
	else
	{
		// If a set is already selected...
		if( NULL != pclCurrentSetSelected )
		{
			if( NULL != GetCurrentProductSetSelected() )
			{
				// Reopen the box for the smart differential pressure controller.
				CSheetDescription *pclSheetDescriptionSmartDpC = m_ViewDescription.GetFromSheetDescriptionID( SD_SmartDpC );


				// Reopen the box for the set.
				CCDButtonBox *pCDButtonBox = GetCDButtonBox( CD_Sets_Box, m_lSetSelectedRow, pclSheetDescriptionSets );

				if( NULL != pCDButtonBox )
				{
					pCDButtonBox->SetButtonState( CCDButtonBox::ButtonState::BoxOpened, true );
				}
			}

			// Unselect previous selection.
			pclSSheet->SetCheckBox( CD_Sets_CheckBox, m_lSetSelectedRow, _T(""), false, true );

			// Remove all sheets after the sets.
			m_ViewDescription.RemoveAllSheetAfter( SD_Sets );
		}

		// Save new set selection.
		SetCurrentSetSelected( pclCellDescriptionProduct );

		// Check checkbox.
		pclSSheet->SetCheckBox( CD_Sets_CheckBox, lRow, _T(""), true, true );
		m_lSetSelectedRow = lRow;

		// Delete Expand/Collapse rows button if exist.
		if( NULL != m_pCDBExpandCollapseRowsSets )
		{
			// Remark: 'm_pCDBExpandCollapseRowsSets' is set to NULL in method!
			DeleteExpandCollapseRowsButton( m_pCDBExpandCollapseRowsSets, pclSheetDescriptionSets );
		}

		// Create Expand/Collapse rows button if needed...
		LPARAM lProductSetTotalCount;
		pclSheetDescriptionSets->GetUserVariable( _SDUV_TOTALPRODUCT_COUNT, lProductSetTotalCount );

		if( lProductSetTotalCount > 1 )
		{
			m_pCDBExpandCollapseRowsSets = CreateExpandCollapseRowsButton( CD_Sets_FirstColumn, lRow, true, CCDButtonExpandCollapseRows::ButtonState::CollapseRow, 
					pclSheetDescriptionSets->GetFirstSelectableRow(), pclSheetDescriptionSets->GetLastSelectableRow( false ), pclSheetDescriptionSets );

			// Show button.
			if( NULL != m_pCDBExpandCollapseRowsSets )
			{
				m_pCDBExpandCollapseRowsSets->SetShowStatus( true );
			}
		}

		// Select the product set (just highlight background).
		pclSSheet->SelectOneRow( lRow, pclSheetDescriptionSets->GetSelectionFrom(), pclSheetDescriptionSets->GetSelectionTo() );

		// Fill corresponding connection set accessories.
		_FillSetContentAccessoryRows();


		CSheetDescription *pclSheetDescriptionSmartDp = m_ViewDescription.GetFromSheetDescriptionID( SD_SmartDpC );


		CCDButtonBox *pCDButtonBox = GetCDButtonBox( CD_Sets_Box, lRow, pclSheetDescriptionSets );

		if( NULL != pCDButtonBox )
		{
			pCDButtonBox->SetButtonState( CCDButtonBox::ButtonState::BoxClosed, true );
		}

		// Try to set the focus on the next table but do not lose the focus if no other table exist.
		CSheetDescription *pclNextSheetDescription = NULL;
		long lNewFocusedRow;
		bool bShiftPressed;

		if( DNFERRC_FocusOnNextGroupOK == DefineNextFocusedRow( pclSheetDescriptionSets, CD_Sets_FirstColumn, lRow, false, lNewFocusedRow, pclNextSheetDescription, bShiftPressed ) )
		{
			long lNewFocusedCol = pclNextSheetDescription->GetFirstParameterColumn();
			PrepareAndSetNewFocus( pclNextSheetDescription, lNewFocusedCol, lNewFocusedRow );
		}
		else
		{
			// Because we have changed current selection background in yellow, if there is no other sheet following current one, we
			// must set focus on current selection.
			PrepareAndSetNewFocus( pclSheetDescriptionSets, lColumn, lRow );
		}

		// Notify a change in the selection.
		SelectionHasChanged( IsSelectionReady() );
	}
}

void CRViewSSelSmartDpC::_FillSetsRows( CSelectedValve *pSelectedTAP )
{
	if( NULL == m_pclIndSelSmartDpCParams || NULL == m_pclIndSelSmartDpCParams->m_pclSelectSmartDpCList || NULL == m_pclIndSelSmartDpCParams->m_pTADB )
	{
		ASSERT_RETURN;
	}

	if( NULL == pSelectedTAP || NULL == dynamic_cast<CDB_SmartControlValve *>( pSelectedTAP->GetProductIDPtr().MP ) )
	{
		return;
	}

	CDB_SmartControlValve *pclSmartControlValve = (CDB_SmartControlValve *)( pSelectedTAP->GetProductIDPtr().MP );

	// Try to retrieve the smart differential pressure controller selected list.
	CSelectSmartDpCList *pclSmartDpCList = m_pclIndSelSmartDpCParams->m_pclSelectSmartDpCList;

	if( NULL == pclSmartDpCList )
	{
		return;
	}

	CRank *pSelectedDpSensorList = NULL;

	// Show only corresponding sets to the smart differential pressure controller for the set.
	CTableSet *pTableSet = pclSmartControlValve->GetTableSet();
	std::set<CDB_Set *> SmartDpCDpSensorSet;

	int iSetCount = 0;

	if( NULL != pTableSet )
	{
		iSetCount = pTableSet->FindCompatibleSet( &SmartDpCDpSensorSet, pSelectedTAP->GetProductIDPtr().ID, _T("") );
	
		if( iSetCount > 0 )
		{
			// Retrieve set list linked to current smart differential pressure controller.
			if( pclSmartDpCList->SelectDpSensorSet( m_pclIndSelSmartDpCParams, &SmartDpCDpSensorSet, pSelectedTAP ) > 0 )
			{
				pSelectedDpSensorList = pclSmartDpCList->GetDpSensorSelected();
			}
		}
	}
	else
	{
		// HYS-1992: Show only corresponding sets to the smart differential pressure controller for the set.
		IDPTR pTableID = pclSmartControlValve->GetDpSensorGroupIDPtr();

		if( NULL != pTableID.MP )
		{
			if( pclSmartDpCList->SelectDpSensor( m_pclIndSelSmartDpCParams, (CTable*)pTableID.MP, pSelectedTAP ) > 0 )
			{
				pSelectedDpSensorList = pclSmartDpCList->GetDpSensorSelected();
			}
		}
	}

	// If no Dp sensor exists
	if( NULL == pSelectedDpSensorList || 0 == pSelectedDpSensorList->GetCount() )
	{
		return;
	}

	// Call 'MultiSpreadBase' method to get a new SSheet.
	CSheetDescription *pclSheetDescriptionSets = CreateSSheet( SD_Sets );

	if( NULL == pclSheetDescriptionSets || NULL == pclSheetDescriptionSets->GetSSheetPointer() )
	{
		return;
	}

	CSSheet *pclSSheet = pclSheetDescriptionSets->GetSSheetPointer();

	pclSSheet->SetBool( SSB_REDRAW, FALSE );

	pclSSheet->Init();
	pclSSheet->SetBool( SSB_ALLOWDRAGDROP, FALSE );
	pclSSheet->SetBool( SSB_HORZSCROLLBAR, FALSE );
	pclSSheet->SetBool( SSB_VERTSCROLLBAR, FALSE );
	pclSSheet->SetBool( SSB_MOVEACTIVEONFOCUS, FALSE );
	pclSSheet->SetTextTip( SS_TT_STATUS_FLOATING, 250, NULL, _LIGHTYELLOW, _TAH_BLACK );

	// Set max rows.
	pclSSheet->SetMaxRows( RD_Sets_FirstAvailRow - 1 );

	// All cells are static by default and filled with _T("")
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	pclSSheet->FormatStaticText( -1, -1, -1, -1, _T("") );

	// Increase ROW height
	double dRowHeight = 12.75;

	pclSSheet->SetRowHeight( RD_Sets_FirstRow, dRowHeight * 0.5 );
	pclSSheet->SetRowHeight( RD_Sets_GroupName, dRowHeight * 1.5 );
	pclSSheet->SetRowHeight( RD_Sets_ColName, dRowHeight * 1.2 );

	// Initialize.
	pclSheetDescriptionSets->Init( 1, pclSSheet->GetMaxRows(), 1, CSheetDescription::SD_ParameterMode_Mono );
	pclSheetDescriptionSets->AddColumnInPixels( CD_Sets_FirstColumn, m_mapSSheetColumnWidth[SD_DpSensor][CD_Sets_FirstColumn] );
	pclSheetDescriptionSets->AddColumnInPixels( CD_Sets_Box, m_mapSSheetColumnWidth[SD_DpSensor][CD_Sets_Box] );
	pclSheetDescriptionSets->AddColumnInPixels( CD_Sets_CheckBox, m_mapSSheetColumnWidth[SD_DpSensor][CD_Sets_CheckBox] );
	pclSheetDescriptionSets->AddColumnInPixels( CD_Sets_Name, m_mapSSheetColumnWidth[SD_DpSensor][CD_Sets_Name] );
	pclSheetDescriptionSets->AddColumnInPixels( CD_Sets_Description, m_mapSSheetColumnWidth[SD_DpSensor][CD_Sets_Description] );
	pclSheetDescriptionSets->AddColumnInPixels( CD_Sets_Pointer, m_mapSSheetColumnWidth[SD_DpSensor][CD_Sets_Pointer] );

	// Set in which column parameter must be saved.
	pclSheetDescriptionSets->AddParameterColumn( CD_Sets_Pointer );

	// Set the focus column.
	pclSheetDescriptionSets->SetActiveColumn( CD_Sets_Name );

	// Set selectable rows.
	pclSheetDescriptionSets->SetSelectableRangeRow( RD_Sets_FirstAvailRow, pclSSheet->GetMaxRows() );

	// Set range for selection.
	pclSheetDescriptionSets->SetFocusColumnRange( CD_Sets_CheckBox, CD_Sets_Description );

	// Add the main title.
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)GetTitleBackgroundColor( pclSheetDescriptionSets ) );

	pclSheetDescriptionSets->SetUserVariable( _SDUV_TITLEFORECOLOR, ( pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor ) ) );
	pclSheetDescriptionSets->SetUserVariable( _SDUV_TITLEBACKCOLOR, ( pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor ) ) );

	pclSSheet->AddCellSpanW( CD_Sets_Box, RD_Sets_GroupName, CD_Sets_Pointer - CD_Sets_Box, 1 );
	pclSSheet->SetStaticText( CD_Sets_Box, RD_Sets_GroupName, IDS_RVIEWSSELSMARDPC_SETSGROUP );

	// Columns title.
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

	pclSSheet->SetStaticText( CD_Sets_Name, RD_Sets_ColName, IDS_SSHEETSSELCV_ACTRNAME );
	pclSSheet->SetStaticText( CD_Sets_Description, RD_Sets_ColName, IDS_RVIEWSSELSMARDPC_DESCRIPTION );

	// Draw black line below column header.
	pclSSheet->SetCellBorder( CD_Sets_CheckBox, RD_Sets_ColName, CD_Sets_Pointer - 1, RD_Sets_ColName, true, SS_BORDERTYPE_BOTTOM );

	// Fill set list.
	long lRow = RD_Sets_FirstAvailRow;
	long lSetCount = 0;
	pclSheetDescriptionSets->SetUserVariable( _SDUV_TOTALPRODUCT_COUNT, 0 );

	bool bAtLeastOneDescriptionSet = false;
	CString str;
	LPARAM lpItemData = 0;

	for( BOOL bContinue = pSelectedDpSensorList->GetFirst( str, lpItemData ); TRUE == bContinue; bContinue = pSelectedDpSensorList->GetNext( str, lpItemData ) ) 
	{
		CDB_Product *pclProductSet = dynamic_cast<CDB_Product *>( (CData *)lpItemData );

		// Add this row.
		// 'true' to specify that this row can be selected.
		pclSheetDescriptionSets->AddRows( 1, true );

		// Add a box.
		CCDButtonBox *pCDButtonBox = CreateCellDescriptionBox( CD_Sets_Box, lRow, true, CCDButtonBox::ButtonState::BoxOpened, pclSheetDescriptionSets );

		if( NULL != pCDButtonBox )
		{
			pCDButtonBox->ApplyInternalChange();
		}

		// Add checkbox.
		pclSSheet->SetCheckBox( CD_Sets_CheckBox, lRow, _T(""), false, true );

		// Add the name.
		CString strName = pclProductSet->GetName();
		CDB_DpSensor *pclDpSensorSet = NULL;

		if( NULL != dynamic_cast<CDB_DpSensor *>( pclProductSet ) )
		{
			pclDpSensorSet = (CDB_DpSensor *)( pclProductSet );
			strName = pclDpSensorSet->GetFullName();
		}
		
		pclSSheet->SetStaticText( CD_Sets_Name, lRow, strName );

		// If we are with a Dp sensor set (And not a connection set) we check if there is an error.
		if( NULL != pclDpSensorSet )
		{
			CDB_DpSensor *pclDpSensor = _GetDpSensorInDpSensorSet( pclDpSensorSet );

			if( NULL != pclDpSensor )
			{
				if( true == _IsDpSensorError( pclDpSensor ) )
				{
					pclSSheet->SetForeColor( CD_Sets_Name, lRow, _RED );
				}
			}
		}

		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

		// Add the description.
		if( false == CString( pclProductSet->GetComment() ).IsEmpty() )
		{
			pclSSheet->SetStaticText( CD_Sets_Description, lRow, pclProductSet->GetComment() );
			bAtLeastOneDescriptionSet = true;
		}

		// Save parameter.
		CreateCellDescriptionProduct( pclSheetDescriptionSets->GetFirstParameterColumn(), lRow, (LPARAM)pclProductSet, pclSheetDescriptionSets );

		lRow++;
		lSetCount++;
	}

	long lLastDataRow = lRow - 1;

	if( false == bAtLeastOneDescriptionSet )
	{
		pclSSheet->ShowCol( CD_Sets_Description, FALSE );
	}

	// Save the total set number.
	pclSheetDescriptionSets->SetUserVariable( _SDUV_TOTALPRODUCT_COUNT, lSetCount );

	// Set that there is no selection at now.
	SetCurrentSetSelected( NULL );

	pclSSheet->SetCellBorder( CD_Sets_CheckBox, lLastDataRow, CD_Sets_Pointer - 1, lLastDataRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );

	pclSSheet->SetBool( SSB_REDRAW, TRUE );

	// Add possibility to change column size.
	// Remark: place this code after any initialization (like adding, removing, resizing, hiding/showing columns).
	CResizingColumnInfo *pclResizingColumnInfo = pclSheetDescriptionSets->GetResizingColumnInfo();
	pclResizingColumnInfo->ActivateFeature( true, false, RD_Sets_FirstAvailRow, lLastDataRow );
	pclResizingColumnInfo->AddRangeColumn( CD_Sets_Name, CD_Sets_Description, RD_Sets_ColName, RD_Sets_ColName );

	// Add possibility to click on title group to reset column width.
	CreateTitleGroupButtons( CD_Sets_CheckBox, CD_Sets_Description, RD_Sets_GroupName, pclSheetDescriptionSets );
}

void CRViewSSelSmartDpC::_FillSetContentAccessoryRows()
{
	if( NULL == m_pclIndSelSmartDpCParams || NULL == m_pclIndSelSmartDpCParams->m_pTADB )
	{
		ASSERT_RETURN;
	}

	// Retrieve the current selected smart differential pressure controller.
	CRank rList;
	int iCount = _GetSetContentAccessoryList( &rList );

	if( 0 == iCount )
	{
		return;
	}

	// Call 'MultiSpreadBase' method to get a new SSheet.
	CSheetDescription *pclSDAccessory = CreateSSheet( SD_SetContent );

	if( NULL == pclSDAccessory || NULL == pclSDAccessory->GetSSheetPointer() )
	{
		return;
	}

	CDB_RuledTable *pclRuledTable = _GetSetContentRuledTabled();

	if( NULL == pclRuledTable )
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

	// Get sheet description of smart differential pressure controller to retrieve width.
	CSheetDescription *pclSDSmartDpC = m_ViewDescription.GetFromSheetDescriptionID( SD_SmartDpC );
	// It's absolutely not normal to have this pointer NULL.
	ASSERT( NULL != pclSDSmartDpC );

	// Take SSheet size and client area size and determine the right width to take in count.
	CRect rectSize = pclSDSmartDpC->GetSSheetPointer()->GetSheetSizeInPixels();

	// Prepare first column width (to match smart differential pressure controller sheet).
	long lFirstColumnWidth = pclSDSmartDpC->GetSSheetPointer()->GetColWidthInPixelsW( CD_SmartDpC_FirstColumn );

	// Do some correction in regards to some columns.
	long lTotalWidth = rectSize.Width() - lFirstColumnWidth;
	lTotalWidth -= pclSDSmartDpC->GetSSheetPointer()->GetColWidthInPixelsW( CD_SmartDpC_Pointer );

	// Try to create 2 columns in just the middle of smart differential pressure controller sheet.
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

	pclSDAccessory->SetUserVariable( _SDUV_TITLEFORECOLOR, (pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor )) );
	pclSDAccessory->SetUserVariable( _SDUV_TITLEBACKCOLOR, (pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor )) );

	pclSSheet->AddCellSpanW( CD_Accessory_Left, RD_Accessory_GroupName, CD_Accessory_LastColumn - CD_Accessory_Left, 1 );
	pclSSheet->SetStaticText( CD_Accessory_Left, RD_Accessory_GroupName, IDS_RVIEWSSELSMARDPC_SETCONTENTACCGROUP );

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
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

		CDB_Product *pclAccessory = (CDB_Product *)lparam;
		VERIFY( NULL != pclAccessory );

		if( false == pclAccessory->IsAnAccessory() )
		{
			bContinue = rList.GetNext( str, lparam );
			continue;
		}

		// Create checkbox accessory.
		CString strName = pclAccessory->GetName();
		CDB_DpSensor *pclDpSensor = dynamic_cast<CDB_DpSensor *>( pclAccessory );

		if( NULL != pclDpSensor )
		{
			strName = ( (CDB_DpSensor *)( pclAccessory ) )->GetFullName();
			strName += _T("; ") + pclDpSensor->GetTempRange( true );
		}

		CCDBCheckboxAccessory *pCheckbox = CreateCheckboxAccessory( lLeftOrRight, lRow, false, true, strName, pclAccessory, pclRuledTable, 
				&m_vecSetContentAccessories, pclSDAccessory );

		if( NULL != pCheckbox )
		{
			pCheckbox->ApplyInternalChange();
		}

		if( true == _IsDpSensorError( pclDpSensor ) )
		{
			// Manually force the line with the accessory name (Because the line is already displayed when calling the "CreateCheckboxAccessory" method
			// above.
			pclSSheet->SetForeColor( lLeftOrRight, lRow, (LPARAM)_RED );

			// Set in red for the accessory description.
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_RED );
		}

		// Description.
		CString strDescription = pclAccessory->GetComment();

		if( NULL != pclDpSensor )
		{
			CString strBurstPressure = WriteCUDouble( _U_DIFFPRESS, pclDpSensor->GetBurstPressure(), true );
			FormatString( strDescription, pclDpSensor->GetComment(), strBurstPressure );
		}
		
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleEllipses, (LPARAM)TRUE );
		pclSSheet->SetStaticText( lLeftOrRight, lRow + 1, strDescription );
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

		// Select automatically set accessories
		VerifyCheckboxAccessories( pclAccessory, true, &m_vecSetContentAccessories );
	}

	VerifyCheckboxAccessories( NULL, false, &m_vecSetContentAccessories );

	m_pCDBExpandCollapseGroupConnectionSetAcc = CreateExpandCollapseGroupButton( CD_Accessory_FirstColumn, RD_Accessory_GroupName, true, 
			CCDButtonExpandCollapseGroup::ButtonState::CollapseRow, pclSDAccessory->GetFirstSelectableRow(), pclSDAccessory->GetLastSelectableRow( false ), pclSDAccessory );

	// Show button.
	if( NULL != m_pCDBExpandCollapseGroupConnectionSetAcc )
	{
		m_pCDBExpandCollapseGroupConnectionSetAcc->SetShowStatus( true );
	}

	pclSSheet->SetCellBorder( CD_Accessory_Left, lRow, CD_Accessory_LastColumn - 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );

	pclSSheet->SetBool( SSB_REDRAW, TRUE );
}

CDB_RuledTable *CRViewSSelSmartDpC::_GetSetContentRuledTabled( CDB_TAProduct *pclSmartDpC, CDB_Product *pclProductSet )
{
	if( NULL == pclSmartDpC )
	{
		pclSmartDpC = GetCurrentSmartDpCSelected();
	}

	if( NULL == pclProductSet )
	{
		pclProductSet = GetCurrentProductSetSelected();
	}

	if( NULL == pclSmartDpC || NULL == pclProductSet )
	{
		return 0;
	}

	CTableSet *pTableSet = pclSmartDpC->GetTableSet();
	CDB_Set *pclSmartDpCDpSensorSet = NULL;
	CDB_RuledTable* pReturned = NULL;
	if( NULL != pTableSet )
	{
		pclSmartDpCDpSensorSet = pTableSet->FindCompatibleSet( pclSmartDpC->GetIDPtr().ID, pclProductSet->GetIDPtr().ID );
		if( NULL == pclSmartDpCDpSensorSet )
		{
			return NULL;
		}
		pReturned = (CDB_RuledTable*)(pclSmartDpCDpSensorSet->GetAccGroupIDPtr().MP);
	}
	else
	{
		// HYS-1992: Smart DpC is not a set.
		IDPTR idptr = pclProductSet->GetAccessoriesGroupIDPtr();
		if( NULL == idptr.MP )
		{
			return NULL;
		}
		pReturned = (CDB_RuledTable*)(idptr.MP);
	}


	// Sanity check.
	return pReturned;
}

int CRViewSSelSmartDpC::_GetSetContentAccessoryList( CRank *pclList, CDB_TAProduct *pclSmartDpC, CDB_Product *pclProductSet )
{
	if( NULL == pclList || NULL == m_pclIndSelSmartDpCParams )
	{
		return 0;
	}

	CDB_RuledTable *pclRuledTable = _GetSetContentRuledTabled( pclSmartDpC, pclProductSet );

	if( NULL == pclRuledTable )
	{
		return 0;
	}

	// Not sorted ! (To keep Dp sensor at the first position if exist).
	return m_pclIndSelSmartDpCParams->m_pTADB->GetAccessories( pclList, pclRuledTable, m_pclIndSelSmartDpCParams->m_eFilterSelection );
}

CDB_DpSensor *CRViewSSelSmartDpC::_GetDpSensorInDpSensorSet( CDB_DpSensor *pclDpSensorSet )
{
	if( NULL == pclDpSensorSet )
	{
		return NULL;
	}

	CDB_DpSensor *pclDpSensor = NULL;
	CRank rList;
	
	if( _GetSetContentAccessoryList( &rList, NULL, pclDpSensorSet ) > 0 )
	{
		CString str;
		LPARAM lparam;

		for( BOOL bContinue = rList.GetFirst( str, lparam ); TRUE == bContinue; bContinue = rList.GetNext( str, lparam ) )
		{
			pclDpSensor = dynamic_cast<CDB_DpSensor *>( (CData *)lparam );

			if( NULL != pclDpSensor )
			{
				break;
			}
		}
	}

	return pclDpSensor;
}

bool CRViewSSelSmartDpC::_IsDpSensorError( CDB_DpSensor *pclDpSensor )
{
	if( NULL == pclDpSensor )
	{
		return false;
	}
	
	bool bError = false;

	if( true == m_pclIndSelSmartDpCParams->m_bIsGroupDpbranchChecked && m_pclIndSelSmartDpCParams->m_dDpBranch > 0.0
			&& ( m_pclIndSelSmartDpCParams->m_dDpBranch < pclDpSensor->GetMinMeasurableDp() 
				|| m_pclIndSelSmartDpCParams->m_dDpBranch > pclDpSensor->GetMaxMeasurableDp() ) )
	{
		bError = true;
	}
	else if( true == m_pclIndSelSmartDpCParams->m_bIsGroupDpbranchChecked && m_pclIndSelSmartDpCParams->m_dDpBranch > 0.0
			&& m_pclIndSelSmartDpCParams->m_dDpBranch > pclDpSensor->GetBurstPressure() )
	{
		bError = true;
	}
	else if( m_pclIndSelSmartDpCParams->m_WC.GetTemp() < pclDpSensor->GetTmin() 
			|| m_pclIndSelSmartDpCParams->m_WC.GetTemp() > pclDpSensor->GetTmax() )
	{
		bError = true;
	}

	return bError;
}

long CRViewSSelSmartDpC::_GetRowOfEditedDpSensor( CSheetDescription *pclSheetDescriptionDpSensor, CDB_Product *pclEditedDpSensor )
{
	if( NULL == pclSheetDescriptionDpSensor || NULL == pclEditedDpSensor )
	{
		return -1;
	}

	// Retrieve list of all products in 'pclSheetDescription'.
	CSheetDescription::vecCellDescription vecCellDescriptionList;
	pclSheetDescriptionDpSensor->GetCellDescriptionList( vecCellDescriptionList, RVSCellDescription::CD_Product );

	// Run all objects.
	long lReturnValue = -1;
	CSheetDescription::vecCellDescriptionIter vecIter = vecCellDescriptionList.begin();

	while( vecIter != vecCellDescriptionList.end() )
	{
		CCellDescriptionProduct *pCDProduct = dynamic_cast<CCellDescriptionProduct *>( *vecIter );

		if( NULL != pCDProduct && 0 != pCDProduct->GetProduct() && pclEditedDpSensor == (CDB_Product *)pCDProduct->GetProduct() )
		{
			lReturnValue = pCDProduct->GetCellPosition().y;
			break;
		}

		vecIter++;
	}

	return lReturnValue;
}

long CRViewSSelSmartDpC::_GetRowOfEditedSet( CSheetDescription *pclSheetDescriptionSets, CDB_Product *pclEditedProductSet )
{
	if( NULL == pclSheetDescriptionSets || NULL == pclEditedProductSet )
	{
		return -1;
	}

	// Retrieve list of all products in 'pclSheetDescription'.
	CSheetDescription::vecCellDescription vecCellDescriptionList;
	pclSheetDescriptionSets->GetCellDescriptionList( vecCellDescriptionList, RVSCellDescription::CD_Product );

	// Run all objects.
	long lReturnValue = -1;
	CSheetDescription::vecCellDescriptionIter vecIter = vecCellDescriptionList.begin();

	while( vecIter != vecCellDescriptionList.end() )
	{
		CCellDescriptionProduct *pCDProduct = dynamic_cast<CCellDescriptionProduct *>( *vecIter );

		if( NULL != pCDProduct && 0 != pCDProduct->GetProduct() && pclEditedProductSet == (CDB_Product *)pCDProduct->GetProduct() )
		{
			lReturnValue = pCDProduct->GetCellPosition().y;
			break;
		}

		vecIter++;
	}

	return lReturnValue;
}

void CRViewSSelSmartDpC::_ReadAllColumnWidth( CDS_IndSelParameter *pclIndSelParameter )
{
	if( NULL == pclIndSelParameter )
	{
		ASSERT_RETURN;
	}

	std::map<UINT, short> mapSDIDVersion;
	mapSDIDVersion[CW_RVIEWSSELSMARTVALVE_SMARTDPC_SHEETID] = CW_RVIEWSSELSMARTVALVE_SMARTDPC_VERSION;
	mapSDIDVersion[CW_RVIEWSSELSMARTVALVE_DPSENSOR_SHEETID] = CW_RVIEWSSELSMARTVALVE_DPSENSOR_VERSION;
	mapSDIDVersion[CW_RVIEWSSELSMARTVALVE_SETS_SHEETID] = CW_RVIEWSSELSMARTVALVE_SETS_VERSION;

	// Container window sheet ID to sheetdescription of this rightview.
	std::map<UINT, short> mapCWtoRW;
	mapCWtoRW[CW_RVIEWSSELSMARTVALVE_SMARTDPC_SHEETID] = SD_SmartDpC;
	mapCWtoRW[CW_RVIEWSSELSMARTVALVE_DPSENSOR_SHEETID] = SD_DpSensor;
	mapCWtoRW[CW_RVIEWSSELSMARTVALVE_SETS_SHEETID] = SD_Sets;

	// By default and before reading registry saved column width force reset column width for all sheets.
	for( auto &iter : mapCWtoRW )
	{
		ResetColumnWidth( iter.second );
	}

	// Access to the 'RViewSSelSmartDpC' group.
	CDS_IndSelParameter::CCWWindow *pclCWWindow = pclIndSelParameter->GetWindowColumnWidth( CW_WINDOWID_INDSELSMARTDPC, true );

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

void CRViewSSelSmartDpC::_WriteAllColumnWidth( CDS_IndSelParameter *pclIndSelParameter )
{
	if( NULL == pclIndSelParameter )
	{
		ASSERT_RETURN;
	}

	std::map<UINT, short> mapSDIDVersion;
	mapSDIDVersion[CW_RVIEWSSELSMARTVALVE_SMARTDPC_SHEETID] = CW_RVIEWSSELSMARTVALVE_SMARTDPC_VERSION;
	mapSDIDVersion[CW_RVIEWSSELSMARTVALVE_DPSENSOR_SHEETID] = CW_RVIEWSSELSMARTVALVE_DPSENSOR_VERSION;
	mapSDIDVersion[CW_RVIEWSSELSMARTVALVE_SETS_SHEETID] = CW_RVIEWSSELSMARTVALVE_SETS_VERSION;

	// Container window sheet ID to sheet description of this right view.
	std::map<UINT, short> mapCWtoRW;
	mapCWtoRW[CW_RVIEWSSELSMARTVALVE_SMARTDPC_SHEETID] = SD_SmartDpC;
	mapCWtoRW[CW_RVIEWSSELSMARTVALVE_DPSENSOR_SHEETID] = SD_DpSensor;
	mapCWtoRW[CW_RVIEWSSELSMARTVALVE_SETS_SHEETID] = SD_Sets;

	// Access to the 'RViewSSelSmartDpC' group.
	CDS_IndSelParameter::CCWWindow *pclCWWindow = pclIndSelParameter->GetWindowColumnWidth( CW_WINDOWID_INDSELSMARTDPC, true );

	for( std::map<UINT, short>::iterator iter = mapSDIDVersion.begin(); iter != mapSDIDVersion.end(); iter++ )
	{
		CDS_IndSelParameter::CCWSheet *pclCWSheet = pclCWWindow->GetCWSheet( iter->first, true );
		pclCWSheet->SetVersion( iter->second );
		pclCWSheet->GetMap() = m_mapSSheetColumnWidth[mapCWtoRW[iter->first]];
	}
}
