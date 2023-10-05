#include "stdafx.h"
#include "afxctl.h"

#include "TASelect.h"
#include "MainFrm.h"
#include "DataStruct.h"
#include "Hydronic.h"

#include "DlgLeftTabSelManager.h"
#include "DlgInfoSSelDpC.h"

#include "RViewSSelSS.h"
#include "RViewSSel6WayValve.h"

CRViewSSel6WayValve *pRViewSSel6WayValve = NULL;
CRViewSSel6WayValve::CRViewSSel6WayValve() : CRViewSSelSS( CMainFrame::RightViewList::eRVSSel6WayValve, false )
{
	m_pclIndSel6WayValveParams = NULL;
	_Init();
	pRViewSSel6WayValve = this;
}

CRViewSSel6WayValve::~CRViewSSel6WayValve()
{
	pRViewSSel6WayValve = NULL;
}

void CRViewSSel6WayValve::Reset()
{
	_Init();
	CRViewSSelSS::Reset();
}

void CRViewSSel6WayValve::Suggest( CProductSelelectionParameters *pclProductSelectionParameters, LPARAM lpParam )
{
	CRViewSSelSS::Suggest( pclProductSelectionParameters, lpParam );

	if( NULL == pclProductSelectionParameters || NULL == dynamic_cast<CIndSel6WayValveParams*>( pclProductSelectionParameters ) )
	{
		ASSERT_RETURN;
	}

	m_pclIndSel6WayValveParams = dynamic_cast<CIndSel6WayValveParams*>( pclProductSelectionParameters );
	
	// To remove all current displayed sheets.
	Reset();

	BeginWaitCursor();
	CWnd::SetRedraw( FALSE );

	long l6WayValveRowSelected = -1;
	CDS_SSel6WayValve *pclSelected6WayValve = NULL;

	if( NULL != m_pclIndSel6WayValveParams->m_SelIDPtr.MP )
	{
		pclSelected6WayValve = reinterpret_cast<CDS_SSel6WayValve *>( (CData*)( m_pclIndSel6WayValveParams->m_SelIDPtr.MP ) );

		if( NULL == pclSelected6WayValve )
		{
			ASSERT( 0 );
		}

		l6WayValveRowSelected = _Fill6WayValveRows( pclSelected6WayValve );
	}
	else
	{
		l6WayValveRowSelected = _Fill6WayValveRows();
	}

	// Verify if sheet description has been well created.
	CSheetDescription *pclSheetDescription6WayValve = m_ViewDescription.GetFromSheetDescriptionID( SD_6WayValve );

	if( NULL != pclSheetDescription6WayValve && NULL != pclSheetDescription6WayValve->GetSSheetPointer() )
	{
		CSSheet *pclSSheet = pclSheetDescription6WayValve->GetSSheetPointer();

		if( l6WayValveRowSelected > -1 && NULL != pclSelected6WayValve )
		{
			CCellDescriptionProduct *pclCD6WayValve = FindCDProduct( l6WayValveRowSelected, (LPARAM)( dynamic_cast<CDB_TAProduct *>( pclSelected6WayValve->Get6WayValveIDPtr().MP ) ),
					pclSheetDescription6WayValve );

			if( NULL != pclCD6WayValve && NULL != pclCD6WayValve->GetProduct() )
			{
				// If we are in edition mode we simulate a click on the product.
				OnClickProduct( pclSheetDescription6WayValve, pclCD6WayValve, pclSheetDescription6WayValve->GetActiveColumn(), l6WayValveRowSelected );

				// Allow to check if we need to change the 'Show all priorities' button or not.
				CheckShowAllPrioritiesButtonState( pclSheetDescription6WayValve, l6WayValveRowSelected );

				// Verify accessories on the 6-way valve.
				CAccessoryList *pcl6WayValveAccessoryList = pclSelected6WayValve->Get6WayValveAccessoryList();
				CAccessoryList::AccessoryItem rAccessoryItem = pcl6WayValveAccessoryList->GetFirst( CAccessoryList::_AT_Accessory );

				while( rAccessoryItem.IDPtr.MP != NULL )
				{
					VerifyCheckboxAccessories( (CDB_Product *)rAccessoryItem.IDPtr.MP, true, &m_mapVariables[SD_Family6WayValve][BothSide].m_vecValveAccessoryList );
					rAccessoryItem = pcl6WayValveAccessoryList->GetNext();
				}

				// Verify if user has selected an actuator with the 6-way valve.
				IDPTR SixWayValveActuatorIDPtr = pclSelected6WayValve->Get6WayValveActuatorIDPtr();
				CSheetDescription *pclSheetDescription6WayValveActuator = m_ViewDescription.GetFromSheetDescriptionID( SD_6WayValveActuator );

				if( _NULL_IDPTR != SixWayValveActuatorIDPtr && NULL != SixWayValveActuatorIDPtr.MP && NULL != pclSheetDescription6WayValveActuator )
				{
					CDB_Actuator *pclEdited6WayValveActuator = dynamic_cast<CDB_Actuator *>( SixWayValveActuatorIDPtr.MP );

					if( NULL != pclEdited6WayValveActuator )
					{
						// Find row number where is the	actuator selected with the 6-way valve.
						long l6WayValveActuatorRowSelected = _GetRowOfEditedActuator( pclSheetDescription6WayValveActuator, pclEdited6WayValveActuator );

						if( l6WayValveActuatorRowSelected != -1 )
						{
							CCellDescriptionProduct *pclCD6WayValveActuator = FindCDProduct( l6WayValveActuatorRowSelected, (LPARAM)pclEdited6WayValveActuator, pclSheetDescription6WayValveActuator );

							if( NULL != pclCD6WayValveActuator && NULL != pclCD6WayValveActuator->GetProduct() )
							{
								// HYS-1877: For edition mode click on product only if it is not.
								if( NULL == GetCurrent6WayValveActuatorSelected() )
								{
									// Simulate a click on the edited actuator.
									OnClickProduct( pclSheetDescription6WayValveActuator, pclCD6WayValveActuator, pclSheetDescription6WayValveActuator->GetActiveColumn(), l6WayValveActuatorRowSelected );
								}

								// Verify accessories on actuator.
								CAccessoryList *pcl6WayValveActuatorAccessoryList = pclSelected6WayValve->Get6WayValveActuatorAccessoryList();
								rAccessoryItem = pcl6WayValveActuatorAccessoryList->GetFirst();

								while( rAccessoryItem.IDPtr.MP != NULL )
								{
									VerifyCheckboxAccessories( (CDB_Product *)rAccessoryItem.IDPtr.MP, true, &m_mapVariables[SD_Family6WayValve][BothSide].m_vecValveActuatorAccessoryList );
									rAccessoryItem = pcl6WayValveActuatorAccessoryList->GetNext();
								}
							}
						}
					}
				}

				if( e6Way_EQMControl == m_pclIndSel6WayValveParams->m_e6WayValveSelectionMode )
				{
					_SuggestPIBCVHelper( BothSide, pclSelected6WayValve );
				}
				else if( e6Way_OnOffControlWithPIBCV == m_pclIndSel6WayValveParams->m_e6WayValveSelectionMode )
				{
					_SuggestPIBCVHelper( CoolingSide, pclSelected6WayValve );
					_SuggestPIBCVHelper( HeatingSide, pclSelected6WayValve );
				}
				else if( e6Way_OnOffControlWithSTAD == m_pclIndSel6WayValveParams->m_e6WayValveSelectionMode )
				{
					_SuggestBVHelper( CoolingSide, pclSelected6WayValve );
					_SuggestBVHelper( HeatingSide, pclSelected6WayValve );
				}
			}
		}
		else
		{
			// HYS-1318 : We control the 6-way valve, 6-way valve actuator and PIBCV
			_VerifyOneProductAndClick( SideDefinition::BothSide, SD_Family6WayValve, SD_TypeValve );
			
			// Define the first product row as the active cell and set a visual focus.
			PrepareAndSetNewFocus( pclSheetDescription6WayValve, CD_6WayValve_Name, RD_6WayValve_FirstAvailRow, 0 );
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

void CRViewSSel6WayValve::FillInSelected( CDS_SelProd *pclSelectedProductToFill )
{
	if( NULL == m_pclIndSel6WayValveParams )
	{
		ASSERT_RETURN;
	}

	CDS_SSel6WayValve *pclSelected6WayValveToFill = dynamic_cast<CDS_SSel6WayValve *>( pclSelectedProductToFill );

	if( NULL == pclSelected6WayValveToFill )
	{
		ASSERT_RETURN;
	}

	// Clear previous selected 6-way valve accessories if we are not in edition mode.
	CAccessoryList *pcl6WayValveAccessoryToFill = pclSelected6WayValveToFill->Get6WayValveAccessoryList();

	// Clear previous selected actuator accessories selected with a 6-way valve if we are not in edition mode.
	CAccessoryList *pcl6WayValveActuatorAccessoryToFill = pclSelected6WayValveToFill->Get6WayValveActuatorAccessoryList();

	bool bIsEditionMode = false;

	if( false == m_pclIndSel6WayValveParams->m_bEditModeRunning )
	{
		pcl6WayValveAccessoryToFill->Clear();
		pcl6WayValveActuatorAccessoryToFill->Clear();
	}
	else
	{
		bIsEditionMode = true;
	}

	// Fill data for 6-way valve.
	CDB_6WayValve *pclCurrent6WayValveSelected = GetCurrent6WayValveSelected();

	if( NULL != pclCurrent6WayValveSelected && NULL != m_pclIndSel6WayValveParams->m_pclSelect6WayValveList )
	{
		// Save the 6-way valve IDPtr.
		pclSelected6WayValveToFill->Set6WayValveIDPtr( pclCurrent6WayValveSelected->GetIDPtr() );

		// If we are not in Edition mode we fill the accessory list. In edition mode we just update accessory list.
		if( false == bIsEditionMode )
		{
			// Retrieve selected accessory and add it.
			for( vecCDCAccessoryListIter vecIter = m_mapVariables[SD_Family6WayValve][BothSide].m_vecValveAccessoryList.begin(); vecIter != m_mapVariables[SD_Family6WayValve][BothSide].m_vecValveAccessoryList.end(); vecIter++ )
			{
				CCDBCheckboxAccessory *pCDBCheckboxAccessory = *vecIter;

				if( NULL != pCDBCheckboxAccessory && true == pCDBCheckboxAccessory->GetCheckStatus() && NULL != pCDBCheckboxAccessory->GetAccessoryPointer() )
				{
					pcl6WayValveAccessoryToFill->Add( pCDBCheckboxAccessory->GetAccessoryPointer()->GetIDPtr(), CAccessoryList::_AT_Accessory,
						pCDBCheckboxAccessory->GetRuledTable() );
				}
			}
		}
		else
		{
			UpdateAccessoryList( m_mapVariables[SD_Family6WayValve][BothSide].m_vecValveAccessoryList, pcl6WayValveAccessoryToFill, CAccessoryList::_AT_Accessory );
		}

		// Selected pipe informations.
		if( NULL != m_pclIndSel6WayValveParams->m_pclSelect6WayValveList->GetSelectPipeList() )
		{
			m_pclIndSel6WayValveParams->m_pclSelect6WayValveList->GetSelectPipeList()->GetMatchingPipe( pclCurrent6WayValveSelected->GetSizeKey(), 
					*pclSelected6WayValveToFill->GetpSelPipe() );
		}
	}

	// Fill data for actuator selected with the 6-way valve.
	CDB_Actuator *pclCurrentSelected6WayValveActuator = GetCurrent6WayValveActuatorSelected();

	if( NULL != pclCurrentSelected6WayValveActuator )
	{
		pclSelected6WayValveToFill->Set6WayValveActuatorIDPtr( pclCurrentSelected6WayValveActuator->GetIDPtr() );
		
		if( false == bIsEditionMode )
		{
			// Retrieve the selected actuator accessories selected with the 6-way valve and add it.
			for( vecCDCAccessoryListIter vecIter = m_mapVariables[SD_Family6WayValve][BothSide].m_vecValveActuatorAccessoryList.begin(); vecIter != m_mapVariables[SD_Family6WayValve][BothSide].m_vecValveActuatorAccessoryList.end(); vecIter++ )
			{
				CCDBCheckboxAccessory *pCDBCheckbox6WayValveActuatorAccessory = *vecIter;

				if( NULL != pCDBCheckbox6WayValveActuatorAccessory && true == pCDBCheckbox6WayValveActuatorAccessory->GetCheckStatus() && NULL != pCDBCheckbox6WayValveActuatorAccessory->GetAccessoryPointer() )
				{
					pcl6WayValveActuatorAccessoryToFill->Add( pCDBCheckbox6WayValveActuatorAccessory->GetAccessoryPointer()->GetIDPtr(), CAccessoryList::_AT_Accessory,
							pCDBCheckbox6WayValveActuatorAccessory->GetRuledTable() );
				}
			}
		}
		else
		{
			// For actuator accessories.
			UpdateAccessoryList( m_mapVariables[SD_Family6WayValve][BothSide].m_vecValveActuatorAccessoryList, pcl6WayValveActuatorAccessoryToFill, CAccessoryList::_AT_Accessory );
		}
	}

	if( e6Way_EQMControl == m_pclIndSel6WayValveParams->m_e6WayValveSelectionMode )
	{
		_FillInSelectedPIBCVHelper( BothSide, pclSelected6WayValveToFill, bIsEditionMode );
	}
	else if( e6Way_OnOffControlWithPIBCV == m_pclIndSel6WayValveParams->m_e6WayValveSelectionMode )
	{
		_FillInSelectedPIBCVHelper( CoolingSide, pclSelected6WayValveToFill, bIsEditionMode );
		_FillInSelectedPIBCVHelper( HeatingSide, pclSelected6WayValveToFill, bIsEditionMode );
	}
	else if( e6Way_OnOffControlWithSTAD == m_pclIndSel6WayValveParams->m_e6WayValveSelectionMode )
	{
		_FillInSelectedBVHelper( CoolingSide, pclSelected6WayValveToFill, bIsEditionMode );
		_FillInSelectedBVHelper( HeatingSide, pclSelected6WayValveToFill, bIsEditionMode );
	}
}

void CRViewSSel6WayValve::OnNewDocument( CDS_IndSelParameter *pclIndSelParameter )
{
	_ReadAllColumnWidth( pclIndSelParameter );
}

void CRViewSSel6WayValve::SaveSelectionParameters( CDS_IndSelParameter *pclIndSelParameter )
{
	_WriteAllColumnWidth( pclIndSelParameter );
}

CDB_6WayValve *CRViewSSel6WayValve::GetCurrent6WayValveSelected()
{
	CDB_6WayValve *pclCurrent6WayValveSelected = NULL;
	CSheetDescription *pclSheetDescription6WayValve = m_ViewDescription.GetFromSheetDescriptionID( SD_6WayValve );

	if( NULL != pclSheetDescription6WayValve )
	{
		// Retrieve the current selected 6-way valve.
		CCellDescriptionProduct *pclCDCurrent6WayValveSelected = NULL;
		LPARAM lpPointer;

		if( true == pclSheetDescription6WayValve->GetUserVariable( _SDUV_SELECTEDPRODUCT, lpPointer ) )
		{
			pclCDCurrent6WayValveSelected = (CCellDescriptionProduct *)lpPointer;
		}

		if( NULL != pclCDCurrent6WayValveSelected && NULL != pclCDCurrent6WayValveSelected->GetProduct() )
		{
			pclCurrent6WayValveSelected = dynamic_cast<CDB_6WayValve *>( (CData *)pclCDCurrent6WayValveSelected->GetProduct() );
		}
	}

	return pclCurrent6WayValveSelected;
}

CDB_Actuator *CRViewSSel6WayValve::GetCurrent6WayValveActuatorSelected()
{
	CDB_Actuator *pclCurrent6WayValveActuatorSelected = NULL;
	CSheetDescription *pclSheetDescription6WayValveActuator = m_ViewDescription.GetFromSheetDescriptionID( SD_6WayValveActuator );

	if( NULL != pclSheetDescription6WayValveActuator )
	{
		// Retrieve the current selected actuator.
		CCellDescriptionProduct *pclCDCurrent6WayValveActuatorSelected = NULL;
		LPARAM lpPointer;

		if( true == pclSheetDescription6WayValveActuator->GetUserVariable( _SDUV_SELECTEDPRODUCT, lpPointer ) )
		{
			pclCDCurrent6WayValveActuatorSelected = (CCellDescriptionProduct *)lpPointer;
		}

		if( NULL != pclCDCurrent6WayValveActuatorSelected && NULL != pclCDCurrent6WayValveActuatorSelected->GetProduct() )
		{
			pclCurrent6WayValveActuatorSelected = dynamic_cast<CDB_Actuator *>( (CData *)pclCDCurrent6WayValveActuatorSelected->GetProduct() );
		}
	}

	return pclCurrent6WayValveActuatorSelected;
}

CDB_TAProduct *CRViewSSel6WayValve::GetCurrentPIBCValveSelected( SideDefinition eSideDefinition )
{
	int iSheetDescriptionID = ( eSideDefinition << SHEETSIDEDEFSHIFT ) + SD_FamilyPIBCValve + SD_TypeValve;

	CDB_TAProduct *pclCurrentPIBCValveSelected = NULL;
	CSheetDescription *pclSheetDescriptionPIBCValve = m_ViewDescription.GetFromSheetDescriptionID( iSheetDescriptionID );

	if( NULL == pclSheetDescriptionPIBCValve )
	{
		return NULL;
	}

	// Retrieve the current selected pressure independent balancing & control valve.
	CCellDescriptionProduct *pclCDCurrentPIBCValveSelected = NULL;
	LPARAM lpPointer;

	if( true == pclSheetDescriptionPIBCValve->GetUserVariable( _SDUV_SELECTEDPRODUCT, lpPointer ) )
	{
		pclCDCurrentPIBCValveSelected = (CCellDescriptionProduct *)lpPointer;
	}

	if( NULL != pclCDCurrentPIBCValveSelected && NULL != pclCDCurrentPIBCValveSelected->GetProduct() )
	{
		pclCurrentPIBCValveSelected = dynamic_cast<CDB_TAProduct *>( (CData *)pclCDCurrentPIBCValveSelected->GetProduct() );
	}

	return pclCurrentPIBCValveSelected;
}

CDB_Actuator *CRViewSSel6WayValve::GetCurrentPIBCValveActuatorSelected( SideDefinition eSideDefinition )
{
	int iSheetDescriptionID = ( eSideDefinition << SHEETSIDEDEFSHIFT ) + SD_FamilyPIBCValve + SD_TypeValveActuator;

	CDB_Actuator *pclCurrentPIBCValveActuatorSelected = NULL;
	CSheetDescription *pclSheetDescriptionPIBCValveActuator = m_ViewDescription.GetFromSheetDescriptionID( iSheetDescriptionID );

	if( NULL == pclSheetDescriptionPIBCValveActuator )
	{
		return NULL;
	}

	// Retrieve the current selected actuator.
	CCellDescriptionProduct *pclCDCurrentControlValveActuatorSelected = NULL;
	LPARAM lpPointer;

	if( true == pclSheetDescriptionPIBCValveActuator->GetUserVariable( _SDUV_SELECTEDPRODUCT, lpPointer ) )
	{
		pclCDCurrentControlValveActuatorSelected = (CCellDescriptionProduct *)lpPointer;
	}

	if( NULL != pclCDCurrentControlValveActuatorSelected && NULL != pclCDCurrentControlValveActuatorSelected->GetProduct() )
	{
		pclCurrentPIBCValveActuatorSelected = dynamic_cast<CDB_Actuator *>( (CData *)pclCDCurrentControlValveActuatorSelected->GetProduct() );
	}

	return pclCurrentPIBCValveActuatorSelected;
}

CDB_TAProduct *CRViewSSel6WayValve::GetCurrentBalancingValveSelected( SideDefinition eSideDefinition )
{
	int iSheetDescriptionID = ( eSideDefinition << SHEETSIDEDEFSHIFT ) + SD_FamilyBalancingValve + SD_TypeValve;
	
	CDB_TAProduct *pclCurrentBalancingValveSelected = NULL;
	CSheetDescription *pclSheetDescriptionBalancingValve = m_ViewDescription.GetFromSheetDescriptionID( iSheetDescriptionID );

	if( NULL == pclSheetDescriptionBalancingValve )
	{
		return NULL;
	}

	// Retrieve the current selected pressure independent balancing & control valve.
	CCellDescriptionProduct *pclCDCurrentPIBCValveSelected = NULL;
	LPARAM lpPointer;

	if( true == pclSheetDescriptionBalancingValve->GetUserVariable( _SDUV_SELECTEDPRODUCT, lpPointer ) )
	{
		pclCDCurrentPIBCValveSelected = (CCellDescriptionProduct *)lpPointer;
	}

	if( NULL != pclCDCurrentPIBCValveSelected && NULL != pclCDCurrentPIBCValveSelected->GetProduct() )
	{
		pclCurrentBalancingValveSelected = dynamic_cast<CDB_TAProduct *>( (CData *)pclCDCurrentPIBCValveSelected->GetProduct() );
	}

	return pclCurrentBalancingValveSelected;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PROTECTED MEMBERS
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CRViewSSel6WayValve::OnTextTipFetch( CSheetDescription *pclSheetDescription, long lColumn, long lRow, WORD *pwMultiLine, SHORT *pnTipWidth, TCHAR *pstrTipText,
		BOOL *pbShowTip )
{
	if( NULL == m_pclIndSel6WayValveParams || NULL == m_pclIndSel6WayValveParams->m_pclSelect6WayValveList || false == m_bInitialised || NULL == pclSheetDescription )
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
	CDB_TAProduct *pTAP = dynamic_cast<CDB_TAProduct *>( (CData *)GetCDProduct( lColumn, lRow, pclSheetDescription ) );
	CSSheet *pclSSheet = pclSheetDescription->GetSSheetPointer();
	CDS_TechnicalParameter *pclTechParam = m_pclIndSel6WayValveParams->m_pTADS->GetpTechParams();

	CDB_6WayValve *pcl6WayValve = NULL;
	CDB_PIControlValve *pclPIBCValve = NULL;
	CSelectedValve *pSelectedTAP = NULL;

	int iSheetDescriptionID = pclSheetDescription->GetSheetDescriptionID();
	SideDefinition eSideDefinition = GETSHEETSIDE( iSheetDescriptionID );
	int iSheetType = GETSHEETTYPE( iSheetDescriptionID );
	int iSheetFamily = GETSHEETFAMILY( iSheetDescriptionID );

	if( SD_Family6WayValve == iSheetFamily )
	{
		switch( iSheetType )
		{
			// Mouse cursor has passed over a 6-way valve.
			case SD_TypeValve:
				pcl6WayValve = dynamic_cast<CDB_6WayValve *>( pTAP );
				pSelectedTAP = GetSelectProduct<CSelectedValve>( pTAP, m_pclIndSel6WayValveParams->m_pclSelect6WayValveList );

				if( CD_6WayValve_TemperatureRange == lColumn && NULL != pSelectedTAP )
				{
					if( true == pSelectedTAP->IsFlagSet( CSelectedBase::eTemperature ) )
					{
						FormatString( str, IDS_SSHEETSSEL_TEMPERROR, pTAP->GetTempRange() );
					}
				}

				if( true == str.IsEmpty() && NULL != pcl6WayValve )
				{
					if( 0 != _tcslen( pcl6WayValve->GetComment() ) )
					{
						str = pcl6WayValve->GetComment();
					}
				}

				break;

			// Mouse cursor has passed over a 6-way valve accessory.
			case SD_TypeValveAccessories:
			case SD_TypeValveActuatorAccessories:
				if( lColumn > CD_Accessory_FirstColumn && lColumn < CD_Accessory_LastColumn )
				{
					TextTipFetchEllipsesHelper( lColumn, lRow, pclSheetDescription, pnTipWidth, &str );
				}

				break;

			// Mouse cursor has passed over an actuator selected with 6-way valve.
			case SD_TypeValveActuator:
			{
				// Try to cast selected object to actuator.
				CDB_ElectroActuator *pclElectroActuator = dynamic_cast<CDB_ElectroActuator *>( (CData *)GetCDProduct( lColumn, lRow, pclSheetDescription ) );

				if( NULL != pclElectroActuator )
				{
					if( CD_Actuator_InputSig == lColumn && _ORANGE == pclSSheet->GetForeColor( lColumn, lRow ) )
					{
						str = TASApp.LoadLocalizedString( IDS_SSHEETSSELCV_FCTDOWNGRADED );
					}
					else if( ( CD_Actuator_Name == lColumn || CD_Actuator_MaxTemp == lColumn) && _RED == pclSSheet->GetForeColor( lColumn, lRow ) )
					{
						str = TASApp.LoadLocalizedString( IDS_SSHEETACT_TT_MAXTEMPERROR );
					}
				}
			}
			break;
		}
	}
	else if( SD_FamilyPIBCValve == iSheetFamily )
	{
		switch( iSheetType )
		{
			// Mouse cursor has passed over a control pressure independent balancing & control valve .
			case SD_TypeValve:
				pclPIBCValve = dynamic_cast<CDB_PIControlValve *>( pTAP );
				pSelectedTAP = GetSelectProduct<CSelectedValve>( pclPIBCValve, m_pclIndSel6WayValveParams->m_pclSelect6WayValveList->GetPICVList( eSideDefinition ) );

				if( CD_PIBCV_Preset == lColumn && NULL != pclPIBCValve )
				{
					// Check what is the color of the text.
					if( NULL != pclSSheet && _ORANGE == pclSSheet->GetForeColor( lColumn, lRow ) )
					{
						CDB_ValveCharacteristic *pValveChar = pclPIBCValve->GetValveCharacteristic();
				
						if( NULL != pValveChar )
						{
							CString str1;
							CString str2 = pValveChar->GetSettingString( pValveChar->GetMinRecSetting() );
							str1.Format( _T("%s < %s"), (LPCTSTR)pclSSheet->GetCellText( lColumn, lRow ), (LPCTSTR)str2 );
							FormatString( str, IDS_SSHEETSSEL_SETTINGERROR, str1 );
						}
					}
				}
				else if( CD_PIBCV_DpMax == lColumn )
				{
					if( m_pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_dDpMax > pclPIBCValve->GetDpmax() )
					{
						CString str2 = WriteCUDouble( _U_DIFFPRESS, m_pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_dDpMax, true, 3, 0 );
						CString str3 = WriteCUDouble( _U_DIFFPRESS, pclPIBCValve->GetDpmax(), true, 3, 0 );
						FormatString( str, IDS_PRODUCTSELECTION_ERROR_DPMAX, str2, str3 );
					}
				}
				else if( CD_PIBCV_TemperatureRange == lColumn && NULL != pSelectedTAP )
				{
					if( true == pSelectedTAP->IsFlagSet( CSelectedBase::eTemperature ) )
					{
						FormatString( str, IDS_SSHEETSSEL_TEMPERROR, pclPIBCValve->GetTempRange() );
					}
				}
				else if( CD_PIBCV_PipeLinDp == lColumn )
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
				else if( CD_PIBCV_PipeV == lColumn )
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

				break;

			// Mouse cursor has passed over a control pressure independent balancing & control valve accessory.
			case SD_TypeValveAccessories:
			case SD_TypeValveActuatorAccessories:
				if( lColumn > CD_Accessory_FirstColumn && lColumn < CD_Accessory_LastColumn )
				{
					TextTipFetchEllipsesHelper( lColumn, lRow, pclSheetDescription, pnTipWidth, &str );
				}

				break;

			// Mouse cursor has passed over an actuator selected with a control pressure independent balancing & control valve.
			case SD_TypeValveActuator:
			{
				// Try to cast selected object to actuator.
				CDB_ElectroActuator *pclElectroActuator = dynamic_cast<CDB_ElectroActuator *>( (CData *)GetCDProduct( lColumn, lRow, pclSheetDescription ) );
						
				if( NULL != pclElectroActuator )
				{
					if( CD_Actuator_CloseOffValue == lColumn )
					{
						// Try to retrieve current selected pressure independent balancing & control valve.
						CDB_PIControlValve *pclCurrentPIBCV = dynamic_cast<CDB_PIControlValve *>( GetCurrentPIBCValveSelected( eSideDefinition ) );
						CSelectedValve *pSelectedCV = GetSelectProduct<CSelectedValve>( pclCurrentPIBCV, m_pclIndSel6WayValveParams->m_pclSelect6WayValveList->GetPICVList( eSideDefinition ) );

						if( NULL != pclCurrentPIBCV && NULL != pSelectedCV )
						{
							CDB_CloseOffChar *pCloseOffChar = (CDB_CloseOffChar *)( pclCurrentPIBCV->GetCloseOffCharIDPtr().MP );

							if( NULL != pCloseOffChar && CDB_CloseOffChar::CloseOffDp == pCloseOffChar->GetLimitType() )
							{
								double dDpmax = pclCurrentPIBCV->GetDpmax();
								double dCloseOffDp = pCloseOffChar->GetCloseOffDp( pclElectroActuator->GetMaxForceTorque() );

								if( -1.0 != dCloseOffDp && -1.0 != dDpmax && dCloseOffDp > dDpmax )
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
		}
	}
	else if( SD_FamilyBalancingValve == iSheetFamily )
	{
		switch( iSheetType )
		{
			// Mouse cursor has passed over a control balancing valve.
			case SD_TypeValve:
			{
				CDB_RegulatingValve *pclRegulatingValve = dynamic_cast<CDB_RegulatingValve *>( pTAP );
				CSelectedValve *pclSelectedBv = GetSelectProduct<CSelectedValve>( pTAP, m_pclIndSel6WayValveParams->m_pclSelect6WayValveList->GetBVList( eSideDefinition ) );

				if( CD_BV_Preset == lColumn && NULL != pclRegulatingValve && NULL != pclSelectedBv )
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
				else if( CD_BV_Dp == lColumn && NULL != pclRegulatingValve && NULL != pclSelectedBv )
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
								double dDpMax = pclRegulatingValve->GetDpmax();

								// IDS_SSHEETSSEL_DPERRORH: "Dp is above the maximum value defined in technical parameters ( %1 )"
								// IDS_SSHEETSSEL_DPERRORH2: "Dp is above the maximum value defined for this valve ( %1 )"
								int iMsgID = ( dDpMax <= 0.0 ) ? IDS_SSHEETSSEL_DPERRORH : IDS_SSHEETSSEL_DPERRORH2;

								str2 += CString( _T(" > ") ) + WriteCUDouble( _U_DIFFPRESS, pclTechParam->VerifyValvMaxDp( pclRegulatingValve ), true );
								FormatString( str, iMsgID, str2 );
								break;
							}

							case CSelectList::BvFlagError::DpToReachTooLow:
								str2 += CString( _T(" > ") ) + WriteCUDouble( _U_DIFFPRESS, m_pclIndSel6WayValveParams->m_dDp, true );

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
				else if( CD_BV_DpFullOpening == lColumn && NULL != pclSelectedBv )
				{
					// Check if flag is set.
					if( true == pclSelectedBv->IsFlagSet( CSelectedBase::eValveFullODp ) )
					{
						if( pclSelectedBv->GetDpFullOpen() < pclTechParam->GetValvMinDp() )
						{
							CString str2 = WriteCUDouble( _U_DIFFPRESS, pclSelectedBv->GetDpFullOpen() );
							str2 += CString( _T(" < ") ) + WriteCUDouble( _U_DIFFPRESS, pclTechParam->GetValvMinDp(), true );

							// "Pressure drop on valve fully open is below the minimum value defined in technical parameters ( %1 )"
							FormatString( str, IDS_SSHEETSSEL_DPFOERROR, str2 );
						}
					}
				}
				else if( CD_BV_DpHalfOpening == lColumn && NULL != pclSelectedBv )
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
						else if( pclSelectedBv->GetDpHalfOpen() > pclTechParam->VerifyValvMaxDp(pTAP) )
						{
							str2 += CString( _T(" > ") ) + WriteCUDouble( _U_DIFFPRESS, pclTechParam->VerifyValvMaxDp(pTAP), true );
							FormatString( str, IDS_SSHEETSSEL_DPHOERRORH, str2 );
						}
					}
				}
				else if( CD_BV_TemperatureRange == lColumn && NULL != pclRegulatingValve )
				{
					if( true == pclSelectedBv->IsFlagSet( CSelectedBase::eTemperature ) )
					{
						FormatString( str, IDS_SSHEETSSEL_TEMPERROR, pclRegulatingValve->GetTempRange() );
					}
				}
				else if( CD_BV_PipeLinDp == lColumn )
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
				else if( CD_BV_PipeV == lColumn )
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

				break;
			}

			// Mouse cursor has passed over a control balancing valve accessory.
			case SD_TypeValveAccessories:
				if( lColumn > CD_Accessory_FirstColumn && lColumn < CD_Accessory_LastColumn )
				{
					TextTipFetchEllipsesHelper( lColumn, lRow, pclSheetDescription, pnTipWidth, &str );
				}

				break;
		}
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

bool CRViewSSel6WayValve::OnClickProduct( CSheetDescription *pclSheetDescription, CCellDescriptionProduct *pclCellDescriptionProduct, long lColumn, long lRow )
{
	// Sanity check.
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() || NULL == pclCellDescriptionProduct )
	{
		return false;
	}

	bool bNeedRefresh = true;
	CSSheet *pclSSheet = pclSheetDescription->GetSSheetPointer();
	pclSSheet->SetBool( SSB_REDRAW, FALSE );

	CDB_RegulatingValve *pBalancingValve = dynamic_cast<CDB_RegulatingValve *>( (CData *)pclCellDescriptionProduct->GetProduct() );

	int iSheetDescriptionID = pclSheetDescription->GetSheetDescriptionID();
	int iSheetType = GETSHEETTYPE( iSheetDescriptionID );
	int iSheetFamily = GETSHEETFAMILY( iSheetDescriptionID );

	if( SD_Family6WayValve == iSheetFamily )
	{
		switch( iSheetType )
		{
			case SD_TypeValve:
				{
					CDB_6WayValve *pcl6WayValve = dynamic_cast<CDB_6WayValve *>( (CData *)pclCellDescriptionProduct->GetProduct() );

					if( NULL != pcl6WayValve )
					{
						_ClickOn6WayValve( pclSheetDescription, pcl6WayValve, pclCellDescriptionProduct, lColumn, lRow );
					}
					// HYS-1318 : The 6-way is already clicked
					if( e6Way_EQMControl == m_pclIndSel6WayValveParams->m_e6WayValveSelectionMode )
					{
						// If we already clicked on 6-way valve the 6-way valve actuator has to be checked
						_VerifyOneProductAndClick( SideDefinition::BothSide, SD_Family6WayValve, SD_TypeValveActuator );
						_VerifyOneProductAndClick( SideDefinition::BothSide, SD_FamilyPIBCValve, SD_TypeValve );
					}
					else if( e6Way_OnOffControlWithPIBCV == m_pclIndSel6WayValveParams->m_e6WayValveSelectionMode )
					{
						// Do not check actuator's 6-way valve because there are always more than one
						_VerifyOneProductAndClick( SideDefinition::CoolingSide, SD_FamilyPIBCValve, SD_TypeValve );
						_VerifyOneProductAndClick( SideDefinition::HeatingSide, SD_FamilyPIBCValve, SD_TypeValve );
					}
					else if( e6Way_OnOffControlWithSTAD == m_pclIndSel6WayValveParams->m_e6WayValveSelectionMode )
					{
						// Do not check actuator's 6-way valve because there are always more than one
						_VerifyOneProductAndClick( SideDefinition::CoolingSide, SD_FamilyBalancingValve, SD_TypeValve );
						_VerifyOneProductAndClick( SideDefinition::HeatingSide, SD_FamilyBalancingValve, SD_TypeValve );
					}
				}
				break;

			case SD_TypeValveActuator:
				{
					CDB_Actuator *pclActuator = dynamic_cast<CDB_Actuator *>( (CData *)pclCellDescriptionProduct->GetProduct() );

					if( NULL != pclActuator )
					{
						_ClickOn6WayValveActuator( pclSheetDescription, pclActuator, pclCellDescriptionProduct, lColumn, lRow );
					}
				}
				break;
		}
	}
	else if( SD_FamilyPIBCValve == iSheetFamily )
	{
		switch( iSheetType )
		{
			case SD_TypeValve:
				{
					CDB_PIControlValve *pclPIBCV = dynamic_cast<CDB_PIControlValve *>( (CData *)pclCellDescriptionProduct->GetProduct() );

					if( NULL != pclPIBCV )
					{
						_ClickOnPIBCValve( pclSheetDescription, pclPIBCV, pclCellDescriptionProduct, lColumn, lRow );
					}
				}
				break;

			case SD_TypeValveActuator:
				{
					CDB_Actuator *pclActuator = dynamic_cast<CDB_Actuator *>( (CData *)pclCellDescriptionProduct->GetProduct() );

					if( NULL != pclActuator )
					{
						_ClickOnPIBCValveActuator( pclSheetDescription, pclActuator, pclCellDescriptionProduct, lColumn, lRow );
					}
				}
				break;
		}
	}
	else if( SD_FamilyBalancingValve == iSheetFamily )
	{
		switch( iSheetType )
		{
			case SD_TypeValve:
				{
					CDB_RegulatingValve *pclBalancingValve = dynamic_cast<CDB_RegulatingValve *>( (CData *)pclCellDescriptionProduct->GetProduct() );

					if( NULL != pclBalancingValve )
					{
						_ClickOnBalancingValve( pclSheetDescription, pclBalancingValve, pclCellDescriptionProduct, lColumn, lRow );
					}
				}
				break;

			default:
				bNeedRefresh = false;
		}
	}

	pclSSheet->SetBool( SSB_REDRAW, TRUE );
	return bNeedRefresh;
}

bool CRViewSSel6WayValve::ResetColumnWidth( short nSheetDescriptionID )
{
	TSpread clTSpread;

	if( FALSE == clTSpread.Create( WS_CHILD, CRect( 0, 0, 0, 0 ), this, 0 ) )
	{
		ASSERT( 0 );
		return false;
	}

	switch( nSheetDescriptionID )
	{
		case SD_6WayValve:
			m_mapSSheetColumnWidth[nSheetDescriptionID][CD_6WayValve_FirstColumn] = clTSpread.ColWidthToLogUnits( 4 );
			m_mapSSheetColumnWidth[nSheetDescriptionID][CD_6WayValve_Box] = clTSpread.ColWidthToLogUnits( 2 );
			m_mapSSheetColumnWidth[nSheetDescriptionID][CD_6WayValve_CheckBox] = clTSpread.ColWidthToLogUnits( 2 );
			m_mapSSheetColumnWidth[nSheetDescriptionID][CD_6WayValve_Name] = clTSpread.ColWidthToLogUnits( 16 );
			m_mapSSheetColumnWidth[nSheetDescriptionID][CD_6WayValve_Material] = clTSpread.ColWidthToLogUnits( 20 );
			// HYS-1884: One column connection
			m_mapSSheetColumnWidth[nSheetDescriptionID][CD_6WayValve_Connection] = clTSpread.ColWidthToLogUnits( 13 );
			m_mapSSheetColumnWidth[nSheetDescriptionID][CD_6WayValve_Version] = clTSpread.ColWidthToLogUnits( 8 );
			m_mapSSheetColumnWidth[nSheetDescriptionID][CD_6WayValve_PN] = clTSpread.ColWidthToLogUnits( 4 );
			m_mapSSheetColumnWidth[nSheetDescriptionID][CD_6WayValve_Size] = clTSpread.ColWidthToLogUnits( 8 );
			m_mapSSheetColumnWidth[nSheetDescriptionID][CD_6WayValve_DpCooling] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[nSheetDescriptionID][CD_6WayValve_DpHeating] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[nSheetDescriptionID][CD_6WayValve_TemperatureRange] = clTSpread.ColWidthToLogUnits( 8 );
			m_mapSSheetColumnWidth[nSheetDescriptionID][CD_6WayValve_Pointer] = clTSpread.ColWidthToLogUnits( 1 );
			break;

		case SD_6WayValveActuator:
			m_mapSSheetColumnWidth[nSheetDescriptionID][CD_Actuator_FirstColumn] = clTSpread.ColWidthToLogUnits( 4 );
			m_mapSSheetColumnWidth[nSheetDescriptionID][CD_Actuator_Box] = clTSpread.ColWidthToLogUnits( 2 );
			m_mapSSheetColumnWidth[nSheetDescriptionID][CD_Actuator_CheckBox] = clTSpread.ColWidthToLogUnits( 2 );
			m_mapSSheetColumnWidth[nSheetDescriptionID][CD_Actuator_Name] = clTSpread.ColWidthToLogUnits( 16 );
			m_mapSSheetColumnWidth[nSheetDescriptionID][CD_Actuator_CloseOffValue] = clTSpread.ColWidthToLogUnits( 10 );
			m_mapSSheetColumnWidth[nSheetDescriptionID][CD_Actuator_ActuatingTime] = clTSpread.ColWidthToLogUnits( 10 );
			m_mapSSheetColumnWidth[nSheetDescriptionID][CD_Actuator_IP] = clTSpread.ColWidthToLogUnits( 10 );
			m_mapSSheetColumnWidth[nSheetDescriptionID][CD_Actuator_PowSupply] = clTSpread.ColWidthToLogUnits( 10 );
			m_mapSSheetColumnWidth[nSheetDescriptionID][CD_Actuator_InputSig] = clTSpread.ColWidthToLogUnits( 10 );
			m_mapSSheetColumnWidth[nSheetDescriptionID][CD_Actuator_OutputSig] = clTSpread.ColWidthToLogUnits( 10 );
			m_mapSSheetColumnWidth[nSheetDescriptionID][CD_Actuator_RelayType] = clTSpread.ColWidthToLogUnits( 10 );
			m_mapSSheetColumnWidth[nSheetDescriptionID][CD_Actuator_FailSafe] = clTSpread.ColWidthToLogUnits( 10 );
			m_mapSSheetColumnWidth[nSheetDescriptionID][CD_Actuator_DefaultReturnPos] = clTSpread.ColWidthToLogUnits( 15 );
			m_mapSSheetColumnWidth[nSheetDescriptionID][CD_Actuator_MaxTemp] = clTSpread.ColWidthToLogUnits( 8 );
			m_mapSSheetColumnWidth[nSheetDescriptionID][CD_Actuator_Pointer] = clTSpread.ColWidthToLogUnits( 1 );
			break;

		case SD_PIBCValveBothSide:
		case SD_PIBCValveHeatingSide:
		case SD_PIBCValveCoolingSide:
			m_mapSSheetColumnWidth[nSheetDescriptionID][CD_PIBCV_FirstColumn] = clTSpread.ColWidthToLogUnits( 4 );
			m_mapSSheetColumnWidth[nSheetDescriptionID][CD_PIBCV_Box] = clTSpread.ColWidthToLogUnits( 2 );
			m_mapSSheetColumnWidth[nSheetDescriptionID][CD_PIBCV_CheckBox] = clTSpread.ColWidthToLogUnits( 2 );
			m_mapSSheetColumnWidth[nSheetDescriptionID][CD_PIBCV_Name] = clTSpread.ColWidthToLogUnits( 16 );
			m_mapSSheetColumnWidth[nSheetDescriptionID][CD_PIBCV_Material] = clTSpread.ColWidthToLogUnits( 12 );
			m_mapSSheetColumnWidth[nSheetDescriptionID][CD_PIBCV_Connection] = clTSpread.ColWidthToLogUnits( 14 );
			m_mapSSheetColumnWidth[nSheetDescriptionID][CD_PIBCV_Version] = clTSpread.ColWidthToLogUnits( 12 );
			m_mapSSheetColumnWidth[nSheetDescriptionID][CD_PIBCV_Size] = clTSpread.ColWidthToLogUnits( 10 );
			m_mapSSheetColumnWidth[nSheetDescriptionID][CD_PIBCV_PN] = clTSpread.ColWidthToLogUnits( 6 );
			m_mapSSheetColumnWidth[nSheetDescriptionID][CD_PIBCV_Rangeability] = clTSpread.ColWidthToLogUnits( 8 );
			m_mapSSheetColumnWidth[nSheetDescriptionID][CD_PIBCV_LeakageRate] = clTSpread.ColWidthToLogUnits( 6 );
			m_mapSSheetColumnWidth[nSheetDescriptionID][CD_PIBCV_Stroke] = clTSpread.ColWidthToLogUnits( 5 );
			m_mapSSheetColumnWidth[nSheetDescriptionID][CD_PIBCV_ImgCharacteristic] = clTSpread.ColWidthToLogUnits( 2 );
			m_mapSSheetColumnWidth[nSheetDescriptionID][CD_PIBCV_ImgSeparator] = clTSpread.ColWidthToLogUnits( 2 );
			m_mapSSheetColumnWidth[nSheetDescriptionID][CD_PIBCV_ImgPushClose] = clTSpread.ColWidthToLogUnits( 2 );
			m_mapSSheetColumnWidth[nSheetDescriptionID][CD_PIBCV_Preset] = clTSpread.ColWidthToLogUnits( 7 );
			// HYS-1380: Add Dp min. heating and Dp min cooling for EQM control
			m_mapSSheetColumnWidth[nSheetDescriptionID][CD_PIBCV_DpMinCooling] = clTSpread.ColWidthToLogUnits( 6 );
			m_mapSSheetColumnWidth[nSheetDescriptionID][CD_PIBCV_DpMinHeating] = clTSpread.ColWidthToLogUnits( 6 );
			m_mapSSheetColumnWidth[nSheetDescriptionID][CD_PIBCV_DpMax] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[nSheetDescriptionID][CD_PIBCV_TemperatureRange] = clTSpread.ColWidthToLogUnits( 8 );
			m_mapSSheetColumnWidth[nSheetDescriptionID][CD_PIBCV_Separator] = clTSpread.ColWidthToLogUnits( 1 );
			m_mapSSheetColumnWidth[nSheetDescriptionID][CD_PIBCV_PipeSize] = clTSpread.ColWidthToLogUnits( 8 );
			m_mapSSheetColumnWidth[nSheetDescriptionID][CD_PIBCV_PipeLinDp] = clTSpread.ColWidthToLogUnits( 8 );
			m_mapSSheetColumnWidth[nSheetDescriptionID][CD_PIBCV_PipeV] = clTSpread.ColWidthToLogUnits( 8 );
			m_mapSSheetColumnWidth[nSheetDescriptionID][CD_PIBCV_Pointer] = clTSpread.ColWidthToLogUnits( 1 );
			break;

		case SD_PIBCValveActuatorBothSide:
		case SD_PIBCValveActuatorHeatingSide:
		case SD_PIBCValveActuatorCoolingSide:
			m_mapSSheetColumnWidth[nSheetDescriptionID][CD_Actuator_FirstColumn] = clTSpread.ColWidthToLogUnits( 4 );
			m_mapSSheetColumnWidth[nSheetDescriptionID][CD_Actuator_Box] = clTSpread.ColWidthToLogUnits( 2 );
			m_mapSSheetColumnWidth[nSheetDescriptionID][CD_Actuator_CheckBox] = clTSpread.ColWidthToLogUnits( 2 );
			m_mapSSheetColumnWidth[nSheetDescriptionID][CD_Actuator_Name] = clTSpread.ColWidthToLogUnits( 30 );
			m_mapSSheetColumnWidth[nSheetDescriptionID][CD_Actuator_CloseOffValue] = clTSpread.ColWidthToLogUnits( 10 );
			m_mapSSheetColumnWidth[nSheetDescriptionID][CD_Actuator_ActuatingTime] = clTSpread.ColWidthToLogUnits( 10 );
			m_mapSSheetColumnWidth[nSheetDescriptionID][CD_Actuator_IP] = clTSpread.ColWidthToLogUnits( 10 );
			m_mapSSheetColumnWidth[nSheetDescriptionID][CD_Actuator_PowSupply] = clTSpread.ColWidthToLogUnits( 20 );
			m_mapSSheetColumnWidth[nSheetDescriptionID][CD_Actuator_InputSig] = clTSpread.ColWidthToLogUnits( 20 );
			m_mapSSheetColumnWidth[nSheetDescriptionID][CD_Actuator_OutputSig] = clTSpread.ColWidthToLogUnits( 20 );
			m_mapSSheetColumnWidth[nSheetDescriptionID][CD_Actuator_RelayType] = clTSpread.ColWidthToLogUnits( 15 );
			m_mapSSheetColumnWidth[nSheetDescriptionID][CD_Actuator_FailSafe] = clTSpread.ColWidthToLogUnits( 5 );
			m_mapSSheetColumnWidth[nSheetDescriptionID][CD_Actuator_DefaultReturnPos] = clTSpread.ColWidthToLogUnits( 15 );
			m_mapSSheetColumnWidth[nSheetDescriptionID][CD_Actuator_MaxTemp] = clTSpread.ColWidthToLogUnits( 8 );
			m_mapSSheetColumnWidth[nSheetDescriptionID][CD_Actuator_Pointer] = clTSpread.ColWidthToLogUnits( 1 );
			break;

		case SD_BalancingValveHeatingSide:
		case SD_BalancingValveCoolingSide:
			m_mapSSheetColumnWidth[nSheetDescriptionID][CD_BV_FirstColumn] = clTSpread.ColWidthToLogUnits( 4 );
			m_mapSSheetColumnWidth[nSheetDescriptionID][CD_BV_CheckBox] = clTSpread.ColWidthToLogUnits( 2 );
			m_mapSSheetColumnWidth[nSheetDescriptionID][CD_BV_Name] = clTSpread.ColWidthToLogUnits( 14 );
			m_mapSSheetColumnWidth[nSheetDescriptionID][CD_BV_Material] = clTSpread.ColWidthToLogUnits( 12 );
			m_mapSSheetColumnWidth[nSheetDescriptionID][CD_BV_Connection] = clTSpread.ColWidthToLogUnits( 14 );
			m_mapSSheetColumnWidth[nSheetDescriptionID][CD_BV_Version] = clTSpread.ColWidthToLogUnits( 12 );
			m_mapSSheetColumnWidth[nSheetDescriptionID][CD_BV_PN] = clTSpread.ColWidthToLogUnits( 6 );
			m_mapSSheetColumnWidth[nSheetDescriptionID][CD_BV_Size] = clTSpread.ColWidthToLogUnits( 10 );
			m_mapSSheetColumnWidth[nSheetDescriptionID][CD_BV_Preset] = clTSpread.ColWidthToLogUnits( 6 );
			m_mapSSheetColumnWidth[nSheetDescriptionID][CD_BV_DpSignal] = clTSpread.ColWidthToLogUnits( 6 );
			m_mapSSheetColumnWidth[nSheetDescriptionID][CD_BV_Dp] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[nSheetDescriptionID][CD_BV_DpFullOpening] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[nSheetDescriptionID][CD_BV_DpHalfOpening] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[nSheetDescriptionID][CD_BV_TemperatureRange] = clTSpread.ColWidthToLogUnits( 8 );
			m_mapSSheetColumnWidth[nSheetDescriptionID][CD_BV_Separator] = clTSpread.ColWidthToLogUnits( 1 );
			m_mapSSheetColumnWidth[nSheetDescriptionID][CD_BV_PipeSize] = clTSpread.ColWidthToLogUnits( 8 );
			m_mapSSheetColumnWidth[nSheetDescriptionID][CD_BV_PipeLinDp] = clTSpread.ColWidthToLogUnits( 8 );
			m_mapSSheetColumnWidth[nSheetDescriptionID][CD_BV_PipeV] = clTSpread.ColWidthToLogUnits( 8 );
			m_mapSSheetColumnWidth[nSheetDescriptionID][CD_BV_Pointer] = clTSpread.ColWidthToLogUnits( 1 );
			break;
	}

	if( clTSpread.GetSafeHwnd() != NULL )
	{
		clTSpread.DestroyWindow();
	}

	return true;
}

bool CRViewSSel6WayValve::IsSelectionReady()
{
	if( NULL == m_pclIndSel6WayValveParams )
	{
		ASSERTA_RETURN( false );
	}

	bool bReady = false;
	CDB_6WayValve *pclSelected6WayValve = GetCurrent6WayValveSelected();

	if( pclSelected6WayValve != NULL && e6Way_Undefined != m_pclIndSel6WayValveParams->m_e6WayValveSelectionMode )
	{
		if( e6Way_Alone == m_pclIndSel6WayValveParams->m_e6WayValveSelectionMode )
		{
			bReady = true;
		}
		else
		{
			if( e6Way_EQMControl == m_pclIndSel6WayValveParams->m_e6WayValveSelectionMode )
			{
				CDB_TAProduct *pclSelectedControlValve = GetCurrentPIBCValveSelected( BothSide );

				if( NULL != pclSelectedControlValve && false == pclSelected6WayValve->IsPartOfaSet() )
				{
					bReady = true;
				}
				else if( true == pclSelected6WayValve->IsPartOfaSet() && NULL != GetCurrentPIBCValveSelected( BothSide ) && NULL != GetCurrent6WayValveActuatorSelected()
						 && NULL != GetCurrentPIBCValveActuatorSelected( BothSide ) )
				{
					// HYS-1877: Consider the complete package before enable selection.
					bReady = true;
				}
			}
			else if( e6Way_OnOffControlWithPIBCV == m_pclIndSel6WayValveParams->m_e6WayValveSelectionMode )
			{
				CDB_TAProduct *pclSelectedControlValve1 = GetCurrentPIBCValveSelected( HeatingSide );
				CDB_TAProduct *pclSelectedControlValve2 = GetCurrentPIBCValveSelected( CoolingSide );

				if( NULL != pclSelectedControlValve1 && NULL != pclSelectedControlValve2 )
				{
					bReady = true;
				}
			}
			else if( e6Way_OnOffControlWithSTAD == m_pclIndSel6WayValveParams->m_e6WayValveSelectionMode )
			{
				CDB_TAProduct *pclSelectedControlValve1 = GetCurrentBalancingValveSelected( HeatingSide );
				CDB_TAProduct *pclSelectedControlValve2 = GetCurrentBalancingValveSelected( CoolingSide );

				if( NULL != pclSelectedControlValve1 && NULL != pclSelectedControlValve2 )
				{
					bReady = true;
				}
			}
		}
	}

	return bReady;
}

void CRViewSSel6WayValve::SetCurrent6WayValveSelected( CCellDescriptionProduct *pclCDCurrent6WayValveSelected )
{
	// Try to retrieve sheet description linked to the 6-way valve.
	CSheetDescription *pclSheetDescription6WayValve = m_ViewDescription.GetFromSheetDescriptionID( SD_6WayValve );

	if( NULL != pclSheetDescription6WayValve )
	{
		pclSheetDescription6WayValve->SetUserVariable( _SDUV_SELECTEDPRODUCT, (LPARAM)pclCDCurrent6WayValveSelected );
	}
}

void CRViewSSel6WayValve::SetCurrent6WayValveActuatorSelected( CCellDescriptionProduct *pclCDCurrent6WayValveActuatorSelected )
{
	// Try to retrieve sheet description linked to actuator selected with the 6_way valve.
	CSheetDescription *pclSheetDescription6WayValveActuator = m_ViewDescription.GetFromSheetDescriptionID( SD_6WayValveActuator );

	if( NULL != pclSheetDescription6WayValveActuator )
	{
		pclSheetDescription6WayValveActuator->SetUserVariable( _SDUV_SELECTEDPRODUCT, (LPARAM)pclCDCurrent6WayValveActuatorSelected );
	}
}

void CRViewSSel6WayValve::SetCurrentPIBCValveSelected( CCellDescriptionProduct *pclCDCurrentPIBCValveSelected, SideDefinition eSideDefinition )
{
	// Try to retrieve sheet description linked to the current pressure independent balancing & control valve selected.
	CSheetDescription *pclSheetDescription6WayValve = m_ViewDescription.GetFromSheetDescriptionID( ( eSideDefinition << SHEETSIDEDEFSHIFT ) + SD_FamilyPIBCValve + SD_TypeValve );

	if( NULL != pclSheetDescription6WayValve )
	{
		pclSheetDescription6WayValve->SetUserVariable( _SDUV_SELECTEDPRODUCT, (LPARAM)pclCDCurrentPIBCValveSelected );
	}
}

void CRViewSSel6WayValve::SetCurrentPIBCValveActuatorSelected( CCellDescriptionProduct *pclCDCurrentPIBCValveActuatorSelected, SideDefinition eSideDefinition )
{
	// Try to retrieve sheet description linked to the current actuator selected with the pressure independent balancing & control valve.
	CSheetDescription *pclSheetDescriptionPIBCValveActuator = m_ViewDescription.GetFromSheetDescriptionID( ( eSideDefinition << SHEETSIDEDEFSHIFT ) + SD_FamilyPIBCValve + SD_TypeValveActuator );

	if( NULL != pclSheetDescriptionPIBCValveActuator )
	{
		pclSheetDescriptionPIBCValveActuator->SetUserVariable( _SDUV_SELECTEDPRODUCT, (LPARAM)pclCDCurrentPIBCValveActuatorSelected );
	}
}

void CRViewSSel6WayValve::SetCurrentBalancingValveSelected( CCellDescriptionProduct *pclCDCurrentBalancingValveSelected, SideDefinition eSideDefinition )
{
	// Try to retrieve sheet description linked to the current balancing valve selected.
	CSheetDescription *pclSheetDescriptionBalancingValve = m_ViewDescription.GetFromSheetDescriptionID( ( eSideDefinition << SHEETSIDEDEFSHIFT ) + SD_FamilyBalancingValve + SD_TypeValve );

	if( NULL != pclSheetDescriptionBalancingValve )
	{
		pclSheetDescriptionBalancingValve->SetUserVariable( _SDUV_SELECTEDPRODUCT, (LPARAM)pclCDCurrentBalancingValveSelected );
	}
}

void CRViewSSel6WayValve::ChangeBoxStateFor6WValveSet( int iButtonState, bool bApplyChange, SideDefinition eSideDefinition, long lRow, CSheetDescription* pclCurrentSheetDescription )
{
	CDB_6WayValve* pcl6WValveValveSelected = GetCurrent6WayValveSelected();

	if( NULL == pcl6WValveValveSelected )
	{
		ASSERT_RETURN;
	}

	long l6WValveSelectedRow = m_mapVariables[SD_Family6WayValve][BothSide].m_lValveSelectedRow;
	long l6WValveActuatorSelectedRow = m_mapVariables[SD_Family6WayValve][BothSide].m_lValveActuatorSelectedRow;
	long lValveSelectedRow = m_mapVariables[SD_FamilyPIBCValve][eSideDefinition].m_lValveSelectedRow;

	CSheetDescription* pclSheetDescription6WValve = m_ViewDescription.GetFromSheetDescriptionID( SD_6WayValve );

	if( NULL != pclSheetDescription6WValve )
	{
		CCDButtonBox* pCDButtonBox = GetCDButtonBox( CD_6WayValve_Box, l6WValveSelectedRow, pclSheetDescription6WValve );

		if( NULL != pCDButtonBox )
		{
			pCDButtonBox->SetButtonState( iButtonState, bApplyChange );
		}
	}

	CSheetDescription* pclSheetDescriptionActuator = m_ViewDescription.GetFromSheetDescriptionID( SD_6WayValveActuator );

	if( NULL != pclSheetDescriptionActuator )
	{
		CCDButtonBox* pCDButtonBox = GetCDButtonBox( CD_Actuator_Box, l6WValveActuatorSelectedRow, pclSheetDescriptionActuator );

		if( NULL != pCDButtonBox )
		{
			pCDButtonBox->SetButtonState( iButtonState, bApplyChange );
		}
	}

	if( BothSide == eSideDefinition )
	{
		CSheetDescription* pclSheetDescriptionPIBCV = m_ViewDescription.GetFromSheetDescriptionID( SD_PIBCValveBothSide );

		if( NULL != pclSheetDescriptionPIBCV )
		{
			CCDButtonBox* pCDButtonBox = GetCDButtonBox( CD_PIBCV_Box, lValveSelectedRow, pclSheetDescriptionPIBCV );

			if( NULL != pCDButtonBox )
			{
				pCDButtonBox->SetButtonState( iButtonState, bApplyChange );
			}
		}

		if( NULL != pclCurrentSheetDescription )
		{
			CCDButtonBox* pCDButtonBox = GetCDButtonBox( CD_Actuator_Box, lRow, pclCurrentSheetDescription );

			if( NULL != pCDButtonBox )
			{
				pCDButtonBox->SetButtonState( iButtonState, bApplyChange );
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PRIVATE MEMBERS
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CRViewSSel6WayValve::_Init()
{
	m_mapVariables.clear();

	std::map<SideDefinition, ClassVariables> mapHelper1;
	ClassVariables clClassVariables;

	mapHelper1.insert( std::pair<SideDefinition, ClassVariables>( BothSide, clClassVariables ) );
	m_mapVariables.insert( std::pair<int, std::map<SideDefinition, ClassVariables>>( SD_Family6WayValve, mapHelper1 ) );

	mapHelper1.insert( std::pair<SideDefinition, ClassVariables>( HeatingSide, clClassVariables ) );
	mapHelper1.insert( std::pair<SideDefinition, ClassVariables>( CoolingSide, clClassVariables ) );
	m_mapVariables.insert( std::pair<int, std::map<SideDefinition, ClassVariables>>( SD_FamilyPIBCValve, mapHelper1 ) );

	mapHelper1.erase( BothSide );
	m_mapVariables.insert( std::pair<int, std::map<SideDefinition, ClassVariables>>( SD_FamilyBalancingValve, mapHelper1 ) );

	for( auto &iterFamily : m_mapVariables )
	{
		for( auto &iterSide : iterFamily.second )
		{
			iterSide.second.m_bDowngradeValveActuatorFunctionality = false;
			iterSide.second.m_lValveSelectedRow = 0;
			iterSide.second.m_lValveActuatorSelectedRow = 0;
			iterSide.second.m_vecValveAccessoryList.clear();
			iterSide.second.m_vecValveActuatorAccessoryList.clear();
			iterSide.second.m_pCDBExpandCollapseRowsValve = NULL;
			iterSide.second.m_pCDBExpandCollapseGroupValveAccessory = NULL;
			iterSide.second.m_pCDBExpandCollapseGroupValveActuator = NULL;
			iterSide.second.m_pCDBExpandCollapseRowsValveActuator = NULL;
			iterSide.second.m_pCDBExpandCollapseGroupValveActuatorAccessory = NULL;
			iterSide.second.m_pCDBShowAllPrioritiesValve = NULL;
		}
	}
}

void CRViewSSel6WayValve::_SuggestPIBCVHelper( SideDefinition eSideDefinition, CDS_SSel6WayValve *pclSelected6WayValve )
{
	if( NULL == pclSelected6WayValve || NULL == pclSelected6WayValve->GetCDSSSelPICv( eSideDefinition ) )
	{
		ASSERT_RETURN;
	}

	int iSheetDescriptionID = ( eSideDefinition << SHEETSIDEDEFSHIFT ) + SD_FamilyPIBCValve + SD_TypeValve;

	if( NULL == m_ViewDescription.GetFromSheetDescriptionID( iSheetDescriptionID ) )
	{
		return;
	}

	CDS_SSelPICv *pclSSelPIBCValve = pclSelected6WayValve->GetCDSSSelPICv( eSideDefinition );

	// Verify if user has selected a pressure independent balancing & control valve.
	CDB_PIControlValve *pclSelectedPIBCValve = pclSSelPIBCValve->GetProductAs<CDB_PIControlValve>();

	if( NULL == pclSelectedPIBCValve )
	{
		return;
	}

	CSheetDescription *pclSheetDescriptionPIBCV = m_ViewDescription.GetFromSheetDescriptionID( iSheetDescriptionID );

	// Find row number where is the pressure independent balancing & control valve.
	long lPIBCValveRowSelected = _GetRowOfEditedPIBCValve( pclSheetDescriptionPIBCV, pclSelectedPIBCValve );

	if( -1 == lPIBCValveRowSelected )
	{
		// HYS-2101: Clear previous PICV in edit mode when the selection mode changed.
		// In this case a selection with a PIBCV bothside is modified to selection with compact-p and the actuator Slider-Co is kept.
		pclSSelPIBCValve->SetActrIDPtr( _NULL_IDPTR );
		pclSSelPIBCValve->SetProductIDPtr( _NULL_IDPTR );
		return;
	}

	CCellDescriptionProduct *pclCDPIBCValve = FindCDProduct( lPIBCValveRowSelected, (LPARAM)pclSelectedPIBCValve, pclSheetDescriptionPIBCV );

	if( NULL == pclCDPIBCValve || NULL == pclCDPIBCValve->GetProduct() )
	{
		return;
	}

	// HYS-1877: For edition mode click on product only if it is not.
	if( NULL == GetCurrentPIBCValveSelected( eSideDefinition ) )
	{
		// Simulate a click on the edited pressure independent balancing & control valve.
		OnClickProduct( pclSheetDescriptionPIBCV, pclCDPIBCValve, pclSheetDescriptionPIBCV->GetActiveColumn(), lPIBCValveRowSelected );
	}

	// Allow to check if we need to change the 'Show all priorities' button or not.
	CheckShowAllPrioritiesButtonState( pclSheetDescriptionPIBCV, lPIBCValveRowSelected );

	// Verify accessories on the pressure independent balancing & control valve.
	CAccessoryList *pclPIBCValveAccessoryList = pclSSelPIBCValve->GetCvAccessoryList();

	if( pclPIBCValveAccessoryList->GetCount() > 0 )
	{
		CAccessoryList::AccessoryItem rAccessoryItem = pclPIBCValveAccessoryList->GetFirst();

		while( rAccessoryItem.IDPtr.MP != NULL )
		{
			VerifyCheckboxAccessories( (CDB_Product *)rAccessoryItem.IDPtr.MP, true, &m_mapVariables[SD_FamilyPIBCValve][eSideDefinition].m_vecValveAccessoryList );
			rAccessoryItem = pclPIBCValveAccessoryList->GetNext();
		}
	}

	// Verify if user has selected an actuator with the pressure independent balancing & control valve.
	IDPTR PIBCValveActuatorIDPtr = pclSSelPIBCValve->GetActrIDPtr();
	iSheetDescriptionID = ( eSideDefinition << SHEETSIDEDEFSHIFT ) + SD_FamilyPIBCValve + SD_TypeValveActuator;
	CSheetDescription *pclSheetDescriptionPIBCValveActuator = m_ViewDescription.GetFromSheetDescriptionID( iSheetDescriptionID );

	if( _NULL_IDPTR == PIBCValveActuatorIDPtr || NULL == PIBCValveActuatorIDPtr.MP || NULL == pclSheetDescriptionPIBCValveActuator )
	{
		return;
	}

	CDB_Actuator *pclEditedPIBCValveActuator = dynamic_cast<CDB_Actuator *>( PIBCValveActuatorIDPtr.MP );

	if( NULL == pclEditedPIBCValveActuator )
	{
		return;
	}

	// Find row number where is the	actuator selected with the pressure independent balancing & control valve.
	long lPIBCValveActuatorRowSelected = _GetRowOfEditedActuator( pclSheetDescriptionPIBCValveActuator, pclEditedPIBCValveActuator );

	if( -1 == lPIBCValveActuatorRowSelected )
	{
		return;
	}

	CCellDescriptionProduct *pclCDPIBCValveActuator = FindCDProduct( lPIBCValveActuatorRowSelected, (LPARAM)pclEditedPIBCValveActuator, pclSheetDescriptionPIBCValveActuator );

	if( NULL == pclCDPIBCValveActuator || NULL == pclCDPIBCValveActuator->GetProduct() )
	{
		return;
	}

	// HYS-1877: For edition mode click on product only if it is not.
	if( NULL == GetCurrentPIBCValveActuatorSelected( eSideDefinition ) )
	{
		// Simulate a click on the edited actuator.
		OnClickProduct( pclSheetDescriptionPIBCValveActuator, pclCDPIBCValveActuator, pclSheetDescriptionPIBCValveActuator->GetActiveColumn(), lPIBCValveActuatorRowSelected );
	}

	// Verify accessories on actuator.
	CAccessoryList *pclPIBCValveActuatorAccessoryList = pclSSelPIBCValve->GetActuatorAccessoryList();

	if( NULL == pclPIBCValveActuatorAccessoryList || 0 == pclPIBCValveActuatorAccessoryList->GetCount() )
	{
		return;
	}

	CAccessoryList::AccessoryItem rAccessoryItem = pclPIBCValveActuatorAccessoryList->GetFirst();

	while( rAccessoryItem.IDPtr.MP != NULL )
	{
		VerifyCheckboxAccessories( (CDB_Product *)rAccessoryItem.IDPtr.MP, true, &m_mapVariables[SD_FamilyPIBCValve][eSideDefinition].m_vecValveActuatorAccessoryList );
		rAccessoryItem = pclPIBCValveActuatorAccessoryList->GetNext();
	}
}

void CRViewSSel6WayValve::_SuggestBVHelper( SideDefinition eSideDefinition, CDS_SSel6WayValve *pclSelected6WayValve )
{
	if( NULL == pclSelected6WayValve || NULL == pclSelected6WayValve->GetCDSSSelBv( eSideDefinition ) )
	{
		ASSERT_RETURN;
	}

	int iSheetDescriptionID = ( eSideDefinition << SHEETSIDEDEFSHIFT ) + SD_FamilyBalancingValve + SD_TypeValve;

	if( NULL == m_ViewDescription.GetFromSheetDescriptionID( iSheetDescriptionID ) )
	{
		return;
	}

	CDS_SSelBv *pclSSelBalancingValve = pclSelected6WayValve->GetCDSSSelBv( eSideDefinition );

	// Verify if user has selected a balancing valve.
	CDB_RegulatingValve *pclSelectedBalancingValve = pclSSelBalancingValve->GetProductAs<CDB_RegulatingValve>();

	if( NULL == pclSelectedBalancingValve )
	{
		return;
	}

	CSheetDescription *pclSheetDescriptionBalancingValve = m_ViewDescription.GetFromSheetDescriptionID( iSheetDescriptionID );

	// Find row number where is the balancing valve.
	long lBalancingValveRowSelected = _GetRowOfEditedBValve( pclSheetDescriptionBalancingValve, pclSelectedBalancingValve );

	if( -1 == lBalancingValveRowSelected )
	{
		return;
	}

	CCellDescriptionProduct *pclCDBalancingValve = FindCDProduct( lBalancingValveRowSelected, (LPARAM)pclSelectedBalancingValve, pclSheetDescriptionBalancingValve );

	if( NULL == pclCDBalancingValve || NULL == pclCDBalancingValve->GetProduct() )
	{
		return;
	}

	// HYS-1877: For edition mode click on product only if it is not.
	if( NULL == GetCurrentBalancingValveSelected( eSideDefinition ) )
	{
		// Simulate a click on the edited balancing valve.
		OnClickProduct( pclSheetDescriptionBalancingValve, pclCDBalancingValve, pclSheetDescriptionBalancingValve->GetActiveColumn(), lBalancingValveRowSelected );
	}

	// Allow to check if we need to change the 'Show all priorities' button or not.
	CheckShowAllPrioritiesButtonState( pclSheetDescriptionBalancingValve, lBalancingValveRowSelected );

	// Verify accessories on the balancing valve.
	CAccessoryList *pclBalancingValveAccessoryList = pclSSelBalancingValve->GetAccessoryList();

	if( NULL == pclBalancingValveAccessoryList || 0 == pclBalancingValveAccessoryList->GetCount() )
	{
		return;
	}

	CAccessoryList::AccessoryItem rAccessoryItem = pclBalancingValveAccessoryList->GetFirst();

	while( rAccessoryItem.IDPtr.MP != NULL )
	{
		VerifyCheckboxAccessories( (CDB_Product *)rAccessoryItem.IDPtr.MP, true, &m_mapVariables[SD_FamilyBalancingValve][eSideDefinition].m_vecValveAccessoryList );
		rAccessoryItem = pclBalancingValveAccessoryList->GetNext();
	}
}

void CRViewSSel6WayValve::_FillInSelectedPIBCVHelper( SideDefinition eSideDefinition, CDS_SSel6WayValve *pclSelected6WayValveToFill, bool bIsEditionMode )
{
	if( NULL == pclSelected6WayValveToFill->GetCDSSSelPICv( eSideDefinition ) )
	{
		ASSERT_RETURN;
	}

	// Fill data for the pressure independent balancing & control valve.
	CDB_TAProduct *pclCurrentSelectedPIBCValve = GetCurrentPIBCValveSelected( eSideDefinition );

	if( NULL == pclCurrentSelectedPIBCValve || NULL == m_pclIndSel6WayValveParams->m_pclSelect6WayValveList
			|| false == m_pclIndSel6WayValveParams->m_pclSelect6WayValveList->IsPIBCValveExist( eSideDefinition ) )
	{
		return;
	}

	CDS_SSelPICv *pclSelectedPIBCValveToFill = pclSelected6WayValveToFill->GetCDSSSelPICv( eSideDefinition );

	// Save the pressure independent balancing & control valve IDPtr.
	pclSelectedPIBCValveToFill->SetProductIDPtr( pclCurrentSelectedPIBCValve->GetIDPtr() );

	CAccessoryList *pclPIBCValveAccessoryToFill = pclSelectedPIBCValveToFill->GetCvAccessoryList();

	if( NULL != pclPIBCValveAccessoryToFill )
	{
		vecCDCAccessoryList &vecPIBCValveAccessoryList = m_mapVariables[SD_FamilyPIBCValve][eSideDefinition].m_vecValveAccessoryList;

		// If we are not in edition mode we fill the accessory list. In edition mode we just update accessory list.
		if( false == bIsEditionMode )
		{
			pclPIBCValveAccessoryToFill->Clear();

			// Retrieve selected accessory and add it.
			for( vecCDCAccessoryListIter vecIter = vecPIBCValveAccessoryList.begin(); vecIter != vecPIBCValveAccessoryList.end(); vecIter++ )
			{
				CCDBCheckboxAccessory *pCDBCheckboxControlValveAccessory = *vecIter;

				if( NULL != pCDBCheckboxControlValveAccessory && true == pCDBCheckboxControlValveAccessory->GetCheckStatus() && NULL != pCDBCheckboxControlValveAccessory->GetAccessoryPointer() )
				{
					pclPIBCValveAccessoryToFill->Add( pCDBCheckboxControlValveAccessory->GetAccessoryPointer()->GetIDPtr(), CAccessoryList::_AT_Accessory,
							pCDBCheckboxControlValveAccessory->GetRuledTable() );
				}
			}
		}
		else
		{
			UpdateAccessoryList( vecPIBCValveAccessoryList, pclPIBCValveAccessoryToFill, CAccessoryList::_AT_Accessory );
		}
	}

	// Fill data for actuator selected with the pressure independent balancing & control valve.
	CDB_Actuator *pclCurrentSelectedPIBCValveActuator = GetCurrentPIBCValveActuatorSelected( eSideDefinition );

	if( NULL != pclCurrentSelectedPIBCValveActuator )
	{
		pclSelectedPIBCValveToFill->SetActrIDPtr( pclCurrentSelectedPIBCValveActuator->GetIDPtr() );

		CAccessoryList *pclPIBCValveActuatorAccessoryToFill = pclSelectedPIBCValveToFill->GetActuatorAccessoryList();

		if( NULL != pclPIBCValveActuatorAccessoryToFill )
		{
			vecCDCAccessoryList &vecPIBCValveActuatorAccessoryList = m_mapVariables[SD_FamilyPIBCValve][eSideDefinition].m_vecValveActuatorAccessoryList;

			if( false == bIsEditionMode )
			{
				pclPIBCValveActuatorAccessoryToFill->Clear();

				// Retrieve the selected actuator accessories selected with the pressure independent balancing & control valve and add it.
				for( vecCDCAccessoryListIter vecIter = vecPIBCValveActuatorAccessoryList.begin(); vecIter != vecPIBCValveActuatorAccessoryList.end(); vecIter++ )
				{
					CCDBCheckboxAccessory *pCDBCheckboxPIBCValveActuatorAccessory = *vecIter;

					if( NULL != pCDBCheckboxPIBCValveActuatorAccessory && true == pCDBCheckboxPIBCValveActuatorAccessory->GetCheckStatus() && NULL != pCDBCheckboxPIBCValveActuatorAccessory->GetAccessoryPointer() )
					{
						pclPIBCValveActuatorAccessoryToFill->Add( pCDBCheckboxPIBCValveActuatorAccessory->GetAccessoryPointer()->GetIDPtr(), CAccessoryList::_AT_Accessory,
								pCDBCheckboxPIBCValveActuatorAccessory->GetRuledTable() );
					}
				}
			}
			else
			{
				// For actuator accessories.
				UpdateAccessoryList( vecPIBCValveActuatorAccessoryList, pclPIBCValveActuatorAccessoryToFill, CAccessoryList::_AT_Accessory );
			}
		}
	}

	if( BothSide != eSideDefinition )
	{
		// Selected pipe informations to save in case of cooling or heating.
		if( NULL != m_pclIndSel6WayValveParams->m_pclSelect6WayValveList->GetPICVList( eSideDefinition )->GetSelectPipeList() )
		{
			m_pclIndSel6WayValveParams->m_pclSelect6WayValveList->GetPICVList( eSideDefinition )->GetSelectPipeList()->GetMatchingPipe( pclCurrentSelectedPIBCValve->GetSizeKey(), 
					*pclSelectedPIBCValveToFill->GetpSelPipe() );
		}
	}
}

void CRViewSSel6WayValve::_FillInSelectedBVHelper( SideDefinition eSideDefinition, CDS_SSel6WayValve *pclSelected6WayValveToFill, bool bIsEditionMode )
{
	if( NULL == pclSelected6WayValveToFill->GetCDSSSelBv( eSideDefinition ) )
	{
		ASSERT_RETURN;
	}

	// Fill data for the balancing valve.
	CDB_TAProduct *pclCurrentSelectedBalancingValve = GetCurrentBalancingValveSelected( eSideDefinition );

	if( NULL == pclCurrentSelectedBalancingValve || NULL == m_pclIndSel6WayValveParams->m_pclSelect6WayValveList
			|| false == m_pclIndSel6WayValveParams->m_pclSelect6WayValveList->IsBalancingValveExist( eSideDefinition ) )
	{
		return;
	}

	CDS_SSelBv *pclSelectedBalancingValveToFill = pclSelected6WayValveToFill->GetCDSSSelBv( eSideDefinition );

	// Save the balancing valve IDPtr.
	pclSelectedBalancingValveToFill->SetProductIDPtr( pclCurrentSelectedBalancingValve->GetIDPtr() );

	// Search balancing valve in CSelectList to set the correct opening.
	for( CSelectedValve *pclSelectedValve = m_pclIndSel6WayValveParams->m_pclSelect6WayValveList->GetBVList( eSideDefinition )->GetFirst<CSelectedValve>(); NULL != pclSelectedValve; 
			pclSelectedValve = m_pclIndSel6WayValveParams->m_pclSelect6WayValveList->GetBVList( eSideDefinition )->GetNext<CSelectedValve>() )
	{
		if( pclSelectedValve->GetpData() == pclCurrentSelectedBalancingValve )	// Product found
		{
			pclSelectedBalancingValveToFill->SetOpening( pclSelectedValve->GetH() );	
			break;
		}
	}

	CAccessoryList *pclBalancingValveAccessoryToFill = pclSelectedBalancingValveToFill->GetAccessoryList();

	if( NULL == pclBalancingValveAccessoryToFill )
	{
		return;
	}

	vecCDCAccessoryList &vecBalancingValveAccessoryList = m_mapVariables[SD_FamilyBalancingValve][eSideDefinition].m_vecValveAccessoryList;

	// If we are not in edition mode we fill the accessory list. In edition mode we just update accessory list.
	if( false == bIsEditionMode )
	{
		// Retrieve selected accessory and add it.
		for( vecCDCAccessoryListIter vecIter = vecBalancingValveAccessoryList.begin(); vecIter != vecBalancingValveAccessoryList.end(); vecIter++ )
		{
			CCDBCheckboxAccessory *pCDBCheckboxControlValveAccessory = *vecIter;

			if( NULL != pCDBCheckboxControlValveAccessory && true == pCDBCheckboxControlValveAccessory->GetCheckStatus() && NULL != pCDBCheckboxControlValveAccessory->GetAccessoryPointer() )
			{
				pclBalancingValveAccessoryToFill->Add( pCDBCheckboxControlValveAccessory->GetAccessoryPointer()->GetIDPtr(), CAccessoryList::_AT_Accessory,
						pCDBCheckboxControlValveAccessory->GetRuledTable() );
			}
		}
	}
	else
	{
		UpdateAccessoryList( vecBalancingValveAccessoryList, pclBalancingValveAccessoryToFill, CAccessoryList::_AT_Accessory );
	}

	// Selected pipe informations to save in case of cooling or heating.
	if( NULL != m_pclIndSel6WayValveParams->m_pclSelect6WayValveList->GetBVList( eSideDefinition )->GetSelectPipeList() )
	{
		m_pclIndSel6WayValveParams->m_pclSelect6WayValveList->GetBVList( eSideDefinition )->GetSelectPipeList()->GetMatchingPipe( pclCurrentSelectedBalancingValve->GetSizeKey(), 
				*pclSelectedBalancingValveToFill->GetpSelPipe() );
	}
}

void CRViewSSel6WayValve::_ClickOn6WayValve( CSheetDescription *pclSheetDescription6WayValve, CDB_6WayValve *pcl6WayValveClicked,
		CCellDescriptionProduct *pclCellDescriptionProduct, long lColumn, long lRow )
{
	if( NULL == m_pclIndSel6WayValveParams || NULL == m_pclIndSel6WayValveParams->m_pclSelect6WayValveList )
	{
		ASSERT( 0 );
		return;
	}

	CSSheet *pclSSheet = pclSheetDescription6WayValve->GetSSheetPointer();

	int iSheetDescriptionID = pclSheetDescription6WayValve->GetSheetDescriptionID();
	SideDefinition eSideDefinition = GETSHEETSIDE( iSheetDescriptionID );
	int iSheetType = GETSHEETTYPE( iSheetDescriptionID );
	int iSheetFamily = GETSHEETFAMILY( iSheetDescriptionID );

	// By default clear 6-way valve actuator, control valve and control valve actuators if they exist.
	_ClickOnProductClearHelper( iSheetFamily, eSideDefinition );

	SetCurrent6WayValveActuatorSelected( NULL );

	if( e6Way_EQMControl == m_pclIndSel6WayValveParams->m_e6WayValveSelectionMode )
	{
		SetCurrentPIBCValveSelected( NULL, BothSide );
		SetCurrentPIBCValveActuatorSelected( NULL, BothSide );
	}
	else if( e6Way_OnOffControlWithPIBCV == m_pclIndSel6WayValveParams->m_e6WayValveSelectionMode )
	{
		SetCurrentPIBCValveSelected( NULL, CoolingSide );
		SetCurrentPIBCValveSelected( NULL, HeatingSide );
		SetCurrentPIBCValveActuatorSelected( NULL, CoolingSide );
		SetCurrentPIBCValveActuatorSelected( NULL, HeatingSide );
	}
	else if( e6Way_OnOffControlWithSTAD == m_pclIndSel6WayValveParams->m_e6WayValveSelectionMode )
	{
		SetCurrentBalancingValveSelected( NULL, CoolingSide );
		SetCurrentBalancingValveSelected( NULL, HeatingSide );
	}

	// Memorize previous focus row.
	long lCurrentFocusedRow = GetCurrentFocusedRow();

	// To be sure to clean all previous data concerning focus.
	KillCurrentFocus();

	// Unselect row where current selection is set (yellow light).
	// Remark: 'KillCurrentFocus' unselect only rows where focus is on (blue light).
	pclSSheet->UnSelectMultipleRows();
	
	LPARAM l6WayValveCount;
	pclSheetDescription6WayValve->GetUserVariable( _SDUV_TOTALPRODUCT_COUNT, l6WayValveCount );

	// Retrieve the current selected 6-way valve if exist.
	CDB_6WayValve *pclCurrent6WayValveSelected = GetCurrent6WayValveSelected();

	CCDButtonExpandCollapseRows *&pCDBExpandCollapseRows6WayValve = m_mapVariables[SD_Family6WayValve][BothSide].m_pCDBExpandCollapseRowsValve;
	long &lValveSelectedRow = m_mapVariables[SD_Family6WayValve][BothSide].m_lValveSelectedRow;

	// If there is already one 6-way valve selected and user click on the current one...
	// Remark: 'm_pCDBExpandCollapseRows6WayValve' is not created if there is only one 6-way valve. Thus we need to check first if there is only one 6-way valve.
	//         Otherwise we have to check if user has clicked on the current one.
	if( NULL != pclCurrent6WayValveSelected && ( ( 1 == l6WayValveCount ) || ( NULL != pCDBExpandCollapseRows6WayValve
			&& lRow == pCDBExpandCollapseRows6WayValve->GetCellPosition().y ) ) )
	{
		// Change focus state (selected to normal) and delete Expand/Collapse rows button.
		// HYS-1877: If control valve is part of a set, change box button to open state.
		if( true == pclCurrent6WayValveSelected->IsPartOfaSet() )
		{
			CCDButtonBox* pCDButtonBox = GetCDButtonBox( CD_6WayValve_Box, lRow, pclSheetDescription6WayValve );

			if( NULL != pCDButtonBox )
			{
				pCDButtonBox->SetButtonState( CCDButtonBox::ButtonState::BoxOpened, true );
			}
		}
		// Uncheck checkbox.
		pclSSheet->SetCheckBox( CD_6WayValve_CheckBox, lRow, _T(""), false, true );

		// Reset current product selected.
		SetCurrent6WayValveSelected( NULL );

		// Delete Expand/Collapse rows button if exist.
		// Remark: 'm_pCDBExpandCollapseRows6WayValve' is set to NULL in method!
		DeleteExpandCollapseRowsButton( pCDBExpandCollapseRows6WayValve, pclSheetDescription6WayValve );

		// Set focus on the 6-way valve currently selected.
		PrepareAndSetNewFocus( pclSheetDescription6WayValve, pclSheetDescription6WayValve->GetActiveColumn(), lRow, 0 );

		// Remove all sheets after 6-way valve.
		m_ViewDescription.RemoveAllSheetAfter( SD_6WayValve );

		lValveSelectedRow = 0;

		// Notify a change in the selection.
		SelectionHasChanged( IsSelectionReady() );
	}
	else
	{
		// If a 6-way valve is already selected...
		if( NULL != pclCurrent6WayValveSelected )
		{
			// HYS-1877: Change box button to open state for previous control valve.
			if( true == pclCurrent6WayValveSelected->IsPartOfaSet() )
			{
				CCDButtonBox* pCDButtonBox = GetCDButtonBox( CD_6WayValve_Box, m_mapVariables[SD_Family6WayValve][BothSide].m_lValveSelectedRow,
															 pclSheetDescription6WayValve );

				if( NULL != pCDButtonBox )
				{
					pCDButtonBox->SetButtonState( CCDButtonBox::ButtonState::BoxOpened, true );
				}
			}
			// Uncheck checkbox.
			pclSSheet->SetCheckBox(CD_6WayValve_CheckBox, m_mapVariables[SD_Family6WayValve][BothSide].m_lValveSelectedRow, _T(""), false, true);

			// Remove all sheets after the 6-way valve.
			m_ViewDescription.RemoveAllSheetAfter( SD_6WayValve );
		}

		// Retrieve the 6-way valve selected.
		CSelectedValve *pSelected6WayValve = GetSelectProduct<CSelectedValve>( pcl6WayValveClicked, m_pclIndSel6WayValveParams->m_pclSelect6WayValveList );

		// If the flow is too low and any valve have been found, we show nothing.
		// HYS-1285 : if _VerifyFlows returns true we can show actuator and continue the selection.
		// if it returns false we keep memorize the row selected finish properly this function 
		if( true == _VerifyFlows( pSelected6WayValve ) )
		{
			// Check checkbox.
			pclSSheet->SetCheckBox( CD_6WayValve_CheckBox, lRow, _T(""), true, true );

			// Save new 6-way valve selection.
			SetCurrent6WayValveSelected( pclCellDescriptionProduct );

			// Delete Expand/Collapse rows button if exist.
			if( NULL != pCDBExpandCollapseRows6WayValve )
			{
				// Remark: 'm_pCDBExpandCollapseRows6WayValve' is set to NULL in method!
				DeleteExpandCollapseRowsButton( pCDBExpandCollapseRows6WayValve, pclSheetDescription6WayValve );
			}

			// Create Expand/Collapse rows button if needed...
			LPARAM l6WayValveTotalCount;
			pclSheetDescription6WayValve->GetUserVariable( _SDUV_TOTALPRODUCT_COUNT, l6WayValveTotalCount );

			if( l6WayValveTotalCount > 1 )
			{
				pCDBExpandCollapseRows6WayValve = CreateExpandCollapseRowsButton( CD_6WayValve_FirstColumn, lRow, true, CCDButtonExpandCollapseRows::ButtonState::CollapseRow,
						pclSheetDescription6WayValve->GetFirstSelectableRow(), pclSheetDescription6WayValve->GetLastSelectableRow( false ), pclSheetDescription6WayValve );

				// Show button.
				if( NULL != pCDBExpandCollapseRows6WayValve )
				{
					pCDBExpandCollapseRows6WayValve->SetShowStatus( true );
				}
			}


			// Select 6-way valve.
			pclSSheet->SelectOneRow( lRow, pclSheetDescription6WayValve->GetSelectionFrom(), pclSheetDescription6WayValve->GetSelectionTo() );

			// Fill accessories available for the current 6-way valve.
			_Fill6WayValveAccessories();

			// Fill actuators available for the current 6-way valve.
			_Fill6WayValveActuators();

			if( e6Way_EQMControl == m_pclIndSel6WayValveParams->m_e6WayValveSelectionMode )
			{
				_FillPIBCValveRows( pSelected6WayValve, BothSide );
			}
			else if( e6Way_OnOffControlWithPIBCV == m_pclIndSel6WayValveParams->m_e6WayValveSelectionMode )
			{
				_FillPIBCValveRows( pSelected6WayValve, CoolingSide );
				_FillPIBCValveRows( pSelected6WayValve, HeatingSide );
			}
			else if( e6Way_OnOffControlWithSTAD == m_pclIndSel6WayValveParams->m_e6WayValveSelectionMode )
			{
				_FillBalancingValveRows( pSelected6WayValve, CoolingSide );
				_FillBalancingValveRows( pSelected6WayValve, HeatingSide );
			}
		
			// Memorize 6-way valve row selected.
			// Remark: Do it here because we need this variable when calling the '_ClickOnControlPIBCV' or '_ClickOnControlBalancingValve' below.
			lValveSelectedRow = lRow;
		}

		// Try to set the focus on the next table but do not lose the focus if no other table exist.
		CSheetDescription *pclNextSheetDescription = NULL;
		long lNewFocusedRow;
		bool bShiftPressed;


		if( DNFERRC_FocusOnNextGroupOK == DefineNextFocusedRow( pclSheetDescription6WayValve, CD_6WayValve_FirstColumn, lRow, false, lNewFocusedRow,
				pclNextSheetDescription, bShiftPressed ) )
		{
			long lNewFocusedCol = pclNextSheetDescription->GetFirstParameterColumn();
			PrepareAndSetNewFocus( pclNextSheetDescription, lNewFocusedCol, lNewFocusedRow );
		}
		else
		{
			// Because we have changed current selection background in yellow, if there is no other sheet following current one, we
			// must set focus on current selection.
			PrepareAndSetNewFocus( pclSheetDescription6WayValve, lColumn, lRow );
		}

		// Notify a change in the selection.
		SelectionHasChanged( IsSelectionReady() );
	}
}

long CRViewSSel6WayValve::_Fill6WayValveRows( CDS_SSel6WayValve *pEdited6WayValve )
{
	if( NULL == m_pclIndSel6WayValveParams || NULL == m_pclIndSel6WayValveParams->m_pclSelect6WayValveList )
	{
		ASSERTA_RETURN( 0 );
	}

	CDB_TAProduct *pEditedTAP = NULL;

	if( NULL != pEdited6WayValve && _NULL_IDPTR != pEdited6WayValve->Get6WayValveIDPtr() )
	{
		pEditedTAP = dynamic_cast<CDB_TAProduct *>( pEdited6WayValve->Get6WayValveIDPtr().MP );
	}

	CSelectedBase *pclSelectedProduct = m_pclIndSel6WayValveParams->m_pclSelect6WayValveList->GetFirst<CSelectedBase>();

	if( NULL == pclSelectedProduct )
	{
		return 0;
	}

	// Call 'MultiSpreadBase' method to get a new SSheet.
	CSheetDescription *pclSheetDescription6WayValve = CreateSSheet( SD_6WayValve );

	if( NULL == pclSheetDescription6WayValve || NULL == pclSheetDescription6WayValve->GetSSheetPointer() )
	{
		return 0;
	}

	CSSheet *pclSSheet = pclSheetDescription6WayValve->GetSSheetPointer();

	pclSSheet->SetBool( SSB_REDRAW, FALSE );

	pclSSheet->Init();
	pclSSheet->SetBool( SSB_ALLOWDRAGDROP, FALSE );
	pclSSheet->SetBool( SSB_HORZSCROLLBAR, FALSE );
	pclSSheet->SetBool( SSB_VERTSCROLLBAR, FALSE );
	pclSSheet->SetBool( SSB_MOVEACTIVEONFOCUS, FALSE );
	pclSSheet->SetTextTip( SS_TT_STATUS_FLOATING, 250, NULL, _LIGHTYELLOW, _TAH_BLACK );

	// Initialize 6-way valve sheet and fill header.
	_InitAndFill6WayValveHeader( pclSheetDescription6WayValve, pclSSheet );

	long lRetRow = -1;
	long lRow = RD_6WayValve_FirstAvailRow;

	long lValveTotalCount = 0;
	pclSheetDescription6WayValve->SetUserVariable( _SDUV_TOTALPRODUCT_COUNT, 0 );
	bool bAtLeastOne6WValvePartOfASet = false;
	pclSheetDescription6WayValve->RestartRemarkGenerator();

	for( CSelectedValve *pclSelected6WayValve = m_pclIndSel6WayValveParams->m_pclSelect6WayValveList->GetFirst<CSelectedValve>(); NULL != pclSelected6WayValve;
			pclSelected6WayValve = m_pclIndSel6WayValveParams->m_pclSelect6WayValveList->GetNext<CSelectedValve>() )
	{
		CDB_6WayValve *pcl6WayValve = dynamic_cast<CDB_6WayValve *>( pclSelected6WayValve->GetpData() );

		if( NULL == pcl6WayValve )
		{
			continue;
		}

		long lRetVal = _FillOne6WayValveRow( pclSheetDescription6WayValve, pclSSheet, pclSelected6WayValve, lRow, pEditedTAP );

		// HYS-1877
		if( NULL != pclSheetDescription6WayValve->GetCellDescription( CD_6WayValve_Box, lRow ) )
		{
			bAtLeastOne6WValvePartOfASet = true;
		}

		if( lRetVal > 0 )
		{
			lRetRow = lRetVal;
		}

		lRow++;
		lValveTotalCount++;
	}

	// HYS-1877: If we are not in selection by package mode OR if there is no valve that belongs to a set...
	if( false == m_pclIndSel6WayValveParams->m_bOnlyForSet || false == bAtLeastOne6WValvePartOfASet )
	{
		pclSSheet->ShowCol( CD_6WayValve_Box, FALSE );
	}

	long lLastDataRow = lRow - 1;
	pclSheetDescription6WayValve->SetUserVariable( _SDUV_TOTALPRODUCT_COUNT, lValveTotalCount );

	pclSSheet->SetCellBorder( CD_6WayValve_CheckBox, lLastDataRow, CD_6WayValve_Pointer - 1, lLastDataRow, true, SS_BORDERTYPE_BOTTOM, 
			SS_BORDERSTYLE_SOLID, _BLACK );

	// Add Remarks.
	// Note: these lines must be placed after creating Show/Hide all priorities button to avoid to take in count these lines when defining
	//       range of row to be Show/Hide.
	lRow = pclSheetDescription6WayValve->WriteRemarks( lRow, CD_6WayValve_CheckBox, CD_6WayValve_TemperatureRange );

	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

	pclSSheet->SetBool( SSB_REDRAW, TRUE );

	// Set that there is no selection at now.
	SetCurrent6WayValveSelected( NULL );

	// Add possibility to change column size.
	// Remark: place this code after any initialization (like adding, removing, resizing, hiding/showing columns).
	CResizingColumnInfo *pclResizingColumnInfo = pclSheetDescription6WayValve->GetResizingColumnInfo();
	pclResizingColumnInfo->ActivateFeature( true, false, RD_6WayValve_FirstAvailRow, lLastDataRow );
	pclResizingColumnInfo->AddRangeColumn( CD_6WayValve_Name, CD_6WayValve_TemperatureRange, RD_6WayValve_ColName, RD_6WayValve_Unit );

	// Add possibility to click on title group to reset column width.
	CreateTitleGroupButtons( CD_6WayValve_Box, CD_6WayValve_TemperatureRange, RD_6WayValve_GroupName, pclSheetDescription6WayValve );

	return lRetRow;
}

void CRViewSSel6WayValve::_InitAndFill6WayValveHeader( CSheetDescription *pclSheetDescription6WayValve, CSSheet *pclSSheet )
{
	if( NULL == m_pclIndSel6WayValveParams || NULL == m_pclIndSel6WayValveParams->m_pPipeDB )
	{
		ASSERT_RETURN;
	}

	// Set title.
	pclSSheet->SetMaxRows( RD_6WayValve_FirstAvailRow - 1 );

	// All cells are static by default and filled with _T("").
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	pclSSheet->FormatStaticText( -1, -1, -1, -1, _T( "" ) );

	// Increase row height.
	double dRowHeight = 12.75;
	pclSSheet->SetRowHeight( RD_6WayValve_FirstRow, dRowHeight * 0.5 );
	pclSSheet->SetRowHeight( RD_6WayValve_GroupName, dRowHeight * 1.5 );
	pclSSheet->SetRowHeight( RD_6WayValve_ColName, dRowHeight * 2 );
	pclSSheet->SetRowHeight( RD_6WayValve_Unit, dRowHeight * 1.2 );

	// Set columns.
	pclSheetDescription6WayValve->Init( 1, pclSSheet->GetMaxRows(), 1, CSheetDescription::SD_ParameterMode_Mono );
	pclSheetDescription6WayValve->AddColumnInPixels( CD_6WayValve_FirstColumn, m_mapSSheetColumnWidth[SD_6WayValve][CD_6WayValve_FirstColumn] );
	pclSheetDescription6WayValve->AddColumnInPixels( CD_6WayValve_Box, m_mapSSheetColumnWidth[SD_6WayValve][CD_6WayValve_Box] );
	pclSheetDescription6WayValve->AddColumnInPixels( CD_6WayValve_CheckBox, m_mapSSheetColumnWidth[SD_6WayValve][CD_6WayValve_CheckBox] );
	pclSheetDescription6WayValve->AddColumnInPixels( CD_6WayValve_Name, m_mapSSheetColumnWidth[SD_6WayValve][CD_6WayValve_Name] );
	pclSheetDescription6WayValve->AddColumnInPixels( CD_6WayValve_Material, m_mapSSheetColumnWidth[SD_6WayValve][CD_6WayValve_Material] );
	// HYS-1884: One column connection
	pclSheetDescription6WayValve->AddColumnInPixels( CD_6WayValve_Connection, m_mapSSheetColumnWidth[SD_6WayValve][CD_6WayValve_Connection] );
	pclSheetDescription6WayValve->AddColumnInPixels( CD_6WayValve_Version, m_mapSSheetColumnWidth[SD_6WayValve][CD_6WayValve_Version] );
	pclSheetDescription6WayValve->AddColumnInPixels( CD_6WayValve_PN, m_mapSSheetColumnWidth[SD_6WayValve][CD_6WayValve_PN] );
	pclSheetDescription6WayValve->AddColumnInPixels( CD_6WayValve_Size, m_mapSSheetColumnWidth[SD_6WayValve][CD_6WayValve_Size] );
	pclSheetDescription6WayValve->AddColumnInPixels( CD_6WayValve_DpCooling, m_mapSSheetColumnWidth[SD_6WayValve][CD_6WayValve_DpCooling] );
	pclSheetDescription6WayValve->AddColumnInPixels( CD_6WayValve_DpHeating, m_mapSSheetColumnWidth[SD_6WayValve][CD_6WayValve_DpHeating] );
	pclSheetDescription6WayValve->AddColumnInPixels( CD_6WayValve_TemperatureRange, m_mapSSheetColumnWidth[SD_6WayValve][CD_6WayValve_TemperatureRange] );
	pclSheetDescription6WayValve->AddColumnInPixels( CD_6WayValve_Pointer, m_mapSSheetColumnWidth[SD_6WayValve][CD_6WayValve_Pointer] );

	// Set in which column parameter must be saved.
	pclSheetDescription6WayValve->AddParameterColumn( CD_6WayValve_Pointer );

	// Set the focus column.
	pclSheetDescription6WayValve->SetActiveColumn( CD_6WayValve_Name );

	// Set range for selection.
	pclSheetDescription6WayValve->SetFocusColumnRange( CD_6WayValve_CheckBox, CD_6WayValve_TemperatureRange );

	// Hide columns corresponding to user selected combos.
	pclSSheet->ShowCol( CD_6WayValve_Connection, ( true == m_pclIndSel6WayValveParams->m_strComboConnectID.IsEmpty() ) ? TRUE : FALSE );
	pclSSheet->ShowCol( CD_6WayValve_Version, ( true == m_pclIndSel6WayValveParams->m_strComboVersionID.IsEmpty() ) ? TRUE : FALSE );

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Row name.

	// Add the main title.
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );

	SideDefinition eSideDefinition = GETSHEETSIDE( pclSheetDescription6WayValve->GetSheetDescriptionID() );
	int iSheetType = GETSHEETTYPE( pclSheetDescription6WayValve->GetSheetDescriptionID() );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)_GetGroupColor( true, eSideDefinition, iSheetType ) );

	pclSheetDescription6WayValve->SetUserVariable( _SDUV_TITLEFORECOLOR, ( pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor ) ) );
	pclSheetDescription6WayValve->SetUserVariable( _SDUV_TITLEBACKCOLOR, ( pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor ) ) );

	pclSSheet->AddCellSpanW( CD_6WayValve_CheckBox, RD_6WayValve_GroupName, CD_6WayValve_Pointer - CD_6WayValve_CheckBox, 1 );
	pclSSheet->SetStaticText( CD_6WayValve_CheckBox, RD_6WayValve_GroupName, IDS_SSHEETSSEL6WAYVALVE_6WAYVALVEGROUP );

	CTable *pclTable = dynamic_cast<CTable *>( m_pclIndSel6WayValveParams->m_pPipeDB->Get( m_pclIndSel6WayValveParams->m_strPipeSeriesID ).MP );
	ASSERT( NULL != pclTable );

	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

	pclSSheet->SetStaticText( CD_6WayValve_Name, RD_6WayValve_ColName, IDS_SSHEETSSEL_NAME );
	pclSSheet->SetStaticText( CD_6WayValve_Material, RD_6WayValve_ColName, IDS_SSHEETSSEL_MATERIAL );
	// HYS-1884: One column connection
	pclSSheet->SetStaticText( CD_6WayValve_Connection, RD_6WayValve_ColName, IDS_SSHEETSSEL6WAYVALVE_CONNECT );
	pclSSheet->SetStaticText( CD_6WayValve_Version, RD_6WayValve_ColName, IDS_SSHEETSSEL_VERSION );
	pclSSheet->SetStaticText( CD_6WayValve_PN, RD_6WayValve_ColName, IDS_SSHEETSSEL_PN );
	pclSSheet->SetStaticText( CD_6WayValve_Size, RD_6WayValve_ColName, IDS_SSHEETSSEL_SIZE );
	pclSSheet->SetStaticText( CD_6WayValve_DpCooling, RD_6WayValve_ColName, IDS_SSHEETSSEL6WAYVALVE_DPCOOLING );
	pclSSheet->SetStaticText( CD_6WayValve_DpHeating, RD_6WayValve_ColName, IDS_SSHEETSSEL6WAYVALVE_DPHEATING );
	pclSSheet->SetStaticText( CD_6WayValve_TemperatureRange, RD_6WayValve_ColName, IDS_SSHEETSSEL_COLTEMPRANGE );
	//
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Row units.
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();
	pclSSheet->SetStaticText( CD_6WayValve_DpCooling, RD_6WayValve_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ) ).c_str() );
	pclSSheet->SetStaticText( CD_6WayValve_DpHeating, RD_6WayValve_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ) ).c_str() );
	pclSSheet->SetStaticText( CD_6WayValve_TemperatureRange, RD_6WayValve_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_TEMPERATURE ) ).c_str() );
	//
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Draw black line below column header.
	pclSSheet->SetCellBorder( CD_6WayValve_CheckBox, RD_6WayValve_Unit, CD_6WayValve_Pointer - 1, RD_6WayValve_Unit, true, SS_BORDERTYPE_BOTTOM );
}

