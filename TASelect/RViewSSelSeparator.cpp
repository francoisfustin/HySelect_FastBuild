#include "stdafx.h"
#include "afxctl.h"

#include "TASelect.h"
#include "MainFrm.h"

#include "DlgLeftTabSelManager.h"

#include "ProductSelectionParameters.h"
#include "RViewSSelSeparator.h"

CRViewSSelSeparator *pRViewSSelSeparator = NULL;

CRViewSSelSeparator::CRViewSSelSeparator() : CRViewSSelSS( CMainFrame::RightViewList::eRVSSelSeparator, false )
{
	m_pclIndSelSeparatorParams = NULL;

	m_pCDBExpandCollapseRows = NULL;
	m_pCDBExpandCollapseGroupAccessory = NULL;
	m_pCDBShowAllPriorities = NULL;
	m_lCtrlSelectedRow = 0;
	pRViewSSelSeparator = this;
}

CRViewSSelSeparator::~CRViewSSelSeparator()
{
	pRViewSSelSeparator = NULL;
}

CDB_TAProduct *CRViewSSelSeparator::GetCurrentSeparatorSelected()
{
	CDB_TAProduct *pclCurrentSeparatorSelected = NULL;
	CSheetDescription *pclSheetDescriptionSeparator = m_ViewDescription.GetFromSheetDescriptionID( SD_Separator );

	if( NULL != pclSheetDescriptionSeparator )
	{
		// Retrieve the current selected separator.
		CCellDescriptionProduct *pclCDCurrentSeparatorSelected = NULL;
		LPARAM lpPointer;
		
		if( true == pclSheetDescriptionSeparator->GetUserVariable( _SDUV_SELECTEDPRODUCT, lpPointer ) )
		{
			pclCDCurrentSeparatorSelected = (CCellDescriptionProduct*)lpPointer;
		}

		if( NULL != pclCDCurrentSeparatorSelected && NULL != pclCDCurrentSeparatorSelected->GetProduct() )
		{
			pclCurrentSeparatorSelected = dynamic_cast<CDB_TAProduct*>( (CData*)pclCDCurrentSeparatorSelected->GetProduct() );
		}
	}

	return pclCurrentSeparatorSelected;
}

void CRViewSSelSeparator::Reset()
{
	m_pCDBExpandCollapseRows = NULL;
	m_pCDBExpandCollapseGroupAccessory = NULL;
	m_pCDBShowAllPriorities = NULL;
	m_lCtrlSelectedRow = 0;
	m_vecAccessoryList.clear();
	CRViewSSelSS::Reset();
}

