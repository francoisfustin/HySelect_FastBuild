#include "stdafx.h"
#include "afxctl.h"

#include "TASelect.h"
#include "MainFrm.h"

#include "DlgLeftTabSelManager.h"

#include "ProductSelectionParameters.h"
#include "RViewSSelSafetyValve.h"

CRViewSSelSafetyValve *pRViewSSelSafetyValve = NULL;

CRViewSSelSafetyValve::CRViewSSelSafetyValve() : CRViewSSelSS( CMainFrame::RightViewList::eRVSSelSafetyValve, false )
{
	m_pclIndSelSafetyValveParams = NULL;
	_Init();
	pRViewSSelSafetyValve = this;
}

CRViewSSelSafetyValve::~CRViewSSelSafetyValve()
{
	pRViewSSelSafetyValve = NULL;
}

void CRViewSSelSafetyValve::Reset()
{
	_Init();
	CRViewSSelSS::Reset();
}

void CRViewSSelSafetyValve::Suggest( CProductSelelectionParameters *pclProductSelectionParameters, LPARAM lpParam )
{
	CRViewSSelSS::Suggest( pclProductSelectionParameters, lpParam );

	if( NULL == pclProductSelectionParameters || NULL == dynamic_cast<CIndSelSafetyValveParams*>( pclProductSelectionParameters ) )
	{
		return;
	}

	m_pclIndSelSafetyValveParams = dynamic_cast<CIndSelSafetyValveParams*>( pclProductSelectionParameters );

	// To remove all current displayed sheets.
	Reset();

	BeginWaitCursor();
	CWnd::SetRedraw( FALSE );

	long lSafetyValveRowSelected = -1;
	CDS_SSelSafetyValve *pclSelectedSafetyValve = NULL;

	// If we are in edition mode...
	if( NULL != m_pclIndSelSafetyValveParams->m_SelIDPtr.MP )
	{
		pclSelectedSafetyValve = reinterpret_cast<CDS_SSelSafetyValve*>( m_pclIndSelSafetyValveParams->m_SelIDPtr.MP );

		if( NULL == pclSelectedSafetyValve )
		{
			ASSERT( 0 );
		}

		lSafetyValveRowSelected = _FillSafetyValveRows( pclSelectedSafetyValve );
	}
	else
	{
		lSafetyValveRowSelected = _FillSafetyValveRows();
	}
	
	// Verify if sheet description has been well created.
	CSheetDescription *pclSheetDescriptionSafetyValve = m_ViewDescription.GetFromSheetDescriptionID( SD_SafetyValve );

	if( NULL != pclSheetDescriptionSafetyValve && NULL != pclSheetDescriptionSafetyValve->GetSSheetPointer() )
	{
		CSSheet *pclSSheet = pclSheetDescriptionSafetyValve->GetSSheetPointer();

		if( lSafetyValveRowSelected > -1 && NULL != pclSelectedSafetyValve )
		{
			CCellDescriptionProduct *pclCDProduct = FindCDProduct( lSafetyValveRowSelected, (LPARAM)( pclSelectedSafetyValve->GetProductAs<CDB_TAProduct>() ), 
					pclSheetDescriptionSafetyValve );

			if( NULL != pclCDProduct && NULL != pclCDProduct->GetProduct() )
			{
				// If we are in edition mode we simulate a click on the product.
				OnClickProduct( pclSheetDescriptionSafetyValve, pclCDProduct, pclSheetDescriptionSafetyValve->GetActiveColumn(), lSafetyValveRowSelected );

				// Allow to check if we need to change the 'Show all priorities' button or not.
				CheckShowAllPrioritiesButtonState( pclSheetDescriptionSafetyValve, lSafetyValveRowSelected );

				// Verify accessories.
				CAccessoryList *pclSafetyValveAccessoryList = pclSelectedSafetyValve->GetAccessoryList();
				
				if( pclSafetyValveAccessoryList->GetCount() > 0 )
				{
					CAccessoryList::AccessoryItem rAccessoryItem = pclSafetyValveAccessoryList->GetFirst();
					
					while( rAccessoryItem.IDPtr.MP != NULL )
					{
						VerifyCheckboxAccessories( (CDB_Product *)rAccessoryItem.IDPtr.MP, true, &m_vecSafetyValveAccessoryList );
						rAccessoryItem = pclSafetyValveAccessoryList->GetNext();
					}
				}

				// Verify if user has selected a blow tank device.
				IDPTR BlowTankIDPtr = pclSelectedSafetyValve->GetBlowTankIDPtr();
				CSheetDescription *pclSheetDescriptionBlowTank = m_ViewDescription.GetFromSheetDescriptionID( SD_BlowTank );

				if( _NULL_IDPTR != BlowTankIDPtr && NULL != BlowTankIDPtr.MP && NULL != pclSheetDescriptionBlowTank )
				{
					CDB_TAProduct *pEditedTAP = dynamic_cast<CDB_TAProduct *>( BlowTankIDPtr.MP );

					if( NULL != pEditedTAP )
					{
						// Find row number where is the blow tank.
						long lBlowTankRowSelected = _GetRowOfEditedBlowTank( pclSheetDescriptionBlowTank, pEditedTAP );

						if( lBlowTankRowSelected != -1 )
						{
							CCellDescriptionProduct *pclCDBlowTank = FindCDProduct( lBlowTankRowSelected, ( LPARAM )pEditedTAP, pclSheetDescriptionBlowTank );

							if( NULL != pclCDBlowTank && NULL != pclCDBlowTank->GetProduct() )
							{
								// Simulate a click on the edited blow tank.
								OnClickProduct( pclSheetDescriptionBlowTank, pclCDBlowTank, pclSheetDescriptionBlowTank->GetActiveColumn(), lBlowTankRowSelected );

								// Allow to check if we need to change the 'Show all priorities' button or not.
								CheckShowAllPrioritiesButtonState( pclSheetDescriptionBlowTank, lBlowTankRowSelected );

								// Verify accessories on blow tank.
								CAccessoryList *pclBlowTankAccessoryList = pclSelectedSafetyValve->GetBlowTankAccessoryList();

								if( pclBlowTankAccessoryList->GetCount() > 0 )
								{
									CAccessoryList::AccessoryItem rAccessoryItem = pclBlowTankAccessoryList->GetFirst();

									while( rAccessoryItem.IDPtr.MP != NULL )
									{
										VerifyCheckboxAccessories( (CDB_Product * )rAccessoryItem.IDPtr.MP, true, &m_vecBlowTankAccessoryList );
										rAccessoryItem = pclBlowTankAccessoryList->GetNext();
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
			PrepareAndSetNewFocus( pclSheetDescriptionSafetyValve, CD_SafetyValve_Name, RD_SafetyValve_FirstAvailRow, 0 );
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

void CRViewSSelSafetyValve::FillInSelected( CDS_SelProd *pSelectedProductToFill )
{
	if( NULL == m_pclIndSelSafetyValveParams || NULL == m_pclIndSelSafetyValveParams->m_pclSelectSafetyValveList )
	{
		ASSERT_RETURN;
	}

	CDS_SSelSafetyValve *pSelectedSafetyValveToFill = dynamic_cast<CDS_SSelSafetyValve *>( pSelectedProductToFill );

	if( NULL == pSelectedSafetyValveToFill )
	{
		ASSERT_RETURN;
	}

	// Clear previous selected safety valve accessories.
	CAccessoryList *pclSafetyValveAccessoryListToFill = pSelectedSafetyValveToFill->GetAccessoryList();	
	// Clear previous selected blow tank accessories.
	CAccessoryList *pclBlowTankAccessoryToFill = pSelectedSafetyValveToFill->GetBlowTankAccessoryList();
	bool bIsEditionMode = false;
	if( false == m_pclIndSelSafetyValveParams->m_bEditModeRunning )
	{
		pclSafetyValveAccessoryListToFill->Clear();
		pclBlowTankAccessoryToFill->Clear();
	}
	else
	{
		bIsEditionMode = true;
	}	

	// Retrieve the current selected safety valve if exist.
	CDB_TAProduct *pclCurrentSafetyValveSelected = GetCurrentSafetyValveSelected();

	if( NULL != pclCurrentSafetyValveSelected )
	{
		pSelectedSafetyValveToFill->SetProductIDPtr( pclCurrentSafetyValveSelected->GetIDPtr() );

		// Search the safety valve in CSelectList to set additional parameters.
		for( CSelectedSafetyValve *pclSelectedSafetyValve = m_pclIndSelSafetyValveParams->m_pclSelectSafetyValveList->GetFirst<CSelectedSafetyValve>(); 
				NULL != pclSelectedSafetyValve; pclSelectedSafetyValve = m_pclIndSelSafetyValveParams->m_pclSelectSafetyValveList->GetNext<CSelectedSafetyValve>() )
		{
			if( pclSelectedSafetyValve->GetpData() == pclCurrentSafetyValveSelected )	// Product found
			{
				pSelectedSafetyValveToFill->SetQuantityNeeded( pclSelectedSafetyValve->GetQuantityNeeded() );
				break;
			}
		}
		if( false == bIsEditionMode )
		{
			// Retrieve selected accessory and add it.
			for( vecCDCAccessoryListIter vecIter = m_vecSafetyValveAccessoryList.begin(); vecIter != m_vecSafetyValveAccessoryList.end(); vecIter++ )
			{
				CCDBCheckboxAccessory *pCDBCheckboxAccessory = *vecIter;

				if( NULL != pCDBCheckboxAccessory && true == pCDBCheckboxAccessory->GetCheckStatus() && NULL != pCDBCheckboxAccessory->GetAccessoryPointer() )
				{
					pclSafetyValveAccessoryListToFill->Add( pCDBCheckboxAccessory->GetAccessoryPointer()->GetIDPtr(), CAccessoryList::_AT_Accessory, 
							pCDBCheckboxAccessory->GetRuledTable() );
				}
			}
		}
		else
		{
			UpdateAccessoryList( m_vecSafetyValveAccessoryList, pclSafetyValveAccessoryListToFill, CAccessoryList::_AT_Accessory );
		}
	}

	// Fill data for blow tank device.
	CDB_BlowTank *pclCurrentBlowTank = dynamic_cast<CDB_BlowTank *>( _GetCurrentBlowTankSelected() );

	if( NULL != pclCurrentBlowTank )
	{
		// Save blow tank IDPtr.
		pSelectedSafetyValveToFill->SetBlowTankIDPtr( pclCurrentBlowTank->GetIDPtr() );
		if( false == bIsEditionMode )
		{
			// Retrieve selected accessory and add it.
			for( vecCDCAccessoryListIter vecIter = m_vecBlowTankAccessoryList.begin(); vecIter != m_vecBlowTankAccessoryList.end(); vecIter++ )
			{
				CCDBCheckboxAccessory *pCDBCheckboxAccessory = *vecIter;

				if( NULL != pCDBCheckboxAccessory && true == pCDBCheckboxAccessory->GetCheckStatus() && NULL != pCDBCheckboxAccessory->GetAccessoryPointer() )
				{
					pclBlowTankAccessoryToFill->Add( pCDBCheckboxAccessory->GetAccessoryPointer()->GetIDPtr(), CAccessoryList::_AT_Accessory, 
							pCDBCheckboxAccessory->GetRuledTable() );
				}
			}
		}
		else
		{
		 	UpdateAccessoryList( m_vecBlowTankAccessoryList, pclBlowTankAccessoryToFill, CAccessoryList::_AT_Accessory );
		}
	}
}

void CRViewSSelSafetyValve::OnNewDocument( CDS_IndSelParameter *pclIndSelParameter )
{
	_ReadAllColumnWidth( pclIndSelParameter );
}

void CRViewSSelSafetyValve::SaveSelectionParameters( CDS_IndSelParameter *pclIndSelParameter )
{
	_WriteAllColumnWidth( pclIndSelParameter );
}

CDB_TAProduct *CRViewSSelSafetyValve::GetCurrentSafetyValveSelected()
{
	CDB_TAProduct *pclCurrentSafetyValveSelected = NULL;
	CSheetDescription *pclSheetDescriptionSafetyValve = m_ViewDescription.GetFromSheetDescriptionID( SD_SafetyValve );

	if( NULL != pclSheetDescriptionSafetyValve )
	{
		// Retrieve the current selected safety valve.
		CCellDescriptionProduct *pclCDCurrentSafetyValveSelected = NULL;
		LPARAM lpPointer;
		
		if( true == pclSheetDescriptionSafetyValve->GetUserVariable( _SDUV_SELECTEDPRODUCT, lpPointer ) )
		{
			pclCDCurrentSafetyValveSelected = (CCellDescriptionProduct *)lpPointer;
		}

		if( NULL != pclCDCurrentSafetyValveSelected && NULL != pclCDCurrentSafetyValveSelected->GetProduct() )
		{
			pclCurrentSafetyValveSelected = dynamic_cast<CDB_TAProduct*>( (CData *)pclCDCurrentSafetyValveSelected->GetProduct() );
		}
	}

	return pclCurrentSafetyValveSelected;
}

CDB_TAProduct *CRViewSSelSafetyValve::GetCurrentBlowTankSelected()
{
	CDB_TAProduct *pclCurrentBlowTankSelected = NULL;
	CSheetDescription *pclSheetDescriptionBlowTank = m_ViewDescription.GetFromSheetDescriptionID( SD_BlowTank );

	if( NULL != pclSheetDescriptionBlowTank )
	{
		// Retrieve the current selected blow tank.
		CCellDescriptionProduct *pclCDCurrentBlowTankSelected = NULL;
		LPARAM lpPointer;

		if( true == pclSheetDescriptionBlowTank->GetUserVariable( _SDUV_SELECTEDPRODUCT, lpPointer ) )
		{
			pclCDCurrentBlowTankSelected = (CCellDescriptionProduct *)lpPointer;
		}


		if( NULL != pclCDCurrentBlowTankSelected && NULL != pclCDCurrentBlowTankSelected->GetProduct() )
		{
			pclCurrentBlowTankSelected = dynamic_cast<CDB_TAProduct *>( (CData *)pclCDCurrentBlowTankSelected->GetProduct() );
		}
	}

	return pclCurrentBlowTankSelected;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PROTECTED MEMBERS
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CRViewSSelSafetyValve::OnTextTipFetch( CSheetDescription *pclSheetDescription, long lColumn, long lRow, WORD *pwMultiLine, 
		SHORT *pnTipWidth, TCHAR *pstrTipText, BOOL *bfShowTip )
{
	if( NULL == m_pclIndSelSafetyValveParams || NULL == m_pclIndSelSafetyValveParams->m_pclSelectSafetyValveList )
	{
		ASSERTA_RETURN( false );
	}

	if( false == m_bInitialised || NULL == pclSheetDescription )
	{
		return false;
	}

	// First, we must call base class.
	// Remark: if base class has displayed a tooltip, it's not necessary to go further in this method.
	if( true == CRViewSSelSS::OnTextTipFetch( pclSheetDescription, lColumn, lRow, pwMultiLine, pnTipWidth, pstrTipText, bfShowTip ) )
	{
		return true;
	}

	CString str;
	bool bReturnValue = false;
	*bfShowTip = false;
	*pwMultiLine = SS_TT_MULTILINE_AUTO;

	CDB_TAProduct *pTAP = dynamic_cast<CDB_TAProduct *>( (CData *)GetCDProduct( lColumn, lRow, pclSheetDescription ) );
	CSSheet *pclSSheet = pclSheetDescription->GetSSheetPointer();
	CDS_TechnicalParameter *pclTechParam = m_pclIndSelSafetyValveParams->m_pTADS->GetpTechParams();
	CSelectedSafetyValve *pclSelectedSafetyValve = NULL;
	CDB_BlowTank *pclBlowTank = NULL;
	CSelectedBase *pclSelectedBlowTank = NULL;

	switch( pclSheetDescription->GetSheetDescriptionID() )
	{
		// Check if mouse cursor has passed over a safety valve.
		case SD_SafetyValve:
			pclSelectedSafetyValve = GetSelectProduct<CSelectedSafetyValve>( pTAP, m_pclIndSelSafetyValveParams->m_pclSelectSafetyValveList );

			if( NULL != pclSelectedSafetyValve )
			{
				if( CD_SafetyValve_TempRange == lColumn )
				{
					if( true == pclSelectedSafetyValve->IsFlagSet( CSelectedBase::eTemperature ) )
					{
						FormatString( str, IDS_SSHEETSSEL_TEMPERROR, pTAP->GetTempRange() );
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

		case SD_SafetyValveAccessory:
			if( lColumn > CD_SafetyValveAccessory_FirstColumn && lColumn < CD_SafetyValveAccessory_LastColumn )
			{
				TextTipFetchEllipsesHelper( lColumn, lRow, pclSheetDescription, pnTipWidth, &str );
			}
			break;

		// Check if mouse cursor has passed over a blow tank.
		case SD_BlowTank:
			pclBlowTank = dynamic_cast<CDB_BlowTank *>( pTAP );
			pclSelectedBlowTank = GetSelectProduct<CSelectedBase>( pclBlowTank, m_pclIndSelSafetyValveParams->m_pclSelectSafetyValveList->GetBlowTankSelected() );

			if( NULL != pclSelectedBlowTank )
			{
				if( CD_BlowTank_TempRange == lColumn )
				{
					if( true == pclSelectedBlowTank->IsFlagSet( CSelectedBase::eTemperature ) )
					{
						FormatString( str, IDS_SSHEETSSEL_TEMPERROR, pTAP->GetTempRange() );
					}
				}

				if( true == str.IsEmpty() && NULL != pclBlowTank )
				{
					if( 0 != _tcslen( pclBlowTank->GetComment() ) )
					{
						str = pclBlowTank->GetComment();
					}
				}
			}
			break;

		case SD_BlowTankAccessory:
			if( lColumn > CD_BlowTankAccessory_FirstColumn && lColumn < CD_BlowTankAccessory_LastColumn )
			{
				TextTipFetchEllipsesHelper( lColumn, lRow, pclSheetDescription, pnTipWidth, &str );
			}
			break;
	}

	if( false == str.IsEmpty() )
	{
		*pnTipWidth = (SHORT)pclSSheet->GetTipTextWidth( str );
		wcsncpy_s( pstrTipText, SS_TT_TEXTMAX, (LPCTSTR)str, SS_TT_TEXTMAX );
		*bfShowTip = true;
		bReturnValue = true;
	}

	return bReturnValue;
}

bool CRViewSSelSafetyValve::OnClickProduct( CSheetDescription *pclSheetDescription, CCellDescriptionProduct *pclCellDescriptionProduct, 
		long lColumn, long lRow )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() || NULL == pclCellDescriptionProduct )
	{
		return false;
	}

	bool bNeedRefresh = true;
	CSSheet *pclSSheet = pclSheetDescription->GetSSheetPointer();
	pclSSheet->SetBool( SSB_REDRAW, FALSE );

	CDB_SafetyValveBase *pclSafetyValveBase = dynamic_cast<CDB_SafetyValveBase *>( (CData *)pclCellDescriptionProduct->GetProduct() );
	CDB_BlowTank *pclBlowTank = dynamic_cast<CDB_BlowTank *>( (CData *)pclCellDescriptionProduct->GetProduct() );

	if( NULL != pclSafetyValveBase )
	{
		// User has clicked on a safety valve.
		_ClickOnSafetyValve( pclSheetDescription, pclSafetyValveBase, pclCellDescriptionProduct, lColumn, lRow );
	}
	else if( NULL != pclBlowTank )
	{
		// User has clicked on a blow tank.
		_ClickOnBlowTank( pclSheetDescription, pclBlowTank, pclCellDescriptionProduct, lColumn, lRow );
	}
	else
	{
		bNeedRefresh = false;
	}

	pclSSheet->SetBool( SSB_REDRAW, TRUE );
	return bNeedRefresh;
}

bool CRViewSSelSafetyValve::ResetColumnWidth( short nSheetDescriptionID )
{
	TSpread clTSpread;
	
	if( FALSE == clTSpread.Create( WS_CHILD, CRect( 0, 0, 0, 0 ), this, 0 ) )
	{
		ASSERTA_RETURN( false );
	}
	
	switch( nSheetDescriptionID )
	{
		case SD_SafetyValve:
			m_mapSSheetColumnWidth[SD_SafetyValve][CD_SafetyValve_FirstColumn] = clTSpread.ColWidthToLogUnits( 4 );
			m_mapSSheetColumnWidth[SD_SafetyValve][CD_SafetyValve_CheckBox] = clTSpread.ColWidthToLogUnits( 2 );
			m_mapSSheetColumnWidth[SD_SafetyValve][CD_SafetyValve_Name] = clTSpread.ColWidthToLogUnits( 14 );
			m_mapSSheetColumnWidth[SD_SafetyValve][CD_SafetyValve_Number] = clTSpread.ColWidthToLogUnits( 11 );
			m_mapSSheetColumnWidth[SD_SafetyValve][CD_SafetyValve_Material] = clTSpread.ColWidthToLogUnits( 12 );
			m_mapSSheetColumnWidth[SD_SafetyValve][CD_SafetyValve_SetPressure] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_SafetyValve][CD_SafetyValve_MediumName] = clTSpread.ColWidthToLogUnits( 18 );
			m_mapSSheetColumnWidth[SD_SafetyValve][CD_SafetyValve_Version] = clTSpread.ColWidthToLogUnits( 11 );
			m_mapSSheetColumnWidth[SD_SafetyValve][CD_SafetyValve_LiftingMechanism] = clTSpread.ColWidthToLogUnits( 13 );
			m_mapSSheetColumnWidth[SD_SafetyValve][CD_SafetyValve_Connection] = clTSpread.ColWidthToLogUnits( 10 );
			m_mapSSheetColumnWidth[SD_SafetyValve][CD_SafetyValve_Size] = clTSpread.ColWidthToLogUnits( 12 );
			m_mapSSheetColumnWidth[SD_SafetyValve][CD_SafetyValve_Power] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_SafetyValve][CD_SafetyValve_Power100PC] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_SafetyValve][CD_SafetyValve_TempRange] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_SafetyValve][CD_SafetyValve_Pointer] = clTSpread.ColWidthToLogUnits( 1 );
			break;

		case SD_BlowTank:
			m_mapSSheetColumnWidth[SD_BlowTank][CD_BlowTank_FirstColumn] = clTSpread.ColWidthToLogUnits( 4 );
			m_mapSSheetColumnWidth[SD_BlowTank][CD_BlowTank_CheckBox] = clTSpread.ColWidthToLogUnits( 2 );
			m_mapSSheetColumnWidth[SD_BlowTank][CD_BlowTank_Name] = clTSpread.ColWidthToLogUnits( 14 );
			m_mapSSheetColumnWidth[SD_BlowTank][CD_BlowTank_Number] = clTSpread.ColWidthToLogUnits( 11 );
			m_mapSSheetColumnWidth[SD_BlowTank][CD_BlowTank_PS] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_BlowTank][CD_BlowTank_MediumName] = clTSpread.ColWidthToLogUnits( 18 );
			m_mapSSheetColumnWidth[SD_BlowTank][CD_BlowTank_Material] = clTSpread.ColWidthToLogUnits( 12 );
			m_mapSSheetColumnWidth[SD_BlowTank][CD_BlowTank_Size] = clTSpread.ColWidthToLogUnits( 10 );
			m_mapSSheetColumnWidth[SD_BlowTank][CD_BlowTank_TempRange] = clTSpread.ColWidthToLogUnits( 8 );
			m_mapSSheetColumnWidth[SD_BlowTank][CD_BlowTank_Pointer] = clTSpread.ColWidthToLogUnits( 1 );
			break;
	}
	
	if( NULL != clTSpread.GetSafeHwnd() )
	{
		clTSpread.DestroyWindow();
	}

	return true;
}

bool CRViewSSelSafetyValve::IsSelectionReady()
{
	CDB_TAProduct *pSelection = GetCurrentSafetyValveSelected();

	if ( NULL != pSelection )
	{
		return ( false == pSelection->IsDeleted() );
	}

	return false; // No selection available
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PRIVATE MEMBERS
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CRViewSSelSafetyValve::_Init( void )
{
	m_lSafetyValveSelectedRow = 0;
	m_lBlowTankSelectedRow = 0;
	m_vecSafetyValveAccessoryList.clear();
	m_vecBlowTankAccessoryList.clear();
	m_pCDBExpandCollapseRowsSafetyValve = NULL;
	m_pCDBExpandCollapseGroupSafetyValveAccessory = NULL;
	m_pCDBShowAllPrioritiesSafetyValve = NULL;
	m_pCDBExpandCollapseRowsBlowTank = NULL;
	m_pCDBExpandCollapseGroupBlowTankAccessory = NULL;
	m_pCDBShowAllPrioritiesBlowTank = NULL;
}

void CRViewSSelSafetyValve::_ClickOnSafetyValve( CSheetDescription *pclSheetDescriptionSafetyValve, CDB_SafetyValveBase *pclSafetyValveClicked, 
		CCellDescriptionProduct *pclCellDescriptionProduct, long lColumn, long lRow )
{
	if( NULL == m_pclIndSelSafetyValveParams || NULL == m_pclIndSelSafetyValveParams->m_pclSelectSafetyValveList )
	{
		ASSERT( 0 );
		return;
	}

	CSSheet *pclSSheet = pclSheetDescriptionSafetyValve->GetSSheetPointer();

	m_lBlowTankSelectedRow = 0;
	m_pCDBExpandCollapseGroupSafetyValveAccessory = NULL;
	m_pCDBExpandCollapseRowsBlowTank = NULL;
	m_pCDBExpandCollapseGroupBlowTankAccessory = NULL;
	m_pCDBShowAllPrioritiesBlowTank = NULL;

	_SetCurrentBlowTankSelected( NULL );
		
	// Memorize previous focus row.
	long lCurrentFocusedRow = GetCurrentFocusedRow();

	// To be sure to clean all previous data concerning focus.
	KillCurrentFocus();

	// Unselect row where current selection is set (yellow light).
	// Remark: 'KillCurrentFocus' unselect only rows where focus is on (blue light).
	pclSSheet->UnSelectMultipleRows();

	// By default clear accessories list.
	m_vecSafetyValveAccessoryList.clear();

	LPARAM lSafetyValveCount;
	pclSheetDescriptionSafetyValve->GetUserVariable( _SDUV_TOTALPRODUCT_COUNT, lSafetyValveCount );

	// Retrieve the current selected safety valve if exist.
	CDB_TAProduct *pclCurrentSafetyValveSelected = GetCurrentSafetyValveSelected();

	// If there is already one safety valve selected and user clicks on the current one...
	// Remark: 'm_pCDBExpandCollapseRows' is not created if there is only one safety valve. Thus we need to check first if there is only one safety valve.
	//         Otherwise we have to check if user has clicked on the current one.
	if( NULL != pclCurrentSafetyValveSelected 
			&& ( ( 1 == lSafetyValveCount) || ( NULL != m_pCDBExpandCollapseRowsSafetyValve && lRow == m_pCDBExpandCollapseRowsSafetyValve->GetCellPosition().y ) ) )
	{
		// Uncheck checkbox.
		pclSSheet->SetCheckBox( CD_SafetyValve_CheckBox, lRow, _T(""), false, true );

		// Reset current product selected.
		_SetCurrentSafetyValveSelected( NULL );

		// Delete Expand/Collapse rows button if exist.
		// Remark: 'm_pCDBExpandCollapseRows' is set to NULL in method!
		DeleteExpandCollapseRowsButton( m_pCDBExpandCollapseRowsSafetyValve, pclSheetDescriptionSafetyValve );

		// Show Show/Hide all priorities button if exist.
		if( NULL != m_pCDBShowAllPrioritiesSafetyValve )
		{
			m_pCDBShowAllPrioritiesSafetyValve->ApplyInternalChange();
		}

		// Set focus on safety valve currently selected.
		PrepareAndSetNewFocus( pclSheetDescriptionSafetyValve, pclSheetDescriptionSafetyValve->GetActiveColumn(), lRow, 0 );

		// Remove all sheets after safety valve.
		m_ViewDescription.RemoveAllSheetAfter( SD_SafetyValve );

		m_lSafetyValveSelectedRow = 0;

		// Notify a change in the selection.
		SelectionHasChanged( IsSelectionReady() );
	}
	else
	{
		// If a safety valve is already selected...
		if( NULL != pclCurrentSafetyValveSelected )
		{
			// Uncheck safety valve.
			pclSSheet->SetCheckBox( CD_SafetyValve_CheckBox, m_lSafetyValveSelectedRow, _T(""), false, true );

			// Remove all sheets after safety valve.
			m_ViewDescription.RemoveAllSheetAfter( SD_SafetyValve );
		}

		// Check safety valve.
		pclSSheet->SetCheckBox( CD_SafetyValve_CheckBox, lRow, _T(""), true, true );

		// Save new safety valve selection.
		_SetCurrentSafetyValveSelected( pclCellDescriptionProduct );

		// Delete Expand/Collapse rows button if exist.
		if( NULL != m_pCDBExpandCollapseRowsSafetyValve )
		{
			// Remark: 'm_pCDBExpandCollapseRows' is set to NULL in method!
			DeleteExpandCollapseRowsButton( m_pCDBExpandCollapseRowsSafetyValve, pclSheetDescriptionSafetyValve );
		}

		// Create Expand/Collapse rows button if needed...
		LPARAM lSafetyValveTotalCount;
		pclSheetDescriptionSafetyValve->GetUserVariable( _SDUV_TOTALPRODUCT_COUNT, lSafetyValveTotalCount );
		
		if( lSafetyValveTotalCount > 1 )
		{
			m_pCDBExpandCollapseRowsSafetyValve = CreateExpandCollapseRowsButton( CD_SafetyValve_FirstColumn, lRow, true, 
					CCDButtonExpandCollapseRows::ButtonState::CollapseRow,  pclSheetDescriptionSafetyValve->GetFirstSelectableRow(), 
					pclSheetDescriptionSafetyValve->GetLastSelectableRow( false ), pclSheetDescriptionSafetyValve );

			// Show button.
			if( NULL != m_pCDBExpandCollapseRowsSafetyValve )
			{
				m_pCDBExpandCollapseRowsSafetyValve->SetShowStatus( true );
			}
		}

		// Select safety valve (just highlight background).
		pclSSheet->SelectOneRow( lRow, pclSheetDescriptionSafetyValve->GetSelectionFrom(), pclSheetDescriptionSafetyValve->GetSelectionTo() );

		// Fill corresponding accessories.
		_FillSafetyValveAccessoryRows();

		// Fill corresponding blow tank list.
		CSelectedSafetyValve *pclSelectedSafetyValve = GetSelectProduct<CSelectedSafetyValve>( pclSafetyValveClicked, m_pclIndSelSafetyValveParams->m_pclSelectSafetyValveList );
		_FillBlowTankRows( pclSelectedSafetyValve );

		m_lSafetyValveSelectedRow = lRow;

		// Try to set the focus on the next table but do not lose the focus if no other table exist.
		CSheetDescription *pclNextSheetDescription = NULL;
		long lNewFocusedRow;
		bool bShiftPressed;
		
		if( DNFERRC_FocusOnNextGroupOK == DefineNextFocusedRow( pclSheetDescriptionSafetyValve, CD_SafetyValve_FirstColumn, lRow, 
				false, lNewFocusedRow, pclNextSheetDescription, bShiftPressed ) )
		{
			long lNewFocusedCol = pclNextSheetDescription->GetFirstParameterColumn();
			PrepareAndSetNewFocus( pclNextSheetDescription, lNewFocusedCol, lNewFocusedRow );
		}
		else
		{
			// Because we have changed current selection background in yellow, if there is no other sheet following current one, we
			// must set focus on current selection.
			PrepareAndSetNewFocus( pclSheetDescriptionSafetyValve, lColumn, lRow );
		}

		// Notify a change in the selection.
		SelectionHasChanged( IsSelectionReady() );
	}
}

long CRViewSSelSafetyValve::_FillSafetyValveRows( CDS_SSelSafetyValve *pEditedProduct )
{
	if( NULL == m_pclIndSelSafetyValveParams || NULL == m_pclIndSelSafetyValveParams->m_pclSelectSafetyValveList
			|| NULL == m_pclIndSelSafetyValveParams->m_pTADS || NULL == m_pclIndSelSafetyValveParams->m_pPipeDB )
	{
		ASSERTA_RETURN( 0 );
	}

	// Call 'MultiSpreadBase' method to get a new SSheet.
	CSheetDescription *pclSheetDescriptionSafetyValve = CreateSSheet( SD_SafetyValve );

	if( NULL == pclSheetDescriptionSafetyValve || NULL == pclSheetDescriptionSafetyValve->GetSSheetPointer() )
	{
		return 0;
	}
	
	CSSheet *pclSSheet = pclSheetDescriptionSafetyValve->GetSSheetPointer();

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
	CSelectedSafetyValve *pclSelectedSafetyValve = m_pclIndSelSafetyValveParams->m_pclSelectSafetyValveList->GetFirst<CSelectedSafetyValve>();

	if( NULL == pclSelectedSafetyValve )
	{
		return 0;
	}

	CDB_TAProduct *pTAP = dynamic_cast<CDB_TAProduct *>( pclSelectedSafetyValve->GetpData() );

	if( NULL == pTAP )
	{
		return 0;
	}

	// Set max rows.
	pclSSheet->SetMaxRows( RD_SafetyValve_FirstAvailRow - 1 );
	
	// All cells are static by default and filled with _T("").
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	pclSSheet->FormatStaticText( -1, -1, -1, -1, _T("") );
	
	// Increase row height.
	double RowHeight = 12.75;
	pclSSheet->SetRowHeight( RD_SafetyValve_FirstRow, RowHeight * 0.5 );
	pclSSheet->SetRowHeight( RD_SafetyValve_GroupName, RowHeight * 1.5 );
	pclSSheet->SetRowHeight( RD_SafetyValve_ColName, RowHeight * 2 );
	pclSSheet->SetRowHeight( RD_SafetyValve_Unit, RowHeight * 1.2 );
	
	// Initialize.
	pclSheetDescriptionSafetyValve->Init( 1, pclSSheet->GetMaxRows(), 1, CSheetDescription::SD_ParameterMode_Mono );
	pclSheetDescriptionSafetyValve->AddColumnInPixels( CD_SafetyValve_FirstColumn, m_mapSSheetColumnWidth[SD_SafetyValve][CD_SafetyValve_FirstColumn] );
	pclSheetDescriptionSafetyValve->AddColumnInPixels( CD_SafetyValve_CheckBox, m_mapSSheetColumnWidth[SD_SafetyValve][CD_SafetyValve_CheckBox] );
	pclSheetDescriptionSafetyValve->AddColumnInPixels( CD_SafetyValve_Name, m_mapSSheetColumnWidth[SD_SafetyValve][CD_SafetyValve_Name] );
	pclSheetDescriptionSafetyValve->AddColumnInPixels( CD_SafetyValve_Number, m_mapSSheetColumnWidth[SD_SafetyValve][CD_SafetyValve_Number] );
	pclSheetDescriptionSafetyValve->AddColumnInPixels( CD_SafetyValve_Material, m_mapSSheetColumnWidth[SD_SafetyValve][CD_SafetyValve_Material] );
	pclSheetDescriptionSafetyValve->AddColumnInPixels( CD_SafetyValve_SetPressure, m_mapSSheetColumnWidth[SD_SafetyValve][CD_SafetyValve_SetPressure] );
	pclSheetDescriptionSafetyValve->AddColumnInPixels( CD_SafetyValve_MediumName, m_mapSSheetColumnWidth[SD_SafetyValve][CD_SafetyValve_MediumName] );
	pclSheetDescriptionSafetyValve->AddColumnInPixels( CD_SafetyValve_Version, m_mapSSheetColumnWidth[SD_SafetyValve][CD_SafetyValve_Version] );
	pclSheetDescriptionSafetyValve->AddColumnInPixels( CD_SafetyValve_LiftingMechanism, m_mapSSheetColumnWidth[SD_SafetyValve][CD_SafetyValve_LiftingMechanism] );
	pclSheetDescriptionSafetyValve->AddColumnInPixels( CD_SafetyValve_Connection, m_mapSSheetColumnWidth[SD_SafetyValve][CD_SafetyValve_Connection] );
	pclSheetDescriptionSafetyValve->AddColumnInPixels( CD_SafetyValve_Size, m_mapSSheetColumnWidth[SD_SafetyValve][CD_SafetyValve_Size] );
	pclSheetDescriptionSafetyValve->AddColumnInPixels( CD_SafetyValve_Power, m_mapSSheetColumnWidth[SD_SafetyValve][CD_SafetyValve_Power] );
	pclSheetDescriptionSafetyValve->AddColumnInPixels( CD_SafetyValve_Power100PC, m_mapSSheetColumnWidth[SD_SafetyValve][CD_SafetyValve_Power100PC] );
	pclSheetDescriptionSafetyValve->AddColumnInPixels( CD_SafetyValve_TempRange, m_mapSSheetColumnWidth[SD_SafetyValve][CD_SafetyValve_TempRange] );
	pclSheetDescriptionSafetyValve->AddColumnInPixels( CD_SafetyValve_Pointer, m_mapSSheetColumnWidth[SD_SafetyValve][CD_SafetyValve_Pointer] );

	// Set in which column parameter must be saved.
	pclSheetDescriptionSafetyValve->AddParameterColumn( CD_SafetyValve_Pointer );
	
	// Set the focus column.
	pclSheetDescriptionSafetyValve->SetActiveColumn( CD_SafetyValve_Name );

	// Set range for selection.
	pclSheetDescriptionSafetyValve->SetFocusColumnRange( CD_SafetyValve_CheckBox, CD_SafetyValve_TempRange );

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

	if( false == pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleWordWrap, (LPARAM)TRUE ) )
	{
		return -1;
	}

	pclSSheet->SetStaticText( CD_SafetyValve_Name, RD_SafetyValve_ColName, IDS_SSHEETSSEL_NAME );
	pclSSheet->SetStaticText( CD_SafetyValve_Number, RD_SafetyValve_ColName, IDS_SSHEETSSELSAFETYVALVE_NUMBER );
	pclSSheet->SetStaticText( CD_SafetyValve_Material, RD_SafetyValve_ColName, IDS_SSHEETSSEL_MATERIAL );
	pclSSheet->SetStaticText( CD_SafetyValve_SetPressure, RD_SafetyValve_ColName, IDS_SSHEETSSELSAFETYVALVE_SETPRESSURE );
	pclSSheet->SetStaticText( CD_SafetyValve_MediumName, RD_SafetyValve_ColName, IDS_SSHEETSSELSAFETYVALVE_MEDIUM );
	pclSSheet->SetStaticText( CD_SafetyValve_Version, RD_SafetyValve_ColName, IDS_SSHEETSSEL_VERSION );
	pclSSheet->SetStaticText( CD_SafetyValve_LiftingMechanism, RD_SafetyValve_ColName, IDS_SSHEETSSELSAFETYVALVE_LIFTINGMECHANISM );
	pclSSheet->SetStaticText( CD_SafetyValve_Connection, RD_SafetyValve_ColName, IDS_SSHEETSSEL_CONNECT );
	pclSSheet->SetStaticText( CD_SafetyValve_Size, RD_SafetyValve_ColName, IDS_SSHEETSSEL_SIZE );
	pclSSheet->SetStaticText( CD_SafetyValve_Power, RD_SafetyValve_ColName, IDS_SSHEETSSELSAFETYVALVE_POWER );
	pclSSheet->SetStaticText( CD_SafetyValve_Power100PC, RD_SafetyValve_ColName, IDS_SSHEETSSELSAFETYVALVE_POWER100PC );
	pclSSheet->SetStaticText( CD_SafetyValve_TempRange, RD_SafetyValve_ColName, IDS_SSHEETSSEL_COLTEMPRANGE );
	//
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Row units.
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();
	pclSSheet->SetStaticText( CD_SafetyValve_SetPressure, RD_SafetyValve_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_PRESSURE ) ).c_str() );
	pclSSheet->SetStaticText( CD_SafetyValve_Power, RD_SafetyValve_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_TH_POWER ) ).c_str() );
	pclSSheet->SetStaticText( CD_SafetyValve_Power100PC, RD_SafetyValve_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_TH_POWER ) ).c_str() );
	pclSSheet->SetStaticText( CD_SafetyValve_TempRange, RD_SafetyValve_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_TEMPERATURE ) ).c_str() );
	//
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Draw black line below column header.
	pclSSheet->SetCellBorder( CD_SafetyValve_CheckBox, RD_SafetyValve_Unit, CD_SafetyValve_Pointer - 1, RD_SafetyValve_Unit, true, SS_BORDERTYPE_BOTTOM );

	long lRetRow = -1;
	CDS_TechnicalParameter *pTechParam = m_pclIndSelSafetyValveParams->m_pTADS->GetpTechParams();
	long lRow = RD_SafetyValve_FirstAvailRow;

	long lSafetyValveTotalCount = 0;
	long lSafetyValveNotPriorityCount = 0;
	bool bShowAllPrioritiesShown = false;
	long lShowAllPrioritiesButtonRow = 0;
	bool bAtLeastOneError;
	bool bAtLeastOneWarning;

	pclSheetDescriptionSafetyValve->SetUserVariable( _SDUV_TOTALPRODUCT_COUNT, 0 );
	pclSheetDescriptionSafetyValve->SetUserVariable( _SDUV_NOPRIORITYPRODUCT_COUNT, 0 );

	pclSheetDescriptionSafetyValve->RestartRemarkGenerator();
	
	for( CSelectedSafetyValve *pclSelectedSafetyValve = m_pclIndSelSafetyValveParams->m_pclSelectSafetyValveList->GetFirst<CSelectedSafetyValve>(); 
			NULL != pclSelectedSafetyValve; pclSelectedSafetyValve = m_pclIndSelSafetyValveParams->m_pclSelectSafetyValveList->GetNext<CSelectedSafetyValve>() )
	{
		CDB_SafetyValveBase *pclSafetyValveBase = dynamic_cast<CDB_SafetyValveBase *>( pclSelectedSafetyValve->GetpData() );

		if( NULL == pclSafetyValveBase ) 
		{
			continue;
		}

		bAtLeastOneError = false;
		bAtLeastOneWarning = false;
		bool bBest = pclSelectedSafetyValve->IsFlagSet(CSelectedBase::eBest );
		
		if( true == pclSelectedSafetyValve->IsFlagSet(CSelectedBase::eNotPriority) )
		{
			lSafetyValveNotPriorityCount++;
			
			if( false == bShowAllPrioritiesShown )
			{
				// Add one empty not selectable row.
				pclSheetDescriptionSafetyValve->AddRows( 1 );
				pclSheetDescriptionSafetyValve->RemoveSelectableRow( lRow );

				pclSSheet->SetCellBorder( CD_SafetyValve_Name, lRow, CD_SafetyValve_Pointer - 1, lRow, true, SS_BORDERTYPE_TOP | SS_BORDERTYPE_BOTTOM, 
						SS_BORDERSTYLE_SOLID, _BLACK );

				lShowAllPrioritiesButtonRow = lRow++;
				bShowAllPrioritiesShown = true;
			}
		}

		// Add this row.
		// 'true' to specify that this row can be selected.
		pclSheetDescriptionSafetyValve->AddRows( 1, true );

		// Set first columns in regards to current status (error, best or normal).
		if( true == bAtLeastOneError )
		{
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_RED );
		}
		else if( true == pclSelectedSafetyValve->IsFlagSet( CSelectedBase::eBest ) )
		{
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_GREEN );
		}
		
		// Add checkbox.
		pclSSheet->SetCheckBox( CD_SafetyValve_CheckBox, lRow, _T(""), false, true );

		// In addition of current flag, we have possibility that safety valve has the flag 'Not available' or 'Deleted'. 
		// In that case, we show safety valve name in red with "*" around it and symbol '!' or '!!' after.
		if( true == pclSafetyValveBase->IsDeleted() )
		{
			pclSheetDescriptionSafetyValve->WriteTextWithFlags( CString( pclSafetyValveBase->GetName() ), CD_SafetyValve_Name, lRow, 
					CSheetDescription::RemarkFlags::Deleted, _T(""), _RED );
		}
		else if( false == pclSafetyValveBase->IsAvailable() )
		{
			pclSheetDescriptionSafetyValve->WriteTextWithFlags( CString( pclSafetyValveBase->GetName() ), CD_SafetyValve_Name, lRow, 
					CSheetDescription::RemarkFlags::NotAvailable, _T(""), _RED );
		}
		else
		{
			pclSSheet->SetStaticText( CD_SafetyValve_Name, lRow, pclSafetyValveBase->GetName() );
		}

		if( true == bAtLeastOneError )
		{
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );
		}
		else if( true == bBest )
		{
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_GREEN );
		}

		pclSSheet->SetStaticText( CD_SafetyValve_Number, lRow, WriteDouble( pclSelectedSafetyValve->GetQuantityNeeded(), 1, 0, 1 ) );
		pclSSheet->SetStaticText( CD_SafetyValve_Material, lRow, pclSafetyValveBase->GetBodyMaterial() );
		pclSSheet->SetStaticText( CD_SafetyValve_SetPressure, lRow, WriteCUDouble( _U_PRESSURE, pclSafetyValveBase->GetSetPressure() ) );
		pclSSheet->SetStaticText( CD_SafetyValve_MediumName, lRow, pclSafetyValveBase->GetMediumName() );
		pclSSheet->SetStaticText( CD_SafetyValve_MediumName, lRow, pclSafetyValveBase->GetMediumName() );
		pclSSheet->SetStaticText( CD_SafetyValve_Version, lRow, pclSafetyValveBase->GetVersion() );
		pclSSheet->SetStaticText( CD_SafetyValve_LiftingMechanism, lRow, pclSafetyValveBase->GetLiftingType() );
		pclSSheet->SetStaticText( CD_SafetyValve_Connection, lRow, pclSafetyValveBase->GetInOutletConnectionString() );
		pclSSheet->SetStaticText( CD_SafetyValve_Size, lRow, pclSafetyValveBase->GetInOutletSizeString() );
		pclSSheet->SetStaticText( CD_SafetyValve_Power, lRow, WriteCUDouble( _U_TH_POWER, m_pclIndSelSafetyValveParams->m_dInstalledPower ) );

		double dPower100PC = pclSafetyValveBase->GetPowerLimit( m_pclIndSelSafetyValveParams->m_strSystemHeatGeneratorTypeID, m_pclIndSelSafetyValveParams->m_strNormID );
		pclSSheet->SetStaticText( CD_SafetyValve_Power100PC, lRow, WriteCUDouble( _U_TH_POWER, dPower100PC ) );
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

		// Temperature range.
		if( true == pclSelectedSafetyValve->IsFlagSet( CSelectedBase::eTemperature ) )
		{
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_RED );
			bAtLeastOneError = true;
		}
		
		pclSSheet->SetStaticText( CD_SafetyValve_TempRange, lRow, pclSafetyValveBase->GetTempRange() );
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );
		
		// Save parameter.
		CreateCellDescriptionProduct( pclSheetDescriptionSafetyValve->GetFirstParameterColumn(), lRow, (LPARAM)pclSafetyValveBase, pclSheetDescriptionSafetyValve );
		
		if( NULL != pEditedTAP && pEditedTAP == pclSafetyValveBase )
		{
			lRetRow = lRow;
		}

		pclSSheet->SetCellBorder( CD_SafetyValve_CheckBox, lRow, CD_SafetyValve_Pointer - 1, lRow, true, SS_BORDERTYPE_BOTTOM,SS_BORDERSTYLE_SOLID, _GRAY );

		lRow++;
		lSafetyValveTotalCount++;
	}

	long lLastDataRow = lRow - 1;

	pclSheetDescriptionSafetyValve->SetUserVariable( _SDUV_TOTALPRODUCT_COUNT, lSafetyValveTotalCount );
	pclSheetDescriptionSafetyValve->SetUserVariable( _SDUV_NOPRIORITYPRODUCT_COUNT, lSafetyValveNotPriorityCount );

	// Now we can create Show/Hide all priorities if needed.
	// Remarks: 2nd condition to avoid to have button if we have only no priority valves.
	if( true == bShowAllPrioritiesShown && lSafetyValveTotalCount > lSafetyValveNotPriorityCount )
	{
		// Create Show/Hide all priorities button in regards to current state!
		CCDButtonShowAllPriorities::ButtonState eState = ( true == GetShowAllPrioritiesFlag( SD_SafetyValve ) ) ? 
				CCDButtonShowAllPriorities::ButtonState::HidePriorities : CCDButtonShowAllPriorities::ButtonState::ShowPriorities;
		
		CCDButtonShowAllPriorities *pclShowAllButton = CreateShowAllPrioritiesButton( CD_SafetyValve_FirstColumn, lShowAllPrioritiesButtonRow, 
				bShowAllPrioritiesShown, eState, lShowAllPrioritiesButtonRow + 1, pclSSheet->GetMaxRows(), pclSheetDescriptionSafetyValve );

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

	pclSSheet->SetCellBorder( CD_SafetyValve_CheckBox, lLastDataRow, CD_SafetyValve_Pointer - 1, lLastDataRow, true, SS_BORDERTYPE_BOTTOM,
			SS_BORDERSTYLE_SOLID, _BLACK );

	// Add Remarks.
	// Note: these lines must be placed after creating Show/Hide all priorities button to avoid to take in count these lines when defining
	//       range of row to be Show/Hide.
	lRow = pclSheetDescriptionSafetyValve->WriteRemarks( lRow, CD_SafetyValve_CheckBox, CD_SafetyValve_Pointer );

	// Add the main title.
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)GetTitleBackgroundColor( pclSheetDescriptionSafetyValve ) );
	pclSheetDescriptionSafetyValve->SetUserVariable( _SDUV_TITLEFORECOLOR, ( pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor ) ) );
	pclSheetDescriptionSafetyValve->SetUserVariable( _SDUV_TITLEBACKCOLOR, ( pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor ) ) );

	pclSSheet->AddCellSpanW( CD_SafetyValve_CheckBox, RD_SafetyValve_GroupName, CD_SafetyValve_Pointer - CD_SafetyValve_CheckBox, 1 );
	pclSSheet->SetStaticText( CD_SafetyValve_CheckBox, RD_SafetyValve_GroupName, IDS_SSHEETSSELSAFETYVALVE_SAFETYVALVEGROUP );
	
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

	pclSSheet->SetBool( SSB_REDRAW, TRUE );

	// Set that there is no selection at now.
	_SetCurrentSafetyValveSelected( NULL );

	// Add possibility to change column size.
	// Remark: place this code after any initialization (like adding, removing, resizing, hiding/showing columns).
	CResizingColumnInfo* pclResizingColumnInfo = pclSheetDescriptionSafetyValve->GetResizingColumnInfo();
	pclResizingColumnInfo->ActivateFeature( true, false, RD_SafetyValve_FirstAvailRow, lLastDataRow );
	pclResizingColumnInfo->AddRangeColumn( CD_SafetyValve_Name, CD_SafetyValve_TempRange, RD_SafetyValve_ColName, RD_SafetyValve_Unit );

	// Add possibility to click on title group to reset column width.
	CreateTitleGroupButtons( CD_SafetyValve_CheckBox, CD_SafetyValve_TempRange, RD_SafetyValve_GroupName, pclSheetDescriptionSafetyValve );

	return lRetRow;
}