long CRViewSSel6WayValve::_FillOne6WayValveRow( CSheetDescription *pclSheetDescription6WayValve, CSSheet *pclSSheet, CSelectedValve *pSelectedTAP, long lRow, 
		CDB_TAProduct *pEditedTAP )
{
	if( NULL == m_pclIndSel6WayValveParams || NULL == m_pclIndSel6WayValveParams->m_pclSelect6WayValveList || NULL == m_pclIndSel6WayValveParams->m_pclSelect6WayValveList->GetSelectPipeList() )
	{
		ASSERTA_RETURN( -1 );
	}
	
	// No need to verify 'pclSheetDescription6WayValve' and 'pclSSheet', it has been already done in '_Fill6WayValveRows'.
	CDS_TechnicalParameter *pTechParam = m_pclIndSel6WayValveParams->m_pTADS->GetpTechParams();

	CDB_6WayValve *pcl6WayValve = dynamic_cast<CDB_6WayValve *>( pSelectedTAP->GetpData() );

	if( NULL == pcl6WayValve )
	{
		ASSERTA_RETURN( -1 );
	}

	long lRetRow = 0;
	bool bAtLeastOneError = false;
	bool bBest = pSelectedTAP->IsFlagSet( CSelectedBase::eBest );

	// Add this row.
	// 'true' to specify that this row can be selected.
	pclSheetDescription6WayValve->AddRows( 1, true );

	// First columns will be set at the end!
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

	// HYS-1877: Add the picture for a 6-way valve Act-Set.
	bool bAtLeastOne6WValvePartOfASet = false;
	if( true == m_pclIndSel6WayValveParams->m_bOnlyForSet && true == pcl6WayValve->IsPartOfaSet() )
	{
		CCDButtonBox* pCDButtonBox = CreateCellDescriptionBox( CD_6WayValve_Box, lRow, true, CCDButtonBox::ButtonState::BoxOpened, pclSheetDescription6WayValve );

		if( NULL != pCDButtonBox )
		{
			pCDButtonBox->ApplyInternalChange();
		}

		bAtLeastOne6WValvePartOfASet = true;
	}

	// Add checkbox.
	pclSSheet->SetCheckBox( CD_6WayValve_CheckBox, lRow, _T(""), false, true );

	// Set the temperature range.
	if( true == pSelectedTAP->IsFlagSet( CSelectedBase::eTemperature ) )
	{
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_RED );
		bAtLeastOneError = true;
	}

	pclSSheet->SetStaticText( CD_6WayValve_TemperatureRange, lRow, ( (CDB_TAProduct *)pSelectedTAP->GetProductIDPtr().MP )->GetTempRange() );
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
	if( true == pcl6WayValve->IsDeleted() )
	{
		pclSheetDescription6WayValve->WriteTextWithFlags( CString( pcl6WayValve->GetName() ), CD_6WayValve_Name, lRow, 
				CSheetDescription::RemarkFlags::Deleted, _T(""), _RED );
	}
	else if( false == pcl6WayValve->IsAvailable() )
	{
		pclSheetDescription6WayValve->WriteTextWithFlags( CString( pcl6WayValve->GetName() ), CD_6WayValve_Name, lRow, 
				CSheetDescription::RemarkFlags::NotAvailable, _T(""), _RED );
	}
	else
	{
		pclSSheet->SetStaticText( CD_6WayValve_Name, lRow, pcl6WayValve->GetName() );
	}

	if( true == bAtLeastOneError )
	{
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );
	}
	else if( true == pSelectedTAP->IsFlagSet( CSelectedBase::eBest ) )
	{
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_GREEN );
	}

	pclSSheet->SetStaticText( CD_6WayValve_Material, lRow, pcl6WayValve->GetBodyMaterial() );
	// HYS-1884: One column connection
	pclSSheet->SetStaticText( CD_6WayValve_Connection, lRow, pcl6WayValve->GetConnect() );
	pclSSheet->SetStaticText( CD_6WayValve_Version, lRow, pcl6WayValve->GetVersion() );
	pclSSheet->SetStaticText( CD_6WayValve_PN, lRow, pcl6WayValve->GetPN().c_str() );
	pclSSheet->SetStaticText( CD_6WayValve_Size, lRow, pcl6WayValve->GetSize() );

	// Dp in cooling mode.
	double dDp = CalcDp( m_pclIndSel6WayValveParams->GetCoolingFlow(), pcl6WayValve->GetKvs(), m_pclIndSel6WayValveParams->GetCoolingWaterChar().GetDens() );
	pclSSheet->SetStaticText( CD_6WayValve_DpCooling, lRow, WriteCUDouble( _U_DIFFPRESS, dDp ) );

	// Dp in heating mode.
	dDp = CalcDp( m_pclIndSel6WayValveParams->GetHeatingFlow(), pcl6WayValve->GetKvs(), m_pclIndSel6WayValveParams->GetHeatingWaterChar().GetDens() );
	pclSSheet->SetStaticText( CD_6WayValve_DpHeating, lRow, WriteCUDouble( _U_DIFFPRESS, dDp ) );

	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

	// Save parameter.
	CreateCellDescriptionProduct( pclSheetDescription6WayValve->GetFirstParameterColumn(), lRow, (LPARAM)pcl6WayValve, pclSheetDescription6WayValve );

	if( NULL != pEditedTAP && pEditedTAP == pcl6WayValve )
	{
		lRetRow = lRow;
	}

	pclSSheet->SetCellBorder( CD_6WayValve_Box, lRow, CD_6WayValve_Pointer - 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _GRAY );

	return lRetRow;
}