void CRViewSSelSeparator::Suggest( CProductSelelectionParameters *pclProductSelectionParameters, LPARAM lpParam )
{
	CRViewSSelSS::Suggest( pclProductSelectionParameters, lpParam );

	if( NULL == pclProductSelectionParameters || NULL == dynamic_cast<CIndSelSeparatorParams*>( pclProductSelectionParameters ) )
	{
		return;
	}

	m_pclIndSelSeparatorParams = dynamic_cast<CIndSelSeparatorParams*>( pclProductSelectionParameters );

	// To remove all current displayed sheets.
	Reset();

	BeginWaitCursor();
	CWnd::SetRedraw( FALSE );

	long lRowSelected = -1;

	CDS_SSelAirVentSeparator *pclSelectedSeparator = NULL;

	// If we are in edition mode...
	if( NULL != m_pclIndSelSeparatorParams->m_SelIDPtr.MP )
	{
		pclSelectedSeparator = reinterpret_cast<CDS_SSelAirVentSeparator*>( m_pclIndSelSeparatorParams->m_SelIDPtr.MP );

		if( NULL == pclSelectedSeparator )
		{
			ASSERT( 0 );
		}

		lRowSelected = _FillSeparatorRows( pclSelectedSeparator );
	}
	else
	{
		lRowSelected = _FillSeparatorRows();
	}
	
	// Verify if sheet description has been well created.
	CSheetDescription *pclSheetDescription = m_ViewDescription.GetFromSheetDescriptionID( SD_Separator );

	if( NULL != pclSheetDescription && NULL != pclSheetDescription->GetSSheetPointer() )
	{
		CSSheet *pclSSheet = pclSheetDescription->GetSSheetPointer();

		if( lRowSelected > -1 && NULL != pclSelectedSeparator )
		{
			CCellDescriptionProduct *pclCDProduct = FindCDProduct( lRowSelected, (LPARAM)( pclSelectedSeparator->GetProductAs<CDB_TAProduct>() ), 
					pclSheetDescription );

			if( NULL != pclCDProduct && NULL != pclCDProduct->GetProduct() )
			{
				// If we are in edition mode we simulate a click on the product.
				OnClickProduct( pclSheetDescription, pclCDProduct, pclSheetDescription->GetActiveColumn(), lRowSelected );

				// Allow to check if we need to change the 'Show all priorities' button or not.
				CheckShowAllPrioritiesButtonState( pclSheetDescription, lRowSelected );

				// Verify accessories.
				CAccessoryList *pclAccessoryList = pclSelectedSeparator->GetAccessoryList();
				
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
			PrepareAndSetNewFocus( pclSheetDescription, CD_Separator_Name, RD_Separator_FirstAvailRow, 0 );
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

void CRViewSSelSeparator::FillInSelected( CDS_SelProd *pSelectedProductToFill )
{
	if( NULL == m_pclIndSelSeparatorParams || NULL == m_pclIndSelSeparatorParams->m_pclSelectSeparatorList 
			|| NULL == m_pclIndSelSeparatorParams->m_pclSelectSeparatorList->GetSelectPipeList() )
	{
		ASSERT_RETURN;
	}

	CDS_SSelAirVentSeparator *pSelectedSeparatorToFill = dynamic_cast<CDS_SSelAirVentSeparator *>( pSelectedProductToFill );

	if( NULL == pSelectedSeparatorToFill )
	{
		return;
	}

	// Clear previous selected accessories.
	CAccessoryList *pclSeparatorAccessoryListToFill = pSelectedSeparatorToFill->GetAccessoryList();
	bool bIsEditionMode = false;
	if( false == m_pclIndSelSeparatorParams->m_bEditModeRunning )
	{
		pclSeparatorAccessoryListToFill->Clear();
	}
	else
	{
		bIsEditionMode = true;
	}

	// Retrieve the current selected separator if exist.
	CDB_TAProduct *pclCurrentSeparatorSelected = GetCurrentSeparatorSelected();

	if( NULL != pclCurrentSeparatorSelected )
	{
		pSelectedSeparatorToFill->SetProductIDPtr( pclCurrentSeparatorSelected->GetIDPtr() );

		// Search the separator in CSelectList to set the correct Dp.
		for( CSelectedSeparator *pclSelectedSeparator = m_pclIndSelSeparatorParams->m_pclSelectSeparatorList->GetFirst<CSelectedSeparator>(); 
				NULL != pclSelectedSeparator; pclSelectedSeparator = m_pclIndSelSeparatorParams->m_pclSelectSeparatorList->GetNext<CSelectedSeparator>() )
		{
			if( pclSelectedSeparator->GetpData() == pclCurrentSeparatorSelected )	// Product found
			{
				pSelectedSeparatorToFill->SetDp( pclSelectedSeparator->GetDp() );
				break;
			}
		}

		if( false == bIsEditionMode )
		{
			// Retrieve selected accessory and add it.
			for( vecCDCAccessoryListIter vecIter = m_vecAccessoryList.begin(); vecIter != m_vecAccessoryList.end(); vecIter++ )
			{
				CCDBCheckboxAccessory *pCDBCheckboxAccessory = *vecIter;

				if( NULL != pCDBCheckboxAccessory && true == pCDBCheckboxAccessory->GetCheckStatus() && NULL != pCDBCheckboxAccessory->GetAccessoryPointer() )
				{
					pclSeparatorAccessoryListToFill->Add( pCDBCheckboxAccessory->GetAccessoryPointer()->GetIDPtr(), CAccessoryList::_AT_Accessory,
						pCDBCheckboxAccessory->GetRuledTable() );
				}
			}
		}
		else
		{
			UpdateAccessoryList( m_vecAccessoryList, pclSeparatorAccessoryListToFill, CAccessoryList::_AT_Accessory );
		}

		// Selected pipe informations.
		m_pclIndSelSeparatorParams->m_pclSelectSeparatorList->GetSelectPipeList()->GetMatchingPipe( pclCurrentSeparatorSelected->GetSizeKey(), 
				*pSelectedSeparatorToFill->GetpSelPipe() );
	}
}

void CRViewSSelSeparator::OnNewDocument( CDS_IndSelParameter *pclIndSelParameter )
{
	_ReadAllColumnWidth( pclIndSelParameter );
}

void CRViewSSelSeparator::SaveSelectionParameters( CDS_IndSelParameter *pclIndSelParameter )
{
	_WriteAllColumnWidth( pclIndSelParameter );
}

bool CRViewSSelSeparator::OnTextTipFetch( CSheetDescription *pclSheetDescription, long lColumn, long lRow, WORD *pwMultiLine, 
		SHORT *pnTipWidth, TCHAR *pstrTipText, BOOL *bfShowTip )
{
	if( NULL == m_pclIndSelSeparatorParams || NULL == m_pclIndSelSeparatorParams->m_pclSelectSeparatorList )
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
	CSelectedSeparator *pclSelectedSeparator = GetSelectProduct<CSelectedSeparator>( pTAP, m_pclIndSelSeparatorParams->m_pclSelectSeparatorList );
	CSSheet *pclSSheet = pclSheetDescription->GetSSheetPointer();
	CDS_TechnicalParameter *pclTechParam = m_pclIndSelSeparatorParams->m_pTADS->GetpTechParams();

	switch( pclSheetDescription->GetSheetDescriptionID() )
	{
		// Check if mouse cursor has passed over a valve.
		case SD_Separator:

			if( NULL != pclSelectedSeparator )
			{
				if( CD_Separator_Dp == lColumn )
				{
					// Check if flag is set.
					if( true == pclSelectedSeparator->IsFlagSet( CSelectedBase::eDp ) )
					{
						// TODO.
					}
				}
				else if( CD_Separator_TemperatureRange == lColumn )
				{
					if( true == pclSelectedSeparator->IsFlagSet( CSelectedBase::eTemperature ) )
					{
						FormatString( str, IDS_SSHEETSSEL_TEMPERROR, pTAP->GetTempRange() );
					}
				}
				else if( CD_Separator_PipeLinDp == lColumn )
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
				else if( CD_Separator_PipeV == lColumn )
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
		*bfShowTip = true;
		bReturnValue = true;
	}

	return bReturnValue;
}

bool CRViewSSelSeparator::OnClickProduct( CSheetDescription *pclSheetDescriptionSeparator, CCellDescriptionProduct *pclCellDescriptionProduct, 
		long lColumn, long lRow )
{
	if( NULL == pclSheetDescriptionSeparator || NULL == pclSheetDescriptionSeparator->GetSSheetPointer() || NULL == pclCellDescriptionProduct )
	{
		return false;
	}

	CDB_TAProduct *pclSeparator = dynamic_cast<CDB_TAProduct *>( (CData *)pclCellDescriptionProduct->GetProduct() );

	if( NULL == pclSeparator )
	{
		return false;
	}

	CSSheet *pclSSheet = pclSheetDescriptionSeparator->GetSSheetPointer();
		
	// Memorize previous focus row.
	long lCurrentFocusedRow = GetCurrentFocusedRow();

	// To be sure to clean all previous data concerning focus.
	KillCurrentFocus();

	// Unselect row where current selection is set (yellow light).
	// Remark: 'KillCurrentFocus' unselect only rows where focus is on (blue light).
	pclSSheet->UnSelectMultipleRows();

	// By default clear accessories list.
	m_vecAccessoryList.clear();

	LPARAM lSeparatorCount;
	pclSheetDescriptionSeparator->GetUserVariable( _SDUV_TOTALPRODUCT_COUNT, lSeparatorCount );

	// Retrieve the current selected separator if exist.
	CDB_TAProduct* pclCurrentSeparatorSelected = GetCurrentSeparatorSelected();

	// If there is already one separator selected and user clicks on the current one...
	// Remark: 'm_pCDBExpandCollapseRows' is not created if there is only one separator. Thus we need to check first if there is only one separator.
	//         Otherwise we have to check if user has clicked on the current one.
	if( NULL != pclCurrentSeparatorSelected 
			&& ( ( 1 == lSeparatorCount) || ( NULL != m_pCDBExpandCollapseRows && lRow == m_pCDBExpandCollapseRows->GetCellPosition().y ) ) )
	{
		// Reset current product selected.
		SetCurrentSeparatorSelected( NULL );

		// Delete Expand/Collapse rows button if exist.
		// Remark: 'm_pCDBExpandCollapseRows' is set to NULL in method!
		DeleteExpandCollapseRowsButton( m_pCDBExpandCollapseRows, pclSheetDescriptionSeparator );

		// Show Show/Hide all priorities button if exist.
		if( NULL != m_pCDBShowAllPriorities )
		{
			m_pCDBShowAllPriorities->ApplyInternalChange();
		}

		// Set focus on separator currently selected.
		PrepareAndSetNewFocus( pclSheetDescriptionSeparator, pclSheetDescriptionSeparator->GetActiveColumn(), lRow, 0 );

		// Uncheck checkbox.
		pclSSheet->SetCheckBox( CD_Separator_CheckBox, lRow, _T(""), false, true );

		m_lCtrlSelectedRow = 0;

		// Remove all sheets after separator.
		m_pCDBExpandCollapseGroupAccessory = NULL;
		m_ViewDescription.RemoveAllSheetAfter( SD_Separator );

		// Notify a change in the selection.
		SelectionHasChanged( IsSelectionReady() );
	}
	else
	{
		// If a separator is already selected...
		if( NULL != pclCurrentSeparatorSelected )
		{
			// Remove all sheets after separator.
			m_pCDBExpandCollapseGroupAccessory = NULL;
			m_ViewDescription.RemoveAllSheetAfter( SD_Separator );

			// Uncheck separator.
			pclSSheet->SetCheckBox( CD_Separator_CheckBox, m_lCtrlSelectedRow, _T(""), false, true );
		}

		// Save new separator selection.
		SetCurrentSeparatorSelected( pclCellDescriptionProduct );

		m_lCtrlSelectedRow = lRow;

		// Check separator.
		pclSSheet->SetCheckBox( CD_Separator_CheckBox, m_lCtrlSelectedRow, _T(""), true, true );

		// Delete Expand/Collapse rows button if exist.
		if( NULL != m_pCDBExpandCollapseRows )
		{
			// Remark: 'm_pCDBExpandCollapseRows' is set to NULL in method!
			DeleteExpandCollapseRowsButton( m_pCDBExpandCollapseRows, pclSheetDescriptionSeparator );
		}

		// Create Expand/Collapse rows button if needed...
		LPARAM lValveTotalCount;
		pclSheetDescriptionSeparator->GetUserVariable( _SDUV_TOTALPRODUCT_COUNT, lValveTotalCount );
		
		if( lValveTotalCount > 1 )
		{
			m_pCDBExpandCollapseRows = CreateExpandCollapseRowsButton( CD_Separator_FirstColumn, lRow, true, 
					CCDButtonExpandCollapseRows::ButtonState::CollapseRow,  pclSheetDescriptionSeparator->GetFirstSelectableRow(), 
					pclSheetDescriptionSeparator->GetLastSelectableRow( false ), pclSheetDescriptionSeparator );

			// Show button.
			if( NULL != m_pCDBExpandCollapseRows )
			{
				m_pCDBExpandCollapseRows->SetShowStatus( true );
			}
		}

		// Select separator (just highlight background).
		pclSSheet->SelectOneRow( lRow, pclSheetDescriptionSeparator->GetSelectionFrom(), pclSheetDescriptionSeparator->GetSelectionTo() );

		// Fill corresponding accessories.
		_FillAccessoryRows();

		// Try to set the focus on the next table but do not lose the focus if no other table exist.
		CSheetDescription *pclNextSheetDescription = NULL;
		long lNewFocusedRow;
		bool fShiftPressed;
		
		if( DNFERRC_FocusOnNextGroupOK == DefineNextFocusedRow( pclSheetDescriptionSeparator, CD_Separator_FirstColumn, lRow, 
				false, lNewFocusedRow, pclNextSheetDescription, fShiftPressed ) )
		{
			long lNewFocusedCol = pclNextSheetDescription->GetFirstParameterColumn();
			PrepareAndSetNewFocus( pclNextSheetDescription, lNewFocusedCol, lNewFocusedRow );
		}
		else
		{
			// Because we have changed current selection background in yellow, if there is no other sheet following current one, we
			// must set focus on current selection.
			PrepareAndSetNewFocus( pclSheetDescriptionSeparator, lColumn, lRow );
		}

		// Notify a change in the selection.
		SelectionHasChanged( IsSelectionReady() );
	}

	return true;
}

bool CRViewSSelSeparator::ResetColumnWidth( short nSheetDescriptionID )
{
	TSpread clTSpread;
	
	if( FALSE == clTSpread.Create( WS_CHILD, CRect( 0, 0, 0, 0 ), this, 0 ) )
	{
		ASSERTA_RETURN( false );
	}
	
	switch( nSheetDescriptionID )
	{
		case SD_Separator:
			m_mapSSheetColumnWidth[SD_Separator][CD_Separator_FirstColumn] = clTSpread.ColWidthToLogUnits( 4 );
			m_mapSSheetColumnWidth[SD_Separator][CD_Separator_CheckBox] = clTSpread.ColWidthToLogUnits( 2 );
			m_mapSSheetColumnWidth[SD_Separator][CD_Separator_Name] = clTSpread.ColWidthToLogUnits( 20 );
			m_mapSSheetColumnWidth[SD_Separator][CD_Separator_Type] = clTSpread.ColWidthToLogUnits( 18 );
			m_mapSSheetColumnWidth[SD_Separator][CD_Separator_Material] = clTSpread.ColWidthToLogUnits( 12 );
			m_mapSSheetColumnWidth[SD_Separator][CD_Separator_Connection] = clTSpread.ColWidthToLogUnits( 14 );
			m_mapSSheetColumnWidth[SD_Separator][CD_Separator_Version] = clTSpread.ColWidthToLogUnits( 18 );
			m_mapSSheetColumnWidth[SD_Separator][CD_Separator_PN] = clTSpread.ColWidthToLogUnits( 6 );
			m_mapSSheetColumnWidth[SD_Separator][CD_Separator_Size] = clTSpread.ColWidthToLogUnits( 10 );
			m_mapSSheetColumnWidth[SD_Separator][CD_Separator_Dp] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_Separator][CD_Separator_TemperatureRange] = clTSpread.ColWidthToLogUnits( 8 );
			m_mapSSheetColumnWidth[SD_Separator][CD_Separator_Separator] = clTSpread.ColWidthToLogUnits( 1 );
			m_mapSSheetColumnWidth[SD_Separator][CD_Separator_PipeSize] = clTSpread.ColWidthToLogUnits( 8 );
			m_mapSSheetColumnWidth[SD_Separator][CD_Separator_PipeLinDp] = clTSpread.ColWidthToLogUnits( 8 );
			m_mapSSheetColumnWidth[SD_Separator][CD_Separator_PipeV] = clTSpread.ColWidthToLogUnits( 8 );
			m_mapSSheetColumnWidth[SD_Separator][CD_Separator_Pointer] = clTSpread.ColWidthToLogUnits( 1 );
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

bool CRViewSSelSeparator::IsSelectionReady()
{
	CDB_TAProduct *pSelection = GetCurrentSeparatorSelected();

	if ( NULL != pSelection )
	{
		return ( false == pSelection->IsDeleted() );
	}

	return false; // No selection available
}

void CRViewSSelSeparator::SetCurrentSeparatorSelected( CCellDescriptionProduct *pclCDCurrentSeparatorSelected )
{
	// Try to retrieve sheet description linked to separator.
	CSheetDescription *pclSheetDescriptionSeparator = m_ViewDescription.GetFromSheetDescriptionID( SD_Separator );

	if( NULL != pclSheetDescriptionSeparator )
	{
		pclSheetDescriptionSeparator->SetUserVariable( _SDUV_SELECTEDPRODUCT, (LPARAM)pclCDCurrentSeparatorSelected );
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PRIVATE MEMBERS
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

long CRViewSSelSeparator::_FillSeparatorRows( CDS_SSelAirVentSeparator *pEditedProduct )
{
	if( NULL == m_pclIndSelSeparatorParams || NULL == m_pclIndSelSeparatorParams->m_pclSelectSeparatorList
			|| NULL == m_pclIndSelSeparatorParams->m_pclSelectSeparatorList->GetSelectPipeList() || NULL == m_pclIndSelSeparatorParams->m_pTADS
			|| NULL == m_pclIndSelSeparatorParams->m_pPipeDB )
	{
		ASSERTA_RETURN( 0 );
	}

	// Call 'MultiSpreadBase' method to get a new SSheet.
	CSheetDescription *pclSheetDescriptionSeparator = CreateSSheet( SD_Separator );

	if( NULL == pclSheetDescriptionSeparator || NULL == pclSheetDescriptionSeparator->GetSSheetPointer() )
	{
		return 0;
	}
	
	CSSheet *pclSSheet = pclSheetDescriptionSeparator->GetSSheetPointer();

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
	CSelectedSeparator *pclSelectedSeparator = m_pclIndSelSeparatorParams->m_pclSelectSeparatorList->GetFirst<CSelectedSeparator>();

	if( NULL == pclSelectedSeparator )
	{
		return 0;
	}

	CDB_TAProduct *pTAP = dynamic_cast<CDB_TAProduct *>( pclSelectedSeparator->GetpData() );

	if( NULL == pTAP )
	{
		return 0;
	}

	bool bValveSettingExist = ( NULL != pTAP->GetValveCharacteristic() );

	// Set max rows.
	pclSSheet->SetMaxRows( RD_Separator_FirstAvailRow - 1 );
	pclSheetDescriptionSeparator->SetSelectableRangeRow( RD_Separator_FirstAvailRow, pclSSheet->GetMaxRows() - 1 );	
	
	// All cells are static by default and filled with _T("").
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	pclSSheet->FormatStaticText( -1, -1, -1, -1, _T("") );
	
	// Increase row height.
	double RowHeight = 12.75;
	pclSSheet->SetRowHeight( RD_Separator_FirstRow, RowHeight * 0.5 );
	pclSSheet->SetRowHeight( RD_Separator_GroupName, RowHeight * 1.5 );
	pclSSheet->SetRowHeight( RD_Separator_ColName, RowHeight * 2 );
	pclSSheet->SetRowHeight( RD_Separator_Unit, RowHeight * 1.2 );
	
	// Initialize.
	pclSheetDescriptionSeparator->Init( 1, pclSSheet->GetMaxRows(), 1, CSheetDescription::SD_ParameterMode_Mono );
	pclSheetDescriptionSeparator->AddColumnInPixels( CD_Separator_FirstColumn, m_mapSSheetColumnWidth[SD_Separator][CD_Separator_FirstColumn] );
	pclSheetDescriptionSeparator->AddColumnInPixels( CD_Separator_CheckBox, m_mapSSheetColumnWidth[SD_Separator][CD_Separator_CheckBox] );
	pclSheetDescriptionSeparator->AddColumnInPixels( CD_Separator_Name, m_mapSSheetColumnWidth[SD_Separator][CD_Separator_Name] );
	pclSheetDescriptionSeparator->AddColumnInPixels( CD_Separator_Type, m_mapSSheetColumnWidth[SD_Separator][CD_Separator_Type] );
	pclSheetDescriptionSeparator->AddColumnInPixels( CD_Separator_Material, m_mapSSheetColumnWidth[SD_Separator][CD_Separator_Material] );
	pclSheetDescriptionSeparator->AddColumnInPixels( CD_Separator_Connection, m_mapSSheetColumnWidth[SD_Separator][CD_Separator_Connection] );
	pclSheetDescriptionSeparator->AddColumnInPixels( CD_Separator_Version, m_mapSSheetColumnWidth[SD_Separator][CD_Separator_Version] );
	pclSheetDescriptionSeparator->AddColumnInPixels( CD_Separator_PN, m_mapSSheetColumnWidth[SD_Separator][CD_Separator_PN] );
	pclSheetDescriptionSeparator->AddColumnInPixels( CD_Separator_Size, m_mapSSheetColumnWidth[SD_Separator][CD_Separator_Size] );
	pclSheetDescriptionSeparator->AddColumnInPixels( CD_Separator_Dp, m_mapSSheetColumnWidth[SD_Separator][CD_Separator_Dp] );
	pclSheetDescriptionSeparator->AddColumnInPixels( CD_Separator_TemperatureRange, m_mapSSheetColumnWidth[SD_Separator][CD_Separator_TemperatureRange] );
	pclSheetDescriptionSeparator->AddColumnInPixels( CD_Separator_Separator, m_mapSSheetColumnWidth[SD_Separator][CD_Separator_Separator] );
	pclSheetDescriptionSeparator->AddColumnInPixels( CD_Separator_PipeSize, m_mapSSheetColumnWidth[SD_Separator][CD_Separator_PipeSize] );
	pclSheetDescriptionSeparator->AddColumnInPixels( CD_Separator_PipeLinDp, m_mapSSheetColumnWidth[SD_Separator][CD_Separator_PipeLinDp] );
	pclSheetDescriptionSeparator->AddColumnInPixels( CD_Separator_PipeV, m_mapSSheetColumnWidth[SD_Separator][CD_Separator_PipeV] );
	pclSheetDescriptionSeparator->AddColumnInPixels( CD_Separator_Pointer, m_mapSSheetColumnWidth[SD_Separator][CD_Separator_Pointer] );

	// Set in which column parameter must be saved.
	pclSheetDescriptionSeparator->AddParameterColumn( CD_Separator_Pointer );
	
	// Set the focus column.
	pclSheetDescriptionSeparator->SetActiveColumn( CD_Separator_Name );

	// Set range for selection.
	pclSheetDescriptionSeparator->SetFocusColumnRange( CD_Separator_CheckBox, CD_Separator_TemperatureRange );

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

	pclSSheet->SetStaticText( CD_Separator_Name, RD_Separator_ColName, IDS_SSHEETSSEL_NAME );
	pclSSheet->SetStaticText( CD_Separator_Type, RD_Separator_ColName, IDS_SSHEETSSEL_TYPE );
	pclSSheet->SetStaticText( CD_Separator_Material, RD_Separator_ColName, IDS_SSHEETSSEL_MATERIAL );
	pclSSheet->SetStaticText( CD_Separator_Connection, RD_Separator_ColName, IDS_SSHEETSSEL_CONNECT );
	pclSSheet->SetStaticText( CD_Separator_Version, RD_Separator_ColName, IDS_SSHEETSSEL_VERSION );
	pclSSheet->SetStaticText( CD_Separator_PN, RD_Separator_ColName, IDS_SSHEETSSEL_PN );
	pclSSheet->SetStaticText( CD_Separator_Size, RD_Separator_ColName, IDS_SSHEETSSEL_SIZE );
	pclSSheet->SetStaticText( CD_Separator_Dp, RD_Separator_ColName, IDS_SSHEETSSEL_DP );
	pclSSheet->SetStaticText( CD_Separator_TemperatureRange, RD_Separator_ColName, IDS_SSHEETSSEL_COLTEMPRANGE );
	pclSSheet->SetStaticText( CD_Separator_Separator, RD_Separator_ColName, _T("") );

	// Pipes.
	pclSSheet->SetStaticText( CD_Separator_PipeSize, RD_Separator_ColName, IDS_SSHEETSSEL_PIPESIZE );
	pclSSheet->SetStaticText( CD_Separator_PipeLinDp, RD_Separator_ColName, IDS_SSHEETSSEL_PIPELINDP );
	pclSSheet->SetStaticText( CD_Separator_PipeV, RD_Separator_ColName, IDS_SSHEETSSEL_PIPEV );
	//
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Row units.
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();
	pclSSheet->SetStaticText( CD_Separator_Dp, RD_Separator_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ) ).c_str() );
	pclSSheet->SetStaticText( CD_Separator_TemperatureRange, RD_Separator_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_TEMPERATURE ) ).c_str() );

	// Units.
	pclSSheet->SetStaticText( CD_Separator_PipeLinDp, RD_Separator_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_LINPRESSDROP ) ).c_str() );
	pclSSheet->SetStaticText( CD_Separator_PipeV, RD_Separator_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_VELOCITY ) ).c_str() );
	//
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Draw black line below column header.
	pclSSheet->SetCellBorder( CD_Separator_CheckBox, RD_Separator_Unit, CD_Separator_Separator - 1, RD_Separator_Unit, true, SS_BORDERTYPE_BOTTOM );
	pclSSheet->SetCellBorder( CD_Separator_PipeSize, RD_Separator_Unit, CD_Separator_Pointer - 1, RD_Separator_Unit, true, SS_BORDERTYPE_BOTTOM );

	CSelectPipe selPipe( m_pclIndSelSeparatorParams );
	long lRetRow = -1;

	CDS_TechnicalParameter *pTechParam = m_pclIndSelSeparatorParams->m_pTADS->GetpTechParams();
	
	long lRow = RD_Separator_FirstAvailRow;

	long lSeparatorTotalCount = 0;
	long lSeparatorNotPriorityCount = 0;
	bool bShowAllPrioritiesShown = false;
	long lShowAllPrioritiesButtonRow = 0;
	bool bAtLeastOneError = false;
	bool bAtLeastOneWarning = false;
	bool bAtLestOneDpValue = false;

	pclSheetDescriptionSeparator->SetUserVariable( _SDUV_TOTALPRODUCT_COUNT, 0 );
	pclSheetDescriptionSeparator->SetUserVariable( _SDUV_NOPRIORITYPRODUCT_COUNT, 0 );

	pclSheetDescriptionSeparator->RestartRemarkGenerator();
	
	for( CSelectedSeparator *pclSelectedSeparator = m_pclIndSelSeparatorParams->m_pclSelectSeparatorList->GetFirst<CSelectedSeparator>(); 
			NULL != pclSelectedSeparator; pclSelectedSeparator = m_pclIndSelSeparatorParams->m_pclSelectSeparatorList->GetNext<CSelectedSeparator>() )
	{
		CDB_TAProduct *pTAP = dynamic_cast<CDB_TAProduct *>( pclSelectedSeparator->GetpData() );

		if( NULL == pTAP ) 
		{
			continue;
		}

		bAtLeastOneError = false;
		bAtLeastOneWarning = false;
		bool bBest = pclSelectedSeparator->IsFlagSet(CSelectedBase::eBest );
		
		if( true == pclSelectedSeparator->IsFlagSet(CSelectedBase::eNotPriority) )
		{
			lSeparatorNotPriorityCount++;
			
			if( false == bShowAllPrioritiesShown )
			{
				// Add one empty not selectable row.
				pclSheetDescriptionSeparator->AddRows( 1 );
				pclSheetDescriptionSeparator->RemoveSelectableRow( lRow );

				pclSSheet->SetCellBorder( CD_Separator_Name, lRow, CD_Separator_Separator - 1, lRow, true, SS_BORDERTYPE_TOP | SS_BORDERTYPE_BOTTOM, 
						SS_BORDERSTYLE_SOLID, _BLACK );

				pclSSheet->SetCellBorder( CD_Separator_PipeSize, lRow, CD_Separator_Pointer - 1, lRow, true, SS_BORDERTYPE_TOP | SS_BORDERTYPE_BOTTOM, 
						SS_BORDERSTYLE_SOLID, _BLACK );

				lShowAllPrioritiesButtonRow = lRow++;
				bShowAllPrioritiesShown = true;
			}
		}

		// Add this row.
		// 'true' to specify that this row can be selected.
		pclSheetDescriptionSeparator->AddRows( 1, true );

		// First columns will be set at the end!

		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

		// Set Dp.
		if( true == pclSelectedSeparator->IsFlagSet( CSelectedBase::eDp ) )
		{
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_ORANGE );
			bAtLeastOneWarning = true;
		}
		
		CString strDp = _T("-");

		if( -1.0 != pclSelectedSeparator->GetDp() )
		{
			strDp = WriteCUDouble( _U_DIFFPRESS, pclSelectedSeparator->GetDp() );
			bAtLestOneDpValue = true;
		}

		pclSSheet->SetStaticText( CD_Separator_Dp, lRow, strDp );
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

		// Temperature range.
		if( true == pclSelectedSeparator->IsFlagSet( CSelectedBase::eTemperature ) )
		{
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_RED );
			bAtLeastOneError = true;
		}
		
		pclSSheet->SetStaticText( CD_Separator_TemperatureRange, lRow, ( (CDB_TAProduct *)pclSelectedSeparator->GetProductIDPtr().MP )->GetTempRange() );
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

		// Now we can set first columns in regards to current status (error, best or normal).
		if( true == bAtLeastOneError )
		{
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_RED );
		}
		else if( true == pclSelectedSeparator->IsFlagSet( CSelectedBase::eBest ) )
		{
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_GREEN );
		}
		
		// Add checkbox.
		pclSSheet->SetCheckBox( CD_Separator_CheckBox, lRow, _T(""), false, true );
		pclSSheet->SetStaticText( CD_Separator_Name, lRow, pTAP->GetName() );

		if( true == bAtLeastOneError )
		{
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );
		}
		else if( true == bBest )
		{
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_GREEN );
		}

		pclSSheet->SetStaticText( CD_Separator_Type, lRow, pTAP->GetType() );
		pclSSheet->SetStaticText( CD_Separator_Material, lRow, pTAP->GetBodyMaterial() );
		pclSSheet->SetStaticText( CD_Separator_Connection, lRow, pTAP->GetConnect() );
		pclSSheet->SetStaticText( CD_Separator_Version, lRow, pTAP->GetVersion() );
		pclSSheet->SetStaticText( CD_Separator_PN, lRow, pTAP->GetPN().c_str() );
		pclSSheet->SetStaticText( CD_Separator_Size, lRow, pTAP->GetSize() );
		
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );
		// Set pipe size.
		m_pclIndSelSeparatorParams->m_pclSelectSeparatorList->GetSelectPipeList()->GetMatchingPipe( pTAP->GetSizeKey(), selPipe );
		pclSSheet->SetStaticText( CD_Separator_PipeSize, lRow, selPipe.GetpPipe()->GetName() );
		
		// Set the LinDp to orange if it is above or below the technical parameters limits.
		if( selPipe.GetLinDp() > pTechParam->GetPipeMaxDp() || selPipe.GetLinDp() < pTechParam->GetPipeMinDp())
		{
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_ORANGE );
		}
		
		pclSSheet->SetStaticText( CD_Separator_PipeLinDp, lRow, WriteCUDouble( _U_LINPRESSDROP, selPipe.GetLinDp() ) );
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

		// Set the Velocity to orange if it is above the technical parameter limit.
		// Orange if it is below the dMinVel.
		if( selPipe.GetU() > pTechParam->GetPipeMaxVel() || selPipe.GetU() < pTechParam->GetPipeMinVel() )
		{
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_ORANGE );
		}
		
		pclSSheet->SetStaticText( CD_Separator_PipeV, lRow, WriteCUDouble( _U_VELOCITY, selPipe.GetU() ) );
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

		// Save parameter.
		CreateCellDescriptionProduct( pclSheetDescriptionSeparator->GetFirstParameterColumn(), lRow, (LPARAM)pTAP, pclSheetDescriptionSeparator );
		
		if( NULL != pEditedTAP && pEditedTAP == pTAP )
		{
			lRetRow = lRow;
		}
		
		// Save parameter.
		CreateCellDescriptionProduct( pclSheetDescriptionSeparator->GetFirstParameterColumn(), lRow, (LPARAM)pTAP, pclSheetDescriptionSeparator );
		
		if( NULL != pEditedTAP && pEditedTAP == pTAP )
		{
			lRetRow = lRow;
		}

		pclSSheet->SetCellBorder( CD_Separator_CheckBox, lRow, CD_Separator_Separator - 1, lRow, true, SS_BORDERTYPE_BOTTOM,SS_BORDERSTYLE_SOLID, _GRAY );
		pclSSheet->SetCellBorder( CD_Separator_PipeSize, lRow, CD_Separator_Pointer - 1, lRow, true, SS_BORDERTYPE_BOTTOM,SS_BORDERSTYLE_SOLID, _GRAY );

		lRow++;
		lSeparatorTotalCount++;
	}

	long lLastDataRow = lRow - 1;

	// Hide columns corresponding to user selected combos.
	pclSSheet->ShowCol( CD_Separator_Type, ( true == m_pclIndSelSeparatorParams->m_strComboTypeID.IsEmpty() ) ? TRUE : FALSE );	
	pclSSheet->ShowCol( CD_Separator_Connection, (true == m_pclIndSelSeparatorParams->m_strComboConnectID.IsEmpty() ) ? TRUE : FALSE );
	pclSSheet->ShowCol( CD_Separator_Version, ( true == m_pclIndSelSeparatorParams->m_strComboVersionID.IsEmpty() ) ? TRUE : FALSE );
	pclSSheet->ShowCol( CD_Separator_Dp, ( true == bAtLestOneDpValue ) ? TRUE : FALSE );

	pclSheetDescriptionSeparator->SetUserVariable( _SDUV_TOTALPRODUCT_COUNT, lSeparatorTotalCount );
	pclSheetDescriptionSeparator->SetUserVariable( _SDUV_NOPRIORITYPRODUCT_COUNT, lSeparatorNotPriorityCount );

	// Now we can create Show/Hide all priorities if needed.
	// Remarks: 2nd condition to avoid to have button if we have only no priority valves.
	if( true == bShowAllPrioritiesShown && lSeparatorTotalCount > lSeparatorNotPriorityCount )
	{
		// Create Show/Hide all priorities button in regards to current state!
		CCDButtonShowAllPriorities::ButtonState eState = ( true == GetShowAllPrioritiesFlag( SD_Separator ) ) ? 
				CCDButtonShowAllPriorities::ButtonState::HidePriorities : CCDButtonShowAllPriorities::ButtonState::ShowPriorities;
		
		CCDButtonShowAllPriorities *pclShowAllButton = CreateShowAllPrioritiesButton( CD_Separator_FirstColumn, lShowAllPrioritiesButtonRow, 
				bShowAllPrioritiesShown, eState, lShowAllPrioritiesButtonRow + 1, pclSSheet->GetMaxRows(), pclSheetDescriptionSeparator );

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

	// Try to merge only if there is more than one separator.
	if( lSeparatorTotalCount > 2 || ( 2 == lSeparatorTotalCount && lSeparatorNotPriorityCount != 1 ) )
	{
		vector<long> vecColumnList;
		vecColumnList.push_back( CD_Separator_PipeSize );
		vecColumnList.push_back( CD_Separator_PipeLinDp );
		vecColumnList.push_back( CD_Separator_PipeV );
		pclSSheet->MergeRowsWithSameValues( RD_Separator_FirstAvailRow, lLastDataRow, vecColumnList );
	}

	pclSSheet->SetCellBorder( CD_Separator_CheckBox, lLastDataRow, CD_Separator_Separator - 1, lLastDataRow, true, SS_BORDERTYPE_BOTTOM,
			SS_BORDERSTYLE_SOLID, _BLACK );

	// Needed by pipe merging.
	pclSSheet->SetCellBorder( CD_Separator_PipeSize, lLastDataRow, CD_Separator_Pointer - 1, lLastDataRow, true, SS_BORDERTYPE_BOTTOM,
			SS_BORDERSTYLE_SOLID, _BLACK );

	// Add Remarks.
	// Note: these lines must be placed after creating Show/Hide all priorities button to avoid to take in count these lines when defining
	//       range of row to be Show/Hide.
	lRow = pclSheetDescriptionSeparator->WriteRemarks( lRow, CD_Separator_CheckBox, CD_Separator_Separator );

	// Add the main title.
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)GetTitleBackgroundColor( pclSheetDescriptionSeparator ) );
	pclSheetDescriptionSeparator->SetUserVariable( _SDUV_TITLEFORECOLOR, ( pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor ) ) );
	pclSheetDescriptionSeparator->SetUserVariable( _SDUV_TITLEBACKCOLOR, ( pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor ) ) );

	pclSSheet->AddCellSpanW( CD_Separator_CheckBox, RD_Separator_GroupName, CD_Separator_Separator - CD_Separator_CheckBox, 1 );
	pclSSheet->SetStaticText( CD_Separator_CheckBox, RD_Separator_GroupName, IDS_SSHEETSSELSEP_SEPGROUP );
	
	pclSSheet->AddCellSpanW( CD_Separator_PipeSize, RD_Separator_GroupName, CD_Separator_Pointer - CD_Separator_PipeSize, 1 );
	
	CTable *pclTable = dynamic_cast<CTable *>( m_pclIndSelSeparatorParams->m_pPipeDB->Get( m_pclIndSelSeparatorParams->m_strPipeSeriesID ).MP );
	ASSERT( NULL != pclTable );

	pclSSheet->SetStaticText( CD_Separator_PipeSize, RD_Separator_GroupName, pclTable->GetName() );
	
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

	pclSSheet->SetBool( SSB_REDRAW, TRUE );

	// Set that there is no selection at now.
	SetCurrentSeparatorSelected( NULL );

	// Add possibility to change column size.
	// Remark: place this code after any initialization (like adding, removing, resizing, hiding/showing columns).
	CResizingColumnInfo* pclResizingColumnInfo = pclSheetDescriptionSeparator->GetResizingColumnInfo();
	pclResizingColumnInfo->ActivateFeature( true, false, RD_Separator_FirstAvailRow, lLastDataRow );
	pclResizingColumnInfo->AddRangeColumn( CD_Separator_Name, CD_Separator_TemperatureRange, RD_Separator_ColName, RD_Separator_Unit );
	pclResizingColumnInfo->AddRangeColumn( CD_Separator_PipeSize, CD_Separator_PipeV, RD_Separator_ColName, RD_Separator_Unit );

	// Add possibility to click on title group to reset column width.
	CreateTitleGroupButtons( CD_Separator_CheckBox, CD_Separator_PipeV, RD_Separator_GroupName, pclSheetDescriptionSeparator );

	return lRetRow;
}