void CRViewSSelSafetyValve::_FillSafetyValveAccessoryRows( )
{
	if( NULL == m_pclIndSelSafetyValveParams || NULL == m_pclIndSelSafetyValveParams->m_pTADB )
	{
		ASSERT_RETURN;
	}

	// Retrieve the current selected safety valve.
	CDB_TAProduct *pclCurrentSafetyValveSelected = GetCurrentSafetyValveSelected();

	if( NULL == pclCurrentSafetyValveSelected )
	{
		return;
	}

	CDB_RuledTable *pRuledTable = (CDB_RuledTable *)( pclCurrentSafetyValveSelected->GetAccessoriesGroupIDPtr().MP );

	if( NULL == pRuledTable )
	{
		return;
	}

	CRank rList;
	int iCount = m_pclIndSelSafetyValveParams->m_pTADB->GetAccessories( &rList, pRuledTable, m_pclIndSelSafetyValveParams->m_eFilterSelection );

	if( 0 == iCount )
	{
		return;
	}

	// Call 'MultiSpreadBase' method to get a new SSheet.
	CSheetDescription *pclSDAccessory = CreateSSheet( SD_SafetyValveAccessory );
	
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
	pclSSheet->SetMaxRows( RD_SafetyValveAccessory_FirstAvailRow + iCount );

	// All cells are static by default and filled with _T("").
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	pclSSheet->FormatStaticText( -1, -1, -1, -1, _T("") );

	// Get sheet description of safety valve to retrieve width.
	CSheetDescription *pclSDSafetyValve = m_ViewDescription.GetFromSheetDescriptionID( SD_SafetyValve );
	// It's absolutely not normal to have this pointer NULL.
	ASSERT( NULL != pclSDSafetyValve );
	
	// Take SSheet size and client area size and determine the right width to take in count.
	CRect rectSize = pclSDSafetyValve->GetSSheetPointer()->GetSheetSizeInPixels();

	// Prepare first column width (to match safety valve sheet).
	long lFirstColumnWidth = pclSDSafetyValve->GetSSheetPointer()->GetColWidthInPixelsW( CD_SafetyValve_FirstColumn );

	// Do some correction in regards to some columns.
	long lTotalWidth = rectSize.Width() - lFirstColumnWidth;
	lTotalWidth -= pclSDSafetyValve->GetSSheetPointer()->GetColWidthInPixelsW( CD_SafetyValve_Pointer );

	// Try to create 2 columns in just the middle of the safety valve sheet.
	long lLeftWidth = (long)( lTotalWidth / 2 );
	long lRightWidth = ( ( lTotalWidth % 2 ) > 0 ) ? lLeftWidth + 1 : lLeftWidth;

	// Set columns.
	// 'CSheetDescription::SD_ParameterMode_Multi' to specify that this sheet can have more than one column with parameter.
	pclSDAccessory->Init( 1, pclSSheet->GetMaxRows(), 2, CSheetDescription::SD_ParameterMode_Multi );
	
	pclSDAccessory->AddColumnInPixels( CD_SafetyValveAccessory_FirstColumn, lFirstColumnWidth );
	pclSDAccessory->AddColumnInPixels( CD_SafetyValveAccessory_Left, lLeftWidth );
	pclSDAccessory->AddColumnInPixels( CD_SafetyValveAccessory_Right, lRightWidth );
	pclSDAccessory->AddColumn( CD_SafetyValveAccessory_LastColumn, 0 );
	
	// These two columns can contain parameter.
	pclSDAccessory->AddParameterColumn( CD_SafetyValveAccessory_Left );
	pclSDAccessory->AddParameterColumn( CD_SafetyValveAccessory_Right );
	
	// Set the focus column (don't set on Left of Right (on a check box) ).
	pclSDAccessory->SetActiveColumn( CD_SafetyValveAccessory_FirstColumn );
	
	// Set selectable rows.
	pclSDAccessory->SetSelectableRangeRow( RD_SafetyValveAccessory_FirstAvailRow, pclSSheet->GetMaxRows() - 1 );

	// Increase row height.
	double RowHeight = 12.75;
	pclSSheet->SetRowHeight( RD_SafetyValveAccessory_FirstRow, RowHeight * 0.5 );
	pclSSheet->SetRowHeight( RD_SafetyValveAccessory_GroupName, RowHeight * 1.5 );

	// Set title.
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)GetTitleBackgroundColor( pclSDAccessory ) );
	
	pclSDAccessory->SetUserVariable( _SDUV_TITLEFORECOLOR, ( pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor ) ) );
	pclSDAccessory->SetUserVariable( _SDUV_TITLEBACKCOLOR, ( pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor ) ) );

	pclSSheet->AddCellSpanW( CD_SafetyValveAccessory_Left, RD_SafetyValveAccessory_GroupName, CD_SafetyValveAccessory_LastColumn - CD_SafetyValveAccessory_Left, 1 );
	pclSSheet->SetStaticText( CD_SafetyValveAccessory_Left, RD_SafetyValveAccessory_GroupName, IDS_SSHEETSSELSAFETYVALVE_SAFETYVALVEACCGROUP );

	// Fill accessories.
	long lRow = RD_SafetyValveAccessory_FirstAvailRow;
	
	CString str;
	LPARAM lparam;

	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_LEFT );

	// Left - Right.
	long lLeftOrRight = CD_SafetyValveAccessory_Left;
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
				pRuledTable, &m_vecSafetyValveAccessoryList, pclSDAccessory );
		
		if( NULL != pCheckbox )
		{
			pCheckbox->ApplyInternalChange();
		}
		
		// Description.
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleEllipses, (LPARAM)TRUE );
		pclSSheet->SetStaticText( lLeftOrRight, lRow + 1, pAccessory->GetComment() );
		bContinue = rList.GetNext( str, lparam );

		// Restart left part.
		if( bContinue && lLeftOrRight == CD_SafetyValveAccessory_Right )
		{
			pclSSheet->SetCellBorder( CD_SafetyValveAccessory_Left, lRow + 1, CD_SafetyValveAccessory_LastColumn - 1, lRow + 1, true, SS_BORDERTYPE_BOTTOM, 
					SS_BORDERSTYLE_SOLID, _GRAY );

			lLeftOrRight = CD_SafetyValveAccessory_Left;
			lRow += 2;
			ASSERT( lRow < pclSSheet->GetMaxRows() );
		}
		else
		{
			lLeftOrRight = CD_SafetyValveAccessory_Right;
		}
		
		if( FALSE == bContinue )
		{
			lRow++;
		}
	}

	VerifyCheckboxAccessories( NULL, false, &m_vecSafetyValveAccessoryList );

	m_pCDBExpandCollapseGroupSafetyValveAccessory = CreateExpandCollapseGroupButton( CD_SafetyValveAccessory_FirstColumn, RD_SafetyValveAccessory_GroupName, true, 
			CCDButtonExpandCollapseGroup::ButtonState::CollapseRow,  pclSDAccessory->GetFirstSelectableRow(), pclSDAccessory->GetLastSelectableRow( false ), 
			pclSDAccessory );

	// Show button.
	if( NULL != m_pCDBExpandCollapseGroupSafetyValveAccessory )
	{
		m_pCDBExpandCollapseGroupSafetyValveAccessory->SetShowStatus( true );
	}

	pclSSheet->SetCellBorder( CD_SafetyValveAccessory_Left, lRow, CD_SafetyValveAccessory_LastColumn - 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );

	pclSSheet->SetBool( SSB_REDRAW, TRUE );
}