void CRViewSSel6WayValve::_Fill6WayValveAccessories( )
{
	if( NULL == m_pclIndSel6WayValveParams || NULL == m_pclIndSel6WayValveParams->m_pTADB )
	{
		ASSERT_RETURN;
	}

	// Check the current 6-way valve selected.
	CDB_6WayValve *pclCurrent6WayValveSelected = GetCurrent6WayValveSelected();

	if( NULL == pclCurrent6WayValveSelected )
	{
		return;
	}

	CDB_RuledTable* pclRuledTable = NULL;
	// HYS-1877: Manage also set accessories.
	if( true == m_pclIndSel6WayValveParams->m_bOnlyForSet )
	{
		CDB_Set* pclCurrentValveSet = NULL;
		// Retrieve the correct set table in regards to the control valve.
		CTableSet* pTableSet = pclCurrent6WayValveSelected->GetTableSet();

		if( NULL != pTableSet )
		{
			pclCurrentValveSet = pTableSet->FindCompatibleSet( pclCurrent6WayValveSelected->GetIDPtr().ID, _T("") );
		}

		if( NULL == pclCurrentValveSet )
		{
			return;
		}

		pclRuledTable = (CDB_RuledTable*)(pclCurrentValveSet->GetAccGroupIDPtr().MP);
	}
	else
	{
		pclRuledTable = (CDB_RuledTable*)(pclCurrent6WayValveSelected->GetAccessoriesGroupIDPtr().MP);
	}

	if( NULL == pclRuledTable )
	{
		return;
	}

	CRank rList;
	m_pclIndSel6WayValveParams->m_pTADB->GetAccessories( &rList, pclRuledTable, m_pclIndSel6WayValveParams->m_eFilterSelection );
	_FillAccessoryRows( BothSide, SD_Family6WayValve, SD_TypeValveAccessories, &rList, pclRuledTable );
}

void CRViewSSel6WayValve::_FillErrorMessages( CSheetDescription *pclSheetDescription, long lRow, CString strMsg )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() )
	{
		return;
	}

	CSSheet *pclSheet = pclSheetDescription->GetSSheetPointer();
	long lFromColumn;
	long lToColumn;
	pclSheetDescription->GetFocusColumnRange( lFromColumn, lToColumn );

	if( lToColumn - lFromColumn < 0 )
	{
		return;
	}

	pclSheetDescription->AddRows( 1 );

	pclSheet->AddCellSpan( lFromColumn, lRow, lToColumn - lFromColumn + 1, 1 );
	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	pclSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
	pclSheet->SetTextPatternProperty( CSSheet::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_LEFT );
	//pclSheet->SetTextPatternProperty( CSSheet::TPP_FontBackColor, (LPARAM)_WHITE );
	pclSheet->SetTextPatternProperty( CSSheet::TPP_FontForeColor, (LPARAM)_RED );
	pclSheet->SetStaticText( lFromColumn, lRow, strMsg );

	pclSheet->SetCellBorder( lFromColumn, lRow, lToColumn, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );
}