void CRViewSSelSeparator::_FillAccessoryRows()
{
	if( NULL == m_pclIndSelSeparatorParams || NULL == m_pclIndSelSeparatorParams->m_pTADB )
	{
		ASSERT_RETURN;
	}

	// Retrieve the current selected separator.
	CDB_TAProduct *pclCurrentSeparatorSelected = GetCurrentSeparatorSelected();

	if( NULL == pclCurrentSeparatorSelected )
	{
		return;
	}

	CDB_RuledTable *pRuledTable = (CDB_RuledTable *)( pclCurrentSeparatorSelected->GetAccessoriesGroupIDPtr().MP );

	if( NULL == pRuledTable )
	{
		return;
	}

	CRank rList;
	int iCount = m_pclIndSelSeparatorParams->m_pTADB->GetAccessories( &rList, pRuledTable, m_pclIndSelSeparatorParams->m_eFilterSelection );

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

	// All cells are static by default and filled with _T("").
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	pclSSheet->FormatStaticText( -1, -1, -1, -1, _T("") );

	// Get sheet description of separator to retrieve width.
	CSheetDescription *pclSDSeparator = m_ViewDescription.GetFromSheetDescriptionID( SD_Separator );
	// It's absolutely not normal to have this pointer NULL.
	ASSERT( NULL != pclSDSeparator );
	
	// Take SSheet size and client area size and determine the right width to take in count.
	CRect rectSize = pclSDSeparator->GetSSheetPointer()->GetSheetSizeInPixels();

	// Prepare first column width (to match separator sheet).
	long lFirstColumnWidth = pclSDSeparator->GetSSheetPointer()->GetColWidthInPixelsW( CD_Separator_FirstColumn );

	// Do some correction in regards to some columns.
	long lTotalWidth = rectSize.Width() - lFirstColumnWidth;
	lTotalWidth -= pclSDSeparator->GetSSheetPointer()->GetColWidthInPixelsW( CD_Separator_Pointer );

	// Try to create 2 columns in just the middle of the separator sheet.
	long lLeftWidth = (long)( lTotalWidth / 2 );
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
	
	// Set the focus column (don't set on Left of Right (on a check box) ).
	pclSDAccessory->SetActiveColumn( CD_Accessory_FirstColumn );
	
	// Set selectable rows.
	pclSDAccessory->SetSelectableRangeRow( RD_Accessory_FirstAvailRow, pclSSheet->GetMaxRows() - 1 );

	// Increase row height.
	double RowHeight = 12.75;
	pclSSheet->SetRowHeight( RD_Accessory_FirstRow, RowHeight * 0.5 );
	pclSSheet->SetRowHeight( RD_Accessory_GroupName, RowHeight * 1.5 );

	// Set title.
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)GetTitleBackgroundColor( pclSDAccessory ) );
	
	pclSDAccessory->SetUserVariable( _SDUV_TITLEFORECOLOR, ( pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor ) ) );
	pclSDAccessory->SetUserVariable( _SDUV_TITLEBACKCOLOR, ( pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor ) ) );

	pclSSheet->AddCellSpanW( CD_Accessory_Left, RD_Accessory_GroupName, CD_Accessory_LastColumn - CD_Accessory_Left, 1 );
	pclSSheet->SetStaticText( CD_Accessory_Left, RD_Accessory_GroupName, IDS_SSHEETSSELSEP_SEPACCGROUP );

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
		CCDBCheckboxAccessory *pCheckbox = CreateCheckboxAccessory( lLeftOrRight, lRow, false, true, strName, pAccessory, 
				pRuledTable, &m_vecAccessoryList, pclSDAccessory );
		
		if( NULL != pCheckbox )
		{
			pCheckbox->ApplyInternalChange();
		}
		
		// Description.
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleEllipses, (LPARAM)TRUE );
		pclSSheet->SetStaticText( lLeftOrRight, lRow + 1, pAccessory->GetComment() );
		bContinue = rList.GetNext( str, lparam );

		// Restart left part.
		if( bContinue && lLeftOrRight == CD_Accessory_Right )
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

	VerifyCheckboxAccessories( NULL, false, &m_vecAccessoryList );

	m_pCDBExpandCollapseGroupAccessory = CreateExpandCollapseGroupButton( CD_Accessory_FirstColumn, RD_Accessory_GroupName, true, 
			CCDButtonExpandCollapseGroup::ButtonState::CollapseRow,  pclSDAccessory->GetFirstSelectableRow(), pclSDAccessory->GetLastSelectableRow( false ), 
			pclSDAccessory );

	// Show button.
	if( NULL != m_pCDBExpandCollapseGroupAccessory )
	{
		m_pCDBExpandCollapseGroupAccessory->SetShowStatus( true );
	}

	pclSSheet->SetCellBorder( CD_Accessory_Left, lRow, CD_Accessory_LastColumn - 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );

	pclSSheet->SetBool( SSB_REDRAW, TRUE );
}