void CRViewSSelSafetyValve::_SetCurrentSafetyValveSelected( CCellDescriptionProduct *pclCDCurrentSeparatorSelected )
{
	// Try to retrieve sheet description linked to the safety valve.
	CSheetDescription *pclSheetDescriptionSafetyValve = m_ViewDescription.GetFromSheetDescriptionID( SD_SafetyValve );

	if( NULL != pclSheetDescriptionSafetyValve )
	{
		pclSheetDescriptionSafetyValve->SetUserVariable( _SDUV_SELECTEDPRODUCT, (LPARAM)pclCDCurrentSeparatorSelected );
	}
}

void CRViewSSelSafetyValve::_ClickOnBlowTank( CSheetDescription *pclSheetDescriptionBlowTank, CDB_BlowTank *pclBlowTank, 
			CCellDescriptionProduct *pclCellDescriptionProduct, long lColumn, long lRow, bool bExternalCall )
{
	if( NULL == m_pclIndSelSafetyValveParams || NULL == m_pclIndSelSafetyValveParams->m_pclSelectSafetyValveList 
			|| NULL == m_pclIndSelSafetyValveParams->m_pclSelectSafetyValveList->GetBlowTankSelected() )
	{
		ASSERT( 0 );
		return;
	}

	// Check safety valve selected.
	CDB_TAProduct *pclCurrentSafetyValveSelected = GetCurrentSafetyValveSelected();

	if( NULL == pclCurrentSafetyValveSelected )
	{
		return;
	}

	CSSheet *pclSSheet = pclSheetDescriptionBlowTank->GetSSheetPointer();

	LPARAM lBlowTankTotalCount;
	pclSheetDescriptionBlowTank->GetUserVariable( _SDUV_TOTALPRODUCT_COUNT, lBlowTankTotalCount );

	// Memorize previous focus row.
	long lCurrentFocusedRow = GetCurrentFocusedRow();

	// To be sure to clean all previous data concerning focus.
	KillCurrentFocus();

	// Unselect row where current selection is set (yellow light).
	// Remark: 'KillCurrentFocus' unselect only rows where focus is on (blue light).
	pclSSheet->UnSelectMultipleRows();

	// By default clear accessories list.
	m_vecBlowTankAccessoryList.clear();
	m_pCDBExpandCollapseGroupBlowTankAccessory = NULL;

	LPARAM lBlowTankCount;
	pclSheetDescriptionBlowTank->GetUserVariable( _SDUV_TOTALPRODUCT_COUNT, lBlowTankCount );

	// Retrieve the current selected blow tank if exist.
	CDB_TAProduct *pclCurrentBlowTankSelected = GetCurrentBlowTankSelected();

	// If there is already one blow tank selected and user click on the current one...
	// Remark: 'm_pCDBExpandCollapseRowsBlowTank' is not created if there is only one blow tank. Thus we need to check first if there is only one blow tank.
	//         Otherwise we have to check if user has clicked on the current one.
	if( NULL != pclCurrentBlowTankSelected 
			&& ( ( 1 == lBlowTankCount ) || ( NULL != m_pCDBExpandCollapseRowsBlowTank && lRow == m_pCDBExpandCollapseRowsBlowTank->GetCellPosition().y ) ) )
	{
		// Reset current product selected.
		_SetCurrentBlowTankSelected( NULL );

		// Uncheck checkbox.
		pclSSheet->SetCheckBox( CD_BlowTank_CheckBox, lRow, _T(""), false, true );

		// Delete Expand/Collapse rows button if exist.
		// Remark: 'm_pCDBExpandCollapseRowsBlowTank' is set to NULL in method!
		DeleteExpandCollapseRowsButton( m_pCDBExpandCollapseRowsBlowTank, pclSheetDescriptionBlowTank );

		// Show Show/Hide all priorities button if exist.
		if( NULL != m_pCDBShowAllPrioritiesBlowTank )
		{
			m_pCDBShowAllPrioritiesBlowTank->ApplyInternalChange();
		}

		// Set focus on blow tank currently selected.
		PrepareAndSetNewFocus( pclSheetDescriptionBlowTank, pclSheetDescriptionBlowTank->GetActiveColumn(), lRow, 0 );

		// Remove all sheets after blow tank.
		m_ViewDescription.RemoveAllSheetAfter( SD_BlowTank );

		m_lBlowTankSelectedRow = 0;

		// Notify a change in the selection.
		SelectionHasChanged( IsSelectionReady() );
	}
	else
	{
		// If a blow tank is already selected...
		if( NULL != pclCurrentBlowTankSelected )
		{
			// Uncheck checkbox.
			pclSSheet->SetCheckBox( CD_BlowTank_CheckBox, m_lBlowTankSelectedRow, _T(""), false, true );

			// Remove all sheets after blow tank.
			m_ViewDescription.RemoveAllSheetAfter( SD_BlowTank );
		}

		// Save new blow tank selection.
		_SetCurrentBlowTankSelected( pclCellDescriptionProduct );

		// Check checkbox.
		pclSSheet->SetCheckBox( CD_BlowTank_CheckBox, lRow, _T(""), true, true );

		// Delete Expand/Collapse rows button if exist.
		if( NULL != m_pCDBExpandCollapseRowsBlowTank )
		{
			// Remark: 'm_pCDBExpandCollapseRowsBlowTank' is set to NULL in method!
			DeleteExpandCollapseRowsButton( m_pCDBExpandCollapseRowsBlowTank, pclSheetDescriptionBlowTank );
		}

		// Create Expand/Collapse rows button if needed...
		if( lBlowTankTotalCount > 1 )
		{
			m_pCDBExpandCollapseRowsBlowTank = CreateExpandCollapseRowsButton( CD_BlowTank_FirstColumn, lRow, true, 
					CCDButtonExpandCollapseRows::ButtonState::CollapseRow, pclSheetDescriptionBlowTank->GetFirstSelectableRow(), 
					pclSheetDescriptionBlowTank->GetLastSelectableRow( false ), pclSheetDescriptionBlowTank );

			// Show button.
			if( NULL != m_pCDBExpandCollapseRowsBlowTank )
			{
				m_pCDBExpandCollapseRowsBlowTank->SetShowStatus( true );
			}
		}

		// Select blow tank (just highlight background).
		pclSSheet->SelectOneRow( lRow, pclSheetDescriptionBlowTank->GetSelectionFrom(), pclSheetDescriptionBlowTank->GetSelectionTo() );

		// Fill accessories available for the current blow tank.
		_FillBlowTankAccessoryRows();

		// Try to set the focus on the next table but do not lose the focus if no other table exist.
		CSheetDescription *pclNextSheetDescription = NULL;
		long lNewFocusedRow;
		bool fShiftPressed;

		if( DNFERRC_FocusOnNextGroupOK == DefineNextFocusedRow( pclSheetDescriptionBlowTank, CD_BlowTank_FirstColumn, lRow, false, lNewFocusedRow,
				pclNextSheetDescription, fShiftPressed ) )
		{
			long lNewFocusedCol = pclNextSheetDescription->GetFirstParameterColumn();
			PrepareAndSetNewFocus( pclNextSheetDescription, lNewFocusedCol, lNewFocusedRow );
		}
		else
		{
			// Because we have changed current selection background in yellow, if there is no other sheet following current one, we
			// must set focus on current selection.
			PrepareAndSetNewFocus( pclSheetDescriptionBlowTank, lColumn, lRow );
		}

		m_lBlowTankSelectedRow = lRow;

		// Notify a change in the selection.
		SelectionHasChanged( IsSelectionReady() );
	}
}