void CRViewSSel6WayValve::_Fill6WayValveActuators()
{
	if( NULL == m_pclIndSel6WayValveParams || NULL == m_pclIndSel6WayValveParams->GetSelectCtrlList() )
	{
		ASSERT_RETURN;
	}

	CDB_6WayValve *pclSelected6WayValve = GetCurrent6WayValveSelected();

	if( NULL == pclSelected6WayValve )
	{
		return;
	}

	CRank clActuatorList;
	_Get6WayValveActuatorList( pclSelected6WayValve, &clActuatorList );

	if( 0 == clActuatorList.GetCount() )
	{
		return;
	}

	// Call 'MultiSpreadBase' method to get a new SSheet.
	UINT uiAfterSheetDescriptionID = ( true == m_ViewDescription.IsSheetDescriptionExist( SD_6WayValveAccessories ) ) ? SD_6WayValveAccessories : SD_6WayValve;
	CSheetDescription *pclSheetDescription6WayValveActuator = CreateSSheet( SD_6WayValveActuator, uiAfterSheetDescriptionID );

	if( NULL == pclSheetDescription6WayValveActuator || NULL == pclSheetDescription6WayValveActuator->GetSSheetPointer() )
	{
		return;
	}

	CSSheet *pclSSheet = pclSheetDescription6WayValveActuator->GetSSheetPointer();

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
	pclSheetDescription6WayValveActuator->Init( 1, pclSSheet->GetMaxRows(), 1, CSheetDescription::SD_ParameterMode_Mono );
	pclSheetDescription6WayValveActuator->AddColumnInPixels( CD_Actuator_FirstColumn, m_mapSSheetColumnWidth[SD_6WayValveActuator][CD_Actuator_FirstColumn] );
	pclSheetDescription6WayValveActuator->AddColumnInPixels( CD_Actuator_Box, m_mapSSheetColumnWidth[SD_6WayValveActuator][CD_Actuator_Box] );
	pclSheetDescription6WayValveActuator->AddColumnInPixels( CD_Actuator_CheckBox, m_mapSSheetColumnWidth[SD_6WayValveActuator][CD_Actuator_CheckBox] );
	pclSheetDescription6WayValveActuator->AddColumnInPixels( CD_Actuator_Name, m_mapSSheetColumnWidth[SD_6WayValveActuator][CD_Actuator_Name] );
	pclSheetDescription6WayValveActuator->AddColumnInPixels( CD_Actuator_CloseOffValue, m_mapSSheetColumnWidth[SD_6WayValveActuator][CD_Actuator_CloseOffValue] );
	pclSheetDescription6WayValveActuator->AddColumnInPixels( CD_Actuator_ActuatingTime, m_mapSSheetColumnWidth[SD_6WayValveActuator][CD_Actuator_ActuatingTime] );
	pclSheetDescription6WayValveActuator->AddColumnInPixels( CD_Actuator_IP, m_mapSSheetColumnWidth[SD_6WayValveActuator][CD_Actuator_IP] );
	pclSheetDescription6WayValveActuator->AddColumnInPixels( CD_Actuator_PowSupply, m_mapSSheetColumnWidth[SD_6WayValveActuator][CD_Actuator_PowSupply] );
	pclSheetDescription6WayValveActuator->AddColumnInPixels( CD_Actuator_InputSig, m_mapSSheetColumnWidth[SD_6WayValveActuator][CD_Actuator_InputSig] );
	pclSheetDescription6WayValveActuator->AddColumnInPixels( CD_Actuator_OutputSig, m_mapSSheetColumnWidth[SD_6WayValveActuator][CD_Actuator_OutputSig] );
	pclSheetDescription6WayValveActuator->AddColumnInPixels( CD_Actuator_RelayType, m_mapSSheetColumnWidth[SD_6WayValveActuator][CD_Actuator_RelayType] );
	pclSheetDescription6WayValveActuator->AddColumnInPixels( CD_Actuator_FailSafe, m_mapSSheetColumnWidth[SD_6WayValveActuator][CD_Actuator_FailSafe] );
	pclSheetDescription6WayValveActuator->AddColumnInPixels( CD_Actuator_DefaultReturnPos, m_mapSSheetColumnWidth[SD_6WayValveActuator][CD_Actuator_DefaultReturnPos] );
	pclSheetDescription6WayValveActuator->AddColumnInPixels( CD_Actuator_MaxTemp, m_mapSSheetColumnWidth[SD_6WayValveActuator][CD_Actuator_MaxTemp] );
	pclSheetDescription6WayValveActuator->AddColumnInPixels( CD_Actuator_Pointer, m_mapSSheetColumnWidth[SD_6WayValveActuator][CD_Actuator_Pointer] );

	// Set in which column parameter must be saved.
	pclSheetDescription6WayValveActuator->AddParameterColumn( CD_Actuator_Pointer );

	// Set the focus column.
	pclSheetDescription6WayValveActuator->SetActiveColumn( CD_Actuator_Name );

	// Set selectable rows.
	pclSheetDescription6WayValveActuator->SetSelectableRangeRow( RD_Actuator_FirstAvailRow, pclSSheet->GetMaxRows() );

	// Set range for selection.
	pclSheetDescription6WayValveActuator->SetFocusColumnRange( CD_Actuator_CheckBox, CD_Actuator_MaxTemp );

	// Add the main title.
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );

	SideDefinition eSideDefinition = GETSHEETSIDE( pclSheetDescription6WayValveActuator->GetSheetDescriptionID() );
	int iSheetType = GETSHEETTYPE( pclSheetDescription6WayValveActuator->GetSheetDescriptionID() );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)_GetGroupColor( false, eSideDefinition, iSheetType ) );
	
	pclSheetDescription6WayValveActuator->SetUserVariable( _SDUV_TITLEFORECOLOR, ( pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor ) ) );
	pclSheetDescription6WayValveActuator->SetUserVariable( _SDUV_TITLEBACKCOLOR, ( pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor ) ) );

	pclSSheet->AddCellSpanW( CD_Actuator_CheckBox, RD_Actuator_GroupName, CD_Actuator_Pointer - CD_Actuator_CheckBox, 1 );
	pclSSheet->SetStaticText( CD_Actuator_CheckBox, RD_Actuator_GroupName, IDS_SSHEETSSEL6WAYVALVE_6WAYVALVEACTRGROUP );

	// Columns title.
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

	pclSSheet->SetStaticText( CD_Actuator_Name, RD_Actuator_ColName, IDS_SSHEETSSELCV_ACTRNAME );

	pclSSheet->ShowCol( CD_Actuator_CloseOffValue, FALSE );
	pclSSheet->ShowCol( CD_Actuator_FailSafe, FALSE );
	pclSSheet->ShowCol( CD_Actuator_DefaultReturnPos, FALSE );
	
	pclSSheet->SetStaticText( CD_Actuator_ActuatingTime, RD_Actuator_ColName, IDS_SSHEETSSELCV_ACTUATINGTIME );
	pclSSheet->SetStaticText( CD_Actuator_IP, RD_Actuator_ColName, IDS_SSHEETSSELCV_ACTRIP );
	pclSSheet->SetStaticText( CD_Actuator_PowSupply, RD_Actuator_ColName, IDS_SSHEETSSELCV_ACTRPOWERSUP );
	pclSSheet->SetStaticText( CD_Actuator_InputSig, RD_Actuator_ColName, IDS_SSHEETSSELCV_ACTRINPUTSIG );
	pclSSheet->SetStaticText( CD_Actuator_OutputSig, RD_Actuator_ColName, IDS_SSHEETSSELCV_ACTROUTPUTSIG );
	pclSSheet->SetStaticText( CD_Actuator_RelayType, RD_Actuator_ColName, IDS_SSHEETSSELCV_ACTRRELAYTYPE );
	
	// HYS-726: To display max. temp. info for actuator.
	pclSSheet->SetStaticText( CD_Actuator_MaxTemp, RD_Actuator_ColName, IDS_SSHEETSSELCV_ACTRMAXTEMP );
	pclSSheet->SetStaticText( CD_Actuator_MaxTemp, RD_Actuator_Unit, IDS_UNITDEGREECELSIUS );

	// Unit.
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();

	pclSSheet->SetStaticText( CD_Actuator_ActuatingTime, RD_Actuator_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_TIME ) ).c_str() );

	pclSSheet->SetCellBorder( CD_Actuator_CheckBox, RD_Actuator_Unit, CD_Actuator_Pointer - 1, RD_Actuator_Unit, true, 
			SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );

	// HYS-1877
	bool bAtLeastOneActrPartOfASet = false;

	// Fill actuator list.
	long lRow = RD_Actuator_FirstAvailRow;
	long l6WayValveActuatorCount = 0;
	pclSheetDescription6WayValveActuator->SetUserVariable( _SDUV_TOTALACTUATOR_COUNT, 0 );
	CString str;
	LPARAM lParam;

	for( BOOL bContinue = clActuatorList.GetFirst( str, lParam ); TRUE == bContinue; bContinue = clActuatorList.GetNext( str, lParam ) )
	{
		CDB_ElectroActuator *pElectroActuator = dynamic_cast<CDB_ElectroActuator *>( (CData *)lParam );

		if( NULL == pElectroActuator )
		{
			continue;
		}

		// Add this row.
		// 'true' to specify that this row can be selected.
		pclSheetDescription6WayValveActuator->AddRows( 1, true );

		// Add checkbox.
		pclSSheet->SetCheckBox( CD_Actuator_CheckBox, lRow, _T(""), false, true );
		
		// HYS-1877: Add the picture for a PIBCV Act-Set.
		if( true == m_pclIndSel6WayValveParams->m_bOnlyForSet && true == pElectroActuator->IsPartOfaSet() )
		{
			CCDButtonBox* pCDButtonBox = CreateCellDescriptionBox( CD_Actuator_Box, lRow, true, CCDButtonBox::ButtonState::BoxOpened, pclSheetDescription6WayValveActuator );

			if( NULL != pCDButtonBox )
			{
				pCDButtonBox->ApplyInternalChange();
			}

			bAtLeastOneActrPartOfASet = true;
		}

		if( m_pclIndSel6WayValveParams->GetHeatingWaterChar().GetTemp() > pElectroActuator->GetTmax() )
		{
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_RED );

			pclSSheet->SetStaticText( CD_Actuator_Name, lRow, pElectroActuator->GetName() );
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );
		}
		else
		{
			pclSSheet->SetStaticText( CD_Actuator_Name, lRow, pElectroActuator->GetName() );
		}

		// Compute the actuating time.
		// Remark: actuating Time is given in s/m or in s/deg; stroke is given in m or in deg.
		pclSSheet->SetStaticText( CD_Actuator_ActuatingTime, lRow, pElectroActuator->GetActuatingTimesStr( pclSelected6WayValve->GetStroke() ) );

		CString str = pElectroActuator->GetIPxxFull();
		pclSSheet->SetStaticText( CD_Actuator_IP, lRow, str );

		CDB_StringID *pclPowerSupply = (CDB_StringID *)( m_pclIndSel6WayValveParams->m_pTADB->Get( (LPCTSTR)m_pclIndSel6WayValveParams->m_strActuatorPowerSupplyID ).MP );
		
		if( ( false == m_pclIndSel6WayValveParams->m_strActuatorPowerSupplyID.IsEmpty() )
				&& ( false == pElectroActuator->IsPowerSupplyAvailable( pclPowerSupply->GetIDPtr() ) ) )
		{
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_ORANGE );
		}
		
		pclSSheet->SetStaticText( CD_Actuator_PowSupply, lRow, pElectroActuator->GetPowerSupplyStr() );
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

		pclSSheet->SetStaticText( CD_Actuator_InputSig, lRow, pElectroActuator->GetInOutSignalsStr( true ) );
		pclSSheet->SetStaticText( CD_Actuator_OutputSig, lRow, pElectroActuator->GetInOutSignalsStr( false ) );
		pclSSheet->SetStaticText( CD_Actuator_RelayType, lRow, pElectroActuator->GetRelayStr() );

		CString sTmaxMedia = ( DBL_MAX == pElectroActuator->GetTmax() ) ? _T("-") : WriteDouble( pElectroActuator->GetTmax(), 3 );

		if( m_pclIndSel6WayValveParams->GetHeatingWaterChar().GetTemp() > pElectroActuator->GetTmax() )
		{
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_RED );
			pclSSheet->SetStaticText( CD_Actuator_MaxTemp, lRow, sTmaxMedia );
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );
		}
		else
		{
			pclSSheet->SetStaticText( CD_Actuator_MaxTemp, lRow, sTmaxMedia );
		}

		pclSSheet->SetCellBorder( CD_Actuator_Box, lRow, CD_Actuator_Pointer - 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _GRAY );

		// Save parameter.
		CreateCellDescriptionProduct( pclSheetDescription6WayValveActuator->GetFirstParameterColumn(), lRow, (LPARAM)pElectroActuator, pclSheetDescription6WayValveActuator );

		lRow++;
		l6WayValveActuatorCount++;
	}

	// HYS-1877: If we are not in selection by package mode OR if there is no valve that belongs to a set...
	if( false == m_pclIndSel6WayValveParams->m_bOnlyForSet || false == bAtLeastOneActrPartOfASet )
	{
		pclSSheet->ShowCol( CD_Actuator_Box, FALSE );
	}

	long lLastDataRow = lRow - 1;

	// Save the total actuator number.
	pclSheetDescription6WayValveActuator->SetUserVariable( _SDUV_TOTALACTUATOR_COUNT, l6WayValveActuatorCount );

	// Set that there is no selection at now.
	SetCurrent6WayValveActuatorSelected( NULL );
	
	// Add possibility to collapse / expand the actuator sheet.
	CCDButtonExpandCollapseGroup *&pclGroupButton = m_mapVariables[SD_Family6WayValve][BothSide].m_pCDBExpandCollapseGroupValveActuator;

	pclGroupButton = CreateExpandCollapseGroupButton( CD_Actuator_FirstColumn, RD_Actuator_GroupName, true,
			CCDButtonExpandCollapseGroup::ButtonState::CollapseRow, pclSheetDescription6WayValveActuator->GetFirstSelectableRow() - 2,
			pclSheetDescription6WayValveActuator->GetLastSelectableRow(false), pclSheetDescription6WayValveActuator );

	// Show button.
	if (NULL != pclGroupButton)
	{
		pclGroupButton->SetShowStatus( true );
	}

	pclSSheet->AddCellSpan( CD_Actuator_FirstColumn, RD_Actuator_GroupName, 2, 1 );
	
	pclSSheet->SetCellBorder( CD_Actuator_CheckBox, lLastDataRow, CD_Actuator_Pointer - 1, lLastDataRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );

	// Add possibility to change column size.
	// Remark: place this code after any initialization (like adding, removing, resizing, hiding/showing columns).
	CResizingColumnInfo *pclResizingColumnInfo = pclSheetDescription6WayValveActuator->GetResizingColumnInfo();
	pclResizingColumnInfo->ActivateFeature( true, false, RD_Actuator_FirstAvailRow, lLastDataRow );
	pclResizingColumnInfo->AddRangeColumn( CD_Actuator_Name, CD_Actuator_MaxTemp, RD_Actuator_ColName, RD_Actuator_Unit );

	// Add possibility to click on title group to reset column width.
	CreateTitleGroupButtons( CD_Actuator_Box, CD_Actuator_MaxTemp, RD_Actuator_GroupName, pclSheetDescription6WayValveActuator );
}

void CRViewSSel6WayValve::_ClickOn6WayValveActuator( CSheetDescription *pclSheetDescription6WayValveActuator, CDB_Actuator *p6WayValveActuatorClicked, 
		CCellDescriptionProduct *pclCellDescriptionProduct, long lColumn, long lRow )
{
	if( NULL == m_pclIndSel6WayValveParams )
	{
		ASSERT_RETURN;
	}

	CSSheet *pclSSheet = pclSheetDescription6WayValveActuator->GetSSheetPointer();

	LPARAM l6WayValveActuatorCount;
	pclSheetDescription6WayValveActuator->GetUserVariable( _SDUV_TOTALACTUATOR_COUNT, l6WayValveActuatorCount );

	m_mapVariables[SD_Family6WayValve][BothSide].m_vecValveActuatorAccessoryList.clear();
	m_mapVariables[SD_Family6WayValve][BothSide].m_pCDBExpandCollapseGroupValveActuatorAccessory = NULL;

	CCDButtonExpandCollapseRows *&pCDBExpandCollapseRowsValveActuator = m_mapVariables[SD_Family6WayValve][BothSide].m_pCDBExpandCollapseRowsValveActuator;
	long &lValveActuatorSelectedRow = m_mapVariables[SD_Family6WayValve][BothSide].m_lValveActuatorSelectedRow;

	// Memorize previous focus row.
	long lCurrentFocusedRow = GetCurrentFocusedRow();

	// To be sure to clean all previous data concerning focus.
	KillCurrentFocus();

	// Unselect row where current selection is set (yellow light).
	// Remark: 'KillCurrentFocus' unselect only rows where focus is on (blue light).
	pclSSheet->UnSelectMultipleRows();

	// Retrieve the current selected actuator if exist.
	CDB_Actuator *pclCurrent6WayValveActuatorSelected = GetCurrent6WayValveActuatorSelected();

	// If there is already one actuator selected and user click on the current one...
	// Remark: 'm_pCDBExpandCollapseRows6WayValveActuator' is not created if there is only one actuator. Thus we need to check first if there is only one actuator.
	//         Otherwise we have to check if user has clicked on the current one.
	if( NULL != pclCurrent6WayValveActuatorSelected && ( ( 1 == l6WayValveActuatorCount ) || ( NULL != pCDBExpandCollapseRowsValveActuator
			&& lRow == pCDBExpandCollapseRowsValveActuator->GetCellPosition().y ) ) )
	{
		// Reset current product selected.
		SetCurrent6WayValveActuatorSelected( NULL );

		// Delete Expand/Collapse rows button if exist.
		// Remark: 'pCDBExpandCollapseRowsValveActuator' is set to NULL in method!
		DeleteExpandCollapseRowsButton( pCDBExpandCollapseRowsValveActuator, pclSheetDescription6WayValveActuator );

		// Remove actuator accessories sheet if exist.
		if( true == m_ViewDescription.IsSheetDescriptionExist( SD_6WayValveActuatorAccessories ) )
		{
			m_ViewDescription.RemoveOneSheetDescription( SD_6WayValveActuatorAccessories );
		}

		// HYS-1877: If actuator is part of a set, change box button to open state.
		if( true == pclCurrent6WayValveActuatorSelected->IsPartOfaSet() )
		{
			// Verify box and open it for 6-way valve, 6-way valve actuator and PIBCV
			ChangeBoxStateFor6WValveSet( CCDButtonBox::ButtonState::BoxOpened, true, BothSide, lRow, NULL );
		}

		// Uncheck checkbox.
		pclSSheet->SetCheckBox( CD_Actuator_CheckBox, lRow, _T(""), false, true );

		// Set focus on the actuator currently selected.
		PrepareAndSetNewFocus( pclSheetDescription6WayValveActuator, pclSheetDescription6WayValveActuator->GetActiveColumn(), lRow, 0 );

		lValveActuatorSelectedRow = 0;

		// Notify a change in the selection.
		SelectionHasChanged( IsSelectionReady() );
	}
	else
	{
		// If an actuator is already selected...
		if( NULL != pclCurrent6WayValveActuatorSelected )
		{
			// Remove actuator accessories sheet if exist.
			if( true == m_ViewDescription.IsSheetDescriptionExist( SD_6WayValveActuatorAccessories ) )
			{
				m_ViewDescription.RemoveOneSheetDescription( SD_6WayValveActuatorAccessories );
			}

			// HYS-1877: Change box button to open state for previous control valve.
			if( true == pclCurrent6WayValveActuatorSelected->IsPartOfaSet() )
			{
				// Verify box and open it for 6-way valve, 6-way valve actuator and PIBCV
				ChangeBoxStateFor6WValveSet( CCDButtonBox::ButtonState::BoxOpened, true, BothSide, lRow, NULL );
			}

			// Uncheck checkbox.
			pclSSheet->SetCheckBox( CD_Actuator_CheckBox, m_mapVariables[SD_Family6WayValve][BothSide].m_lValveActuatorSelectedRow, _T(""), false, true );
		}

		// Save new actuator selection.
		SetCurrent6WayValveActuatorSelected( pclCellDescriptionProduct );

		// Delete Expand/Collapse rows button if exist.
		if( NULL != pCDBExpandCollapseRowsValveActuator )
		{
			// Remark: 'pCDBExpandCollapseRowsValveActuator' is set to NULL in method!
			DeleteExpandCollapseRowsButton( pCDBExpandCollapseRowsValveActuator, pclSheetDescription6WayValveActuator );
		}

		// Create Expand/Collapse rows button if needed...
		if( l6WayValveActuatorCount > 1 )
		{
			pCDBExpandCollapseRowsValveActuator = CreateExpandCollapseRowsButton( CD_Actuator_FirstColumn, lRow, true, 
					CCDButtonExpandCollapseRows::ButtonState::CollapseRow, pclSheetDescription6WayValveActuator->GetFirstSelectableRow(), 
					pclSheetDescription6WayValveActuator->GetLastSelectableRow( false ), pclSheetDescription6WayValveActuator );

			// Show button.
			if( NULL != pCDBExpandCollapseRowsValveActuator )
			{
				pCDBExpandCollapseRowsValveActuator->SetShowStatus( true );
			}
		}

		// Check checkbox.
		pclSSheet->SetCheckBox( CD_Actuator_CheckBox, lRow, _T(""), true, true );

		// Fill accessories on actuator if exist.
		_Fill6WayValveActuatorAccessories();

		// Select actuator (just highlight background).
		pclSSheet->SelectOneRow( lRow, pclSheetDescription6WayValveActuator->GetSelectionFrom(), pclSheetDescription6WayValveActuator->GetSelectionTo() );

		// Try to set the focus on the next table but do not lose the focus if no other table exist.
		CSheetDescription *pclNextSheetDescription = NULL;
		long lNewFocusedRow;
		bool bShiftPressed;

		if( DNFERRC_FocusOnNextGroupOK == DefineNextFocusedRow( pclSheetDescription6WayValveActuator, CD_Actuator_FirstColumn, lRow, false, lNewFocusedRow,
				pclNextSheetDescription, bShiftPressed ) )
		{
			long lNewFocusedCol = pclNextSheetDescription->GetFirstParameterColumn();
			PrepareAndSetNewFocus( pclNextSheetDescription, lNewFocusedCol, lNewFocusedRow );
		}
		else
		{
			// Because we have changed current selection background in yellow, if there is no other sheet following current one, we
			// must set focus on current selection.
			PrepareAndSetNewFocus( pclSheetDescription6WayValveActuator, lColumn, lRow );
		}

		lValveActuatorSelectedRow = lRow;

		// Notify a change in the selection.
		SelectionHasChanged( IsSelectionReady() );
	}

	// HYS-1877: If we are in case of set and no 6-way valve actuator is selected then the pibcv should be unselected ...
	CSheetDescription* pclSheetDescriptionPIBCV = m_ViewDescription.GetFromSheetDescriptionID( SD_PIBCValveBothSide );

	if( true == m_pclIndSel6WayValveParams->m_bOnlyForSet && NULL != pclSheetDescriptionPIBCV && NULL == GetCurrent6WayValveActuatorSelected() 
		&& NULL != GetCurrentPIBCValveSelected( BothSide ) && true == GetCurrentPIBCValveSelected( BothSide )->IsPartOfaSet() )
	{
		// Retrieve selected pibcv.
		CCellDescriptionProduct* pclCDProduct = NULL;
		long lPibcvRow = m_mapVariables[SD_FamilyPIBCValve][BothSide].m_lValveSelectedRow;
		long lColParam = pclSheetDescriptionPIBCV->GetFirstParameterColumn();
		LPARAM lProduct = GetCDProduct( lColParam, lPibcvRow, pclSheetDescriptionPIBCV, &pclCDProduct );

		if( NULL != pclCDProduct )
		{
			// Simulate a click on the selected pibcv.
			// Remark: the change of box state is done in this method.
			_ClickOnPIBCValve( pclSheetDescriptionPIBCV, dynamic_cast<CDB_PIControlValve*>((CData*)lProduct), pclCDProduct, lColParam, lPibcvRow );
		}
	}
}

void CRViewSSel6WayValve::_Fill6WayValveActuatorAccessories()
{
	if( NULL == m_pclIndSel6WayValveParams )
	{
		ASSERT_RETURN;
	}

	// Check the current 6-way valve selected.
	CDB_6WayValve *pclCurrent6WayValveSelected = GetCurrent6WayValveSelected();

	if( NULL == pclCurrent6WayValveSelected )
	{
		ASSERT_RETURN;
	}

	// Check current actuator selected.
	CDB_Actuator *pclCurrent6WayValveActuatorSelected = GetCurrent6WayValveActuatorSelected();

	if( NULL == pclCurrent6WayValveActuatorSelected )
	{
		ASSERT_RETURN;
	}

	// Retrieve accessories belonging to the selected actuator.
	CRank r6WayValveActuatorAccessoryList;
	CDB_RuledTableBase *pclRuledTable = NULL;
	_GetActuatorAccessoryList( pclCurrent6WayValveActuatorSelected, &r6WayValveActuatorAccessoryList, &pclRuledTable );

	if( r6WayValveActuatorAccessoryList.GetCount() > 0 )
	{
		_FillAccessoryRows( BothSide, SD_Family6WayValve, SD_TypeValveActuatorAccessories, &r6WayValveActuatorAccessoryList, pclRuledTable );
	}
}

void CRViewSSel6WayValve::_Get6WayValveActuatorList( CDB_6WayValve *pclSelected6WayValve, CRank *pclActuatorList )
{
	if( NULL == m_pclIndSel6WayValveParams || NULL == m_pclIndSel6WayValveParams->m_pTADB || NULL == pclSelected6WayValve || NULL == pclActuatorList )
	{
		ASSERT_RETURN;
	}

	pclActuatorList->PurgeAll();

	// Retrieve the actuator group on the selected control valve.
	CTable *pclActuatorGroup = (CTable *)( pclSelected6WayValve->GetActuatorGroupIDPtr().MP );

	if( NULL == pclActuatorGroup )
	{
		return;
	}

	// Retrieve list of all actuators in this group.
	CRank rList( false );

	for( IDPTR idptr = pclActuatorGroup->GetFirst(); NULL != idptr.MP; idptr = pclActuatorGroup->GetNext( idptr.MP ) )
	{
		rList.Add( idptr.ID, 0, ( LPARAM )idptr.MP );
	}

	CDB_6WayValveActuatorCompatibility *pcl6WayValveActrComp = dynamic_cast<CDB_6WayValveActuatorCompatibility *>( TASApp.GetpTADB()->Get( _T("6WAY_ACTR_COMP") ).MP );
	ASSERT( NULL != pcl6WayValveActrComp );
	
	CDB_6WayValveActuatorCompatibility::ActuatorFilters *pActuatorFilters = pcl6WayValveActrComp->GetOneActutorFilters( m_pclIndSel6WayValveParams->m_e6WayValveSelectionMode );

	if( NULL == pActuatorFilters )
	{
		ASSERT_RETURN;
	}

	// Now we have to check the list to filter on the 'Fast electrical connection' checkbox and in regards to 
	// the compatibility table.
	CString str;
	LPARAM lParam;

	for( BOOL bContinue = rList.GetFirst( str, lParam ); TRUE == bContinue; bContinue = rList.GetNext( str, lParam ) )
	{
		CDB_ElectroActuator *pElectroActuator = dynamic_cast<CDB_ElectroActuator *>( (CData *)lParam );

		if( NULL == pElectroActuator )
		{
			continue;
		}
		
		// HYS-1429 : Do not show unavailable actuators
		if( false == pElectroActuator->IsSelectable( true ) )
		{
			continue;
		}

		if( true == m_pclIndSel6WayValveParams->m_bCheckFastConnection && 0 != CString( pElectroActuator->GetVersionID() ).CompareNoCase( _T("VERS_CO") ) )
		{
			// If we want a fast electrical connection but the actuator isn't compatible...
			continue;
		}

		if( false == m_pclIndSel6WayValveParams->m_bCheckFastConnection && 0 == CString( pElectroActuator->GetVersionID() ).CompareNoCase( _T("VERS_CO") ) )
		{
			// If we don't want a fast electrical connection and the actuator is a CO...
			continue;
		}

		// Verify power supply compatibility.
		if( pActuatorFilters->m_6WayValveActuatorPowerSupplyIDCompatibilityList.size() > 0 )
		{
			bool bFound = false;

			for( auto &iterPowerSupplyIDPtr : pActuatorFilters->m_6WayValveActuatorPowerSupplyIDCompatibilityList )
			{
				if( true == pElectroActuator->IsPowerSupplyAvailable( iterPowerSupplyIDPtr ) )
				{
					bFound = true;
					break;
				}
			}

			if( false == bFound )
			{
				continue;
			}
		}

		// Verify input signal compatibility.
		if( pActuatorFilters->m_6WayValveActuatorInputSignalIDCompatibilityList.size() > 0 )
		{
			bool bFound = false;

			for( auto &iterInputSignalIDPtr : pActuatorFilters->m_6WayValveActuatorInputSignalIDCompatibilityList )
			{
				if( true == pElectroActuator->IsInputSignalAvailable( iterInputSignalIDPtr ) )
				{
					bFound = true;
					break;
				}
			}

			if( false == bFound )
			{
				continue;
			}
		}

		pclActuatorList->Add( pElectroActuator->GetIDPtr().ID, pElectroActuator->GetOrderKey(), (LPARAM)pElectroActuator, false, false );
	}
}

bool CRViewSSel6WayValve::_VerifyFlows( CSelectedValve *pSelected6WayValve )
{
	if( NULL == m_pclIndSel6WayValveParams || NULL == m_pclIndSel6WayValveParams->m_pclSelect6WayValveList
			|| NULL == m_pclIndSel6WayValveParams->m_pTADB || NULL == m_pclIndSel6WayValveParams->m_pPipeDB )
	{
		ASSERTA_RETURN( false );
	}

	if( NULL == pSelected6WayValve )
	{
		return false;
	}

	bool bOK = true;
	CSelect6WayValveList *pcl6WayValveList = m_pclIndSel6WayValveParams->m_pclSelect6WayValveList;
	CSelectPICVList *pclSelectedPICvList1 = NULL;
	CSelectPICVList *pclSelectedPICvList2 = NULL;
	bool bAtLeastOneSizeShiftProblem = false;

	if( e6Way_EQMControl == m_pclIndSel6WayValveParams->m_e6WayValveSelectionMode )
	{
		if( pcl6WayValveList->SelectPIBCValve( m_pclIndSel6WayValveParams, pSelected6WayValve, BothSide, &bAtLeastOneSizeShiftProblem  ) > 0 )
		{
			pclSelectedPICvList1 = pcl6WayValveList->GetPICVList( BothSide );
		}
	}
	else if( e6Way_OnOffControlWithPIBCV == m_pclIndSel6WayValveParams->m_e6WayValveSelectionMode )
	{
		if( pcl6WayValveList->SelectPIBCValve( m_pclIndSel6WayValveParams, pSelected6WayValve, HeatingSide, &bAtLeastOneSizeShiftProblem  ) > 0 )
		{
			pclSelectedPICvList1 = pcl6WayValveList->GetPICVList( HeatingSide );
		}

		if( pcl6WayValveList->SelectPIBCValve( m_pclIndSel6WayValveParams, pSelected6WayValve, CoolingSide, &bAtLeastOneSizeShiftProblem  ) > 0 )
		{
			pclSelectedPICvList2 = pcl6WayValveList->GetPICVList( CoolingSide );
		}
	}
	else
	{
		return true;
	}

	bool bStop = false;
	bool bAllSettingsForCoolingFlowToLow = true;
	bool bAllSettingsForHeatingFlowToLow = true;
	SideDefinition eWhichSideNoFound = SideDefinition::Undefined;

	// HYS-1355: For now, there is only TA-Slider 160 for TA-Compact-P DN10/25 and TA-Modulator DN10/32.
	CDB_ElectroActuator *pclTASlider160 = dynamic_cast<CDB_ElectroActuator *>( TASApp.GetpTADB()->Get( _T("SLIDER160-STD1M") ).MP );
	ASSERT( NULL != pclTASlider160 );

	// Remark: we start with the heating that has generally a lower flow defined than the cooling.
	for( int iLoop = 0; iLoop < 2 && false == bStop; iLoop++ )
	{
		CSelectPICVList *pclSelectedPICvList = ( 0 == iLoop ) ? pclSelectedPICvList1 : pclSelectedPICvList2;

		if( NULL == pclSelectedPICvList )
		{
			continue;
		}

		for( CSelectedValve *pclSelectedPICv = pclSelectedPICvList->GetFirst<CSelectedValve>(); NULL != pclSelectedPICv && false == bStop;
				pclSelectedPICv = pclSelectedPICvList->GetNext<CSelectedValve>() )
		{
			CDB_PIControlValve *pclPICv = dynamic_cast<CDB_PIControlValve *>( pclSelectedPICv->GetpData() );

			if( NULL == pclPICv || NULL == pclPICv->GetPICVCharacteristic() )
			{
				continue;
			}

			if( e6Way_EQMControl == m_pclIndSel6WayValveParams->m_e6WayValveSelectionMode )
			{
				// In EQM mode we use the same valve for cooling and heating.
				double dHCooling = pclPICv->GetPresetting( m_pclIndSel6WayValveParams->GetCoolingFlow(), m_pclIndSel6WayValveParams->GetCoolingWaterChar().GetDens(), 
						m_pclIndSel6WayValveParams->GetCoolingWaterChar().GetKinVisc() );

				double dHHeating = pclPICv->GetPresetting( m_pclIndSel6WayValveParams->GetHeatingFlow(), m_pclIndSel6WayValveParams->GetHeatingWaterChar().GetDens(), 
						m_pclIndSel6WayValveParams->GetHeatingWaterChar().GetKinVisc() );

				if( dHCooling >= pclPICv->GetPICVCharacteristic()->GetMinSettingWithActuator( pclTASlider160 ) )
				{
					bAllSettingsForCoolingFlowToLow = false;
				}
				
				if( dHHeating >= pclPICv->GetPICVCharacteristic()->GetMinSettingWithActuator( pclTASlider160 ) )
				{
					bAllSettingsForHeatingFlowToLow = false;
				}

				if( dHCooling >= pclPICv->GetPICVCharacteristic()->GetMinSettingWithActuator( pclTASlider160 ) 
						&& dHHeating >= pclPICv->GetPICVCharacteristic()->GetMinSettingWithActuator( pclTASlider160 ) )
				{
					// If this current valve satisfy the two flow, we can stop the loop.
					bStop = true;
				}
			}
			else
			{
				// In On/off with PIBCv we have two different valves.
				double dH = 0.0;

				if( 0 == iLoop )
				{
					dH = pclPICv->GetPresetting( m_pclIndSel6WayValveParams->GetHeatingFlow(), m_pclIndSel6WayValveParams->GetHeatingWaterChar().GetDens(), 
							m_pclIndSel6WayValveParams->GetHeatingWaterChar().GetKinVisc() );
				}
				else
				{
					dH = pclPICv->GetPresetting( m_pclIndSel6WayValveParams->GetCoolingFlow(), m_pclIndSel6WayValveParams->GetCoolingWaterChar().GetDens(), 
							m_pclIndSel6WayValveParams->GetCoolingWaterChar().GetKinVisc() );					
				}

				if( dH >= pclPICv->GetPICVCharacteristic()->GetMinSettingWithActuator( pclTASlider160 ) )
				{
					if( 0 == iLoop )
					{
						bAllSettingsForHeatingFlowToLow = false;
					}
					else
					{
						bAllSettingsForCoolingFlowToLow = false;
					}
				}
			}
		}

		if( e6Way_OnOffControlWithPIBCV == m_pclIndSel6WayValveParams->m_e6WayValveSelectionMode && 0 == iLoop && true == bAllSettingsForHeatingFlowToLow )
		{
			// If no valve has been found in heating, we don't need to do the second loop with the cooling mode.
			bStop = true;
		}
	}

	if( true == bAllSettingsForCoolingFlowToLow || true == bAllSettingsForHeatingFlowToLow )
	{
		CString str;

		// Due to the , all valves found have their presseting below the recommended value.
		if( true == bAllSettingsForCoolingFlowToLow && true == bAllSettingsForHeatingFlowToLow )
		{
			FormatString( str, IDS_RVIEWSSELCO_TOOLOWFLOW, TASApp.LoadLocalizedString( IDS_RVIEWSSELCO_COOLINGANDHEATING ) );
		}
		else if( true == bAllSettingsForCoolingFlowToLow )
		{
			FormatString( str, IDS_RVIEWSSELCO_TOOLOWFLOW, TASApp.LoadLocalizedString( IDS_RVIEWSSELCO_COOLING ) );
		}
		else
		{
			FormatString( str, IDS_RVIEWSSELCO_TOOLOWFLOW, TASApp.LoadLocalizedString( IDS_RVIEWSSELCO_HEATING ) );
		}
		
		AfxMessageBox( str, MB_OK | MB_ICONEXCLAMATION );
		bOK = false;
	}

	return bOK;
}