void CRViewSSelSeparator::_ReadAllColumnWidth( CDS_IndSelParameter *pclIndSelParameter )
{
	// Access to the 'RViewSSelSeparator' group.
	CDS_IndSelParameter::CCWWindow *pclCWWindow = pclIndSelParameter->GetWindowColumnWidth( CW_WINDOWID_INDSELSEPARATOR, true );

	// Access to the 'separator' group.
	ResetColumnWidth( SD_Separator );
	CDS_IndSelParameter::CCWSheet *pclCWSheet = pclCWWindow->GetCWSheet( CW_RVIEWSSELSEPARATOR_SHEETID_SEPARATOR );
	
	if( NULL != pclCWSheet && m_mapSSheetColumnWidth[SD_Separator].size() == pclCWSheet->GetMap().size() )
	{
		short nVersion = pclCWSheet->GetVersion();

		if( nVersion == CW_RVIEWSSELSEPARATOR_SEPARATOR_VERSION )
		{
			m_mapSSheetColumnWidth[SD_Separator] = pclCWSheet->GetMap();
		}
	}
}

void CRViewSSelSeparator::_WriteAllColumnWidth( CDS_IndSelParameter *pclIndSelParameter )
{
	// Access to the 'RViewSSelSeparator' group.
	CDS_IndSelParameter::CCWWindow *pclCWWindow = pclIndSelParameter->GetWindowColumnWidth( CW_WINDOWID_INDSELSEPARATOR, true );

	// Write the 'separator' group.
	CDS_IndSelParameter::CCWSheet *pclCWSheet = pclCWWindow->GetCWSheet( CW_RVIEWSSELSEPARATOR_SHEETID_SEPARATOR, true );
	pclCWSheet->SetVersion( CW_RVIEWSSELSEPARATOR_SEPARATOR_VERSION );
	pclCWSheet->GetMap() = m_mapSSheetColumnWidth[SD_Separator];
}