void CRViewSSelSafetyValve::_InitAndFillBlowTankHeader( CSheetDescription *pclSheetDescriptionBlowTank, CSSheet *pclSSheet )
{
	if( NULL == m_pclIndSelSafetyValveParams || NULL == m_pclIndSelSafetyValveParams->m_pclSelectSafetyValveList 
			|| NULL == m_pclIndSelSafetyValveParams->m_pclSelectSafetyValveList->GetBlowTankSelected() )
	{
		ASSERT_RETURN;
	}

	// Flags for column customizing
	CSelectedBase *pclSelectedProduct = m_pclIndSelSafetyValveParams->m_pclSelectSafetyValveList->GetBlowTankSelected()->GetFirst<CSelectedBase>();

	if( NULL == pclSelectedProduct )
	{
		return;
	}

	CDB_TAProduct *pTAP = dynamic_cast<CDB_TAProduct *>( pclSelectedProduct->GetpData() );

	if( NULL == pTAP )
	{
		return;
	}

	pclSSheet->SetMaxRows( RD_BlowTank_FirstAvailRow - 1 );

	// All cells are static by default and filled with _T("")
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	pclSSheet->FormatStaticText( -1, -1, -1, -1, _T( "" ) );

	// Increase row height.
	double dRowHeight = 12.75;
	pclSSheet->SetRowHeight( RD_BlowTank_FirstRow, dRowHeight * 0.5 );
	pclSSheet->SetRowHeight( RD_BlowTank_GroupName, dRowHeight * 1.5 );
	pclSSheet->SetRowHeight( RD_BlowTank_ColName, dRowHeight * 2 );
	pclSSheet->SetRowHeight( RD_BlowTank_Unit, dRowHeight * 1.2 );

	// Set columns.
	pclSheetDescriptionBlowTank->Init( 1, pclSSheet->GetMaxRows(), 1, CSheetDescription::SD_ParameterMode_Mono );
	pclSheetDescriptionBlowTank->AddColumnInPixels( CD_BlowTank_FirstColumn, m_mapSSheetColumnWidth[SD_BlowTank][CD_BlowTank_FirstColumn] );
	pclSheetDescriptionBlowTank->AddColumnInPixels( CD_BlowTank_CheckBox, m_mapSSheetColumnWidth[SD_BlowTank][CD_BlowTank_CheckBox] );
	pclSheetDescriptionBlowTank->AddColumnInPixels( CD_BlowTank_Name, m_mapSSheetColumnWidth[SD_BlowTank][CD_BlowTank_Name] );
	pclSheetDescriptionBlowTank->AddColumnInPixels( CD_BlowTank_Number, m_mapSSheetColumnWidth[SD_BlowTank][CD_BlowTank_Number] );
	pclSheetDescriptionBlowTank->AddColumnInPixels( CD_BlowTank_PS, m_mapSSheetColumnWidth[SD_BlowTank][CD_BlowTank_PS] );
	pclSheetDescriptionBlowTank->AddColumnInPixels( CD_BlowTank_MediumName, m_mapSSheetColumnWidth[SD_BlowTank][CD_BlowTank_MediumName] );
	pclSheetDescriptionBlowTank->AddColumnInPixels( CD_BlowTank_Material, m_mapSSheetColumnWidth[SD_BlowTank][CD_BlowTank_Material] );
	pclSheetDescriptionBlowTank->AddColumnInPixels( CD_BlowTank_Size, m_mapSSheetColumnWidth[SD_BlowTank][CD_BlowTank_Size] );
	pclSheetDescriptionBlowTank->AddColumnInPixels( CD_BlowTank_TempRange, m_mapSSheetColumnWidth[SD_BlowTank][CD_BlowTank_TempRange] );
	pclSheetDescriptionBlowTank->AddColumnInPixels( CD_BlowTank_Pointer, m_mapSSheetColumnWidth[SD_BlowTank][CD_BlowTank_Pointer] );

	// Set in which column parameter must be saved.
	pclSheetDescriptionBlowTank->AddParameterColumn( CD_BlowTank_Pointer );

	// Set the focus column.
	pclSheetDescriptionBlowTank->SetActiveColumn( CD_BlowTank_Name );

	// Set range for selection.
	pclSheetDescriptionBlowTank->SetFocusColumnRange( CD_BlowTank_CheckBox, CD_BlowTank_TempRange );

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Row name.

	// Add the main title.
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, ( LPARAM )GetTitleBackgroundColor( pclSheetDescriptionBlowTank ) );
	pclSheetDescriptionBlowTank->SetUserVariable( _SDUV_TITLEFORECOLOR, ( pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor ) ) );
	pclSheetDescriptionBlowTank->SetUserVariable( _SDUV_TITLEBACKCOLOR, ( pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor ) ) );

	pclSSheet->AddCellSpanW( CD_BlowTank_CheckBox, RD_BlowTank_GroupName, CD_BlowTank_Pointer - CD_BlowTank_CheckBox, 1 );
	pclSSheet->SetStaticText( CD_BlowTank_CheckBox, RD_BlowTank_GroupName, IDS_SSHEETSSELSAFETYVALVE_BLOWTANK );

	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, ( LPARAM )FALSE );

	pclSSheet->SetStaticText( CD_BlowTank_Name, RD_BlowTank_ColName, IDS_SSHEETSSEL_NAME );
	pclSSheet->SetStaticText( CD_BlowTank_Number, RD_BlowTank_ColName, IDS_SSHEETSSELSAFETYVALVE_NROFVESSELS );
	pclSSheet->SetStaticText( CD_BlowTank_PS, RD_BlowTank_ColName, IDS_SSHEETSSEL_PN );
	pclSSheet->SetStaticText( CD_BlowTank_MediumName, RD_BlowTank_ColName, IDS_SSHEETSSELSAFETYVALVE_MEDIUM );
	pclSSheet->SetStaticText( CD_BlowTank_Material, RD_BlowTank_ColName, IDS_SSHEETSSEL_MATERIAL );
	pclSSheet->SetStaticText( CD_BlowTank_Size, RD_BlowTank_ColName, IDS_SSHEETSSEL_SIZE );
	pclSSheet->SetStaticText( CD_BlowTank_TempRange, RD_BlowTank_ColName, IDS_SSHEETSSEL_COLTEMPRANGE );
	//
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Row units
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();
	pclSSheet->SetStaticText( CD_BlowTank_PS, RD_BlowTank_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_PRESSURE ) ).c_str() );
	pclSSheet->SetStaticText( CD_BlowTank_TempRange, RD_BlowTank_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_TEMPERATURE ) ).c_str() );
	//
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Draw black line below column header.
	pclSSheet->SetCellBorder( CD_BlowTank_CheckBox, RD_BlowTank_Unit, CD_BlowTank_Pointer - 1, RD_BlowTank_Unit, true, SS_BORDERTYPE_BOTTOM );
}