void CRViewSSel6WayValve::_ClickOnPIBCValve( CSheetDescription *pclSheetDescriptionPIBCValve, CDB_PIControlValve *pclPIBCValve, 
		CCellDescriptionProduct *pclCellDescriptionProduct, long lColumn, long lRow )
{
	if( NULL == m_pclIndSel6WayValveParams || NULL == m_pclIndSel6WayValveParams->m_pclSelect6WayValveList )
	{
		ASSERT( 0 );
		return;
	}

	CSSheet *pclSSheet = pclSheetDescriptionPIBCValve->GetSSheetPointer();

	LPARAM lPIBCValveTotalCount;
	pclSheetDescriptionPIBCValve->GetUserVariable( _SDUV_TOTALPRODUCT_COUNT, lPIBCValveTotalCount );

	// Memorize previous focus row.
	long lCurrentFocusedRow = GetCurrentFocusedRow();

	// To be sure to clean all previous data concerning focus.
	KillCurrentFocus();

	// Unselect row where current selection is set (yellow light).
	// Remark: 'KillCurrentFocus' unselect only rows where focus is on (blue light).
	pclSSheet->UnSelectMultipleRows();

	int iSheetDescriptionID = pclSheetDescriptionPIBCValve->GetSheetDescriptionID();
	SideDefinition eSideDefinition = GETSHEETSIDE( iSheetDescriptionID );
	int iSheetType = GETSHEETTYPE( iSheetDescriptionID );
	int iSheetFamily = GETSHEETFAMILY( iSheetDescriptionID );

	// By default clear accessories list.
	_ClickOnProductClearHelper( iSheetFamily, eSideDefinition );

	// Retrieve the current selected pressure independent balancing & control valve if exist.
	CDB_TAProduct *pclCurrentPIBCValveSelected = GetCurrentPIBCValveSelected( eSideDefinition );

	CCDButtonExpandCollapseRows *&pCDBExpandCollapseRowsValve = m_mapVariables[SD_FamilyPIBCValve][eSideDefinition].m_pCDBExpandCollapseRowsValve;
	CCDButtonShowAllPriorities *&pCDBShowAllPrioritiesValve = m_mapVariables[SD_FamilyPIBCValve][eSideDefinition].m_pCDBShowAllPrioritiesValve;
	long &lValveSelectedRow = m_mapVariables[SD_FamilyPIBCValve][eSideDefinition].m_lValveSelectedRow;

	// If there is already one pressure independent balancing & control valve selected and user click on the current one...
	// Remark: 'pCDBExpandCollapseRowsValve' is not created if there is only one pressure independent balancing & control valve. 
	//         Thus we need to check first if there is only one pressure independent balancing & control valve. 
	//         Otherwise we have to check if user has clicked on the current one.
	if( NULL != pclCurrentPIBCValveSelected 
			&& ( ( 1 == lPIBCValveTotalCount ) || ( NULL != pCDBExpandCollapseRowsValve && lRow == pCDBExpandCollapseRowsValve->GetCellPosition().y ) ) )
	{
		// Reset current product selected.
		SetCurrentPIBCValveSelected( NULL, eSideDefinition );

		// HYS-1877: If control valve is part of a set, change box button to open state.
		if( true == pclCurrentPIBCValveSelected->IsPartOfaSet() )
		{
			// Verify box and open it for 6-way valve, 6-way valve actuator and PIBCV
			ChangeBoxStateFor6WValveSet( CCDButtonBox::ButtonState::BoxOpened, true, eSideDefinition, lRow, NULL );
		}

		// Uncheck checkbox.
		pclSSheet->SetCheckBox( CD_PIBCV_CheckBox, lRow, _T(""), false, true );

		// Delete Expand/Collapse rows button if exist.
		// Remark: 'm_pCDBExpandCollapseRowsPIBCValve' is set to NULL in method!
		DeleteExpandCollapseRowsButton( pCDBExpandCollapseRowsValve, pclSheetDescriptionPIBCValve );

		// Show Show/Hide all priorities button if exist.
		if( NULL != pCDBShowAllPrioritiesValve )
		{
			pCDBShowAllPrioritiesValve->ApplyInternalChange();
		}

		// Set focus on the pressure independent balancing & control valve currently selected.
		PrepareAndSetNewFocus( pclSheetDescriptionPIBCValve, pclSheetDescriptionPIBCValve->GetActiveColumn(), lRow, 0 );

		// Remove all sheets after the pressure independent balancing & control valve.
		_RemoveAllSheetAfter( eSideDefinition, iSheetFamily, iSheetType );

		lValveSelectedRow = 0;

		// Notify a change in the selection.
		SelectionHasChanged( IsSelectionReady() );
	}
	else
	{
		// If a pressure independent balancing & control valve is already selected...
		if( NULL != pclCurrentPIBCValveSelected )
		{
			// HYS-1877: Change box button to open state for previous control valve.
			if( true == pclCurrentPIBCValveSelected->IsPartOfaSet() )
			{
				// Verify box and open it for 6-way valve, 6-way valve actuator and PIBCV
				ChangeBoxStateFor6WValveSet( CCDButtonBox::ButtonState::BoxOpened, true, eSideDefinition, lRow, NULL );
			}

			// Uncheck checkbox.
			pclSSheet->SetCheckBox( CD_PIBCV_CheckBox, lValveSelectedRow, _T(""), false, true );

			// Remove all sheets after the pressure independent balancing & control valve.
			_RemoveAllSheetAfter( eSideDefinition, iSheetFamily, iSheetType );
		}

		// Save new the pressure independent balancing & control valve selection.
		SetCurrentPIBCValveSelected( pclCellDescriptionProduct, eSideDefinition );

		// Check checkbox.
		pclSSheet->SetCheckBox( CD_PIBCV_CheckBox, lRow, _T(""), true, true );

		// Delete Expand/Collapse rows button if exist.
		if( NULL != pCDBExpandCollapseRowsValve )
		{
			// Remark: 'm_pCDBExpandCollapseRowsPIBCValve' is set to NULL in method!
			DeleteExpandCollapseRowsButton( pCDBExpandCollapseRowsValve, pclSheetDescriptionPIBCValve );
		}

		// Create Expand/Collapse rows button if needed...
		if( lPIBCValveTotalCount > 1 )
		{
			pCDBExpandCollapseRowsValve = CreateExpandCollapseRowsButton( CD_PIBCV_FirstColumn, lRow, true, 
					CCDButtonExpandCollapseRows::ButtonState::CollapseRow, pclSheetDescriptionPIBCValve->GetFirstSelectableRow(), 
					pclSheetDescriptionPIBCValve->GetLastSelectableRow( false ), pclSheetDescriptionPIBCValve );

			// Show button.
			if( NULL != pCDBExpandCollapseRowsValve )
			{
				pCDBExpandCollapseRowsValve->SetShowStatus( true );
			}
		}

		// Select the pressure independent balancing & control valve (just highlight background).
		pclSSheet->SelectOneRow( lRow, pclSheetDescriptionPIBCValve->GetSelectionFrom(), pclSheetDescriptionPIBCValve->GetSelectionTo() );

		// Fill accessories available for the control pressure independent balancing & control valve.
		_FillPIBCValveAccessories( eSideDefinition );

		// Fill actuators available for the control pressure independent balancing & control valve.
		_FillPIBCValveActuators( eSideDefinition );

		// Memorize PIBCV row selected.
        // Remark: needed if call to '_ClickOnPIBCValveActuator' is done below.
		lValveSelectedRow = lRow;

		// HYS-1877: If user has selected Pibcv-Actuator set and we have only one actuator...
		CSheetDescription* pSheetDescriptionActuator = m_ViewDescription.GetFromSheetDescriptionID( (eSideDefinition << SHEETSIDEDEFSHIFT) + SD_FamilyPIBCValve + SD_TypeValveActuator );
		bool bSetFocus = true;

		if( true == m_pclIndSel6WayValveParams->m_bOnlyForSet && true == pclPIBCValve->IsPartOfaSet() && NULL != pSheetDescriptionActuator )
		{
			// Verify first if we have only one actuator.
			LPARAM lActuatorTotalCount;
			pSheetDescriptionActuator->GetUserVariable( _SDUV_TOTALACTUATOR_COUNT, lActuatorTotalCount );

			if( 1 == lActuatorTotalCount )
			{
				// ... we can than automatically select actuator and close boxes in front of Pibcv.

				// Retrieve actuator.
				CCellDescriptionProduct* pclCDProduct = NULL;
				long lActuatorRow = pSheetDescriptionActuator->GetFirstSelectableRow();
				long lColParam = pSheetDescriptionActuator->GetFirstParameterColumn();
				LPARAM lProduct = GetCDProduct( lColParam, lActuatorRow, pSheetDescriptionActuator, &pclCDProduct );

				if( NULL != pclCDProduct )
				{
					// Simulate a click on the edited balancing valve.
					// Remark: the change of box state is done in this method.
					_ClickOnPIBCValveActuator( pSheetDescriptionActuator, dynamic_cast<CDB_Actuator*>((CData*)lProduct), pclCDProduct, pSheetDescriptionActuator->GetSelectionFrom(),
									  lActuatorRow );
					bSetFocus = false;
				}
			}
		}

		if( true == bSetFocus )
		{
			// Try to set the focus on the next table but do not lose the focus if no other table exist.
			CSheetDescription* pclNextSheetDescription = NULL;
			long lNewFocusedRow;
			bool bShiftPressed;

			if( DNFERRC_FocusOnNextGroupOK == DefineNextFocusedRow( pclSheetDescriptionPIBCValve, CD_PIBCV_FirstColumn, lRow, false, lNewFocusedRow,
																	pclNextSheetDescription, bShiftPressed ) )
			{
				long lNewFocusedCol = pclNextSheetDescription->GetFirstParameterColumn();
				PrepareAndSetNewFocus( pclNextSheetDescription, lNewFocusedCol, lNewFocusedRow );
			}
			else
			{
				// Because we have changed current selection background in yellow, if there is no other sheet following current one, we
				// must set focus on current selection.
				PrepareAndSetNewFocus( pclSheetDescriptionPIBCValve, lColumn, lRow );
			}
		}

		// Notify a change in the selection.
		SelectionHasChanged( IsSelectionReady() );
	}
}

void CRViewSSel6WayValve::_FillPIBCValveRows( CSelectedValve *pSelected6WayValve, SideDefinition eSideDefinition )
{
	if( NULL == m_pclIndSel6WayValveParams || NULL == m_pclIndSel6WayValveParams->m_pclSelect6WayValveList
			|| NULL == m_pclIndSel6WayValveParams->m_pTADB || NULL == m_pclIndSel6WayValveParams->m_pPipeDB )
	{
		ASSERT_RETURN;
	}

	if( NULL == pSelected6WayValve )
	{
		return;
	}

	// Retrieve the pressure independent balancing & control valve list linked to the current 6-way valve.
	// Remark: the search has been done in the '_VeriyFlows' method.
	CSelectPICVList *pclSelectedPICvList = pclSelectedPICvList = m_pclIndSel6WayValveParams->m_pclSelect6WayValveList->GetPICVList( eSideDefinition );

	// Call 'MultiSpreadBase' method to get a new SSheet.
	int iSheetDescriptionID = ( eSideDefinition << SHEETSIDEDEFSHIFT ) + SD_FamilyPIBCValve + SD_TypeValve;
	CSheetDescription *pclSheetDescriptionPIBCValve = CreateSSheet( iSheetDescriptionID );

	if( NULL == pclSheetDescriptionPIBCValve || NULL == pclSheetDescriptionPIBCValve->GetSSheetPointer() )
	{
		return;
	}

	CSSheet *pclSSheet = pclSheetDescriptionPIBCValve->GetSSheetPointer();

	pclSSheet->SetBool( SSB_REDRAW, FALSE );

	pclSSheet->Init();
	pclSSheet->SetBool( SSB_ALLOWDRAGDROP, FALSE );
	pclSSheet->SetBool( SSB_HORZSCROLLBAR, FALSE );
	pclSSheet->SetBool( SSB_VERTSCROLLBAR, FALSE );
	pclSSheet->SetBool( SSB_MOVEACTIVEONFOCUS, FALSE );
	pclSSheet->SetTextTip( SS_TT_STATUS_FLOATING, 250, NULL, _LIGHTYELLOW, _TAH_BLACK );

	// Initialize the pressure independent balancing & control valve sheet and fill header.
	_InitAndFillPIBCValveHeader( pclSheetDescriptionPIBCValve, pclSSheet, eSideDefinition );

	if( false == m_pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_bIsDpMaxChecked )
	{
		pclSSheet->ShowCol( CD_PIBCV_DpMax, FALSE );
	}

	if( e6Way_EQMControl == m_pclIndSel6WayValveParams->m_e6WayValveSelectionMode )
	{
		pclSSheet->ShowCol( CD_PIBCV_Preset, FALSE );
	}

	// HYS-1380: Two column Dp min. for cooling and heating. 
	if( eSideDefinition == CoolingSide )
	{
		pclSSheet->ShowCol( CD_PIBCV_DpMinHeating, FALSE );
	}
	else if( eSideDefinition == HeatingSide )
	{
		pclSSheet->ShowCol( CD_PIBCV_DpMinCooling, FALSE );
	}

	long lRetRow = -1;
	long lRow = RD_PIBCValve_FirstAvailRow;

	long lValveTotalCount = 0;
	long lValveNotPriorityCount = 0;
	bool bShowAllPrioritiesShown = false;
	long lShowAllPrioritiesButtonRow = 0;
	// HYS-1877
	bool bAtLeastOnePIBCVPartOfASet = false;

	pclSheetDescriptionPIBCValve->SetUserVariable( _SDUV_TOTALPRODUCT_COUNT, 0 );
	pclSheetDescriptionPIBCValve->SetUserVariable( _SDUV_NOPRIORITYPRODUCT_COUNT, 0 );
	pclSheetDescriptionPIBCValve->RestartRemarkGenerator();

	if( NULL != pclSelectedPICvList && pclSelectedPICvList->GetCount() > 0 )
	{
		// HYS-1355: For now, there is only TA-Slider 160 for TA-Compact-P DN10/25 and TA-Modulator DN10/32.
		CDB_ElectroActuator *pclTASlider160 = dynamic_cast<CDB_ElectroActuator *>( TASApp.GetpTADB()->Get( _T("SLIDER160-STD1M") ).MP );
		ASSERT( NULL != pclTASlider160 );

		for( CSelectedValve *pclSelectedPICv = pclSelectedPICvList->GetFirst<CSelectedValve>(); NULL != pclSelectedPICv;
				pclSelectedPICv = pclSelectedPICvList->GetNext<CSelectedValve>() )
		{
			CDB_PIControlValve *pclPICv = dynamic_cast<CDB_PIControlValve *>( pclSelectedPICv->GetpData() );

			if( NULL == pclPICv )
			{
				continue;
			}

			// Check first if the presetting is correct.
			double dH = 0.0;

			if( BothSide == eSideDefinition || HeatingSide == eSideDefinition )
			{
				dH = pclPICv->GetPresetting( m_pclIndSel6WayValveParams->GetHeatingFlow(), m_pclIndSel6WayValveParams->GetHeatingWaterChar().GetDens(), 
						m_pclIndSel6WayValveParams->GetHeatingWaterChar().GetKinVisc() );
			}
			else
			{
				dH = pclPICv->GetPresetting( m_pclIndSel6WayValveParams->GetCoolingFlow(), m_pclIndSel6WayValveParams->GetCoolingWaterChar().GetDens(), 
						m_pclIndSel6WayValveParams->GetCoolingWaterChar().GetKinVisc() );
			}

			if( dH < pclPICv->GetPICVCharacteristic()->GetMinSettingWithActuator( pclTASlider160 ) )
			{
				continue;
			}

			if( true == pclSelectedPICv->IsFlagSet( CSelectedBase::eNotPriority ) )
			{
				lValveNotPriorityCount++;

				if( false == bShowAllPrioritiesShown )
				{
					// Add one empty not selectable row.
					pclSheetDescriptionPIBCValve->AddRows( 1 );
					pclSheetDescriptionPIBCValve->RemoveSelectableRow( lRow );

					pclSSheet->SetCellBorder( CD_PIBCV_CheckBox, lRow, CD_PIBCV_Separator - 1, lRow, true,
						SS_BORDERTYPE_TOP | SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );

					pclSSheet->SetCellBorder( CD_PIBCV_PipeSize, lRow, CD_PIBCV_Pointer - 1, lRow, true,
						SS_BORDERTYPE_TOP | SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );

					lShowAllPrioritiesButtonRow = lRow++;
					bShowAllPrioritiesShown = true;
				}
			}

			// If no error...
			if( 0 == _FillOnePIBCValveRow( pclSheetDescriptionPIBCValve, pclSSheet, pclSelectedPICv, lRow ) )
			{
				// HYS-1877
				if( NULL != pclSheetDescriptionPIBCValve->GetCellDescription( CD_PIBCV_Box, lRow ) )
				{
					bAtLeastOnePIBCVPartOfASet = true;
				}

				lRow++;
				lValveTotalCount++;
			}
		}
	}

	// HYS-1877: If we are not in selection by package mode OR if there is no valve that belongs to a set...
	if( false == m_pclIndSel6WayValveParams->m_bOnlyForSet || false == bAtLeastOnePIBCVPartOfASet )
	{
		pclSSheet->ShowCol( CD_PIBCV_Box, FALSE );
	}

	long lLastDataRow = lRow - 1;
	pclSheetDescriptionPIBCValve->SetUserVariable( _SDUV_TOTALPRODUCT_COUNT, lValveTotalCount );
	pclSheetDescriptionPIBCValve->SetUserVariable( _SDUV_NOPRIORITYPRODUCT_COUNT, lValveNotPriorityCount );

	// Now we can create Show/All priorities if needed.
	// Remarks: 2nd condition to avoid to have button if we have only no priority valves.
	if( true == bShowAllPrioritiesShown && lValveTotalCount > lValveNotPriorityCount )
	{
		// Create Show/Hide all priorities button in regards to current state!
		CCDButtonShowAllPriorities::ButtonState eState = ( true == GetShowAllPrioritiesFlag( iSheetDescriptionID ) ) ? CCDButtonShowAllPriorities::ButtonState::HidePriorities 
				: CCDButtonShowAllPriorities::ButtonState::ShowPriorities;

		CCDButtonShowAllPriorities *pclShowAllButton = CreateShowAllPrioritiesButton( CD_PIBCV_FirstColumn, lShowAllPrioritiesButtonRow, 
				bShowAllPrioritiesShown,  eState, lShowAllPrioritiesButtonRow + 1, pclSSheet->GetMaxRows(), pclSheetDescriptionPIBCValve );

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
			pclSSheet->ShowRow( lShowAllPrioritiesButtonRow, FALSE );
		}
	}

	// Try to merge only if there is more than one PICV.
	if( lValveTotalCount > 1 )
	{
		vector<long> vecColumnList;
		vecColumnList.push_back( CD_PIBCV_PipeSize );
		vecColumnList.push_back( CD_PIBCV_PipeLinDp );
		vecColumnList.push_back( CD_PIBCV_PipeV );
		pclSSheet->MergeRowsWithSameValues( RD_PIBCValve_FirstAvailRow, lLastDataRow, vecColumnList );
	}

	pclSSheet->SetCellBorder( CD_PIBCV_CheckBox, lLastDataRow, CD_PIBCV_Separator - 1, lLastDataRow, true, SS_BORDERTYPE_BOTTOM,
			SS_BORDERSTYLE_SOLID, _BLACK );

	// Needed by pipe merging.
	pclSSheet->SetCellBorder( CD_PIBCV_PipeSize, lLastDataRow, CD_PIBCV_Pointer- 1, lLastDataRow, true, SS_BORDERTYPE_BOTTOM,
			SS_BORDERSTYLE_SOLID, _BLACK );

	// Add Remarks.
	// Note: these lines must be placed after creating Show/Hide all priorities button to avoid to take in count these lines when defining
	//       range of row to be Show/Hide.
	lRow = pclSheetDescriptionPIBCValve->WriteRemarks( lRow, CD_PIBCV_CheckBox, CD_PIBCV_Separator );
		
	// Add the main title.
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );
	
	int iSheetType = GETSHEETTYPE( pclSheetDescriptionPIBCValve->GetSheetDescriptionID() );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)_GetGroupColor( false, eSideDefinition, iSheetType ) );
	
	pclSheetDescriptionPIBCValve->SetUserVariable( _SDUV_TITLEFORECOLOR, ( pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor ) ) );
	pclSheetDescriptionPIBCValve->SetUserVariable( _SDUV_TITLEBACKCOLOR, ( pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor ) ) );

	CString strTitle = TASApp.LoadLocalizedString( IDS_SSHEETSSELCV_PICVGROUP );

	if( e6Way_OnOffControlWithPIBCV == m_pclIndSel6WayValveParams->m_e6WayValveSelectionMode )
	{
		strTitle += _T(" - ") + TASApp.LoadLocalizedString( ( HeatingSide == eSideDefinition ) ? IDS_SSHEETSSEL6WAYVALVE_HEATINGSIDE : IDS_SSHEETSSEL6WAYVALVE_COOLINGSIDE );
	}

	pclSSheet->AddCellSpanW( CD_PIBCV_CheckBox, RD_PIBCValve_GroupName, CD_PIBCV_Separator - CD_PIBCV_CheckBox, 1 );
	pclSSheet->SetStaticText( CD_PIBCV_CheckBox, RD_PIBCValve_GroupName, strTitle );
	pclSSheet->AddCellSpanW( CD_PIBCV_PipeSize, RD_PIBCValve_GroupName, CD_PIBCV_Pointer - CD_PIBCV_PipeSize, 1 );
	
	CTable *pclTable = dynamic_cast<CTable *>( m_pclIndSel6WayValveParams->m_pPipeDB->Get( m_pclIndSel6WayValveParams->m_strPipeSeriesID ).MP );
	ASSERT( NULL != pclTable );

	pclSSheet->SetStaticText( CD_PIBCV_PipeSize, RD_PIBCValve_GroupName, pclTable->GetName() );
	

	// If no pressure independent balancing & control valve exist...
	if( NULL == pclSelectedPICvList || 0 == pclSelectedPICvList->GetCount() )
	{
		_FillErrorMessages( pclSheetDescriptionPIBCValve, RD_PIBCValve_ColName + 1, TASApp.LoadLocalizedString( IDS_RVIEWSSELCO_NOPIBCV ) );
		return;
	}

	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

	// Add possibility to change column size.
	// Remark: place this code after any initialization (like adding, removing, resizing, hiding/showing columns).
	CResizingColumnInfo *pclResizingColumnInfo = pclSheetDescriptionPIBCValve->GetResizingColumnInfo();
	pclResizingColumnInfo->ActivateFeature( true, false, RD_PIBCValve_FirstAvailRow, lLastDataRow );
	pclResizingColumnInfo->AddRangeColumn( CD_PIBCV_Name, CD_PIBCV_TemperatureRange, RD_PIBCValve_ColName, RD_PIBCValve_Unit );
	pclResizingColumnInfo->AddRangeColumn( CD_PIBCV_PipeSize, CD_PIBCV_PipeV, RD_PIBCValve_ColName, RD_PIBCValve_Unit );

	// Add possibility to click on title group to reset column width.
	CreateTitleGroupButtons( CD_PIBCV_Box, CD_PIBCV_PipeV, RD_PIBCValve_GroupName, pclSheetDescriptionPIBCValve );
}

void CRViewSSel6WayValve::_InitAndFillPIBCValveHeader( CSheetDescription *pclSheetDescriptionPIBCValve, CSSheet *pclSSheet, SideDefinition eSideDefinition )
{
	if( NULL == m_pclIndSel6WayValveParams || NULL == m_pclIndSel6WayValveParams->m_pclSelect6WayValveList 
			|| NULL == m_pclIndSel6WayValveParams->m_pclSelect6WayValveList->GetPICVList( eSideDefinition ) || NULL == m_pclIndSel6WayValveParams->m_pPipeDB )
	{
		ASSERT_RETURN;
	}

	// Set title.
	pclSSheet->SetMaxRows( RD_PIBCValve_FirstAvailRow - 1 );

	// All cells are static by default and filled with _T("").
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	pclSSheet->FormatStaticText( -1, -1, -1, -1, _T( "" ) );

	// Increase row height.
	double dRowHeight = 12.75;
	pclSSheet->SetRowHeight( RD_PIBCValve_FirstRow, dRowHeight * 0.5 );
	pclSSheet->SetRowHeight( RD_PIBCValve_GroupName, dRowHeight * 1.5 );
	pclSSheet->SetRowHeight( RD_PIBCValve_ColName, dRowHeight * 2 );
	pclSSheet->SetRowHeight( RD_PIBCValve_Unit, dRowHeight * 1.2 );

	// Set columns.
	int iSheetDescriptionID = pclSheetDescriptionPIBCValve->GetSheetDescriptionID();
	pclSheetDescriptionPIBCValve->Init( 1, pclSSheet->GetMaxRows(), 1, CSheetDescription::SD_ParameterMode_Mono );
	pclSheetDescriptionPIBCValve->AddColumnInPixels( CD_PIBCV_FirstColumn, m_mapSSheetColumnWidth[iSheetDescriptionID][CD_PIBCV_FirstColumn] );
	pclSheetDescriptionPIBCValve->AddColumnInPixels( CD_PIBCV_Box, m_mapSSheetColumnWidth[iSheetDescriptionID][CD_PIBCV_Box] );
	pclSheetDescriptionPIBCValve->AddColumnInPixels( CD_PIBCV_CheckBox, m_mapSSheetColumnWidth[iSheetDescriptionID][CD_PIBCV_CheckBox] );
	pclSheetDescriptionPIBCValve->AddColumnInPixels( CD_PIBCV_Name, m_mapSSheetColumnWidth[iSheetDescriptionID][CD_PIBCV_Name] );
	pclSheetDescriptionPIBCValve->AddColumnInPixels( CD_PIBCV_Material, m_mapSSheetColumnWidth[iSheetDescriptionID][CD_PIBCV_Material] );
	pclSheetDescriptionPIBCValve->AddColumnInPixels( CD_PIBCV_Connection, m_mapSSheetColumnWidth[iSheetDescriptionID][CD_PIBCV_Connection] );
	pclSheetDescriptionPIBCValve->AddColumnInPixels( CD_PIBCV_Version, m_mapSSheetColumnWidth[iSheetDescriptionID][CD_PIBCV_Version] );
	pclSheetDescriptionPIBCValve->AddColumnInPixels( CD_PIBCV_Size, m_mapSSheetColumnWidth[iSheetDescriptionID][CD_PIBCV_Size] );
	pclSheetDescriptionPIBCValve->AddColumnInPixels( CD_PIBCV_PN, m_mapSSheetColumnWidth[iSheetDescriptionID][CD_PIBCV_PN] );
	pclSheetDescriptionPIBCValve->AddColumnInPixels( CD_PIBCV_Rangeability, m_mapSSheetColumnWidth[iSheetDescriptionID][CD_PIBCV_Rangeability] );
	pclSheetDescriptionPIBCValve->AddColumnInPixels( CD_PIBCV_LeakageRate, m_mapSSheetColumnWidth[iSheetDescriptionID][CD_PIBCV_LeakageRate] );
	pclSheetDescriptionPIBCValve->AddColumnInPixels( CD_PIBCV_Stroke, m_mapSSheetColumnWidth[iSheetDescriptionID][CD_PIBCV_Stroke] );
	pclSheetDescriptionPIBCValve->AddColumnInPixels( CD_PIBCV_ImgCharacteristic, m_mapSSheetColumnWidth[iSheetDescriptionID][CD_PIBCV_ImgCharacteristic] );
	pclSheetDescriptionPIBCValve->AddColumnInPixels( CD_PIBCV_ImgSeparator, m_mapSSheetColumnWidth[iSheetDescriptionID][CD_PIBCV_ImgSeparator] );
	pclSheetDescriptionPIBCValve->AddColumnInPixels( CD_PIBCV_ImgPushClose, m_mapSSheetColumnWidth[iSheetDescriptionID][CD_PIBCV_ImgPushClose] );
	pclSheetDescriptionPIBCValve->AddColumnInPixels( CD_PIBCV_Preset, m_mapSSheetColumnWidth[iSheetDescriptionID][CD_PIBCV_Preset] );
	// HYS-1380: Add Dp min. Cooling and Dp min.Heating for EQM control
	pclSheetDescriptionPIBCValve->AddColumnInPixels( CD_PIBCV_DpMinCooling, m_mapSSheetColumnWidth[iSheetDescriptionID][CD_PIBCV_DpMinCooling] );
	pclSheetDescriptionPIBCValve->AddColumnInPixels( CD_PIBCV_DpMinHeating, m_mapSSheetColumnWidth[iSheetDescriptionID][CD_PIBCV_DpMinHeating] );
	pclSheetDescriptionPIBCValve->AddColumnInPixels( CD_PIBCV_DpMax, m_mapSSheetColumnWidth[iSheetDescriptionID][CD_PIBCV_DpMax] );
	pclSheetDescriptionPIBCValve->AddColumnInPixels( CD_PIBCV_TemperatureRange, m_mapSSheetColumnWidth[iSheetDescriptionID][CD_PIBCV_TemperatureRange] );
	pclSheetDescriptionPIBCValve->AddColumnInPixels( CD_PIBCV_Separator, m_mapSSheetColumnWidth[iSheetDescriptionID][CD_PIBCV_Separator] );
	pclSheetDescriptionPIBCValve->AddColumnInPixels( CD_PIBCV_PipeSize, m_mapSSheetColumnWidth[iSheetDescriptionID][CD_PIBCV_PipeSize] );
	pclSheetDescriptionPIBCValve->AddColumnInPixels( CD_PIBCV_PipeLinDp, m_mapSSheetColumnWidth[iSheetDescriptionID][CD_PIBCV_PipeLinDp] );
	pclSheetDescriptionPIBCValve->AddColumnInPixels( CD_PIBCV_PipeV, m_mapSSheetColumnWidth[iSheetDescriptionID][CD_PIBCV_PipeV] );
	pclSheetDescriptionPIBCValve->AddColumnInPixels( CD_PIBCV_Pointer, m_mapSSheetColumnWidth[iSheetDescriptionID][CD_PIBCV_Pointer] );

	// Set in which column parameter must be saved.
	pclSheetDescriptionPIBCValve->AddParameterColumn( CD_PIBCV_Pointer );

	// Set the focus column.
	pclSheetDescriptionPIBCValve->SetActiveColumn( CD_PIBCV_Name );

	// Set range for selection.
	pclSheetDescriptionPIBCValve->SetFocusColumnRange( CD_PIBCV_CheckBox, CD_PIBCV_TemperatureRange );

	// Columns Title.
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap, (LPARAM)TRUE );
	pclSSheet->SetStaticText( CD_PIBCV_Name, RD_PIBCValve_ColName, IDS_SSHEETSSEL_NAME );
	pclSSheet->SetStaticText( CD_PIBCV_Material, RD_PIBCValve_ColName, IDS_SSHEETSSEL_MATERIAL );
	pclSSheet->SetStaticText( CD_PIBCV_Connection, RD_PIBCValve_ColName, IDS_SSHEETSSEL_CONNECT );
	pclSSheet->SetStaticText( CD_PIBCV_Version, RD_PIBCValve_ColName, IDS_SSHEETSSEL_VERSION );
	pclSSheet->SetStaticText( CD_PIBCV_Size, RD_PIBCValve_ColName, IDS_SSHEETSSEL_SIZE );
	pclSSheet->SetStaticText( CD_PIBCV_PN, RD_PIBCValve_ColName, IDS_SSHEETSSEL_PN );
	pclSSheet->SetStaticText( CD_PIBCV_Rangeability, RD_PIBCValve_ColName, IDS_SSHEETSSELCV_RANGEABILITY );
	pclSSheet->SetStaticText( CD_PIBCV_LeakageRate, RD_PIBCValve_ColName, IDS_SSHEETSSELCV_LEAKAGERATE );
	pclSSheet->SetStaticText( CD_PIBCV_Stroke, RD_PIBCValve_ColName, IDS_SSHEETSSELCV_CVSTROKE );
	pclSSheet->SetStaticText( CD_PIBCV_Preset, RD_PIBCValve_ColName, IDS_SSHEETSSEL_PRESET );
	// HYS-1380: Add Dp min. Cooling and Dp min.Heating for EQM control
	if( eSideDefinition == CoolingSide )
	{
		pclSSheet->SetStaticText( CD_PIBCV_DpMinCooling, RD_PIBCValve_ColName, IDS_SSHEETSSELCV_DPMIN );
	}
	else if( eSideDefinition == HeatingSide )
	{
		pclSSheet->SetStaticText( CD_PIBCV_DpMinHeating, RD_PIBCValve_ColName, IDS_SSHEETSSELCV_DPMIN );
	}
	else
	{
		pclSSheet->SetStaticText( CD_PIBCV_DpMinCooling, RD_PIBCValve_ColName, IDS_SSHEETSSELPIBCV_DPMINCOOLING );
		pclSSheet->SetStaticText( CD_PIBCV_DpMinHeating, RD_PIBCValve_ColName, IDS_SSHEETSSELPIBCV_DPMINHEATING );
	}
	pclSSheet->SetStaticText( CD_PIBCV_DpMax, RD_PIBCValve_ColName, IDS_RVIEWSSELCTRL_DPMAX );
	pclSSheet->SetStaticText( CD_PIBCV_TemperatureRange, RD_PIBCValve_ColName, IDS_SSHEETSSEL_COLTEMPRANGE );

	pclSSheet->SetStaticText( CD_PIBCV_Separator, RD_PIBCValve_ColName, _T( "" ) );


	// Flags for column customizing
	CSelectedBase *pclSelectedProduct = m_pclIndSel6WayValveParams->m_pclSelect6WayValveList->GetPICVList( eSideDefinition )->GetFirst<CSelectedValve>();

	if( NULL == pclSelectedProduct )
	{
		return;
	}

	CDB_PIControlValve *pclPIBCValve = dynamic_cast<CDB_PIControlValve *>( pclSelectedProduct->GetpData() );

	if( NULL == pclPIBCValve )
	{
		return;
	}

	// Pipes
	pclSSheet->SetStaticText( CD_PIBCV_PipeSize, RD_PIBCValve_ColName, IDS_SSHEETSSEL_PIPESIZE );
	pclSSheet->SetStaticText( CD_PIBCV_PipeLinDp, RD_PIBCValve_ColName, IDS_SSHEETSSEL_PIPELINDP );
	pclSSheet->SetStaticText( CD_PIBCV_PipeV, RD_PIBCValve_ColName, IDS_SSHEETSSEL_PIPEV );

	// Units.
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();
	pclSSheet->SetStaticText( CD_PIBCV_LeakageRate, RD_PIBCValve_Unit, IDS_SSHEETSSELCV_LEAKAGERATEUNIT );
	// HYS-1380: Add Dp min. Cooling and Dp min.Heating for EQM control
	pclSSheet->SetStaticText( CD_PIBCV_DpMinCooling, RD_PIBCValve_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ) ).c_str() );
	pclSSheet->SetStaticText( CD_PIBCV_DpMinHeating, RD_PIBCValve_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ) ).c_str() );
	pclSSheet->SetStaticText( CD_PIBCV_TemperatureRange, RD_PIBCValve_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_TEMPERATURE ) ).c_str() );
	pclSSheet->SetStaticText( CD_PIBCV_Preset, RD_PIBCValve_Unit, IDS_SHEETHDR_TURNSPOS );

	// Units.
	CDB_CloseOffChar *pclCloseOffChar = (CDB_CloseOffChar *)pclPIBCValve->GetCloseOffCharIDPtr().MP;

	if( NULL != pclCloseOffChar )
	{
		if( CDB_CloseOffChar::eOpenType::Linear == pclCloseOffChar->GetOpenType() )
		{
			pclSSheet->SetStaticText( CD_PIBCV_Stroke, RD_PIBCValve_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_DIAMETER ) ).c_str() );
		}
		else	//Circular
		{
			pclSSheet->SetStaticText( CD_PIBCV_Stroke, RD_PIBCValve_Unit, IDS_ANGULARDEGRE );
		}
	}
	else
	{
		pclSSheet->SetStaticText( CD_PIBCV_Stroke, RD_PIBCValve_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_DIAMETER ) ).c_str() );
	}

	pclSSheet->SetStaticText( CD_PIBCV_DpMax, RD_PIBCValve_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ) ).c_str() );
	pclSSheet->SetStaticText( CD_PIBCV_PipeLinDp, RD_PIBCValve_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_LINPRESSDROP ) ).c_str() );
	pclSSheet->SetStaticText( CD_PIBCV_PipeV, RD_PIBCValve_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_VELOCITY ) ).c_str() );

	// Draw black line below column header.
	pclSSheet->SetCellBorder( CD_PIBCV_CheckBox, RD_PIBCValve_Unit, CD_PIBCV_Separator - 1, RD_PIBCValve_Unit, true, SS_BORDERTYPE_BOTTOM );
	pclSSheet->SetCellBorder( CD_PIBCV_PipeSize, RD_PIBCValve_Unit, CD_PIBCV_Pointer - 1, RD_PIBCValve_Unit, true, SS_BORDERTYPE_BOTTOM );

	// Set that there is no selection at now.
	SetCurrentPIBCValveSelected( NULL, eSideDefinition );
}

long CRViewSSel6WayValve::_FillOnePIBCValveRow( CSheetDescription *pclSheetDescriptionPIBCValve, CSSheet *pclSSheet, CSelectedValve *pclSelectedPICValve, long lRow )
{
	if( NULL == m_pclIndSel6WayValveParams || NULL == m_pclIndSel6WayValveParams->m_pclSelect6WayValveList )
	{
		ASSERTA_RETURN( -1 );
	}

	SideDefinition eSideDefinition = GETSHEETSIDE( pclSheetDescriptionPIBCValve->GetSheetDescriptionID() );
	
	if( NULL == m_pclIndSel6WayValveParams->m_pclSelect6WayValveList->GetPICVList( eSideDefinition )
			|| NULL == m_pclIndSel6WayValveParams->m_pclSelect6WayValveList->GetPICVList( eSideDefinition )->GetSelectPipeList() )
	{
		ASSERTA_RETURN( -1 );
	}

	CDS_TechnicalParameter *pTechParam = m_pclIndSel6WayValveParams->m_pTADS->GetpTechParams();

	CDB_PIControlValve *pclPIBCValve = dynamic_cast<CDB_PIControlValve *>( pclSelectedPICValve->GetpData() );

	if( NULL == pclPIBCValve )
	{
		ASSERTA_RETURN( -1 );
	}

	bool bAtLeastOneError = false;
	bool bAtLeastOneWarning = false;
	bool bBest = pclSelectedPICValve->IsFlagSet(CSelectedBase::eBest );

	// Add this row.
	// 'true' to specify that this row can be selected.
	pclSheetDescriptionPIBCValve->AddRows( 1, true );

	// First columns will be set at the end!

	// HYS-1877: Add the picture for a PIBCV Act-Set.
	bool bAtLeastOnePIBCVPartOfASet = false;
	if( true == m_pclIndSel6WayValveParams->m_bOnlyForSet && true == pclPIBCValve->IsPartOfaSet() )
	{
		CCDButtonBox* pCDButtonBox = CreateCellDescriptionBox( CD_PIBCV_Box, lRow, true, CCDButtonBox::ButtonState::BoxOpened, pclSheetDescriptionPIBCValve );

		if( NULL != pCDButtonBox )
		{
			pCDButtonBox->ApplyInternalChange();
		}

		bAtLeastOnePIBCVPartOfASet = true;
	}

	// Add checkbox.
	pclSSheet->SetCheckBox( CD_PIBCV_CheckBox, lRow, _T(""), false, true );

	if( e6Way_OnOffControlWithPIBCV == m_pclIndSel6WayValveParams->m_e6WayValveSelectionMode )
	{
		// Set the presetting first to determine if the orange color must be used.
		CString str = _T("-");
		double dPresetting = pclSelectedPICValve->GetH();
		
		// Remark: to access valve characteristic of the PICV valve, don't call the base class method 'GetValveCharacteristic'.
		//         For 'CDB_PIControlValve', 'CDB_ThermostaticValve' and 'CDB_FlowLimitedControlValve', valve characteristics are now set in respective 
		//         'CDB_PICVCharacteristic', 
		CDB_ValveCharacteristic *pclValveCharacteristic = (CDB_ValveCharacteristic *)pclPIBCValve->GetValveCharDataPointer();
		
		if( NULL != pclValveCharacteristic && -1.0 != dPresetting )
		{
			str = pclValveCharacteristic->GetSettingString( dPresetting );
		}

		// If full opening indicator is set after the opening...
		if( -1 != str.Find( _T('*') ) )
		{
			str.Replace( _T('*'), _T(' ') );
			pclSheetDescriptionPIBCValve->WriteTextWithFlags( str, CD_PIBCV_Preset, lRow, CSheetDescription::RemarkFlags::FullOpening );
		}
		else
		{
			if( NULL != pclValveCharacteristic && -1.0 != dPresetting && dPresetting < pclValveCharacteristic->GetMinRecSetting() )
			{
				pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_ORANGE );
				bAtLeastOneWarning = true;
			}

			pclSSheet->SetStaticText( CD_PIBCV_Preset, lRow, str );
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );
		}
	}

	// Set the rangeability.
	pclSSheet->SetStaticText( CD_PIBCV_Rangeability, lRow, pclPIBCValve->GetStrRangeability().c_str() );

	// Set the leakage rate.
	pclSSheet->SetStaticText( CD_PIBCV_LeakageRate, lRow, WriteCUDouble( _U_NODIM, pclPIBCValve->GetLeakageRate() * 100 ) );
				
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

	// Set the Dp min value.
	// HYS-825: Dp min from pPICv instead of pclSelectedPICv
	double dHeatingDpMin = pclPIBCValve->GetDpmin( m_pclIndSel6WayValveParams->GetHeatingFlow(), m_pclIndSel6WayValveParams->GetHeatingWaterChar().GetDens() );
	double dCoolingDpMin = pclPIBCValve->GetDpmin( m_pclIndSel6WayValveParams->GetCoolingFlow(), m_pclIndSel6WayValveParams->GetCoolingWaterChar().GetDens() );
	
	// HYS-1380: Add Dp min. Cooling and Dp min.Heating for EQM control. Show the corresponding Dp min. not the minimum.
	if( eSideDefinition == CoolingSide )
	{
		pclSSheet->SetStaticText( CD_PIBCV_DpMinCooling, lRow, WriteCUDouble( _U_DIFFPRESS, dCoolingDpMin ) );
	}
	else if( eSideDefinition == HeatingSide )
	{
		pclSSheet->SetStaticText( CD_PIBCV_DpMinHeating, lRow, WriteCUDouble( _U_DIFFPRESS, dHeatingDpMin ) );
	}
	else
	{
		pclSSheet->SetStaticText( CD_PIBCV_DpMinCooling, lRow, WriteCUDouble( _U_DIFFPRESS, dCoolingDpMin ) );
		pclSSheet->SetStaticText( CD_PIBCV_DpMinHeating, lRow, WriteCUDouble( _U_DIFFPRESS, dHeatingDpMin ) );
	}

	// Set the stroke value.
	CDB_CloseOffChar *pCloseOffChar = (CDB_CloseOffChar *)pclPIBCValve->GetCloseOffCharIDPtr().MP;
		
	if( NULL != pCloseOffChar )
	{
		if( CDB_CloseOffChar::eOpenType::Linear == pCloseOffChar->GetOpenType() )
		{
			pclSSheet->SetStaticText( CD_PIBCV_Stroke, lRow, WriteCUDouble( _U_DIAMETER, pclPIBCValve->GetStroke() ) );
		}
		else
		{
			pclSSheet->SetStaticText( CD_PIBCV_Stroke, lRow, WriteDouble( pclPIBCValve->GetStroke(), 0, 0) );
		}
	}
	else
	{
		pclSSheet->SetStaticText( CD_PIBCV_Stroke, lRow, WriteCUDouble( _U_DIAMETER, pclPIBCValve->GetStroke() ) ); 
	}

	// Set the characteristic picture.
	CDB_ControlProperties::eCTRLCHAR CtrlChar = pclPIBCValve->GetCtrlProp()->GetCvCtrlChar();
		
	if( CDB_ControlProperties::Linear == CtrlChar )
	{
		pclSSheet->SetPictureCellWithID( IDI_CHARACTLIN, CD_PIBCV_ImgCharacteristic, lRow, CSSheet::PictureCellType::Icon );
	}
	else if( CDB_ControlProperties::EqualPc == CtrlChar )
	{
		pclSSheet->SetPictureCellWithID( IDI_CHARACTEQM, CD_PIBCV_ImgCharacteristic, lRow, CSSheet::PictureCellType::Icon );
	}
	else
	{
		pclSSheet->SetPictureCellWithID( IDI_CHARACTNONE, CD_PIBCV_ImgCharacteristic, lRow, CSSheet::PictureCellType::Icon );
	}

	// Set the push to open/close picture.
	IDPTR IDPtr = pclPIBCValve->GetTypeIDPtr();
	CDB_ControlProperties::ePushOrPullToClose PushClose = pclPIBCValve->GetCtrlProp()->GetCvPushClose();
		
	if( CDB_ControlProperties::PushToClose == PushClose )
	{
		pclSSheet->SetPictureCellWithID( IDI_PUSHCLOSE, CD_PIBCV_ImgPushClose, lRow, CSSheet::PictureCellType::Icon );
	}
	else if( CDB_ControlProperties::PullToClose == PushClose )
	{
		pclSSheet->SetPictureCellWithID( IDI_PUSHOPEN, CD_PIBCV_ImgPushClose, lRow, CSSheet::PictureCellType::Icon );
	}
	else if( CDB_ControlProperties::Undef == PushClose )
	{
		pclSSheet->ShowCol( CD_PIBCV_ImgSeparator, FALSE );
		pclSSheet->ShowCol( CD_PIBCV_ImgPushClose, FALSE );
	}

	// Set Dp max.
	if( true == m_pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_bIsDpMaxChecked )
	{
		if( pclPIBCValve->GetDpmax() < m_pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_dDpMax )
		{
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_RED );
			bAtLeastOneError = true;
		}

		pclSSheet->SetStaticText( CD_PIBCV_DpMax, lRow, WriteDouble( CDimValue::SItoCU( _U_DIFFPRESS, pclPIBCValve->GetDpmax() ), 3, 0 ) );
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );
	}

	// Set the temperature range.
	if( true == pclSelectedPICValve->IsFlagSet( CSelectedBase::eTemperature ) )
	{
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_RED );
		bAtLeastOneError = true;
	}

	pclSSheet->SetStaticText( CD_PIBCV_TemperatureRange, lRow, ( (CDB_TAProduct *)( pclSelectedPICValve->GetProductIDPtr().MP ) )->GetTempRange() );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

	// Now we can set first columns in regards to current status (error, best or normal).
	if( true == bAtLeastOneError )
	{
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_RED );
	}
	else if( true == pclSelectedPICValve->IsFlagSet( CSelectedBase::eBest ) )
	{
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_GREEN );
	}
		
	// In addition of current flag, we have possibility that valve has the flag 'Not available' or 'Deleted'. In that case, we show valve name in red with "*" around it and
	// symbol '!' or '!!' after.
	if( true == pclPIBCValve->IsDeleted() )
	{
		pclSheetDescriptionPIBCValve->WriteTextWithFlags( CString( pclPIBCValve->GetName() ), CD_PIBCV_Name, lRow, 
				CSheetDescription::RemarkFlags::Deleted, _T(""), _RED );
	}
	else if( false == pclPIBCValve->IsAvailable() )
	{
		pclSheetDescriptionPIBCValve->WriteTextWithFlags( CString( pclPIBCValve->GetName() ), CD_PIBCV_Name, lRow, 
				CSheetDescription::RemarkFlags::NotAvailable, _T(""), _RED );
	}
	else
	{
		pclSSheet->SetStaticText( CD_PIBCV_Name, lRow, pclPIBCValve->GetName() );
	}
		
	if( true == bAtLeastOneError )
	{
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );
	}
	else if( true == pclSelectedPICValve->IsFlagSet( CSelectedBase::eBest ) )
	{
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_GREEN );
	}

	pclSSheet->SetStaticText( CD_PIBCV_Material, lRow, pclPIBCValve->GetBodyMaterial() );
	pclSSheet->SetStaticText( CD_PIBCV_Connection, lRow, pclPIBCValve->GetConnect() );
	pclSSheet->SetStaticText( CD_PIBCV_Version, lRow, pclPIBCValve->GetVersion() );
	pclSSheet->SetStaticText( CD_PIBCV_PN, lRow, pclPIBCValve->GetPN().c_str() );
	pclSSheet->SetStaticText( CD_PIBCV_Size, lRow, pclPIBCValve->GetSize() );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

	pclSSheet->SetStaticText( CD_PIBCV_Separator, lRow );

	// Set pipe size.
	CSelectPipe selPipe( m_pclIndSel6WayValveParams );

	// Pay attention: we need to take here the appropriate pipe list (Not the one that is in the 'm_pclIndSel6WayValveParams->m_pclSelect6WayValveList'.
	if( e6Way_EQMControl == m_pclIndSel6WayValveParams->m_e6WayValveSelectionMode )
	{
		m_pclIndSel6WayValveParams->m_pclSelect6WayValveList->GetSelectPipeList()->GetMatchingPipe( pclPIBCValve->GetSizeKey(), selPipe );
	}
	else
	{
		m_pclIndSel6WayValveParams->m_pclSelect6WayValveList->GetPICVList( eSideDefinition )->GetSelectPipeList()->GetMatchingPipe( pclPIBCValve->GetSizeKey(), selPipe );
	}

	pclSSheet->SetStaticText( CD_PIBCV_PipeSize, lRow, selPipe.GetpPipe()->GetName() );
		
	// Set the LinDp to orange if it is above or below the technical parameters limits.
	if( selPipe.GetLinDp() > pTechParam->GetPipeMaxDp() || selPipe.GetLinDp() < pTechParam->GetPipeMinDp() )
	{
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_ORANGE );
	}
		
	pclSSheet->SetStaticText( CD_PIBCV_PipeLinDp, lRow, WriteCUDouble( _U_LINPRESSDROP, selPipe.GetLinDp() ) );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

	// Set the velocity to orange if it is above the technical parameter limit.
	// Orange if it is below the dMinVel.
	if( selPipe.GetU() > pTechParam->GetPipeMaxVel() || selPipe.GetU() < pTechParam->GetPipeMinVel() )
	{
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_ORANGE );
	}
		
	pclSSheet->SetStaticText( CD_PIBCV_PipeV, lRow, WriteCUDouble( _U_VELOCITY, selPipe.GetU() ) );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

	// Save parameter.
	CreateCellDescriptionProduct( pclSheetDescriptionPIBCValve->GetFirstParameterColumn(), lRow, (LPARAM)pclPIBCValve, pclSheetDescriptionPIBCValve );

	pclSSheet->SetCellBorder( CD_PIBCV_Box, lRow, CD_PIBCV_Separator- 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _GRAY );
	pclSSheet->SetCellBorder( CD_PIBCV_PipeSize, lRow, CD_PIBCV_Pointer - 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _GRAY );

	return 0;
}

void CRViewSSel6WayValve::_FillPIBCValveAccessories( SideDefinition eSideDefinition )
{
	if( NULL == m_pclIndSel6WayValveParams || NULL == m_pclIndSel6WayValveParams->m_pTADB )
	{
		ASSERT_RETURN;
	}

	CDB_TAProduct *pclCurrentPIBCValveSelected = GetCurrentPIBCValveSelected( eSideDefinition );

	if( NULL == pclCurrentPIBCValveSelected )
	{
		return;
	}

	CDB_RuledTable* pclRuledTable = NULL;
	// HYS-1877: Manage also set accessories.
	if( true == m_pclIndSel6WayValveParams->m_bOnlyForSet )
	{
		CDB_Set* pclCurrentValveSet = NULL;
		// Retrieve the correct set table in regards to the control valve.
		CTableSet* pTableSet = pclCurrentPIBCValveSelected->GetTableSet();

		if( NULL != pTableSet )
		{
			pclCurrentValveSet = pTableSet->FindCompatibleSet6WayValve( pclCurrentPIBCValveSelected->GetIDPtr().ID, _T( "" ) );
		}

		if( NULL == pclCurrentValveSet )
		{
			return;
		}

		pclRuledTable = (CDB_RuledTable*)(pclCurrentValveSet->GetAccGroupIDPtr().MP);
	}
	else
	{
		pclRuledTable = (CDB_RuledTable*)(pclCurrentPIBCValveSelected->GetAccessoriesGroupIDPtr().MP);
	}

	if( NULL == pclRuledTable )
	{
		return;
	}

	CRank rList;
	m_pclIndSel6WayValveParams->m_pTADB->GetAccessories( &rList, pclRuledTable, m_pclIndSel6WayValveParams->m_eFilterSelection );
	_FillAccessoryRows( eSideDefinition, SD_FamilyPIBCValve, SD_TypeValveAccessories, &rList, pclRuledTable );
}

void CRViewSSel6WayValve::_FillPIBCValveActuators( SideDefinition eSideDefinition )
{
	if( NULL == m_pclIndSel6WayValveParams || NULL == m_pclIndSel6WayValveParams->m_pclSelect6WayValveList 
			|| NULL == m_pclIndSel6WayValveParams->m_pclSelect6WayValveList->GetPICVList( eSideDefinition ) )
	{
		ASSERT_RETURN;
	}

	if( e6Way_EQMControl != m_pclIndSel6WayValveParams->m_e6WayValveSelectionMode )
	{
		// No actuator with PIBCV if we are not in the EQM control application type.
		return;
	}

	CDB_PIControlValve *pclSelectedPIBCValve = dynamic_cast<CDB_PIControlValve *>( GetCurrentPIBCValveSelected( eSideDefinition ) );

	if( NULL == pclSelectedPIBCValve )
	{
		return;
	}

	CRank clActuatorList;
	// HYS-1877: Consider set selection.
	_GetPIBCValveActuatorList( pclSelectedPIBCValve, &clActuatorList, eSideDefinition, m_pclIndSel6WayValveParams->m_bOnlyForSet );

	if( 0 == clActuatorList.GetCount() )
	{
		return;
	}

	CDB_CloseOffChar *pCloseOffChar = (CDB_CloseOffChar *)( pclSelectedPIBCValve->GetCloseOffCharIDPtr().MP );

	// Call 'MultiSpreadBase' method to get a new SSheet.
	UINT uiSheetDescriptionID = ( eSideDefinition << SHEETSIDEDEFSHIFT ) + SD_FamilyPIBCValve + SD_TypeValveActuator;
	UINT uiAfterSheetDescriptionID = ( eSideDefinition << SHEETSIDEDEFSHIFT ) + SD_FamilyPIBCValve + SD_TypeValveAccessories;

	if( false == m_ViewDescription.IsSheetDescriptionExist( uiAfterSheetDescriptionID ) )
	{	
		uiAfterSheetDescriptionID = ( eSideDefinition << SHEETSIDEDEFSHIFT ) + SD_FamilyPIBCValve + SD_TypeValve;

		if( false == m_ViewDescription.IsSheetDescriptionExist( uiAfterSheetDescriptionID ) )
		{
			ASSERT_RETURN;
		}
	}

	CSheetDescription *pclSheetDescriptionActuator = CreateSSheet( uiSheetDescriptionID, uiAfterSheetDescriptionID );

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
	pclSheetDescriptionActuator->AddColumnInPixels( CD_Actuator_FirstColumn, m_mapSSheetColumnWidth[uiSheetDescriptionID][CD_Actuator_FirstColumn] );
	pclSheetDescriptionActuator->AddColumnInPixels( CD_Actuator_Box, m_mapSSheetColumnWidth[uiSheetDescriptionID][CD_Actuator_Box] );
	pclSheetDescriptionActuator->AddColumnInPixels( CD_Actuator_CheckBox, m_mapSSheetColumnWidth[uiSheetDescriptionID][CD_Actuator_CheckBox] );
	pclSheetDescriptionActuator->AddColumnInPixels( CD_Actuator_Name, m_mapSSheetColumnWidth[uiSheetDescriptionID][CD_Actuator_Name] );
	pclSheetDescriptionActuator->AddColumnInPixels( CD_Actuator_CloseOffValue, m_mapSSheetColumnWidth[uiSheetDescriptionID][CD_Actuator_CloseOffValue] );
	pclSheetDescriptionActuator->AddColumnInPixels( CD_Actuator_ActuatingTime, m_mapSSheetColumnWidth[uiSheetDescriptionID][CD_Actuator_ActuatingTime] );
	pclSheetDescriptionActuator->AddColumnInPixels( CD_Actuator_IP, m_mapSSheetColumnWidth[uiSheetDescriptionID][CD_Actuator_IP] );
	pclSheetDescriptionActuator->AddColumnInPixels( CD_Actuator_PowSupply, m_mapSSheetColumnWidth[uiSheetDescriptionID][CD_Actuator_PowSupply] );
	pclSheetDescriptionActuator->AddColumnInPixels( CD_Actuator_InputSig, m_mapSSheetColumnWidth[uiSheetDescriptionID][CD_Actuator_InputSig] );
	pclSheetDescriptionActuator->AddColumnInPixels( CD_Actuator_OutputSig, m_mapSSheetColumnWidth[uiSheetDescriptionID][CD_Actuator_OutputSig] );
	pclSheetDescriptionActuator->AddColumnInPixels( CD_Actuator_RelayType, m_mapSSheetColumnWidth[uiSheetDescriptionID][CD_Actuator_RelayType] );
	pclSheetDescriptionActuator->AddColumnInPixels( CD_Actuator_FailSafe, m_mapSSheetColumnWidth[uiSheetDescriptionID][CD_Actuator_FailSafe] );
	pclSheetDescriptionActuator->AddColumnInPixels( CD_Actuator_DefaultReturnPos, m_mapSSheetColumnWidth[uiSheetDescriptionID][CD_Actuator_DefaultReturnPos] );
	pclSheetDescriptionActuator->AddColumnInPixels( CD_Actuator_MaxTemp, m_mapSSheetColumnWidth[uiSheetDescriptionID][CD_Actuator_MaxTemp] );
	pclSheetDescriptionActuator->AddColumnInPixels( CD_Actuator_Pointer, m_mapSSheetColumnWidth[uiSheetDescriptionID][CD_Actuator_Pointer] );

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

	int iSheetType = GETSHEETTYPE( pclSheetDescriptionActuator->GetSheetDescriptionID() );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)_GetGroupColor( false, eSideDefinition, iSheetType ) );
	
	pclSheetDescriptionActuator->SetUserVariable( _SDUV_TITLEFORECOLOR, ( pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor ) ) );
	pclSheetDescriptionActuator->SetUserVariable( _SDUV_TITLEBACKCOLOR, ( pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor ) ) );

	pclSSheet->AddCellSpanW( CD_Actuator_CheckBox, RD_Actuator_GroupName, CD_Actuator_Pointer - CD_Actuator_CheckBox, 1 );

	CString strTitle = TASApp.LoadLocalizedString( IDS_SSHEETSSELCV_ACTRGROUP );

	if( e6Way_OnOffControlWithPIBCV == m_pclIndSel6WayValveParams->m_e6WayValveSelectionMode )
	{
		strTitle += _T(" - ") + TASApp.LoadLocalizedString( ( HeatingSide == eSideDefinition ) ? IDS_SSHEETSSEL6WAYVALVE_HEATINGSIDE : IDS_SSHEETSSEL6WAYVALVE_COOLINGSIDE );
	}

	pclSSheet->SetStaticText( CD_Actuator_CheckBox, RD_Actuator_GroupName, strTitle );

	// Columns title.
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

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
	pclSSheet->SetStaticText( CD_Actuator_DefaultReturnPos, RD_Actuator_ColName, IDS_SSHEETSSEL_ACTRDRP );
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

	// Retrieve Dp on the current valve.
	double dDpOnCV = -1.0;
	CSelectedValve *pSelectedTAP = GetSelectProduct<CSelectedValve>( pclSelectedPIBCValve, m_pclIndSel6WayValveParams->m_pclSelect6WayValveList->GetPICVList( eSideDefinition ) );

	if( NULL != pSelectedTAP )
	{
		dDpOnCV = pSelectedTAP->GetDp();
	}

	// HYS-1877
	bool bAtLeastOneActrPartOfASet = false;

	// Fill actuator list.
	long lRow = RD_Actuator_FirstAvailRow;
	long lActuatorCount = 0;
	pclSheetDescriptionActuator->SetUserVariable( _SDUV_TOTALACTUATOR_COUNT, 0 );
	CString str;
	LPARAM lParam;

	for( BOOL bContinue = clActuatorList.GetFirst( str, lParam ); TRUE == bContinue; bContinue = clActuatorList.GetNext( str, lParam ) )
	{
		CDB_ElectroActuator *pElectroActuator = dynamic_cast<CDB_ElectroActuator *>( (CData *)lParam );

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

					if( pSelectedTAP->GetDp() > 0.0 && dCloseOffDp < pSelectedTAP->GetDp() )
					{ 
						// If there is a Dp defined on the valve but actuator can't close it, we continue.
						continue;
					}

					if( pSelectedTAP->GetDp() <= 0.0 && pclSelectedPIBCValve->GetDpmax() > 0.0 && dCloseOffDp < pclSelectedPIBCValve->GetDpmax() )
					{
						// If there is no Dp on the valve but Dp max exist but actuator can't close at Dp max, we continue.
						continue;
					}

					bActuatorMatch = true;
					break;
				}

				if( false == bActuatorMatch )
				{
					continue;
				}
			}
		}

		// Add this row.
		// 'true' to specify that this row can be selected.
		pclSheetDescriptionActuator->AddRows( 1, true );

		// Add checkbox.
		pclSSheet->SetCheckBox( CD_Actuator_CheckBox, lRow, _T(""), false, true );

		// HYS-1877: Add the picture for a PIBCV Act-Set.
		if( true == m_pclIndSel6WayValveParams->m_bOnlyForSet && true == pElectroActuator->IsPartOfaSet() )
		{
			CCDButtonBox* pCDButtonBox = CreateCellDescriptionBox( CD_Actuator_Box, lRow, true, CCDButtonBox::ButtonState::BoxOpened, pclSheetDescriptionActuator );

			if( NULL != pCDButtonBox )
			{
				pCDButtonBox->ApplyInternalChange();
			}

			bAtLeastOneActrPartOfASet = true;
		}

		// HYS-726.
		if( m_pclIndSel6WayValveParams->m_WC.GetTemp() > pElectroActuator->GetTmax() )
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
					if( pclSelectedPIBCValve->GetDpmax() > 0.0 && dCloseOffDp > pclSelectedPIBCValve->GetDpmax() )
					{
						pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_ORANGE );
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
		pclSSheet->SetStaticText( CD_Actuator_ActuatingTime, lRow, pElectroActuator->GetActuatingTimesStr( pclSelectedPIBCValve->GetStroke() ) );

		CString str = pElectroActuator->GetIPxxFull();
		pclSSheet->SetStaticText( CD_Actuator_IP, lRow, str );

		CDB_StringID *pclPowerSupply = (CDB_StringID *)( m_pclIndSel6WayValveParams->m_pTADB->Get( (LPCTSTR)m_pclIndSel6WayValveParams->m_strActuatorPowerSupplyID ).MP );

		if( ( false == m_pclIndSel6WayValveParams->m_strActuatorPowerSupplyID.IsEmpty() )
				&& ( false == pElectroActuator->IsPowerSupplyAvailable( pclPowerSupply->GetIDPtr() ) ) )
		{
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_ORANGE );
		}
		pclSSheet->SetStaticText( CD_Actuator_PowSupply, lRow, pElectroActuator->GetPowerSupplyStr() );
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

		str.Empty();
		CDB_StringID *pclInputSignal = (CDB_StringID *)( m_pclIndSel6WayValveParams->m_pTADB->Get( (LPCTSTR)m_pclIndSel6WayValveParams->m_strActuatorInputSignalID ).MP );

		if( true == m_mapVariables[SD_FamilyPIBCValve][eSideDefinition].m_bDowngradeValveActuatorFunctionality && false == m_pclIndSel6WayValveParams->m_strActuatorInputSignalID.IsEmpty()
				&& false == pElectroActuator->IsInputSignalAvailable( pclInputSignal->GetIDPtr() ) )
		{
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_ORANGE );
		}

		pclSSheet->SetStaticText( CD_Actuator_InputSig, lRow, pElectroActuator->GetInOutSignalsStr( true ) );
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

		pclSSheet->SetStaticText( CD_Actuator_OutputSig, lRow, pElectroActuator->GetInOutSignalsStr( false ) );

		pclSSheet->SetStaticText( CD_Actuator_RelayType, lRow, pElectroActuator->GetRelayStr() );

		// Fail-Safe.
		// HYS-1458 : If fail-safe checkbox is checked we have to get a fail-safe type. If the checkbox is not checked we have to get No fail-safe. 
		// if these conditions are not satisfied, fail-safe function is not fit.
		if( -1 != m_pclIndSel6WayValveParams->m_iActuatorFailSafeFunction 
			&& ( ( pElectroActuator->GetFailSafe() >= CDB_ElectroActuator::FailSafeType::eFSTypeElectronic && pElectroActuator->GetFailSafe() < CDB_ElectroActuator::FailSafeType::eFSTypeLast
					&& m_pclIndSel6WayValveParams->m_iActuatorFailSafeFunction != 1 )
				|| ( (int)pElectroActuator->GetFailSafe() == 0 && m_pclIndSel6WayValveParams->m_iActuatorFailSafeFunction != 0 ) ) )
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
		if( ( CDB_ControlValve::DRPFunction::drpfAll != m_pclIndSel6WayValveParams->m_eActuatorDRPFunction && CDB_ControlValve::DRPFunction::drpfUndefined != m_pclIndSel6WayValveParams->m_eActuatorDRPFunction )
				&& ( pclSelectedPIBCValve->GetCompatibleDRPFunction( (int)pElectroActuator->GetDefaultReturnPos() ) != m_pclIndSel6WayValveParams->m_eActuatorDRPFunction ) )
		{
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_ORANGE );
		}

		pclSSheet->SetStaticText( CD_Actuator_DefaultReturnPos, lRow, CString( pElectroActuator->GetDefaultReturnPosStr( pElectroActuator->GetDefaultReturnPos() ).c_str() ) );
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );
		
		// HYS-726.
		CString sTmaxMedia = ( DBL_MAX == pElectroActuator->GetTmax() ) ? _T("-") : WriteDouble( pElectroActuator->GetTmax(), 3 );

		if( m_pclIndSel6WayValveParams->m_WC.GetTemp() > pElectroActuator->GetTmax() )
		{
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_RED );
			pclSSheet->SetStaticText( CD_Actuator_MaxTemp, lRow, sTmaxMedia );
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );
		}
		else
		{
			pclSSheet->SetStaticText( CD_Actuator_MaxTemp, lRow, sTmaxMedia );
		}

		pclSSheet->SetCellBorder( CD_Actuator_Box, lRow, CD_Actuator_Pointer - 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _GRAY );

		// Save parameter.
		CreateCellDescriptionProduct( pclSheetDescriptionActuator->GetFirstParameterColumn(), lRow, (LPARAM)pElectroActuator, pclSheetDescriptionActuator );

		lRow++;
		lActuatorCount++;
	}

	// HYS-1877
	if( false == m_pclIndSel6WayValveParams->m_bOnlyForSet || false == bAtLeastOneActrPartOfASet )
	{
		pclSSheet->ShowCol( CD_Actuator_Box, FALSE );
	}

	long lLastDataRow = lRow - 1;

	// Save the total actuator number.
	pclSheetDescriptionActuator->SetUserVariable( _SDUV_TOTALACTUATOR_COUNT, lActuatorCount );

	// Set that there is no selection at now.
	SetCurrentPIBCValveActuatorSelected( NULL, eSideDefinition );
	
	// Add possibility to collapse / expand actuator sheet
	CCDButtonExpandCollapseGroup *&pclGroupButton = m_mapVariables[SD_FamilyPIBCValve][eSideDefinition].m_pCDBExpandCollapseGroupValveActuator;
	
	pclGroupButton = CreateExpandCollapseGroupButton( CD_Actuator_FirstColumn, RD_Actuator_GroupName, true,
			CCDButtonExpandCollapseGroup::ButtonState::CollapseRow, pclSheetDescriptionActuator->GetFirstSelectableRow() - 2,
			pclSheetDescriptionActuator->GetLastSelectableRow(false), pclSheetDescriptionActuator );

	// Show button.
	if( NULL != pclGroupButton )
	{
		pclGroupButton->SetShowStatus( true );
	}
	
	pclSSheet->AddCellSpan( CD_Actuator_FirstColumn, RD_Actuator_GroupName, 2, 1 );

	pclSSheet->SetCellBorder( CD_Actuator_CheckBox, lLastDataRow, CD_Actuator_Pointer - 1, lLastDataRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );

	// Add possibility to change column size.
	// Remark: place this code after any initialization (like adding, removing, resizing, hiding/showing columns).
	CResizingColumnInfo *pclResizingColumnInfo = pclSheetDescriptionActuator->GetResizingColumnInfo();
	pclResizingColumnInfo->ActivateFeature( true, false, RD_Actuator_FirstAvailRow, lLastDataRow );
	pclResizingColumnInfo->AddRangeColumn( CD_Actuator_Name, CD_Actuator_MaxTemp, RD_Actuator_ColName, RD_Actuator_Unit );

	// Add possibility to click on title group to reset column width.
	CreateTitleGroupButtons( CD_Actuator_Box, CD_Actuator_MaxTemp, RD_Actuator_GroupName, pclSheetDescriptionActuator );
}

void CRViewSSel6WayValve::_ClickOnPIBCValveActuator( CSheetDescription *pclSheetDescriptionPIBCValveActuator, CDB_Actuator *pclPIBCValveActuatorClicked, 
		CCellDescriptionProduct *pclCellDescriptionProduct, long lColumn, long lRow )
{
	if( NULL == m_pclIndSel6WayValveParams )
	{
		ASSERT_RETURN;
	}

	CSSheet *pclSSheet = pclSheetDescriptionPIBCValveActuator->GetSSheetPointer();

	LPARAM lPIBCValveActuatorCount;
	pclSheetDescriptionPIBCValveActuator->GetUserVariable( _SDUV_TOTALACTUATOR_COUNT, lPIBCValveActuatorCount );

	int iSheetDescriptionID = pclSheetDescriptionPIBCValveActuator->GetSheetDescriptionID();
	SideDefinition eSideDefinition = GETSHEETSIDE( iSheetDescriptionID );
	int iSheetType = GETSHEETTYPE( iSheetDescriptionID );
	int iSheetFamily = GETSHEETFAMILY( iSheetDescriptionID );

	m_mapVariables[SD_FamilyPIBCValve][eSideDefinition].m_vecValveActuatorAccessoryList.clear();
	m_mapVariables[SD_FamilyPIBCValve][eSideDefinition].m_pCDBExpandCollapseGroupValveActuatorAccessory = NULL;

	CCDButtonExpandCollapseRows *&pCDBExpandCollapseRowsValveActuator = m_mapVariables[SD_FamilyPIBCValve][eSideDefinition].m_pCDBExpandCollapseRowsValveActuator;
	long &lValveSelectedRow = m_mapVariables[SD_FamilyPIBCValve][eSideDefinition].m_lValveActuatorSelectedRow;

	// Memorize previous focus row.
	long lCurrentFocusedRow = GetCurrentFocusedRow();

	// To be sure to clean all previous data concerning focus.
	KillCurrentFocus();

	// Unselect row where current selection is set (yellow light).
	// Remark: 'KillCurrentFocus' unselect only rows where focus is on (blue light).
	pclSSheet->UnSelectMultipleRows();

	// Retrieve the current selected actuator if exist.
	CDB_Actuator *pclCurrentPIBCValveActuatorSelected = GetCurrentPIBCValveActuatorSelected( eSideDefinition );

	// If there is already one actuator selected and user click on the current one...
	// Remark: 'pclCurrentPIBCValveActuatorSelected' is not created if there is only one actuator. Thus we need to check first if there is only one actuator.
	//         Otherwise we have to check if user has clicked on the current one.
	if( NULL != pclCurrentPIBCValveActuatorSelected && ( ( 1 == lPIBCValveActuatorCount ) || ( NULL != pCDBExpandCollapseRowsValveActuator
			&& lRow == pCDBExpandCollapseRowsValveActuator->GetCellPosition().y ) ) )
	{
		// Reset current product selected.
		SetCurrentPIBCValveActuatorSelected( NULL, eSideDefinition );

		// Delete Expand/Collapse rows button if exist.
		// Remark: 'pclCurrentPIBCValveActuatorSelected' is set to NULL in method!
		DeleteExpandCollapseRowsButton( pCDBExpandCollapseRowsValveActuator, pclSheetDescriptionPIBCValveActuator );

		// Remove actuator accessories sheet if exist.
		_RemoveAllSheetAfter( eSideDefinition, iSheetFamily, iSheetType );

		// HYS-1877: If the TA-6-way control valve is part of a set, change box button to open state.
		if( true == m_pclIndSel6WayValveParams->m_bOnlyForSet )
		{
			ChangeBoxStateFor6WValveSet( CCDButtonBox::ButtonState::BoxOpened, true, eSideDefinition, lRow, pclSheetDescriptionPIBCValveActuator );
		}

		// Uncheck checkbox.
		pclSSheet->SetCheckBox( CD_Actuator_CheckBox, lRow, _T(""), false, true );

		// Set focus on the actuator currently selected.
		PrepareAndSetNewFocus( pclSheetDescriptionPIBCValveActuator, pclSheetDescriptionPIBCValveActuator->GetActiveColumn(), lRow, 0 );

		lValveSelectedRow = 0;

		// Notify a change in the selection.
		SelectionHasChanged( IsSelectionReady() );
	}
	else
	{
		// If an actuator is already selected...
		if( NULL != pclCurrentPIBCValveActuatorSelected )
		{
			// Remove actuator accessories sheet if exist.
			_RemoveAllSheetAfter( eSideDefinition, iSheetFamily, iSheetType );

			// HYS-1877: Change box button to open state for previous TA-6-way control valve.
			if( true == m_pclIndSel6WayValveParams->m_bOnlyForSet )
			{
				ChangeBoxStateFor6WValveSet( CCDButtonBox::ButtonState::BoxOpened, true, eSideDefinition, lRow, pclSheetDescriptionPIBCValveActuator );
			}

			// Uncheck checkbox.
			pclSSheet->SetCheckBox( CD_Actuator_CheckBox, lValveSelectedRow, _T(""), false, true );
		}

		// Save new actuator selection.
		SetCurrentPIBCValveActuatorSelected( pclCellDescriptionProduct, eSideDefinition );

		// Delete Expand/Collapse rows button if exist.
		if( NULL != pCDBExpandCollapseRowsValveActuator )
		{
			// Remark: 'm_pCDBExpandCollapseRowsControlValveActuator' is set to NULL in method!
			DeleteExpandCollapseRowsButton( pCDBExpandCollapseRowsValveActuator, pclSheetDescriptionPIBCValveActuator );
		}

		// Create Expand/Collapse rows button if needed...
		if( lPIBCValveActuatorCount > 1 )
		{
			pCDBExpandCollapseRowsValveActuator = CreateExpandCollapseRowsButton( CD_Actuator_FirstColumn, lRow, true, 
					CCDButtonExpandCollapseRows::ButtonState::CollapseRow, pclSheetDescriptionPIBCValveActuator->GetFirstSelectableRow(), 
					pclSheetDescriptionPIBCValveActuator->GetLastSelectableRow( false ), pclSheetDescriptionPIBCValveActuator );

			// Show button.
			if( NULL != pCDBExpandCollapseRowsValveActuator )
			{
				pCDBExpandCollapseRowsValveActuator->SetShowStatus( true );
			}
		}

		// HYS-1877: If we are in selection by package mode and the current control valve and actuator belong to a set...
		// Change box button to close state.
		if( true == m_pclIndSel6WayValveParams->m_bOnlyForSet )
		{
			ChangeBoxStateFor6WValveSet( CCDButtonBox::ButtonState::BoxClosed, true, eSideDefinition, lRow, pclSheetDescriptionPIBCValveActuator );
		}

		// Check checkbox.
		pclSSheet->SetCheckBox( CD_Actuator_CheckBox, lRow, _T(""), true, true );

		// Fill accessories on actuator if exist.
		_FillPIBCValveActuatorAccessories( eSideDefinition );

		// Select actuator (just highlight background).
		pclSSheet->SelectOneRow( lRow, pclSheetDescriptionPIBCValveActuator->GetSelectionFrom(), pclSheetDescriptionPIBCValveActuator->GetSelectionTo() );

		// Try to set the focus on the next table but do not lose the focus if no other table exist.
		CSheetDescription *pclNextSheetDescription = NULL;
		long lNewFocusedRow;
		bool bShiftPressed;

		if( DNFERRC_FocusOnNextGroupOK == DefineNextFocusedRow( pclSheetDescriptionPIBCValveActuator, CD_Actuator_FirstColumn, lRow, false, lNewFocusedRow,
				pclNextSheetDescription, bShiftPressed ) )
		{
			long lNewFocusedCol = pclNextSheetDescription->GetFirstParameterColumn();
			PrepareAndSetNewFocus( pclNextSheetDescription, lNewFocusedCol, lNewFocusedRow );
		}
		else
		{
			// Because we have changed current selection background in yellow, if there is no other sheet following current one, we
			// must set focus on current selection.
			PrepareAndSetNewFocus( pclSheetDescriptionPIBCValveActuator, lColumn, lRow );
		}

		lValveSelectedRow = lRow;

		// Notify a change in the selection.
		SelectionHasChanged( IsSelectionReady() );
	}
}