void CRViewSSelSafetyValve::_FillBlowTankRows( CSelectedSafetyValve *pSelectedTAPSafetyValve )
{
	if( NULL == m_pclIndSelSafetyValveParams || NULL == m_pclIndSelSafetyValveParams->m_pclSelectSafetyValveList || NULL == m_pclIndSelSafetyValveParams->m_pTADB )
	{
		ASSERT( 0 );
		return;
	}

	if( NULL == pSelectedTAPSafetyValve )
	{
		return;
	}

	CDB_SafetyValveBase *pclSafetyValve = dynamic_cast<CDB_SafetyValveBase *>( pSelectedTAPSafetyValve->GetpData() );

	if( NULL == pclSafetyValve )
	{
		return;
	}

	// Try to retrieve safety valve selected list.
	CSelectSafetyValveList *pclSafetyValveList = m_pclIndSelSafetyValveParams->m_pclSelectSafetyValveList;

	if( NULL == pclSafetyValveList )
	{
		return;
	}

	CSelectList *pSelectedBlowTankList = NULL;

	// Retrieve balancing valve list linked to current safety valve.
	if( pclSafetyValveList->SelectBlowTank( m_pclIndSelSafetyValveParams, pSelectedTAPSafetyValve ) > 0 )
	{
		pSelectedBlowTankList = m_pclIndSelSafetyValveParams->m_pclSelectSafetyValveList->GetBlowTankSelected();
	}

	// If no blow tank exist...
	if( NULL == pSelectedBlowTankList || 0 == pSelectedBlowTankList->GetCount() )
	{
		return;
	}

	// Call 'MultiSpreadBase' method to get a new SSheet.
	CSheetDescription *pclSheetDescriptionBlowTank = CreateSSheet( SD_BlowTank );

	if( NULL == pclSheetDescriptionBlowTank || NULL == pclSheetDescriptionBlowTank->GetSSheetPointer() )
	{
		return;
	}

	CSSheet *pclSSheet = pclSheetDescriptionBlowTank->GetSSheetPointer();

	pclSSheet->SetBool( SSB_REDRAW, FALSE );

	pclSSheet->Init();
	pclSSheet->SetBool( SSB_ALLOWDRAGDROP, FALSE );
	pclSSheet->SetBool( SSB_HORZSCROLLBAR, FALSE );
	pclSSheet->SetBool( SSB_VERTSCROLLBAR, FALSE );
	pclSSheet->SetBool( SSB_MOVEACTIVEONFOCUS, FALSE );
	pclSSheet->SetTextTip( SS_TT_STATUS_FLOATING, 250, NULL, _LIGHTYELLOW, _TAH_BLACK );

	// Initialize blow tank sheet and fill header.
	_InitAndFillBlowTankHeader( pclSheetDescriptionBlowTank, pclSSheet );

	long lRow = RD_BlowTank_FirstAvailRow;

	long lBlowTankNumbers = 0;
	long lBlowTankNotPriorityNumbers = 0;
	bool bShowAllPrioritiesShown = false;
	long lShowAllPrioritiesButtonRow = 0;

	pclSheetDescriptionBlowTank->SetUserVariable( _SDUV_TOTALPRODUCT_COUNT, 0 );
	pclSheetDescriptionBlowTank->SetUserVariable( _SDUV_NOPRIORITYPRODUCT_COUNT, 0 );

	pclSheetDescriptionBlowTank->RestartRemarkGenerator();

	for( CSelectedBase *pclSelectedBlowTank = pSelectedBlowTankList->GetFirst<CSelectedBase>(); NULL != pclSelectedBlowTank;
		 pclSelectedBlowTank = pSelectedBlowTankList->GetNext<CSelectedBase>() )
	{
		CDB_TAProduct *pTAP = dynamic_cast<CDB_TAProduct *>( pclSelectedBlowTank->GetpData() );

		if( NULL == pTAP )
		{
			continue;
		}

		if( true == pclSelectedBlowTank->IsFlagSet( CSelectedBase::eNotPriority ) )
		{
			lBlowTankNotPriorityNumbers++;

			if( false == bShowAllPrioritiesShown )
			{
				// Add one empty not selectable row.
				pclSheetDescriptionBlowTank->AddRows( 1 );
				pclSheetDescriptionBlowTank->RemoveSelectableRow( lRow );

				pclSSheet->SetCellBorder( CD_BlowTank_CheckBox, lRow, CD_BlowTank_Pointer - 1, lRow, true, 
						SS_BORDERTYPE_TOP | SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );

				lShowAllPrioritiesButtonRow = lRow++;
				bShowAllPrioritiesShown = true;
			}
		}

		// If no error...
		if( 0 == _FillOneBlowTankRow( pclSheetDescriptionBlowTank, pclSSheet, pclSelectedBlowTank, lRow ) )
		{
			lRow++;
			lBlowTankNumbers++;
		}
	}

	long lLastDataRow = lRow - 1;

	pclSheetDescriptionBlowTank->SetUserVariable( _SDUV_TOTALPRODUCT_COUNT, lBlowTankNumbers );
	pclSheetDescriptionBlowTank->SetUserVariable( _SDUV_NOPRIORITYPRODUCT_COUNT, lBlowTankNotPriorityNumbers );

	// Now we can create Show/Hide all priorities if needed.
	// Remarks: 2nd condition to avoid to have button if we have only no priority valves.
	if( true == bShowAllPrioritiesShown && lBlowTankNumbers > lBlowTankNotPriorityNumbers )
	{
		// Create Show/Hide all priorities button in regards to current state!
		CCDButtonShowAllPriorities::ButtonState eState = ( true == GetShowAllPrioritiesFlag( SD_BlowTank ) ) ? 
				CCDButtonShowAllPriorities::ButtonState::HidePriorities : CCDButtonShowAllPriorities::ButtonState::ShowPriorities;
		
		CCDButtonShowAllPriorities *pclShowAllButton = CreateShowAllPrioritiesButton( CD_BlowTank_FirstColumn, lShowAllPrioritiesButtonRow, 
				bShowAllPrioritiesShown, eState, lShowAllPrioritiesButtonRow + 1, pclSSheet->GetMaxRows(), pclSheetDescriptionBlowTank );

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

	pclSSheet->SetCellBorder( CD_BlowTank_CheckBox, lLastDataRow, CD_BlowTank_Pointer - 1, lLastDataRow, true, 
			SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );

	// Add Remarks.
	// Note: these lines must be placed after creating Show/Hide all priorities button to avoid to take in count these lines when defining
	//       range of row to be Show/Hide.
	lRow = pclSheetDescriptionBlowTank->WriteRemarks( lRow, CD_BlowTank_CheckBox, CD_BlowTank_Pointer );

	pclSSheet->SetBool( SSB_REDRAW, TRUE );

	// Set that there is no selection at now.
	_SetCurrentBlowTankSelected( NULL );

	// Add possibility to change column size.
	// Remark: place this code after any initialization (like adding, removing, resizing, hiding/showing columns).
	CResizingColumnInfo *pclResizingColumnInfo = pclSheetDescriptionBlowTank->GetResizingColumnInfo();
	pclResizingColumnInfo->ActivateFeature( true, false, RD_BlowTank_FirstAvailRow, lLastDataRow );
	pclResizingColumnInfo->AddRangeColumn( CD_BlowTank_Name, CD_BlowTank_TempRange, RD_BlowTank_ColName, RD_BlowTank_Unit );

	// Add possibility to click on title group to reset column width.
	CreateTitleGroupButtons( CD_BlowTank_CheckBox, CD_BlowTank_TempRange, RD_BlowTank_GroupName, pclSheetDescriptionBlowTank );
}

long CRViewSSelSafetyValve::_FillOneBlowTankRow( CSheetDescription *pclSheetDescriptionBlowTank, CSSheet *pclSSheet, CSelectedBase *pSelectedTAP, long lRow )
{
	if( NULL == m_pclIndSelSafetyValveParams || NULL == m_pclIndSelSafetyValveParams->m_pclSelectSafetyValveList || NULL == GetCurrentSafetyValveSelected() )
	{
		ASSERTA_RETURN( -1 );
	}

	CDB_TAProduct *pclSafetyValve = GetCurrentSafetyValveSelected();
	CSelectedSafetyValve *pclSelectedSafetyValve = GetSelectProduct<CSelectedSafetyValve>( pclSafetyValve, m_pclIndSelSafetyValveParams->m_pclSelectSafetyValveList );
	
	if( NULL == pclSelectedSafetyValve )
	{
		ASSERTA_RETURN( -1 );
	}

	CDS_TechnicalParameter *pTechParam = m_pclIndSelSafetyValveParams->m_pTADS->GetpTechParams();
	CDB_BlowTank *pclBlowTank = dynamic_cast<CDB_BlowTank *>( pSelectedTAP->GetpData() );

	if( NULL == pclBlowTank )
	{
		ASSERTA_RETURN( -1 );
	}

	// Add this row.
	// 'true' to specify that this row can be selected.
	pclSheetDescriptionBlowTank->AddRows( 1, true );

	// Add checkbox.
	pclSSheet->SetCheckBox( CD_BlowTank_CheckBox, lRow, _T(""), false, true );

	if( true == pSelectedTAP->IsFlagSet( CSelectedBase::eBest ) )
	{
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, ( LPARAM )_GREEN );
	}

	// In addition of current flag, we have possibility that valve has the flag 'Not available' or 'Deleted'. In that case, we show valve name in red with "*" around it and
	// symbol '!' or '!!' after.
	if( true == pclBlowTank->IsDeleted() )
	{
		pclSheetDescriptionBlowTank->WriteTextWithFlags( CString( pclBlowTank->GetName() ), CD_BlowTank_Name, lRow, CSheetDescription::RemarkFlags::Deleted, _T(""), _RED );
	}
	else if( false == pclBlowTank->IsAvailable() )
	{
		pclSheetDescriptionBlowTank->WriteTextWithFlags( CString( pclBlowTank->GetName() ), CD_BlowTank_Name, lRow, CSheetDescription::RemarkFlags::NotAvailable, _T(""), _RED );
	}
	else
	{
		pclSSheet->SetStaticText( CD_BlowTank_Name, lRow, pclBlowTank->GetName() );
	}

	if( true == pSelectedTAP->IsFlagSet( CSelectedBase::eBest ) )
	{
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, ( LPARAM )_GREEN );
	}
	
	pclSSheet->SetStaticText( CD_BlowTank_Number, lRow, WriteDouble( pclSelectedSafetyValve->GetQuantityNeeded(), 1, 0, 1 ) );
	pclSSheet->SetStaticText( CD_BlowTank_PS, lRow, WriteCUDouble( _U_PRESSURE, pclBlowTank->GetPmaxmax() ) );
	pclSSheet->SetStaticText( CD_BlowTank_MediumName, lRow, pclBlowTank->GetMediumName() );
	pclSSheet->SetStaticText( CD_BlowTank_Material, lRow, pclBlowTank->GetBodyMaterial() );
	pclSSheet->SetStaticText( CD_BlowTank_Size, lRow, CString( pclBlowTank->GetAllSizesString() ) );

	// Temperature range.
	if( true == pSelectedTAP->IsFlagSet( CSelectedBase::eTemperature ) )
	{
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, ( LPARAM )_RED );
	}

	pclSSheet->SetStaticText( CD_BlowTank_TempRange, lRow, pclBlowTank->GetTempRange() );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, ( LPARAM )_BLACK );

	// Save parameter.
	CreateCellDescriptionProduct( pclSheetDescriptionBlowTank->GetFirstParameterColumn(), lRow, ( LPARAM )pclBlowTank, pclSheetDescriptionBlowTank );

	pclSSheet->SetCellBorder( CD_BlowTank_CheckBox, lRow, CD_BlowTank_Pointer - 1, lRow, true, SS_BORDERTYPE_BOTTOM, 
			SS_BORDERSTYLE_SOLID, _GRAY );

	return 0;
}

void CRViewSSelSafetyValve::_FillBlowTankAccessoryRows()
{
	if( NULL == m_pclIndSelSafetyValveParams || NULL == m_pclIndSelSafetyValveParams->m_pTADB )
	{
		ASSERT_RETURN;
	}

	CDB_TAProduct *pclCurrentBlowTankSelected = GetCurrentBlowTankSelected();

	if( NULL == pclCurrentBlowTankSelected )
	{
		return;
	}

	CDB_RuledTable *pRuledTable = ( CDB_RuledTable * )( pclCurrentBlowTankSelected->GetAccessoriesGroupIDPtr().MP );

	if( NULL == pRuledTable )
	{
		return;
	}

	CRank rList;
	int iCount = m_pclIndSelSafetyValveParams->m_pTADB->GetAccessories( &rList, pRuledTable, m_pclIndSelSafetyValveParams->m_eFilterSelection );

	if( 0 == iCount )
	{
		return;
	}

	// Call 'MultiSpreadBase' method to get a new SSheet.
	CSheetDescription *pclSheetDescriptionBlowTankAccessory = CreateSSheet( SD_BlowTankAccessory );

	if( NULL == pclSheetDescriptionBlowTankAccessory || NULL == pclSheetDescriptionBlowTankAccessory->GetSSheetPointer() )
	{
		return;
	}

	CSSheet *pclSSheet = pclSheetDescriptionBlowTankAccessory->GetSSheetPointer();

	pclSSheet->SetBool( SSB_REDRAW, FALSE );

	pclSSheet->Init();
	pclSSheet->SetBool( SSB_ALLOWDRAGDROP, FALSE );
	pclSSheet->SetBool( SSB_HORZSCROLLBAR, FALSE );
	pclSSheet->SetBool( SSB_VERTSCROLLBAR, FALSE );
	pclSSheet->SetTextTip( SS_TT_STATUS_FLOATING, 250, NULL, _LIGHTYELLOW, _TAH_BLACK );

	// Two lines by accessories, but two accessories by line (reason why we don't multiply iCount by 2).
	pclSSheet->SetMaxRows( RD_BlowTankAccessory_FirstAvailRow + iCount - 1 + ( iCount % 2 ) );

	// All cells are static by default and filled with _T("")
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	pclSSheet->FormatStaticText( -1, -1, -1, -1, _T( "" ) );

	// Get sheet description of blow tank to retrieve width.
	CSheetDescription *pclSheetDescriptionBlowTank = m_ViewDescription.GetFromSheetDescriptionID( SD_BlowTank );
	// It's absolutely not normal to have this pointer NULL.
	ASSERT( NULL != pclSheetDescriptionBlowTank );

	// Take SSheet size and client area size and determine the right width to take in count.
	CRect rectSize = pclSheetDescriptionBlowTank->GetSSheetPointer()->GetSheetSizeInPixels( false );

	// Prepare first column width (to match balancing valve sheet).
	long lFirstColumnWidth = pclSheetDescriptionBlowTank->GetSSheetPointer()->GetColWidthInPixelsW( CD_BlowTank_FirstColumn );

	// Do some correction in regards to some columns.
	long lTotalWidth = rectSize.Width() - lFirstColumnWidth;
	lTotalWidth -= pclSheetDescriptionBlowTank->GetSSheetPointer()->GetColWidthInPixelsW( CD_BlowTank_Pointer );

	// Try to create 2 columns in just the middle of balancing valve sheet.
	long lLeftWidth = ( long )( lTotalWidth / 2 );
	long lRightWidth = ( ( lTotalWidth % 2 ) > 0 ) ? lLeftWidth + 1 : lLeftWidth;

	// Set columns.
	// 'CSheetDescription::SD_ParameterMode_Multi' to set that this sheet can have more than one column with parameter.
	pclSheetDescriptionBlowTankAccessory->Init( 1, pclSSheet->GetMaxRows(), 2, CSheetDescription::SD_ParameterMode_Multi );

	pclSheetDescriptionBlowTankAccessory->AddColumnInPixels( CD_BlowTankAccessory_FirstColumn, lFirstColumnWidth );
	pclSheetDescriptionBlowTankAccessory->AddColumnInPixels( CD_BlowTankAccessory_Left, lLeftWidth );
	pclSheetDescriptionBlowTankAccessory->AddColumnInPixels( CD_BlowTankAccessory_Right, lRightWidth );
	pclSheetDescriptionBlowTankAccessory->AddColumn( CD_BlowTankAccessory_LastColumn, 0 );

	// These two columns can contain parameter.
	pclSheetDescriptionBlowTankAccessory->AddParameterColumn( CD_BlowTankAccessory_Left );
	pclSheetDescriptionBlowTankAccessory->AddParameterColumn( CD_BlowTankAccessory_Right );

	// Set the focus column (don't set on Left of Right (on a check box) ).
	pclSheetDescriptionBlowTankAccessory->SetActiveColumn( CD_BlowTankAccessory_FirstColumn );

	// Set selectable rows.
	pclSheetDescriptionBlowTankAccessory->SetSelectableRangeRow( RD_BlowTankAccessory_FirstAvailRow, pclSSheet->GetMaxRows() - 1 );

	// Increase row height.
	double dRowHeight = 12.75;
	pclSSheet->SetRowHeight( RD_BlowTankAccessory_FirstRow, dRowHeight * 0.5 );
	pclSSheet->SetRowHeight( RD_BlowTankAccessory_GroupName, dRowHeight * 1.5 );

	// Set title.
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, ( LPARAM )GetTitleBackgroundColor( pclSheetDescriptionBlowTankAccessory ) );

	pclSheetDescriptionBlowTankAccessory->SetUserVariable( _SDUV_TITLEFORECOLOR,
			( pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor ) ) );
	pclSheetDescriptionBlowTankAccessory->SetUserVariable( _SDUV_TITLEBACKCOLOR,
			( pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor ) ) );

	pclSSheet->AddCellSpanW( CD_BlowTankAccessory_Left, RD_BlowTankAccessory_GroupName, CD_BlowTankAccessory_LastColumn - CD_BlowTankAccessory_Left, 1 );
	pclSSheet->SetStaticText( CD_BlowTankAccessory_Left, RD_BlowTankAccessory_GroupName, IDS_SSHEETSSELSAFETYVALVE_BLOWTANKACCESSORIES );

	// Fill accessories.
	long lRow = RD_BlowTankAccessory_FirstAvailRow;

	CString str;
	LPARAM lparam;

	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, ( LPARAM )FALSE );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, ( LPARAM )SSS_ALIGN_LEFT );

	// Left - Right.
	long lLeftOrRight = CD_BlowTankAccessory_Left;
	BOOL bContinue = rList.GetFirst( str, lparam );

	while( TRUE == bContinue )
	{
		CDB_Product *pAccessory = (CDB_Product * )lparam;
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

		CCDBCheckboxAccessory *pclCheckbox = CreateCheckboxAccessory( lLeftOrRight, lRow, false, true, strName, 
				pAccessory, pRuledTable, &m_vecBlowTankAccessoryList, pclSheetDescriptionBlowTankAccessory );

		if( NULL != pclCheckbox )
		{
			pclCheckbox->ApplyInternalChange();
		}

		// Description.
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleEllipses, ( LPARAM )TRUE );
		pclSSheet->SetStaticText( lLeftOrRight, lRow + 1, pAccessory->GetComment() );
		bContinue = rList.GetNext( str, lparam );

		// Restart left part.
		if( TRUE == bContinue && CD_BlowTankAccessory_Right == lLeftOrRight )
		{
			pclSSheet->SetCellBorder( CD_BlowTankAccessory_Left, lRow + 1, CD_BlowTankAccessory_LastColumn - 1, lRow + 1, true, SS_BORDERTYPE_BOTTOM, 
					SS_BORDERSTYLE_SOLID, _GRAY );

			lLeftOrRight = CD_BlowTankAccessory_Left;
			lRow += 2;
			ASSERT( lRow < pclSSheet->GetMaxRows() );
		}
		else
		{
			lLeftOrRight = CD_BlowTankAccessory_Right;
		}

		if( !bContinue )
		{
			lRow++;
		}
	}

	VerifyCheckboxAccessories( NULL, false, &m_vecBlowTankAccessoryList );

	m_pCDBExpandCollapseGroupBlowTankAccessory = CreateExpandCollapseGroupButton( CD_BlowTankAccessory_FirstColumn, RD_BlowTankAccessory_GroupName, true,
			CCDButtonExpandCollapseGroup::ButtonState::CollapseRow, pclSheetDescriptionBlowTankAccessory->GetFirstSelectableRow(), 
			pclSheetDescriptionBlowTankAccessory->GetLastSelectableRow( false ), pclSheetDescriptionBlowTankAccessory );

	// Show button.
	if( NULL != m_pCDBExpandCollapseGroupBlowTankAccessory )
	{
		m_pCDBExpandCollapseGroupBlowTankAccessory->SetShowStatus( true );
	}

	pclSSheet->SetCellBorder( CD_BlowTankAccessory_Left, lRow, CD_BlowTankAccessory_LastColumn - 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );

	pclSSheet->SetBool( SSB_REDRAW, TRUE );
}