void CRViewSSel6WayValve::_FillPIBCValveActuatorAccessories( SideDefinition eSideDefinition )
{
	if( NULL == m_pclIndSel6WayValveParams )
	{
		ASSERT_RETURN;
	}

	// Check the current pressure independent balancing & control valve selected.
	CDB_PIControlValve *pclCurrentPIBCValveSelected = dynamic_cast<CDB_PIControlValve *>( GetCurrentPIBCValveSelected( eSideDefinition ) );

	if( NULL == pclCurrentPIBCValveSelected )
	{
		ASSERT_RETURN;
	}

	// Check current actuator selected.
	CDB_Actuator *pclCurrentPIBCValveActuatorSelected = GetCurrentPIBCValveActuatorSelected( eSideDefinition );

	if( NULL == pclCurrentPIBCValveActuatorSelected )
	{
		ASSERT_RETURN;
	}

	// Retrieve accessories belonging to the selected actuator.
	CRank rActuatorAccessoryList;
	CDB_RuledTableBase *pclRuledTable = NULL;
	_GetActuatorAccessoryList( pclCurrentPIBCValveActuatorSelected, &rActuatorAccessoryList, &pclRuledTable );

	if( rActuatorAccessoryList.GetCount() > 0 )
	{
		_FillAccessoryRows( eSideDefinition, SD_FamilyPIBCValve, SD_TypeValveActuatorAccessories, &rActuatorAccessoryList, pclRuledTable );
	}
}

void CRViewSSel6WayValve::_GetPIBCValveActuatorList( CDB_PIControlValve *pclSelectedPIBCValve, CRank *pclActuatorList, SideDefinition eSideDefinition, bool bOnlyForSet )
{
	if( NULL == m_pclIndSel6WayValveParams || NULL == m_pclIndSel6WayValveParams->m_pTADB || NULL == pclSelectedPIBCValve || NULL == pclActuatorList )
	{
		ASSERT_RETURN;
	}

	pclActuatorList->PurgeAll();

	// Retrieve the actuator group on the selected pressure independent balancing & control valve.
	CTable *pclActuatorGroup = (CTable *)( pclSelectedPIBCValve->GetActuatorGroupIDPtr().MP );

	if( NULL == pclActuatorGroup )
	{
		return;
	}

	// Retrieve list of all actuators in this group.
	CRank rList( false );

	for( IDPTR idptr = pclActuatorGroup->GetFirst(); NULL != idptr.MP; idptr = pclActuatorGroup->GetNext( idptr.MP ) )
	{
		rList.Add( idptr.ID, 0, ( LPARAM )idptr.MP );
	}

	CDB_6WayValveActuatorCompatibility *pcl6WayValveActrComp = dynamic_cast<CDB_6WayValveActuatorCompatibility *>( TASApp.GetpTADB()->Get( _T("6WAY_ACTR_COMP") ).MP );
	ASSERT( NULL != pcl6WayValveActrComp );
	
	CDB_6WayValveActuatorCompatibility::ActuatorFilters *pActuatorFilters = pcl6WayValveActrComp->GetOneActutorFilters( m_pclIndSel6WayValveParams->m_e6WayValveSelectionMode );

	if( NULL == pActuatorFilters )
	{
		ASSERT_RETURN;
	}

	// Now we have to check the list to filter on the 'Fast electrical connection' checkbox.
	CString str;
	LPARAM lparam;

	for( BOOL bContinue = rList.GetFirst( str, lparam ); TRUE == bContinue; bContinue = rList.GetNext( str, lparam ) )
	{
		CDB_ElectroActuator *pElectroActuator = dynamic_cast<CDB_ElectroActuator *>( (CData *)lparam );

		if( NULL == pElectroActuator )
		{
			continue;
		}

		// HYS-1380 : Do not show unavailable actuators
		if( false == pElectroActuator->IsSelectable( true ) )
		{
			continue;
		}

		// Verify family compatibility.
		if( pActuatorFilters->m_PIBCValveActuatorFamilyIDCompatibilityList.size() > 0 )
		{
			bool bFound = false;

			for( auto &iterFamilyIDPtr : pActuatorFilters->m_PIBCValveActuatorFamilyIDCompatibilityList )
			{
				if( 0 == _tcscmp( pElectroActuator->GetFamilyID(), iterFamilyIDPtr.ID ) )
				{
					bFound = true;
					break;
				}
			}

			if( false == bFound )
			{
				continue;
			}
		}

		if( true == m_pclIndSel6WayValveParams->m_bCheckFastConnection && 0 != pElectroActuator->GetRelayID().CompareNoCase( _T("RELAY_CO") ) )
		{
			// If we want a fast electrical connection but the actuator isn't compatible...
			continue;
		}

		if( false == m_pclIndSel6WayValveParams->m_bCheckFastConnection && 0 != pElectroActuator->GetRelayID().CompareNoCase( _T("RELAY_24V") ) 
				&& 0 != pElectroActuator->GetRelayID().CompareNoCase( _T("RELAY_220V") ) )
		{
			// If we don't want a fast electrical connection and the actuator is a CO...
			continue;
		}

		// HYS-1252 : Verify the value of input signal combo box
		if( false == m_pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_strActuatorInputSignalID.IsEmpty() )
		{
			CTable *pSignalsTab = static_cast<CTable *>( TASApp.GetpTADB()->Get( _T("SIG_TAB") ).MP );
			bool bActuator_found = false;
			for( int iLoop = 0; iLoop < (int)pElectroActuator->GetNumOfInputSignalsIDptr(); iLoop++ )
			{
				IDPTR InputSignalIDPtr = pElectroActuator->GetInputSignalsIDPtr( iLoop );

				if( NULL == InputSignalIDPtr.MP )
				{
					continue;
				}

				if( 0 == StringCompare( InputSignalIDPtr.ID, m_pclIndSel6WayValveParams->m_clIndSelPIBCVParams.m_strActuatorInputSignalID ) )
				{
					bActuator_found = true;
					break;
				}
			}
			if( false == bActuator_found )
			{
				continue;
			}
		}
		
		// HYS-1877
		if( true == bOnlyForSet )
		{
			if( false == pElectroActuator->IsPartOfaSet() )
			{
				continue;
			}
			
			CTableSet* pTableSet = ((CDB_Product*)pclSelectedPIBCValve)->GetTableSet();
			if( NULL == pTableSet )
			{
				continue;
			}

			if( NULL == pTableSet->FindCompatibleSet6WayValve( pclSelectedPIBCValve->GetIDPtr().ID, pElectroActuator->GetIDPtr().ID ) )
			{
				continue;
			}
		}

		pclActuatorList->Add( pElectroActuator->GetIDPtr().ID, pElectroActuator->GetOrderKey(), (LPARAM)pElectroActuator, false, false );
	}
}

void CRViewSSel6WayValve::_ClickOnBalancingValve( CSheetDescription *pclSheetDescriptionBalancingValve, CDB_RegulatingValve *pclBalancingValve, 
		CCellDescriptionProduct *pclCellDescriptionProduct, long lColumn, long lRow )
{
	if( NULL == m_pclIndSel6WayValveParams )
	{
		ASSERT_RETURN;
	}

	CSSheet *pclSSheet = pclSheetDescriptionBalancingValve->GetSSheetPointer();

	LPARAM lBalancingValveTotalCount;
	pclSheetDescriptionBalancingValve->GetUserVariable( _SDUV_TOTALPRODUCT_COUNT, lBalancingValveTotalCount );

	// Memorize previous focus row.
	long lCurrentFocusedRow = GetCurrentFocusedRow();

	// To be sure to clean all previous data concerning focus.
	KillCurrentFocus();

	// Unselect row where current selection is set (yellow light).
	// Remark: 'KillCurrentFocus' unselect only rows where focus is on (blue light).
	pclSSheet->UnSelectMultipleRows();

	int iSheetDescriptionID = pclSheetDescriptionBalancingValve->GetSheetDescriptionID();
	SideDefinition eSideDefinition = GETSHEETSIDE( iSheetDescriptionID );
	int iSheetType = GETSHEETTYPE( iSheetDescriptionID );
	int iSheetFamily = GETSHEETFAMILY( iSheetDescriptionID );

	// By default clear accessories list.
	m_mapVariables[SD_FamilyBalancingValve][eSideDefinition].m_vecValveAccessoryList.clear();
	m_mapVariables[SD_FamilyBalancingValve][eSideDefinition].m_pCDBExpandCollapseGroupValveAccessory = NULL;

	CCDButtonExpandCollapseRows *&pCDBExpandCollapseRowsValve = m_mapVariables[SD_FamilyBalancingValve][eSideDefinition].m_pCDBExpandCollapseRowsValve;
	CCDButtonShowAllPriorities *&pCDBShowAllPrioritiesValve = m_mapVariables[SD_FamilyBalancingValve][eSideDefinition].m_pCDBShowAllPrioritiesValve;
	long &lValveSelectedRow = m_mapVariables[SD_FamilyBalancingValve][eSideDefinition].m_lValveSelectedRow;

	LPARAM lBalancingValveCount;
	pclSheetDescriptionBalancingValve->GetUserVariable( _SDUV_TOTALPRODUCT_COUNT, lBalancingValveCount );

	// Retrieve the current selected balancing valve if exist.
	CDB_TAProduct *pclCurrentBalancingValveSelected = GetCurrentBalancingValveSelected( eSideDefinition );

	// If there is already one balancing valve selected and user click on the current one...
	// Remark: 'pCDBExpandCollapseRowsValve' is not created if there is only one balancing valve. Thus we need to check first if there is only one balancing valve.
	//         Otherwise we have to check if user has clicked on the current one.
	if( NULL != pclCurrentBalancingValveSelected 
			&& ( ( 1 == lBalancingValveCount ) || ( NULL != pCDBExpandCollapseRowsValve && lRow == pCDBExpandCollapseRowsValve->GetCellPosition().y ) ) )
	{
		// Reset current product selected.
		SetCurrentBalancingValveSelected( NULL, eSideDefinition );

		// Uncheck checkbox.
		pclSSheet->SetCheckBox( CD_BV_CheckBox, lRow, _T(""), false, true );

		// Delete Expand/Collapse rows button if exist.
		// Remark: 'm_pCDBExpandCollapseRowsBalancingValve' is set to NULL in method!
		DeleteExpandCollapseRowsButton( pCDBExpandCollapseRowsValve, pclSheetDescriptionBalancingValve );

		// Show Show/Hide all priorities button if exist.
		if( NULL != pCDBShowAllPrioritiesValve )
		{
			pCDBShowAllPrioritiesValve->ApplyInternalChange();
		}

		// Set focus on the balancing valve currently selected.
		PrepareAndSetNewFocus( pclSheetDescriptionBalancingValve, pclSheetDescriptionBalancingValve->GetActiveColumn(), lRow, 0 );

		// Remove all sheets after balancing valve.
		_RemoveAllSheetAfter( eSideDefinition, iSheetFamily, iSheetType );

		lValveSelectedRow = 0;

		// Notify a change in the selection.
		SelectionHasChanged( IsSelectionReady() );
	}
	else
	{
		// If a balancing valve is already selected...
		if( NULL != pclCurrentBalancingValveSelected )
		{
			// Uncheck checkbox.
			pclSSheet->SetCheckBox( CD_BV_CheckBox, lValveSelectedRow, _T(""), false, true );

			// Remove all sheets after the balancing valve.
			_RemoveAllSheetAfter( eSideDefinition, iSheetFamily, iSheetType );
		}

		// Save the new balancing valve selection.
		SetCurrentBalancingValveSelected( pclCellDescriptionProduct, eSideDefinition );

		// Check checkbox.
		pclSSheet->SetCheckBox( CD_BV_CheckBox, lRow, _T(""), true, true );

		// Delete Expand/Collapse rows button if exist.
		if( NULL != pCDBExpandCollapseRowsValve )
		{
			// Remark: 'm_pCDBExpandCollapseRowsBalancingValve' is set to NULL in method!
			DeleteExpandCollapseRowsButton( pCDBExpandCollapseRowsValve, pclSheetDescriptionBalancingValve );
		}

		// Create Expand/Collapse rows button if needed...
		if( lBalancingValveTotalCount > 1 )
		{
			pCDBExpandCollapseRowsValve = CreateExpandCollapseRowsButton( CD_BV_FirstColumn, lRow, true, 
					CCDButtonExpandCollapseRows::ButtonState::CollapseRow, pclSheetDescriptionBalancingValve->GetFirstSelectableRow(), 
					pclSheetDescriptionBalancingValve->GetLastSelectableRow( false ), pclSheetDescriptionBalancingValve );

			// Show button.
			if( NULL != pCDBExpandCollapseRowsValve )
			{
				pCDBExpandCollapseRowsValve->SetShowStatus( true );
			}
		}

		// Select the balancing valve (just highlight background).
		pclSSheet->SelectOneRow( lRow, pclSheetDescriptionBalancingValve->GetSelectionFrom(), pclSheetDescriptionBalancingValve->GetSelectionTo() );

		// Fill accessories available for the current balancing valve.
		_FillBalancingValveAccessories( eSideDefinition );

		// Try to set the focus on the next table but do not lose the focus if no other table exist.
		CSheetDescription *pclNextSheetDescription = NULL;
		long lNewFocusedRow;
		bool fShiftPressed;

		if( DNFERRC_FocusOnNextGroupOK == DefineNextFocusedRow( pclSheetDescriptionBalancingValve, CD_BV_FirstColumn, lRow, false, lNewFocusedRow,
				pclNextSheetDescription, fShiftPressed ) )
		{
			long lNewFocusedCol = pclNextSheetDescription->GetFirstParameterColumn();
			PrepareAndSetNewFocus( pclNextSheetDescription, lNewFocusedCol, lNewFocusedRow );
		}
		else
		{
			// Because we have changed current selection background in yellow, if there is no other sheet following current one, we
			// must set focus on current selection.
			PrepareAndSetNewFocus( pclSheetDescriptionBalancingValve, lColumn, lRow );
		}

		lValveSelectedRow = lRow;

		// Notify a change in the selection.
		SelectionHasChanged( IsSelectionReady() );
	}
}

void CRViewSSel6WayValve::_FillBalancingValveRows( CSelectedValve *pSelected6WayValve, SideDefinition eSideDefinition )
{
	if( NULL == m_pclIndSel6WayValveParams || NULL == m_pclIndSel6WayValveParams->m_pTADB || NULL == m_pclIndSel6WayValveParams->m_pclSelect6WayValveList )
	{
		ASSERT_RETURN;
	}

	if( NULL == pSelected6WayValve )
	{
		return;
	}

	CSelect6WayValveList *pcl6WayValveList = m_pclIndSel6WayValveParams->m_pclSelect6WayValveList;

	// Retrieve the balancing valve list linked to current the 6-way valve.
	CSelectList *pclSelectedBalancingValveList = NULL;
	bool bSizeShiftProblem = false;

	if( pcl6WayValveList->SelectBalancingValve( m_pclIndSel6WayValveParams, pSelected6WayValve, eSideDefinition, &bSizeShiftProblem  ) > 0 )
	{
		pclSelectedBalancingValveList = pcl6WayValveList->GetBVList( eSideDefinition );
	}

	// If no balancing valve exist...
	if( NULL == pclSelectedBalancingValveList || 0 == pclSelectedBalancingValveList->GetCount() )
	{
		return;
	}

	// Call 'MultiSpreadBase' method to get a new SSheet.
	int iSheetDescriptionID = ( eSideDefinition << SHEETSIDEDEFSHIFT ) + SD_FamilyBalancingValve + SD_TypeValve;
	CSheetDescription *pclSheetDescriptionBalancingValve = CreateSSheet( iSheetDescriptionID );

	if( NULL == pclSheetDescriptionBalancingValve || NULL == pclSheetDescriptionBalancingValve->GetSSheetPointer() )
	{
		return;
	}

	CSSheet *pclSSheet = pclSheetDescriptionBalancingValve->GetSSheetPointer();

	pclSSheet->SetBool( SSB_REDRAW, FALSE );

	pclSSheet->Init();
	pclSSheet->SetBool( SSB_ALLOWDRAGDROP, FALSE );
	pclSSheet->SetBool( SSB_HORZSCROLLBAR, FALSE );
	pclSSheet->SetBool( SSB_VERTSCROLLBAR, FALSE );
	pclSSheet->SetBool( SSB_MOVEACTIVEONFOCUS, FALSE );
	pclSSheet->SetTextTip( SS_TT_STATUS_FLOATING, 250, NULL, _LIGHTYELLOW, _TAH_BLACK );

	// Initialize the balancing valve sheet and fill header.
	_InitAndFillBalancingValveHeader( pclSheetDescriptionBalancingValve, pclSSheet, eSideDefinition );

	long lRow = RD_BalancingValve_FirstAvailRow;

	long lBalancingValveNumbers = 0;
	long lBalancingValveNotPriorityNumbers = 0;
	bool bShowAllPrioritiesShown = false;
	long lShowAllPrioritiesButtonRow = 0;

	pclSheetDescriptionBalancingValve->SetUserVariable( _SDUV_TOTALPRODUCT_COUNT, 0 );
	pclSheetDescriptionBalancingValve->SetUserVariable( _SDUV_NOPRIORITYPRODUCT_COUNT, 0 );
	pclSheetDescriptionBalancingValve->RestartRemarkGenerator();

	for( CSelectedValve *pclSelectedBalancingValve = pclSelectedBalancingValveList->GetFirst<CSelectedValve>(); NULL != pclSelectedBalancingValve;
		 pclSelectedBalancingValve = pclSelectedBalancingValveList->GetNext<CSelectedValve>() )
	{
		CDB_TAProduct *pTAP = dynamic_cast<CDB_TAProduct *>( pclSelectedBalancingValve->GetpData() );

		if( NULL == pTAP )
		{
			continue;
		}

		if( true == pclSelectedBalancingValve->IsFlagSet( CSelectedBase::eNotPriority ) )
		{
			lBalancingValveNotPriorityNumbers++;

			if( false == bShowAllPrioritiesShown )
			{
				// Add one empty not selectable row.
				pclSheetDescriptionBalancingValve->AddRows( 1 );
				pclSheetDescriptionBalancingValve->RemoveSelectableRow( lRow );

				pclSSheet->SetCellBorder( CD_BV_CheckBox, lRow, CD_BV_Separator - 1, lRow, true, 
						SS_BORDERTYPE_TOP | SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );

				pclSSheet->SetCellBorder( CD_BV_PipeSize, lRow, CD_BV_Pointer - 1, lRow, true, 
						SS_BORDERTYPE_TOP | SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );

				lShowAllPrioritiesButtonRow = lRow++;
				bShowAllPrioritiesShown = true;
			}
		}

		// If no error...
		if( 0 == _FillOneBalancingValveRow( pclSheetDescriptionBalancingValve, pclSSheet, pclSelectedBalancingValve, lRow ) )
		{
			lRow++;
			lBalancingValveNumbers++;
		}
	}

	long lLastDataRow = lRow - 1;

	pclSheetDescriptionBalancingValve->SetUserVariable( _SDUV_TOTALPRODUCT_COUNT, lBalancingValveNumbers );
	pclSheetDescriptionBalancingValve->SetUserVariable( _SDUV_NOPRIORITYPRODUCT_COUNT, lBalancingValveNotPriorityNumbers );

	// Now we can create Show/Hide all priorities if needed.
	// Remarks: 2nd condition to avoid to have button if we have only no priority valves.
	if( true == bShowAllPrioritiesShown && lBalancingValveNumbers > lBalancingValveNotPriorityNumbers )
	{
		// Create Show/Hide all priorities button in regards to current state!
		CCDButtonShowAllPriorities::ButtonState eState = ( true == GetShowAllPrioritiesFlag( iSheetDescriptionID ) ) ? CCDButtonShowAllPriorities::ButtonState::HidePriorities :
				CCDButtonShowAllPriorities::ButtonState::ShowPriorities;
		
		CCDButtonShowAllPriorities *pclShowAllButton = CreateShowAllPrioritiesButton( CD_BV_FirstColumn, lShowAllPrioritiesButtonRow, 
				bShowAllPrioritiesShown, eState, lShowAllPrioritiesButtonRow + 1, pclSSheet->GetMaxRows(), pclSheetDescriptionBalancingValve );

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
	if( lBalancingValveNumbers > 2 || ( 2 == lBalancingValveNumbers && lBalancingValveNotPriorityNumbers != 1 ) )
	{
		vector<long> vecColumnList;
		vecColumnList.push_back( CD_BV_PipeSize );
		vecColumnList.push_back( CD_BV_PipeLinDp );
		vecColumnList.push_back( CD_BV_PipeV );
		pclSSheet->MergeRowsWithSameValues( RD_BalancingValve_FirstAvailRow, lLastDataRow, vecColumnList );
	}

	pclSSheet->SetCellBorder( CD_BV_CheckBox, lLastDataRow, CD_BV_Separator - 1, lLastDataRow, true, 
			SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );

	// Needed by pipe merging
	pclSSheet->SetCellBorder( CD_BV_PipeSize, lLastDataRow, CD_BV_Pointer - 1, lLastDataRow, true, 
			SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );

	// Add Remarks.
	// Note: these lines must be placed after creating Show/Hide all priorities button to avoid to take in count these lines when defining
	//       range of row to be Show/Hide.
	lRow = pclSheetDescriptionBalancingValve->WriteRemarks( lRow, CD_BV_CheckBox, CD_BV_Separator );

	pclSSheet->SetBool( SSB_REDRAW, TRUE );

	// Set that there is no selection at now.
	SetCurrentBalancingValveSelected( NULL, eSideDefinition );

	// Add possibility to change column size.
	// Remark: place this code after any initialization (like adding, removing, resizing, hiding/showing columns).
	CResizingColumnInfo *pclResizingColumnInfo = pclSheetDescriptionBalancingValve->GetResizingColumnInfo();
	pclResizingColumnInfo->ActivateFeature( true, false, RD_BalancingValve_FirstAvailRow, lLastDataRow );
	pclResizingColumnInfo->AddRangeColumn( CD_BV_Name, CD_BV_TemperatureRange, RD_BalancingValve_ColName, RD_BalancingValve_Unit );
	pclResizingColumnInfo->AddRangeColumn( CD_BV_PipeSize, CD_BV_PipeV, RD_BalancingValve_ColName, RD_BalancingValve_Unit );

	// Add possibility to click on title group to reset column width.
	CreateTitleGroupButtons( CD_BV_CheckBox, CD_BV_PipeV, RD_BalancingValve_GroupName, pclSheetDescriptionBalancingValve );
}

void CRViewSSel6WayValve::_InitAndFillBalancingValveHeader( CSheetDescription *pclSheetDescriptionBalancingValve, CSSheet *pclSSheet, SideDefinition eSideDefinition )
{
	if( NULL == m_pclIndSel6WayValveParams || NULL == m_pclIndSel6WayValveParams->m_pPipeDB || NULL == m_pclIndSel6WayValveParams->m_pclSelect6WayValveList 
			|| NULL == m_pclIndSel6WayValveParams->m_pclSelect6WayValveList->GetBVList( eSideDefinition ) )
	{
		ASSERT_RETURN;
	}

	// Flags for column customizing
	CSelectedBase *pclSelectedProduct = m_pclIndSel6WayValveParams->m_pclSelect6WayValveList->GetBVList( eSideDefinition )->GetFirst<CSelectedValve>();

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
	int iSheetDescriptionID = pclSheetDescriptionBalancingValve->GetSheetDescriptionID();
	pclSheetDescriptionBalancingValve->Init( 1, pclSSheet->GetMaxRows(), 1, CSheetDescription::SD_ParameterMode_Mono );
	pclSheetDescriptionBalancingValve->AddColumnInPixels( CD_BV_FirstColumn, m_mapSSheetColumnWidth[iSheetDescriptionID][CD_BV_FirstColumn] );
	pclSheetDescriptionBalancingValve->AddColumnInPixels( CD_BV_CheckBox, m_mapSSheetColumnWidth[iSheetDescriptionID][CD_BV_CheckBox] );
	pclSheetDescriptionBalancingValve->AddColumnInPixels( CD_BV_Name, m_mapSSheetColumnWidth[iSheetDescriptionID][CD_BV_Name] );
	pclSheetDescriptionBalancingValve->AddColumnInPixels( CD_BV_Material, m_mapSSheetColumnWidth[iSheetDescriptionID][CD_BV_Material] );
	pclSheetDescriptionBalancingValve->AddColumnInPixels( CD_BV_Connection, m_mapSSheetColumnWidth[iSheetDescriptionID][CD_BV_Connection] );
	pclSheetDescriptionBalancingValve->AddColumnInPixels( CD_BV_Version, m_mapSSheetColumnWidth[iSheetDescriptionID][CD_BV_Version] );
	pclSheetDescriptionBalancingValve->AddColumnInPixels( CD_BV_PN, m_mapSSheetColumnWidth[iSheetDescriptionID][CD_BV_PN] );
	pclSheetDescriptionBalancingValve->AddColumnInPixels( CD_BV_Size, m_mapSSheetColumnWidth[iSheetDescriptionID][CD_BV_Size] );
	pclSheetDescriptionBalancingValve->AddColumnInPixels( CD_BV_Preset, m_mapSSheetColumnWidth[iSheetDescriptionID][CD_BV_Preset] );
	pclSheetDescriptionBalancingValve->AddColumnInPixels( CD_BV_DpSignal, m_mapSSheetColumnWidth[iSheetDescriptionID][CD_BV_DpSignal] );
	pclSheetDescriptionBalancingValve->AddColumnInPixels( CD_BV_Dp, m_mapSSheetColumnWidth[iSheetDescriptionID][CD_BV_Dp] );
	pclSheetDescriptionBalancingValve->AddColumnInPixels( CD_BV_DpFullOpening, m_mapSSheetColumnWidth[iSheetDescriptionID][CD_BV_DpFullOpening] );
	pclSheetDescriptionBalancingValve->AddColumnInPixels( CD_BV_DpHalfOpening, m_mapSSheetColumnWidth[iSheetDescriptionID][CD_BV_DpHalfOpening] );
	pclSheetDescriptionBalancingValve->AddColumnInPixels( CD_BV_TemperatureRange, m_mapSSheetColumnWidth[iSheetDescriptionID][CD_BV_TemperatureRange] );
	pclSheetDescriptionBalancingValve->AddColumnInPixels( CD_BV_Separator, m_mapSSheetColumnWidth[iSheetDescriptionID][CD_BV_Separator] );
	pclSheetDescriptionBalancingValve->AddColumnInPixels( CD_BV_PipeSize, m_mapSSheetColumnWidth[iSheetDescriptionID][CD_BV_PipeSize] );
	pclSheetDescriptionBalancingValve->AddColumnInPixels( CD_BV_PipeLinDp, m_mapSSheetColumnWidth[iSheetDescriptionID][CD_BV_PipeLinDp] );
	pclSheetDescriptionBalancingValve->AddColumnInPixels( CD_BV_PipeV, m_mapSSheetColumnWidth[iSheetDescriptionID][CD_BV_PipeV] );
	pclSheetDescriptionBalancingValve->AddColumnInPixels( CD_BV_Pointer, m_mapSSheetColumnWidth[iSheetDescriptionID][CD_BV_Pointer] );

	// Set in which column parameter must be saved.
	pclSheetDescriptionBalancingValve->AddParameterColumn( CD_BV_Pointer );

	// Set the focus column.
	pclSheetDescriptionBalancingValve->SetActiveColumn( CD_BV_Name );

	// Set range for selection.
	pclSheetDescriptionBalancingValve->SetFocusColumnRange( CD_BV_CheckBox, CD_BV_TemperatureRange );

	// Hide columns for DpSignal if not needed.
	pclSSheet->ShowCol( CD_BV_DpSignal, ( true == bKvSignalExist ) ? TRUE : FALSE );

	// Column Full opening is show if the user doesn't input specify pressure drop.
	pclSSheet->ShowCol( CD_BV_DpFullOpening, ( false == m_pclIndSel6WayValveParams->m_bDpEnabled ) ? TRUE : FALSE );
	pclSSheet->ShowCol( CD_BV_DpHalfOpening, ( false == m_pclIndSel6WayValveParams->m_bDpEnabled ) ? TRUE : FALSE );

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Row name.

	// Add the main title.
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );

	int iSheetType = GETSHEETTYPE( pclSheetDescriptionBalancingValve->GetSheetDescriptionID() );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)_GetGroupColor( false, eSideDefinition, iSheetType ) );
	
	pclSheetDescriptionBalancingValve->SetUserVariable( _SDUV_TITLEFORECOLOR, ( pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor ) ) );
	pclSheetDescriptionBalancingValve->SetUserVariable( _SDUV_TITLEBACKCOLOR, ( pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor ) ) );

	CString strTitle = TASApp.LoadLocalizedString( IDS_SSHEETSSELDPC_BVGROUP );
	strTitle += _T(" - ") + TASApp.LoadLocalizedString( ( HeatingSide == eSideDefinition ) ? IDS_SSHEETSSEL6WAYVALVE_HEATINGSIDE : IDS_SSHEETSSEL6WAYVALVE_COOLINGSIDE );

	pclSSheet->AddCellSpanW( CD_BV_CheckBox, RD_BalancingValve_GroupName, CD_BV_Separator - CD_BV_CheckBox, 1 );
	pclSSheet->SetStaticText( CD_BV_CheckBox, RD_BalancingValve_GroupName, strTitle );
	pclSSheet->AddCellSpanW( CD_BV_PipeSize, RD_BalancingValve_GroupName, CD_BV_Pointer - CD_BV_PipeSize, 1 );

	CTable *pclTable = dynamic_cast<CTable *>( m_pclIndSel6WayValveParams->m_pPipeDB->Get( m_pclIndSel6WayValveParams->m_strPipeSeriesID ).MP );
	ASSERT( NULL != pclTable );

	pclSSheet->SetStaticText( CD_BV_PipeSize, RD_BalancingValve_GroupName, pclTable->GetName() );

	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

	pclSSheet->SetStaticText( CD_BV_Name, RD_BalancingValve_ColName, IDS_SSHEETSSEL_NAME );
	pclSSheet->SetStaticText( CD_BV_Material, RD_BalancingValve_ColName, IDS_SSHEETSSEL_MATERIAL );
	pclSSheet->SetStaticText( CD_BV_Connection, RD_BalancingValve_ColName, IDS_SSHEETSSEL_CONNECT );
	pclSSheet->SetStaticText( CD_BV_Version, RD_BalancingValve_ColName, IDS_SSHEETSSEL_VERSION );
	pclSSheet->SetStaticText( CD_BV_PN, RD_BalancingValve_ColName, IDS_SSHEETSSEL_PN );
	pclSSheet->SetStaticText( CD_BV_Size, RD_BalancingValve_ColName, IDS_SSHEETSSEL_SIZE );
	pclSSheet->SetStaticText( CD_BV_Preset, RD_BalancingValve_ColName, IDS_SSHEETSSEL_PRESET );
	pclSSheet->SetStaticText( CD_BV_DpSignal, RD_BalancingValve_ColName, IDS_SHEETHDR_SIGNAL );
	pclSSheet->SetStaticText( CD_BV_Dp, RD_BalancingValve_ColName, IDS_SSHEETSSEL_DP );
	pclSSheet->SetStaticText( CD_BV_DpFullOpening, RD_BalancingValve_ColName, IDS_SSHEETSSEL_DPFO );
	pclSSheet->SetStaticText( CD_BV_DpHalfOpening, RD_BalancingValve_ColName, IDS_SSHEETSSEL_DPHO );
	pclSSheet->SetStaticText( CD_BV_TemperatureRange, RD_BalancingValve_ColName, IDS_SSHEETSSEL_COLTEMPRANGE );

	pclSSheet->SetStaticText( CD_BV_Separator, RD_BalancingValve_ColName, _T( "" ) );

	// Pipes
	pclSSheet->SetStaticText( CD_BV_PipeSize, RD_BalancingValve_ColName, IDS_SSHEETSSEL_PIPESIZE );
	pclSSheet->SetStaticText( CD_BV_PipeLinDp, RD_BalancingValve_ColName, IDS_SSHEETSSEL_PIPELINDP );
	pclSSheet->SetStaticText( CD_BV_PipeV, RD_BalancingValve_ColName, IDS_SSHEETSSEL_PIPEV );
	//
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Row units
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();
	pclSSheet->SetStaticText( CD_BV_Dp, RD_BalancingValve_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ) ).c_str() );
	pclSSheet->SetStaticText( CD_BV_DpSignal, RD_BalancingValve_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ) ).c_str() );
	pclSSheet->SetStaticText( CD_BV_DpFullOpening, RD_BalancingValve_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ) ).c_str() );
	pclSSheet->SetStaticText( CD_BV_DpHalfOpening, RD_BalancingValve_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ) ).c_str() );
	pclSSheet->SetStaticText( CD_BV_TemperatureRange, RD_BalancingValve_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_TEMPERATURE ) ).c_str() );
	pclSSheet->SetStaticText( CD_BV_Preset, RD_BalancingValve_Unit, IDS_SHEETHDR_TURNSPOS );

	// Units
	pclSSheet->SetStaticText( CD_BV_PipeLinDp, RD_BalancingValve_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_LINPRESSDROP ) ).c_str() );
	pclSSheet->SetStaticText( CD_BV_PipeV, RD_BalancingValve_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_VELOCITY ) ).c_str() );
	//
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Draw black line below column header.
	pclSSheet->SetCellBorder( CD_BV_CheckBox, RD_BalancingValve_Unit, CD_BV_Separator - 1, RD_BalancingValve_Unit, true, 
			SS_BORDERTYPE_BOTTOM );

	pclSSheet->SetCellBorder( CD_BV_PipeSize, RD_BalancingValve_Unit, CD_BV_Pointer - 1, RD_BalancingValve_Unit, true,
			SS_BORDERTYPE_BOTTOM );
}