CDB_TAProduct *CRViewSSelSafetyValve::_GetCurrentBlowTankSelected()
{
	CDB_TAProduct *pclCurrentBlowTankSelected = NULL;
	CSheetDescription *pclSheetDescriptionBlowTank = m_ViewDescription.GetFromSheetDescriptionID( SD_BlowTank );

	if( NULL != pclSheetDescriptionBlowTank )
	{
		// Retrieve the current selected blow tank.
		CCellDescriptionProduct *pclCDCurrentBlowTankSelected = NULL;
		LPARAM lpPointer;

		if( true == pclSheetDescriptionBlowTank->GetUserVariable( _SDUV_SELECTEDPRODUCT, lpPointer ) )
		{
			pclCDCurrentBlowTankSelected = (CCellDescriptionProduct *)lpPointer;
		}

		if( NULL != pclCDCurrentBlowTankSelected && NULL != pclCDCurrentBlowTankSelected->GetProduct() )
		{
			pclCurrentBlowTankSelected = dynamic_cast<CDB_TAProduct *>( (CData *)pclCDCurrentBlowTankSelected->GetProduct() );
		}
	}

	return pclCurrentBlowTankSelected;
}

void CRViewSSelSafetyValve::_SetCurrentBlowTankSelected( CCellDescriptionProduct *pclCDCurrentBlowTankSelected )
{
	// Try to retrieve sheet description linked to blow tank.
	CSheetDescription *pclSheetDescriptionBlowTank = m_ViewDescription.GetFromSheetDescriptionID( SD_BlowTank );

	if( NULL != pclSheetDescriptionBlowTank )
	{
		pclSheetDescriptionBlowTank->SetUserVariable( _SDUV_SELECTEDPRODUCT, ( LPARAM )pclCDCurrentBlowTankSelected );
	}
}

long CRViewSSelSafetyValve::_GetRowOfEditedBlowTank( CSheetDescription *pclSheetDescriptionBlowTank, CDB_TAProduct *pEditedTAP )
{
	if( NULL == pclSheetDescriptionBlowTank || NULL == pEditedTAP )
	{
		return -1;
	}

	// Retrieve list of all products in 'pclSheetDescription'.
	CSheetDescription::vecCellDescription vecCellDescriptionList;
	pclSheetDescriptionBlowTank->GetCellDescriptionList( vecCellDescriptionList, RVSCellDescription::CD_Product );

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

void CRViewSSelSafetyValve::_ReadAllColumnWidth( CDS_IndSelParameter *pclIndSelParameter )
{
	if( NULL == pclIndSelParameter )
	{
		ASSERT_RETURN;
	}
	
	std::map<UINT, short> mapSDIDVersion;
	mapSDIDVersion[CW_RVIEWSSELSAFETYVALVE_SHEETID_SAFETYVALVE] = CW_RVIEWSSELSAFETYVALVE_SAFETYVALVE_VERSION;
	mapSDIDVersion[CW_RVIEWSSELSAFETYVALVE_SHEETID_BLOWTANK] = CW_RVIEWSSELSAFETYVALVE_BLOWTANK_VERSION;

	// Container window sheet ID to sheetdescription of this rightview.
	std::map<UINT, short> mapCWtoRW;
	mapCWtoRW[CW_RVIEWSSELSAFETYVALVE_SHEETID_SAFETYVALVE] = SD_SafetyValve;
	mapCWtoRW[CW_RVIEWSSELSAFETYVALVE_SHEETID_BLOWTANK] = SD_BlowTank;

	// By default and before reading registry saved column width force reset column width for all sheets.
	for( auto &iter : mapCWtoRW )
	{
		ResetColumnWidth( iter.second );
	}

	// Access to the 'RViewSSelSafetyValve' group.
	CDS_IndSelParameter::CCWWindow *pclCWWindow = pclIndSelParameter->GetWindowColumnWidth( CW_WINDOWID_INDSELSAFETYVALVE, true );

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

void CRViewSSelSafetyValve::_WriteAllColumnWidth( CDS_IndSelParameter *pclIndSelParameter )
{
	if( NULL == pclIndSelParameter )
	{
		ASSERT_RETURN;
	}
	
	std::map<UINT, short> mapSDIDVersion;
	mapSDIDVersion[CW_RVIEWSSELSAFETYVALVE_SHEETID_SAFETYVALVE] = CW_RVIEWSSELSAFETYVALVE_SAFETYVALVE_VERSION;
	mapSDIDVersion[CW_RVIEWSSELSAFETYVALVE_SHEETID_BLOWTANK] = CW_RVIEWSSELSAFETYVALVE_BLOWTANK_VERSION;

	// Container window sheet ID to sheetdescription of this rightview.
	std::map<UINT, short> mapCWtoRW;
	mapCWtoRW[CW_RVIEWSSELSAFETYVALVE_SHEETID_SAFETYVALVE] = SD_SafetyValve;
	mapCWtoRW[CW_RVIEWSSELSAFETYVALVE_SHEETID_BLOWTANK] = SD_BlowTank;

	// Access to the 'RViewSSelSafetyValve' group.
	CDS_IndSelParameter::CCWWindow *pclCWWindow = pclIndSelParameter->GetWindowColumnWidth( CW_WINDOWID_INDSELSAFETYVALVE, true );

	for( auto &iter : mapSDIDVersion )
	{
		CDS_IndSelParameter::CCWSheet *pclCWSheet = pclCWWindow->GetCWSheet( iter.first, true );
		pclCWSheet->SetVersion( iter.second );
		pclCWSheet->GetMap() = m_mapSSheetColumnWidth[mapCWtoRW[iter.first]];
	}
}