long CRViewSSel6WayValve::_FillOneBalancingValveRow( CSheetDescription *pclSheetDescriptionBalancingValve, CSSheet *pclSSheet, CSelectedValve *pSelectedBalancingValve, long lRow )
{
	if( NULL == m_pclIndSel6WayValveParams || NULL == m_pclIndSel6WayValveParams->m_pclSelect6WayValveList || NULL == m_pclIndSel6WayValveParams->m_pTADS )
	{
		ASSERTA_RETURN( -1 );
	}

	SideDefinition eSideDefinition = GETSHEETSIDE( pclSheetDescriptionBalancingValve->GetSheetDescriptionID() );
	
	if( NULL == m_pclIndSel6WayValveParams->m_pclSelect6WayValveList->GetBVList( eSideDefinition )
			|| NULL == m_pclIndSel6WayValveParams->m_pclSelect6WayValveList->GetBVList( eSideDefinition )->GetSelectPipeList() )
	{
		ASSERTA_RETURN( -1 );
	}

	CDS_TechnicalParameter *pTechParam = m_pclIndSel6WayValveParams->m_pTADS->GetpTechParams();

	CDB_TAProduct *pTAP = dynamic_cast<CDB_TAProduct *>( pSelectedBalancingValve->GetpData() );

	if( NULL == pTAP )
	{
		ASSERTA_RETURN( -1 );
	}

	bool bKvSignalExist = ( NULL != pTAP->IsKvSignalEquipped() );
	bool bAtLeastOneError = false;
	bool bAtLeastOneWarning = false;
	bool bBest = pSelectedBalancingValve->IsFlagSet( CSelectedBase::eBest );

	// Add this row.
	// 'true' to specify that this row can be selected.
	pclSheetDescriptionBalancingValve->AddRows( 1, true );

	// First columns will be set at the end!
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

	// Add checkbox.
	pclSSheet->SetCheckBox( CD_BV_CheckBox, lRow, _T(""), false, true );

	// Set the hand wheel presetting.
	CString str = _T("-");

	if( NULL != pTAP && NULL != pTAP->GetValveCharacteristic() )
	{
		str = pTAP->GetValveCharacteristic()->GetSettingString( pSelectedBalancingValve->GetH() );
	}

	if( true == pSelectedBalancingValve->IsFlagSet( CSelectedBase::eValveMaxSetting ) )
	{
		pclSheetDescriptionBalancingValve->WriteTextWithFlags( str, CD_BV_Preset, lRow, CSheetDescription::RemarkFlags::FullOpening );
	}
	else
	{
		if( true == pSelectedBalancingValve->IsFlagSet( CSelectedBase::eValveSetting ) )
		{
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_ORANGE );
			bAtLeastOneWarning = true;
		}

		pclSSheet->SetStaticText( CD_BV_Preset, lRow, str );
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );
	}

	if( true == pSelectedBalancingValve->IsFlagSet( CSelectedBase::eDp ) )
	{
		// Set in red only error if Dp is below or above min or max technical parameter.
		if( true == m_pclIndSel6WayValveParams->m_bDpEnabled && pSelectedBalancingValve->GetDp() > 0.0 )
		{
			if( pSelectedBalancingValve->GetDp() > pTechParam->VerifyValvMaxDp( pTAP ) )
			{
				pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_RED );
				bAtLeastOneError = true;
			}
			else if( pSelectedBalancingValve->GetDp() < pTechParam->GetValvMinDp( pTAP->GetTypeIDPtr().ID ) )
			{
				pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_ORANGE );
				bAtLeastOneWarning = true;
			}
		}
	}

	pclSSheet->SetStaticText( CD_BV_Dp, lRow, WriteCUDouble( _U_DIFFPRESS, pSelectedBalancingValve->GetDp() ) );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

	// Set signal.
	if( true == bKvSignalExist )
	{
		pclSSheet->SetStaticText( CD_BV_DpSignal, lRow, WriteCUDouble( _U_DIFFPRESS, pSelectedBalancingValve->GetDpSignal() ) );
	}

	// If Dp unknown.
	if( false == m_pclIndSel6WayValveParams->m_bDpEnabled || -1 == pSelectedBalancingValve->GetDp() )
	{
		if( true == pSelectedBalancingValve->IsFlagSet( CSelectedBase::eValveFullODp ) )
		{
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_ORANGE );
			bAtLeastOneWarning = true;
		}

		pclSSheet->SetStaticText( CD_BV_DpFullOpening, lRow, WriteCUDouble( _U_DIFFPRESS, pSelectedBalancingValve->GetDpFullOpen() ) );
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

		if( true == pSelectedBalancingValve->IsFlagSet( CSelectedBase::eValveHalfODp ) )
		{
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM) _ORANGE );
			bAtLeastOneWarning = true;
		}

		pclSSheet->SetStaticText( CD_BV_DpHalfOpening, lRow, WriteCUDouble( _U_DIFFPRESS, pSelectedBalancingValve->GetDpHalfOpen() ) );
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );
	}

	// Temperature range.
	if( true == pSelectedBalancingValve->IsFlagSet( CSelectedBase::eTemperature ) )
	{
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_RED );
		bAtLeastOneError = true;
	}

	pclSSheet->SetStaticText( CD_BV_TemperatureRange, lRow, ( (CDB_TAProduct *)pSelectedBalancingValve->GetProductIDPtr().MP )->GetTempRange() );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

	// Now we can set first columns in regards to current status (error, best or normal).
	if( true == bAtLeastOneError )
	{
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_RED );
	}
	else if( true == pSelectedBalancingValve->IsFlagSet( CSelectedBase::eBest ) )
	{
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_GREEN );
	}

	// In addition of current flag, we have possibility that valve has the flag 'Not available' or 'Deleted'. In that case, we show valve name in red with "*" around it and
	// symbol '!' or '!!' after.
	if( true == pTAP->IsDeleted() )
	{
		pclSheetDescriptionBalancingValve->WriteTextWithFlags( CString( pTAP->GetName() ), CD_BV_Name, lRow, CSheetDescription::RemarkFlags::Deleted, _T(""), _RED );
	}
	else if( false == pTAP->IsAvailable() )
	{
		pclSheetDescriptionBalancingValve->WriteTextWithFlags( CString( pTAP->GetName() ), CD_BV_Name, lRow, CSheetDescription::RemarkFlags::NotAvailable, _T(""), _RED );
	}
	else
	{
		pclSSheet->SetStaticText( CD_BV_Name, lRow, pTAP->GetName() );
	}

	if( true == bAtLeastOneError )
	{
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );
	}
	else if( true == pSelectedBalancingValve->IsFlagSet( CSelectedBase::eBest ) )
	{
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_GREEN );
	}

	pclSSheet->SetStaticText( CD_BV_Material, lRow, pTAP->GetBodyMaterial() );
	pclSSheet->SetStaticText( CD_BV_Connection, lRow, pTAP->GetConnect() );
	pclSSheet->SetStaticText( CD_BV_Version, lRow, pTAP->GetVersion() );
	pclSSheet->SetStaticText( CD_BV_PN, lRow, pTAP->GetPN().c_str() );
	pclSSheet->SetStaticText( CD_BV_Size, lRow, pTAP->GetSize() );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

	// Set pipe size.
	CSelectPipe selPipe( m_pclIndSel6WayValveParams );

	// Pay attention: we need to take here the appropriate pipe list (Not the one that is in the 'm_pclIndSel6WayValveParams->m_pclSelect6WayValveList'.
	m_pclIndSel6WayValveParams->m_pclSelect6WayValveList->GetBVList( eSideDefinition )->GetSelectPipeList()->GetMatchingPipe( pTAP->GetSizeKey(), selPipe );
	pclSSheet->SetStaticText( CD_BV_PipeSize, lRow, selPipe.GetpPipe()->GetName() );

	// Set the LinDp to orange if it is above or below the technical parameters limits.
	if( selPipe.GetLinDp() > pTechParam->GetPipeMaxDp() || selPipe.GetLinDp() < pTechParam->GetPipeMinDp() )
	{
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_ORANGE );
	}

	pclSSheet->SetStaticText( CD_BV_PipeLinDp, lRow, WriteCUDouble( _U_LINPRESSDROP, selPipe.GetLinDp() ) );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

	// Set the velocity to orange if it is above the technical parameter limit.
	// Orange if it is below the dMinVel.
	if( selPipe.GetU() > pTechParam->GetPipeMaxVel() || selPipe.GetU() < pTechParam->GetPipeMinVel() )
	{
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_ORANGE );
	}

	pclSSheet->SetStaticText( CD_BV_PipeV, lRow, WriteCUDouble( _U_VELOCITY, selPipe.GetU() ) );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

	// Save parameter.
	CreateCellDescriptionProduct( pclSheetDescriptionBalancingValve->GetFirstParameterColumn(), lRow, (LPARAM)pTAP, pclSheetDescriptionBalancingValve );

	pclSSheet->SetCellBorder( CD_BV_CheckBox, lRow, CD_BV_Separator - 1, lRow, true, SS_BORDERTYPE_BOTTOM, 
			SS_BORDERSTYLE_SOLID, _GRAY );

	pclSSheet->SetCellBorder( CD_BV_PipeSize, lRow, CD_BV_Pointer - 1, lRow, true, SS_BORDERTYPE_BOTTOM, 
			SS_BORDERSTYLE_SOLID, _GRAY );

	return 0;
}

void CRViewSSel6WayValve::_FillBalancingValveAccessories( SideDefinition eSideDefinition )
{
	if( NULL == m_pclIndSel6WayValveParams || NULL == m_pclIndSel6WayValveParams->m_pTADB )
	{
		ASSERT_RETURN;
	}

	// Check the current balancing valve selected.
	CDB_RegulatingValve *pclCurrentBalancingValveSelected = dynamic_cast<CDB_RegulatingValve *>( GetCurrentBalancingValveSelected( eSideDefinition ) );

	if( NULL == pclCurrentBalancingValveSelected )
	{
		return;
	}

	CDB_RuledTable *pclRuledTable = (CDB_RuledTable *)( pclCurrentBalancingValveSelected->GetAccessoriesGroupIDPtr().MP );

	if( NULL == pclRuledTable )
	{
		return;
	}

	CRank rList;
	m_pclIndSel6WayValveParams->m_pTADB->GetAccessories( &rList, pclRuledTable, m_pclIndSel6WayValveParams->m_eFilterSelection );
	_FillAccessoryRows( eSideDefinition, SD_FamilyBalancingValve, SD_TypeValveAccessories, &rList, pclRuledTable );
}

void CRViewSSel6WayValve::_VerifyOneProductAndClick( SideDefinition eSideDefinition, int iSheetFamily, int iSheetType )
{
	int iSheetDescriptionID = ( eSideDefinition << SHEETSIDEDEFSHIFT ) + iSheetFamily + iSheetType;
	CSheetDescription *pclSheetDescriptionValve = m_ViewDescription.GetFromSheetDescriptionID( iSheetDescriptionID );

	if( NULL == pclSheetDescriptionValve )
	{
		return;
	}

	bool bCheckActuator = false;
	long lTotalCount;
	pclSheetDescriptionValve->GetUserVariable( _SDUV_TOTALPRODUCT_COUNT, lTotalCount );

	// If only one valve, we can selected it.
	if( 1 == lTotalCount && SD_TypeValve == iSheetType )
	{
		CCellDescriptionProduct *pclCDValve = dynamic_cast<CCellDescriptionProduct *>( pclSheetDescriptionValve->GetFirstCellDescription( RVSCellDescription::CD_Product ) );
		ASSERT( NULL != pclCDValve );

		// Note: First available row is always the same for all the valve sheets.
		OnClickProduct( pclSheetDescriptionValve, pclCDValve, pclSheetDescriptionValve->GetActiveColumn(), RD_6WayValve_FirstAvailRow );

		bCheckActuator = true;
	}

	if( SD_TypeValveActuator == iSheetType || true == bCheckActuator )
	{
		// Check now actuator.
		iSheetDescriptionID = ( eSideDefinition << SHEETSIDEDEFSHIFT ) + iSheetFamily + SD_TypeValveActuator;
		CSheetDescription* pclSheetDescriptionActuator = m_ViewDescription.GetFromSheetDescriptionID( iSheetDescriptionID );

		if( NULL == pclSheetDescriptionActuator )
		{
			return;
		}

		pclSheetDescriptionActuator->GetUserVariable( _SDUV_TOTALACTUATOR_COUNT, lTotalCount );
		
		// If the actuator is already checked not do this again
		CDB_6WayValve* pclCurrent6WayValveSelected = GetCurrent6WayValveSelected();
		CDB_Actuator* pclCurrent6WayValveActSelected = GetCurrent6WayValveActuatorSelected();
		if( ( NULL == pclCurrent6WayValveSelected && NULL != pclCurrent6WayValveActSelected )
			|| ( NULL != pclCurrent6WayValveSelected && NULL == pclCurrent6WayValveActSelected ) )
		{
			// If only one 6-way valve actuator, we can selected it.
			if( 1 == lTotalCount )
			{
				CCellDescriptionProduct* pclCDValveActuator = dynamic_cast<CCellDescriptionProduct*>( pclSheetDescriptionActuator->GetFirstCellDescription( RVSCellDescription::CD_Product ) );
				ASSERT( NULL != pclCDValveActuator );

				// Note: First available row is always the same for all the actuator sheets.
				OnClickProduct( pclSheetDescriptionActuator, pclCDValveActuator, pclSheetDescriptionActuator->GetActiveColumn(), RD_Actuator_FirstAvailRow );
			}
		}
	}
}

void CRViewSSel6WayValve::_ClickOnProductClearHelper( int iSheetFamily, SideDefinition eSideDefinition )
{
	if( SD_Family6WayValve == iSheetFamily )
	{
		SETMAPVARIABLE( SD_Family6WayValve, BothSide, m_lValveActuatorSelectedRow, 0 );
		SETMAPVARIABLEALLSIDES( SD_FamilyPIBCValve, m_lValveSelectedRow, 0 );
		SETMAPVARIABLEALLSIDES( SD_FamilyPIBCValve, m_lValveActuatorSelectedRow, 0 );
		SETMAPVARIABLEALLSIDES( SD_FamilyBalancingValve, m_lValveSelectedRow, 0 );
		
		CLEARMAPVARIABLEVECTOR( SD_Family6WayValve, BothSide, m_vecValveAccessoryList );
		CLEARMAPVARIABLEVECTOR( SD_Family6WayValve, BothSide, m_vecValveActuatorAccessoryList );
		CLEARMAPVARIABLEVECTORALLSIDES( SD_FamilyPIBCValve, m_vecValveAccessoryList );
		CLEARMAPVARIABLEVECTORALLSIDES( SD_FamilyPIBCValve, m_vecValveActuatorAccessoryList );
		CLEARMAPVARIABLEVECTORALLSIDES( SD_FamilyBalancingValve, m_vecValveAccessoryList );

		SETMAPVARIABLE( SD_Family6WayValve, BothSide, m_pCDBExpandCollapseGroupValveAccessory, NULL );
		SETMAPVARIABLE( SD_Family6WayValve, BothSide, m_pCDBExpandCollapseGroupValveActuator, NULL );
		SETMAPVARIABLE( SD_Family6WayValve, BothSide, m_pCDBExpandCollapseRowsValveActuator, NULL );
		SETMAPVARIABLE( SD_Family6WayValve, BothSide, m_pCDBExpandCollapseGroupValveActuatorAccessory, NULL );

		SETMAPVARIABLEALLSIDES( SD_FamilyPIBCValve, m_pCDBExpandCollapseRowsValve, NULL );
		SETMAPVARIABLEALLSIDES( SD_FamilyPIBCValve, m_pCDBExpandCollapseGroupValveAccessory, NULL );
		SETMAPVARIABLEALLSIDES( SD_FamilyPIBCValve, m_pCDBExpandCollapseGroupValveActuator, NULL );
		SETMAPVARIABLEALLSIDES( SD_FamilyPIBCValve, m_pCDBExpandCollapseRowsValveActuator, NULL );
		SETMAPVARIABLEALLSIDES( SD_FamilyPIBCValve, m_pCDBExpandCollapseGroupValveActuatorAccessory, NULL );
		SETMAPVARIABLEALLSIDES( SD_FamilyPIBCValve, m_pCDBShowAllPrioritiesValve, NULL );

		SETMAPVARIABLEALLSIDES( SD_FamilyBalancingValve, m_pCDBExpandCollapseRowsValve, NULL );
		SETMAPVARIABLEALLSIDES( SD_FamilyBalancingValve, m_pCDBExpandCollapseGroupValveAccessory, NULL );
		SETMAPVARIABLEALLSIDES( SD_FamilyBalancingValve, m_pCDBShowAllPrioritiesValve, NULL );
	}
	else if( SD_FamilyPIBCValve == iSheetFamily )
	{
		CLEARMAPVARIABLEVECTOR( SD_FamilyPIBCValve, eSideDefinition, m_vecValveAccessoryList );
		CLEARMAPVARIABLEVECTOR( SD_FamilyPIBCValve, eSideDefinition, m_vecValveActuatorAccessoryList );

		SETMAPVARIABLE( SD_FamilyPIBCValve, eSideDefinition, m_pCDBExpandCollapseGroupValveAccessory, NULL );
		SETMAPVARIABLE( SD_FamilyPIBCValve, eSideDefinition, m_pCDBExpandCollapseGroupValveActuator, NULL );
		SETMAPVARIABLE( SD_FamilyPIBCValve, eSideDefinition, m_pCDBExpandCollapseRowsValveActuator, NULL );
		SETMAPVARIABLE( SD_FamilyPIBCValve, eSideDefinition, m_pCDBExpandCollapseGroupValveActuatorAccessory, NULL );
		SETMAPVARIABLE( SD_FamilyPIBCValve, eSideDefinition, m_pCDBShowAllPrioritiesValve, NULL );
	}
	else if( SD_FamilyBalancingValve == iSheetFamily )
	{
		CLEARMAPVARIABLEVECTOR( SD_FamilyBalancingValve, eSideDefinition, m_vecValveAccessoryList );

		SETMAPVARIABLE( SD_FamilyBalancingValve, eSideDefinition, m_pCDBExpandCollapseGroupValveAccessory, NULL );
	}
}

void CRViewSSel6WayValve::_RemoveAllSheetAfter( SideDefinition eSideDefinition, int iSheetFamily, int iSheetType )
{
	for( int iSheetLoop = SD_TypeLast; iSheetLoop > iSheetType; iSheetLoop >>= 1 )
	{
		if( true == m_ViewDescription.IsSheetDescriptionExist( ( eSideDefinition << SHEETSIDEDEFSHIFT ) + iSheetFamily + iSheetLoop ) )
		{
			m_ViewDescription.RemoveOneSheetDescription( ( eSideDefinition << SHEETSIDEDEFSHIFT ) + iSheetFamily + iSheetLoop );
		}
	}
}

COLORREF CRViewSSel6WayValve::_GetGroupColor( bool bIsMain, SideDefinition eSideDefinition, int iSheetType )
{
	COLORREF colorRef = 0;

	if( BothSide == eSideDefinition )
	{
		if( false == m_pclIndSel6WayValveParams->m_bEditModeRunning )
		{
			colorRef = ( true == bIsMain ) ? _IMI_TITLE_GROUP1 : _IMI_TITLE_GROUP3;
		}
		else
		{
			colorRef = _TAH_TITLE_MAIN_REEDIT;
		}
	}
	else if( HeatingSide == eSideDefinition )
	{
		if( false == m_pclIndSel6WayValveParams->m_bEditModeRunning )
		{
			colorRef = _IMI_TITLE_GROUPHEATING;
		}
		else
		{
			colorRef = _TAH_TITLE_MAIN_REEDIT;
		}
	}
	else if( CoolingSide == eSideDefinition )
	{
		if( false == m_pclIndSel6WayValveParams->m_bEditModeRunning )
		{
			colorRef = _IMI_TITLE_GROUPCOOLING;
		}
		else
		{
			colorRef = _TAH_TITLE_SECOND_REEDIT;
		}
	}

	return colorRef;
}

bool CRViewSSel6WayValve::_IsActuatorFitForPIBCV( CDB_ElectroActuator *pclElectroActuator, CDB_ControlValve *pclSelectedControlValve, 
		bool bDowngradeActuatorFunctionality, bool bAcceptAllFailSafe )
{
	if( NULL == m_pclIndSel6WayValveParams || NULL == pclElectroActuator || NULL == pclSelectedControlValve )
	{
		ASSERT( 0 );
		return false;
	}

	if( false == pclElectroActuator->IsSelectable( true ) )
	{
		return false;
	}
	
	CDB_ControlValve::DRPFunction CurrentActuatorDRPFct = pclSelectedControlValve->GetCompatibleDRPFunction( (int)pclElectroActuator->GetDefaultReturnPos() );
	CDB_ControlValve::DRPFunction ComboDRPFctChoice;
	
	if( false == bAcceptAllFailSafe )
	{
		ComboDRPFctChoice = m_pclIndSel6WayValveParams->m_eActuatorDRPFunction;
	}
	else
	{
		ComboDRPFctChoice = CDB_ControlValve::DRPFunction::drpfAll;
	}

	bool bReturn = false;
	CIndSelCtrlParamsBase *plcIndSelParams = (CIndSelCtrlParamsBase *)( &m_pclIndSel6WayValveParams->m_clIndSelPIBCVParams );

	return pclElectroActuator->IsActuatorFit( plcIndSelParams->m_strActuatorPowerSupplyID, plcIndSelParams->m_strActuatorInputSignalID, 
				plcIndSelParams->m_iActuatorFailSafeFunction, ComboDRPFctChoice, CurrentActuatorDRPFct, plcIndSelParams->m_eCvCtrlType, 
				bDowngradeActuatorFunctionality );
}

void CRViewSSel6WayValve::_GetActuatorAccessoryList( CDB_Actuator *pclSeletedActuator, CRank *pclActuatorAccessoryList, CDB_RuledTableBase **ppclRuledTable )
{
	if( NULL == m_pclIndSel6WayValveParams || NULL == m_pclIndSel6WayValveParams->m_pTADB )
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

	m_pclIndSel6WayValveParams->m_pTADB->GetAccessories( pclActuatorAccessoryList, *ppclRuledTable, m_pclIndSel6WayValveParams->m_eFilterSelection );
}

long CRViewSSel6WayValve::_GetRowOfEditedPIBCValve( CSheetDescription *pclSheetDescriptionPIBCValve, CDB_TAProduct *pEditedTAP )
{
	if( NULL == pclSheetDescriptionPIBCValve || NULL == pEditedTAP )
	{
		return -1;
	}

	// Retrieve list of all products in 'pclSheetDescription'.
	CSheetDescription::vecCellDescription vecCellDescriptionList;
	pclSheetDescriptionPIBCValve->GetCellDescriptionList( vecCellDescriptionList, RVSCellDescription::CD_Product );

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

long CRViewSSel6WayValve::_GetRowOfEditedActuator( CSheetDescription *pclSheetDescriptionActuator, CDB_Actuator *pActuator )
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

long CRViewSSel6WayValve::_GetRowOfEditedBValve( CSheetDescription *pclSheetDescriptionBalancingValve, CDB_TAProduct *pEditedTAP )
{
	if( NULL == pclSheetDescriptionBalancingValve || NULL == pEditedTAP )
	{
		return -1;
	}

	// Retrieve list of all products in 'pclSheetDescription'.
	CSheetDescription::vecCellDescription vecCellDescriptionList;
	pclSheetDescriptionBalancingValve->GetCellDescriptionList( vecCellDescriptionList, RVSCellDescription::CD_Product );

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

void CRViewSSel6WayValve::_FillAccessoryRows( SideDefinition eSideDefinition, int iSheetFamily, int iSheetType, CRank *pclList, CDB_RuledTableBase *pclRuledTable )
{
	if( NULL == m_pclIndSel6WayValveParams )
	{
		ASSERT_RETURN;
	}

	int iCount = pclList->GetCount();

	if( 0 == iCount )
	{
		return;
	}

	int iTitleID = 0;
	vecCDCAccessoryList *pvecAccessories = NULL;
	CCDButtonExpandCollapseGroup **ppclGroupButton = NULL;
	
	if( SD_TypeValveAccessories == iSheetType )
	{
		iTitleID = IDS_SSHEETSSEL6WAYVALVE_ACCGROUP;
		pvecAccessories = &m_mapVariables[iSheetFamily][eSideDefinition].m_vecValveAccessoryList;
		ppclGroupButton = &m_mapVariables[iSheetFamily][eSideDefinition].m_pCDBExpandCollapseGroupValveAccessory;
	}
	else if( SD_TypeValveActuatorAccessories == iSheetType )
	{
		iTitleID = IDS_SSHEETSSELCV_ACTRACCGROUP;
		pvecAccessories = &m_mapVariables[iSheetFamily][eSideDefinition].m_vecValveActuatorAccessoryList;
		ppclGroupButton = &m_mapVariables[iSheetFamily][eSideDefinition].m_pCDBExpandCollapseGroupValveActuatorAccessory;
	}
	else
	{
		ASSERT_RETURN;
	}

	CString strTitle = TASApp.LoadLocalizedString( iTitleID );

	if( e6Way_OnOffControlWithPIBCV == m_pclIndSel6WayValveParams->m_e6WayValveSelectionMode && BothSide != eSideDefinition )
	{
		strTitle += _T(" - ") + TASApp.LoadLocalizedString( ( HeatingSide == eSideDefinition ) ? IDS_SSHEETSSEL6WAYVALVE_HEATINGSIDE : IDS_SSHEETSSEL6WAYVALVE_COOLINGSIDE );
	}

	// Call 'MultiSpreadBase' method to get a new SSheet.
	UINT uiAfterSheetDescriptionID = eSideDefinition << SHEETSIDEDEFSHIFT;
	uiAfterSheetDescriptionID += iSheetFamily;
	uiAfterSheetDescriptionID += ( SD_TypeValveAccessories == iSheetType ) ? SD_TypeValve : SD_TypeValveActuator;
	CSheetDescription *pclSheetDescription = CreateSSheet( ( eSideDefinition << SHEETSIDEDEFSHIFT ) + iSheetFamily + iSheetType, uiAfterSheetDescriptionID );

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

	if( NULL == pclPreviousSheetDescription )
	{
		ASSERT_RETURN;
	}

	// Take sheet size and client area size and determine the right width to take in count.
	CRect rectSize = pclPreviousSheetDescription->GetSSheetPointer()->GetSheetSizeInPixels( false );

	// Prepare first column width (to match to the previous sheet).
	long lFirstColumnWidth = 0;
	long lLastColumnWidth = 0;

	switch( iSheetFamily )
	{
		case SD_Family6WayValve:

			if( SD_TypeValveAccessories == iSheetType )
			{
				lFirstColumnWidth = pclPreviousSheetDescription->GetSSheetPointer()->GetColWidthInPixelsW( CD_6WayValve_FirstColumn );
				lFirstColumnWidth += pclPreviousSheetDescription->GetSSheetPointer()->GetColWidthInPixelsW( CD_6WayValve_Box );
				lLastColumnWidth = pclPreviousSheetDescription->GetSSheetPointer()->GetColWidthInPixelsW( CD_6WayValve_Pointer );
			}
			else
			{
				lFirstColumnWidth = pclPreviousSheetDescription->GetSSheetPointer()->GetColWidthInPixelsW( CD_Actuator_FirstColumn );
				lFirstColumnWidth += pclPreviousSheetDescription->GetSSheetPointer()->GetColWidthInPixelsW( CD_Actuator_Box );
				lLastColumnWidth = pclPreviousSheetDescription->GetSSheetPointer()->GetColWidthInPixelsW( CD_Actuator_Pointer );
			}

			break;

		case SD_FamilyPIBCValve:

			if( SD_TypeValveAccessories == iSheetType )
			{
				lFirstColumnWidth = pclPreviousSheetDescription->GetSSheetPointer()->GetColWidthInPixelsW( CD_PIBCV_FirstColumn );
				lFirstColumnWidth += pclPreviousSheetDescription->GetSSheetPointer()->GetColWidthInPixelsW( CD_PIBCV_Box );
				lLastColumnWidth = pclPreviousSheetDescription->GetSSheetPointer()->GetColWidthInPixelsW( CD_PIBCV_Pointer );
			}
			else
			{
				lFirstColumnWidth = pclPreviousSheetDescription->GetSSheetPointer()->GetColWidthInPixelsW( CD_Actuator_FirstColumn );
				lFirstColumnWidth += pclPreviousSheetDescription->GetSSheetPointer()->GetColWidthInPixelsW( CD_Actuator_Box );
				lLastColumnWidth = pclPreviousSheetDescription->GetSSheetPointer()->GetColWidthInPixelsW( CD_Actuator_Pointer );
			}

			break;

		case SD_FamilyBalancingValve:
			lFirstColumnWidth = pclPreviousSheetDescription->GetSSheetPointer()->GetColWidthInPixelsW( CD_BV_FirstColumn );
			lLastColumnWidth = pclPreviousSheetDescription->GetSSheetPointer()->GetColWidthInPixelsW( CD_BV_Pointer );
			break;
	}

	// Do some correction in regards to some columns.
	long lTotalWidth = rectSize.Width() - lFirstColumnWidth;
	lTotalWidth -= lLastColumnWidth;

	// Try to create 2 columns in just the middle of previous sheet.
	long lLeftWidth = ( long )( lTotalWidth / 2 );
	long lRightWidth = ( ( lTotalWidth % 2 ) > 0 ) ? lLeftWidth + 1 : lLeftWidth;

	// Set columns.
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
	if( false == strTitle.IsEmpty() )
	{
		// Increase row height.
		double dRowHeight = 12.75;
		pclSSheet->SetRowHeight( RD_Accessory_FirstRow, dRowHeight * 0.5 );
		pclSSheet->SetRowHeight( RD_Accessory_GroupName, dRowHeight * 1.5 );

		// Set title.
		pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)_GetGroupColor( false, eSideDefinition, iSheetType ) );
		pclSheetDescription->SetUserVariable( _SDUV_TITLEFORECOLOR, ( pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor ) ) );
		pclSheetDescription->SetUserVariable( _SDUV_TITLEBACKCOLOR, ( pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor ) ) );

		pclSSheet->AddCellSpanW( CD_Accessory_Left, RD_Accessory_GroupName, CD_Accessory_LastColumn - CD_Accessory_Left, 1 );
		pclSSheet->SetStaticText( CD_Accessory_Left, RD_Accessory_GroupName, strTitle );
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
		//VERIFY( 0 != pAccessory->IsClass( CLASS( CDB_Product ) ) );
		if( false == pAccessory->IsAnAccessory() )
		{
			continue;
		}

		CString strName = _T("");

		if( true == pclRuledTable->IsByPair( pAccessory->GetIDPtr().ID ) )
		{
			strName += _T("2x ");
		}

		strName += pAccessory->GetName();

		// Create checkbox accessory.

		// If accessory has its property 'IsAttached' to true and user is in selection by set, he can't select it.
		bool bEnabled = true;

		// HYS-1968: if we work on a 6wayvalve set, disable Pibcv accessories.
		if( true == m_pclIndSel6WayValveParams->m_bOnlyForSet )
		{
			bEnabled = false;
		}

		CCDBCheckboxAccessory *pCheckbox = CreateCheckboxAccessory( lLeftOrRight, lRow, false, bEnabled, strName, pAccessory, pclRuledTable,
				pvecAccessories, pclSheetDescription );

		if( NULL != pCheckbox )
		{
			pCheckbox->ApplyInternalChange();

			if( false == bEnabled && true == pAccessory->IsAttached() )
			{
				pCheckbox->SetToolTip( TASApp.LoadLocalizedString( AFXMSG_ACCATTACHEDNOTINSET ) );
			}
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

		// HYS-1877: Select automatically Set accessories.
		if( true == m_pclIndSel6WayValveParams->m_bOnlyForSet )
		{
			VerifyCheckboxAccessories( pAccessory, true, pvecAccessories, true );
		}
	}

	if( false == m_pclIndSel6WayValveParams->m_bOnlyForSet )
	{
		VerifyCheckboxAccessories( NULL, false, pvecAccessories );
	}

	if( NULL != ppclGroupButton )
	{
		*ppclGroupButton = CreateExpandCollapseGroupButton( CD_Accessory_FirstColumn, RD_Accessory_GroupName, true,
				CCDButtonExpandCollapseGroup::ButtonState::ExpandRow, pclSheetDescription->GetFirstSelectableRow(), 
				pclSheetDescription->GetLastSelectableRow( false ), pclSheetDescription );

		// Show button.
		if( NULL != *ppclGroupButton )
		{
			( *ppclGroupButton )->ApplyInternalChange();
		}
	}

	pclSSheet->SetCellBorder( CD_Accessory_Left, lRow, CD_Accessory_LastColumn - 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );

	pclSSheet->SetBool( SSB_REDRAW, TRUE );
}

void CRViewSSel6WayValve::_ReadAllColumnWidth( CDS_IndSelParameter *pclIndSelParameter )
{
	if( NULL == pclIndSelParameter )
	{
		ASSERT_RETURN;
	}

	std::map<UINT, short> mapSDIDVersion;
	mapSDIDVersion[CW_RVIEWSSELDPC_SHEETID_6WAYVALVE] = CW_RVIEWSSEL6WAYVALVE_6WAYVALVE_VERSION;
	mapSDIDVersion[CW_RVIEWSSELDPC_SHEETID_6WAYVALVE_ACTUATOR] = CW_RVIEWSSEL6WAYVALVE_6WAYVALVE_ACTUATOR_VERSION;
	mapSDIDVersion[CW_RVIEWSSELDPC_SHEETID_PIBCVALVEBOTHSIDE] = CW_RVIEWSSEL6WAYVALVE_PIBCVALVEBOTHSIDE_VERSION;
	mapSDIDVersion[CW_RVIEWSSELDPC_SHEETID_PIBCVALVECOOLINGSIDE] = CW_RVIEWSSEL6WAYVALVE_PIBCVALVECOOLIGSIDE_VERSION;
	mapSDIDVersion[CW_RVIEWSSELDPC_SHEETID_PIBCVALVEHEATINGSIDE] = CW_RVIEWSSEL6WAYVALVE_PIBCVALVEHEATINGSIDE_VERSION;
	mapSDIDVersion[CW_RVIEWSSELDPC_SHEETID_PIBCVALVEBOTHSIDE_ACTUATOR] = CW_RVIEWSSEL6WAYVALVE_PIBCVALVEBOTHSIDE_ACTUATOR_VERSION;
	mapSDIDVersion[CW_RVIEWSSELDPC_SHEETID_PIBCVALVECOOLINGSIDE_ACTUATOR] = CW_RVIEWSSEL6WAYVALVE_PIBCVALVECOOLINSQIDE_ACTUATOR_VERSION;
	mapSDIDVersion[CW_RVIEWSSELDPC_SHEETID_PIBCVALVEHEATINGSIDE_ACTUATOR] = CW_RVIEWSSEL6WAYVALVE_PIBCVALVEHEATINGSIDE_ACTUATOR_VERSION;
	mapSDIDVersion[CW_RVIEWSSELDPC_SHEETID_BALANCINGVALVECOOLINGSIDE] = CW_RVIEWSSEL6WAYVALVE_BALANCINGVALVECOOLINGSIDE_VERSION;
	mapSDIDVersion[CW_RVIEWSSELDPC_SHEETID_BALANCINGVALVEHEATINGSIDE] = CW_RVIEWSSEL6WAYVALVE_BALANCINGVALVEHEATINGSIDE_VERSION;

	// Container window sheet ID to sheetdescription of this rightview.
	std::map<UINT, short> mapCWtoRW;
	mapCWtoRW[CW_RVIEWSSELDPC_SHEETID_6WAYVALVE] = SD_6WayValve;
	mapCWtoRW[CW_RVIEWSSELDPC_SHEETID_6WAYVALVE_ACTUATOR] = SD_6WayValveActuator;
	mapCWtoRW[CW_RVIEWSSELDPC_SHEETID_PIBCVALVEBOTHSIDE] = SD_PIBCValveBothSide;
	mapCWtoRW[CW_RVIEWSSELDPC_SHEETID_PIBCVALVECOOLINGSIDE] = SD_PIBCValveCoolingSide;
	mapCWtoRW[CW_RVIEWSSELDPC_SHEETID_PIBCVALVEHEATINGSIDE] = SD_PIBCValveHeatingSide;
	mapCWtoRW[CW_RVIEWSSELDPC_SHEETID_PIBCVALVEBOTHSIDE_ACTUATOR] = SD_PIBCValveActuatorBothSide;
	mapCWtoRW[CW_RVIEWSSELDPC_SHEETID_PIBCVALVECOOLINGSIDE_ACTUATOR] = SD_PIBCValveActuatorCoolingSide;
	mapCWtoRW[CW_RVIEWSSELDPC_SHEETID_PIBCVALVEHEATINGSIDE_ACTUATOR] = SD_PIBCValveActuatorHeatingSide;
	mapCWtoRW[CW_RVIEWSSELDPC_SHEETID_BALANCINGVALVECOOLINGSIDE] = SD_BalancingValveCoolingSide;
	mapCWtoRW[CW_RVIEWSSELDPC_SHEETID_BALANCINGVALVEHEATINGSIDE] = SD_BalancingValveHeatingSide;

	// By default and before reading registry saved column width force reset column width for all sheets.
	for( auto &iter : mapCWtoRW )
	{
		ResetColumnWidth( iter.second );
	}
	
	// Access to the 'RViewSSel6WayValve' group.
	CDS_IndSelParameter::CCWWindow *pclCWWindow = pclIndSelParameter->GetWindowColumnWidth( CW_WINDOWID_INDSEL6WAYVALVE, true );

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

void CRViewSSel6WayValve::_WriteAllColumnWidth( CDS_IndSelParameter *pclIndSelParameter )
{
	if( NULL == pclIndSelParameter )
	{
		ASSERT_RETURN;
	}

	std::map<UINT, short> mapSDIDVersion;
	mapSDIDVersion[CW_RVIEWSSELDPC_SHEETID_6WAYVALVE] = CW_RVIEWSSEL6WAYVALVE_6WAYVALVE_VERSION;
	mapSDIDVersion[CW_RVIEWSSELDPC_SHEETID_6WAYVALVE_ACTUATOR] = CW_RVIEWSSEL6WAYVALVE_6WAYVALVE_ACTUATOR_VERSION;
	mapSDIDVersion[CW_RVIEWSSELDPC_SHEETID_PIBCVALVEBOTHSIDE] = CW_RVIEWSSEL6WAYVALVE_PIBCVALVEBOTHSIDE_VERSION;
	mapSDIDVersion[CW_RVIEWSSELDPC_SHEETID_PIBCVALVECOOLINGSIDE] = CW_RVIEWSSEL6WAYVALVE_PIBCVALVECOOLIGSIDE_VERSION;
	mapSDIDVersion[CW_RVIEWSSELDPC_SHEETID_PIBCVALVEHEATINGSIDE] = CW_RVIEWSSEL6WAYVALVE_PIBCVALVEHEATINGSIDE_VERSION;
	mapSDIDVersion[CW_RVIEWSSELDPC_SHEETID_PIBCVALVEBOTHSIDE_ACTUATOR] = CW_RVIEWSSEL6WAYVALVE_PIBCVALVEBOTHSIDE_ACTUATOR_VERSION;
	mapSDIDVersion[CW_RVIEWSSELDPC_SHEETID_PIBCVALVECOOLINGSIDE_ACTUATOR] = CW_RVIEWSSEL6WAYVALVE_PIBCVALVECOOLINSQIDE_ACTUATOR_VERSION;
	mapSDIDVersion[CW_RVIEWSSELDPC_SHEETID_PIBCVALVEHEATINGSIDE_ACTUATOR] = CW_RVIEWSSEL6WAYVALVE_PIBCVALVEHEATINGSIDE_ACTUATOR_VERSION;
	mapSDIDVersion[CW_RVIEWSSELDPC_SHEETID_BALANCINGVALVECOOLINGSIDE] = CW_RVIEWSSEL6WAYVALVE_BALANCINGVALVECOOLINGSIDE_VERSION;
	mapSDIDVersion[CW_RVIEWSSELDPC_SHEETID_BALANCINGVALVEHEATINGSIDE] = CW_RVIEWSSEL6WAYVALVE_BALANCINGVALVEHEATINGSIDE_VERSION;

	// Container window sheet ID to sheetdescription of this rightview.
	std::map<UINT, short> mapCWtoRW;
	mapCWtoRW[CW_RVIEWSSELDPC_SHEETID_6WAYVALVE] = SD_6WayValve;
	mapCWtoRW[CW_RVIEWSSELDPC_SHEETID_6WAYVALVE_ACTUATOR] = SD_6WayValveActuator;
	mapCWtoRW[CW_RVIEWSSELDPC_SHEETID_PIBCVALVEBOTHSIDE] = SD_PIBCValveBothSide;
	mapCWtoRW[CW_RVIEWSSELDPC_SHEETID_PIBCVALVECOOLINGSIDE] = SD_PIBCValveCoolingSide;
	mapCWtoRW[CW_RVIEWSSELDPC_SHEETID_PIBCVALVEHEATINGSIDE] = SD_PIBCValveHeatingSide;
	mapCWtoRW[CW_RVIEWSSELDPC_SHEETID_PIBCVALVEBOTHSIDE_ACTUATOR] = SD_PIBCValveActuatorBothSide;
	mapCWtoRW[CW_RVIEWSSELDPC_SHEETID_PIBCVALVECOOLINGSIDE_ACTUATOR] = SD_PIBCValveActuatorCoolingSide;
	mapCWtoRW[CW_RVIEWSSELDPC_SHEETID_PIBCVALVEHEATINGSIDE_ACTUATOR] = SD_PIBCValveActuatorHeatingSide;
	mapCWtoRW[CW_RVIEWSSELDPC_SHEETID_BALANCINGVALVECOOLINGSIDE] = SD_BalancingValveCoolingSide;
	mapCWtoRW[CW_RVIEWSSELDPC_SHEETID_BALANCINGVALVEHEATINGSIDE] = SD_BalancingValveHeatingSide;

	// Access to the 'RViewSSelXXX' group.
	CDS_IndSelParameter::CCWWindow *pclCWWindow = pclIndSelParameter->GetWindowColumnWidth( CW_WINDOWID_INDSEL6WAYVALVE, true );

	for( auto &iter : mapSDIDVersion )
	{
		CDS_IndSelParameter::CCWSheet *pclCWSheet = pclCWWindow->GetCWSheet( iter.first, true );
		pclCWSheet->SetVersion( iter.second );
		pclCWSheet->GetMap() = m_mapSSheetColumnWidth[mapCWtoRW[iter.first]];
	}
}
