#include "stdafx.h"
#include "afxctl.h"

#include "TASelect.h"
#include "MainFrm.h"
#include "Hydronic.h"

#include "DlgLeftTabSelManager.h"

#include "RViewSSelSS.h"
#include "RViewSSelSmartControlValve.h"

CRViewSSelSmartControlValve *pRViewSSelSmartControlValve = NULL;
CRViewSSelSmartControlValve::CRViewSSelSmartControlValve() : CRViewSSelSS( CMainFrame::RightViewList::eRVSSelSmartControlValve, false )
{
	m_pclIndSelSmartControlValveParams = NULL;

	m_pCDBExpandCollapseRows = NULL;
	m_pCDBExpandCollapseGroupAccessory = NULL;
	m_pCDBShowAllPriorities = NULL;
	m_lSmartControlValveSelectedRow = -1;
	
	pRViewSSelSmartControlValve = this;
}

CRViewSSelSmartControlValve::~CRViewSSelSmartControlValve()
{
	pRViewSSelSmartControlValve = NULL;
}

CDB_TAProduct *CRViewSSelSmartControlValve::GetCurrentSmartControlValveSelected( void )
{
	CDB_TAProduct *pclCurrentSmartControlValveSelected = NULL;
	CSheetDescription *pclSheetDescriptionSmartControlValve = m_ViewDescription.GetFromSheetDescriptionID( SD_SmartControlValve );

	if( NULL != pclSheetDescriptionSmartControlValve )
	{
		// Retrieve the current selected smart control valve.
		CCellDescriptionProduct *pclCDCurrentSmartControlValveSelected = NULL;
		LPARAM lpPointer;
		
		if( true == pclSheetDescriptionSmartControlValve->GetUserVariable( _SDUV_SELECTEDPRODUCT, lpPointer ) )
		{
			pclCDCurrentSmartControlValveSelected = (CCellDescriptionProduct *)lpPointer;
		}

		if( NULL != pclCDCurrentSmartControlValveSelected && NULL != pclCDCurrentSmartControlValveSelected->GetProduct() )
		{
			pclCurrentSmartControlValveSelected = dynamic_cast<CDB_TAProduct *>( (CData *)pclCDCurrentSmartControlValveSelected->GetProduct() );
		}
	}

	return pclCurrentSmartControlValveSelected;
}

void CRViewSSelSmartControlValve::Reset()
{
	m_pCDBExpandCollapseRows = NULL;
	m_pCDBExpandCollapseGroupAccessory = NULL;
	m_pCDBShowAllPriorities = NULL;
	m_vecAccessoryList.clear();
	m_lSmartControlValveSelectedRow = -1;
	CRViewSSelSS::Reset();
}

void CRViewSSelSmartControlValve::Suggest( CProductSelelectionParameters *pclProductSelectionParameters, LPARAM lpParam )
{
	CRViewSSelSS::Suggest( pclProductSelectionParameters, lpParam );

	if( NULL == pclProductSelectionParameters || NULL == dynamic_cast<CIndSelSmartControlValveParams *>( pclProductSelectionParameters ) )
	{
		ASSERT_RETURN;
	}

	m_pclIndSelSmartControlValveParams = dynamic_cast<CIndSelSmartControlValveParams *>( pclProductSelectionParameters );

	// To remove all current displayed sheets.
	Reset();

	BeginWaitCursor();
	CWnd::SetRedraw( FALSE );

	long lRowSelected = -1;

	CDS_SSelSmartControlValve *pSelectedSmartControlValve = NULL;

	// If we are in edition mode...
	if( NULL != m_pclIndSelSmartControlValveParams->m_SelIDPtr.MP )
	{
		pSelectedSmartControlValve = reinterpret_cast<CDS_SSelSmartControlValve*>( (CData*)( m_pclIndSelSmartControlValveParams->m_SelIDPtr.MP ) );

		if( NULL == pSelectedSmartControlValve )
		{
			ASSERT_RETURN;
		}

		lRowSelected = _FillSmartControlValveRows( pSelectedSmartControlValve );
	}
	else
	{
		lRowSelected = _FillSmartControlValveRows();
	}
	
	// Verify if sheet description has been well created.
	CSheetDescription *pclSheetDescription = m_ViewDescription.GetFromSheetDescriptionID( SD_SmartControlValve );

	if( NULL != pclSheetDescription && NULL != pclSheetDescription->GetSSheetPointer() )
	{
		CSSheet *pclSSheet = pclSheetDescription->GetSSheetPointer();

		if( lRowSelected > -1 && NULL != pSelectedSmartControlValve )
		{
			CCellDescriptionProduct *pclCDProduct = FindCDProduct( lRowSelected, (LPARAM)( pSelectedSmartControlValve->GetProductAs<CDB_TAProduct>() ), pclSheetDescription );

			if( NULL != pclCDProduct && NULL != pclCDProduct->GetProduct() )
			{
				// If we are in edition mode we simulate a click on the product.
				OnClickProduct( pclSheetDescription, pclCDProduct, pclSheetDescription->GetActiveColumn(), lRowSelected );

				// Allow to check if we need to change the 'Show all priorities' button or not.
				CheckShowAllPrioritiesButtonState( pclSheetDescription, lRowSelected );

				// Verify accessories.
				CAccessoryList *pclAccessoryList = pSelectedSmartControlValve->GetAccessoryList();

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
			PrepareAndSetNewFocus( pclSheetDescription, CD_SmartControlValve_Name, RD_SmartControlValve_FirstAvailRow, 0 );
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

void CRViewSSelSmartControlValve::FillInSelected( CDS_SelProd *pSelectedProductToFill )
{
	if( NULL == m_pclIndSelSmartControlValveParams )
	{
		ASSERT_RETURN;
	}

	CDS_SSelSmartControlValve *pSelectedSmartControlValveToFill = dynamic_cast<CDS_SSelSmartControlValve *>( pSelectedProductToFill );

	if( NULL == pSelectedSmartControlValveToFill )
	{
		ASSERT_RETURN;
	}

	// HYS-987: Clear previous selected accessories if wa are not in edition mode.
	CAccessoryList *pclSmartControlValveAccessoryListToFill = pSelectedSmartControlValveToFill->GetAccessoryList();
	bool bIsEditionMode = false;
	if( false == m_pclIndSelSmartControlValveParams->m_bEditModeRunning )
	{
		pclSmartControlValveAccessoryListToFill->Clear();
	}
	else
	{
		bIsEditionMode = true;
	}

	// Retrieve the current selected smart control valve if exist.
	CDB_TAProduct *pclCurrentSmartControlValveSelected = GetCurrentSmartControlValveSelected();

	if( NULL != pclCurrentSmartControlValveSelected && NULL != m_pclIndSelSmartControlValveParams->m_pclSelectSmartControlValveList )
	{
		pSelectedSmartControlValveToFill->SetProductIDPtr( pclCurrentSmartControlValveSelected->GetIDPtr() );

		// HYS-987
		if( false == bIsEditionMode )
		{
			// Retrieve selected accessory and add it.
			for( vecCDCAccessoryListIter vecIter = m_vecAccessoryList.begin(); vecIter != m_vecAccessoryList.end(); vecIter++ )
			{
				CCDBCheckboxAccessory *pCDBCheckboxAccessory = *vecIter;

				if( NULL != pCDBCheckboxAccessory && true == pCDBCheckboxAccessory->GetCheckStatus() && NULL != pCDBCheckboxAccessory->GetAccessoryPointer() )
				{
					pclSmartControlValveAccessoryListToFill->Add( pCDBCheckboxAccessory->GetAccessoryPointer()->GetIDPtr(), CAccessoryList::_AT_Accessory,
						pCDBCheckboxAccessory->GetRuledTable() );
				}
			}
		}
		else
		{
			UpdateAccessoryList( m_vecAccessoryList, pclSmartControlValveAccessoryListToFill, CAccessoryList::_AT_Accessory );
		}

		// Selected pipe informations.
		if( NULL != m_pclIndSelSmartControlValveParams->m_pclSelectSmartControlValveList->GetSelectPipeList() )
		{
			m_pclIndSelSmartControlValveParams->m_pclSelectSmartControlValveList->GetSelectPipeList()->GetMatchingPipe( pclCurrentSmartControlValveSelected->GetSizeKey(), 
					*pSelectedSmartControlValveToFill->GetpSelPipe() );
		}
	}
}

void CRViewSSelSmartControlValve::OnNewDocument( CDS_IndSelParameter *pclIndSelParameter )
{
	_ReadAllColumnWidth( pclIndSelParameter );
}

void CRViewSSelSmartControlValve::SaveSelectionParameters( CDS_IndSelParameter *pclIndSelParameter )
{
	_WriteAllColumnWidth( pclIndSelParameter );
}

bool CRViewSSelSmartControlValve::OnTextTipFetch( CSheetDescription *pclSheetDescription, long lColumn, long lRow, WORD *pwMultiLine, SHORT *pnTipWidth, 
	TCHAR *pstrTipText, BOOL *pfShowTip )
{
	if( NULL == m_pclIndSelSmartControlValveParams || NULL == m_pclIndSelSmartControlValveParams->m_pclSelectSmartControlValveList || false == m_bInitialised || NULL == pclSheetDescription )
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
	CDB_SmartControlValve *pclSmartControlValve = dynamic_cast<CDB_SmartControlValve *>( pTAP );
	CSelectedValve *pclSelectedSmartControlValve = GetSelectProduct<CSelectedValve>( pTAP, m_pclIndSelSmartControlValveParams->m_pclSelectSmartControlValveList );
	CSSheet *pclSSheet = pclSheetDescription->GetSSheetPointer();
	CDS_TechnicalParameter *pclTechParam = m_pclIndSelSmartControlValveParams->m_pTADS->GetpTechParams();
	
	switch( pclSheetDescription->GetSheetDescriptionID() )
	{
		// Check if mouse cursor has passed over a valve.
		case SD_SmartControlValve:

			if( NULL != pclSelectedSmartControlValve )
			{
				if( CD_SmartControlValve_TemperatureRange == lColumn )
				{
					if( true == pclSelectedSmartControlValve->IsFlagSet( CSelectedBase::eTemperature ) )
					{
						FormatString( str, IDS_SSHEETSSEL_TEMPERROR, pclSmartControlValve->GetTempRange() );
					}
				}
				else if( CD_SmartControlValve_PipeLinDp == lColumn )
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
				else if( CD_SmartControlValve_PipeV == lColumn )
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
				else if( CD_SmartControlValve_MoreInfo == lColumn && NULL != pclSmartControlValve )
				{
					str = TASApp.LoadLocalizedString( IDS_SSHEETSSELCV_ACTRPOWERSUP ) + _T( ": " ) + pclSmartControlValve->GetPowerSupplyStr();
					str += _T( "\n" ) + TASApp.LoadLocalizedString( IDS_SSHEETSSELCV_ACTRINPUTSIG ) + _T( ": " ) + pclSmartControlValve->GetInOutSignalsStr( true );
					str += _T( "\n" ) + TASApp.LoadLocalizedString( IDS_SSHEETSSELCV_ACTROUTPUTSIG ) + _T( ": " ) + pclSmartControlValve->GetInOutSignalsStr( false );
				}
				// HYS-1939: TA-Smart Dp - 06 - Individual selection: create the datastruct object.
				else if( CD_SmartControlValve_DpMax == lColumn )
				{
					if( m_pclIndSelSmartControlValveParams->m_dDpMax > pclSmartControlValve->GetDpmax() )
					{
						CString str2 = WriteCUDouble( _U_DIFFPRESS, m_pclIndSelSmartControlValveParams->m_dDpMax, true, 3, 0 );
						CString str3 = WriteCUDouble( _U_DIFFPRESS, pclSmartControlValve->GetDpmax(), true, 3, 0 );
						FormatString( str, IDS_PRODUCTSELECTION_ERROR_DPMAX, str2, str3 );
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

bool CRViewSSelSmartControlValve::OnClickProduct( CSheetDescription *pclSheetDescriptionSmartControlValve, CCellDescriptionProduct *pclCellDescriptionProduct, long lColumn, long lRow )
{
	if( NULL == pclSheetDescriptionSmartControlValve || NULL == pclSheetDescriptionSmartControlValve->GetSSheetPointer() || NULL == pclCellDescriptionProduct )
	{
		ASSERTA_RETURN( false );
	}

	CDB_TAProduct *pclSmartControlValve = dynamic_cast<CDB_TAProduct *>( (CData *)pclCellDescriptionProduct->GetProduct() );

	if( NULL == pclSmartControlValve )
	{
		ASSERTA_RETURN( false );
	}

	CSSheet *pclSSheet = pclSheetDescriptionSmartControlValve->GetSSheetPointer();
		
	// Memorize previous focus row.
	long lCurrentFocusedRow = GetCurrentFocusedRow();

	// To be sure to clean all previous data concerning focus.
	KillCurrentFocus();

	// Unselect row where current selection is set (yellow light).
	// Remark: 'KillCurrentFocus' unselect only rows where focus is on (blue light).
	pclSSheet->UnSelectMultipleRows();

	// By default clear accessories list.
	m_vecAccessoryList.clear();

	LPARAM lSmartControlValveCount;
	pclSheetDescriptionSmartControlValve->GetUserVariable( _SDUV_TOTALPRODUCT_COUNT, lSmartControlValveCount );

	// Retrieve the current selected smart control valve if exist.
	CDB_TAProduct *pclCurrentSmartControlValveSelected = GetCurrentSmartControlValveSelected();

	// If there is already one smart control valve selected and user clicks on the current one...
	// Remark: 'm_pCDBExpandCollapseRows' is not created if there is only one smart control valve. Thus we need to check first if there is only one smart control valve.
	//         Otherwise we have to check if user has clicked on the current one.
	if( NULL != pclCurrentSmartControlValveSelected && ( ( 1 == lSmartControlValveCount) || ( NULL != m_pCDBExpandCollapseRows && lRow == m_pCDBExpandCollapseRows->GetCellPosition().y ) ) )
	{
		// Reset current product selected.
		SetCurrentSmartControlValveSelected( NULL );

		// Uncheck checkbox.
		pclSSheet->SetCheckBox(CD_SmartControlValve_CheckBox, lRow, _T(""), false, true);
		m_lSmartControlValveSelectedRow = -1;

		// Delete Expand/Collapse rows button if exist.
		// Remark: 'm_pCDBExpandCollapseRows' is set to NULL in method!
		DeleteExpandCollapseRowsButton( m_pCDBExpandCollapseRows, pclSheetDescriptionSmartControlValve );

		// Show Show/Hide all priorities button if exist.
		if( NULL != m_pCDBShowAllPriorities )
		{
			m_pCDBShowAllPriorities->ApplyInternalChange();
		}

		// Set focus on smart control valve currently selected.
		PrepareAndSetNewFocus( pclSheetDescriptionSmartControlValve, pclSheetDescriptionSmartControlValve->GetActiveColumn(), lRow, 0 );

		// Remove all sheets after smart control valve.
		m_pCDBExpandCollapseGroupAccessory = NULL;
		m_ViewDescription.RemoveAllSheetAfter( SD_SmartControlValve );

		// Notify a change in the selection.
		SelectionHasChanged( IsSelectionReady() );
	}
	else
	{
		// If a smart control valve is already selected...
		if( NULL != pclCurrentSmartControlValveSelected )
		{
			// Remove all sheets after smart control valve.
			m_pCDBExpandCollapseGroupAccessory = NULL;
			m_ViewDescription.RemoveAllSheetAfter( SD_SmartControlValve );
		}

		// Save new smart control valve selection.
		SetCurrentSmartControlValveSelected( pclCellDescriptionProduct );

		// Unselect previous selection.
		if( m_lSmartControlValveSelectedRow > -1 )
		{
			pclSSheet->SetCheckBox( CD_SmartControlValve_CheckBox, m_lSmartControlValveSelectedRow, _T(""), false, true );
		}

		// Check checkbox.
		pclSSheet->SetCheckBox( CD_SmartControlValve_CheckBox, lRow, _T(""), true, true );
		m_lSmartControlValveSelectedRow = lRow;

		// Delete Expand/Collapse rows button if exist.
		if( NULL != m_pCDBExpandCollapseRows )
		{
			// Remark: 'm_pCDBExpandCollapseRows' is set to NULL in method!
			DeleteExpandCollapseRowsButton( m_pCDBExpandCollapseRows, pclSheetDescriptionSmartControlValve );
		}

		// Create Expand/Collapse rows button if needed...
		LPARAM lValveTotalCount;
		pclSheetDescriptionSmartControlValve->GetUserVariable( _SDUV_TOTALPRODUCT_COUNT, lValveTotalCount );
		
		if( lValveTotalCount > 1 )
		{
			m_pCDBExpandCollapseRows = CreateExpandCollapseRowsButton( CD_SmartControlValve_FirstColumn, lRow, true, 
					CCDButtonExpandCollapseRows::ButtonState::CollapseRow, pclSheetDescriptionSmartControlValve->GetFirstSelectableRow(), 
					pclSheetDescriptionSmartControlValve->GetLastSelectableRow( false ), pclSheetDescriptionSmartControlValve );

			// Show button.
			if( NULL != m_pCDBExpandCollapseRows )
			{
				m_pCDBExpandCollapseRows->SetShowStatus( true );
			}
		}

		// Select smart control valve (just highlight background).
		pclSSheet->SelectOneRow( lRow, pclSheetDescriptionSmartControlValve->GetSelectionFrom(), pclSheetDescriptionSmartControlValve->GetSelectionTo() );

		// Fill corresponding accessories.
		_FillAccessoryRows();

		// Try to set the focus on the next table but do not lose the focus if no other table exist.
		CSheetDescription *pclNextSheetDescription = NULL;
		long lNewFocusedRow;
		bool fShiftPressed;
		
		if( DNFERRC_FocusOnNextGroupOK == DefineNextFocusedRow( pclSheetDescriptionSmartControlValve, CD_SmartControlValve_FirstColumn, lRow, false, 
				lNewFocusedRow, pclNextSheetDescription, fShiftPressed ) )
		{
			long lNewFocusedCol = pclNextSheetDescription->GetFirstParameterColumn();
			PrepareAndSetNewFocus( pclNextSheetDescription, lNewFocusedCol, lNewFocusedRow );
		}
		else
		{
			// Because we have changed current selection background in yellow, if there is no other sheet following current one, we
			// must set focus on current selection.
			PrepareAndSetNewFocus( pclSheetDescriptionSmartControlValve, lColumn, lRow );
		}

		// Notify a change in the selection.
		SelectionHasChanged( IsSelectionReady() );
	}

	return true;
}

bool CRViewSSelSmartControlValve::ResetColumnWidth( short nSheetDescriptionID )
{
	TSpread clTSpread;

	if( FALSE == clTSpread.Create( WS_CHILD, CRect( 0, 0, 0, 0 ), this, 0 ) )
	{
		ASSERT( 0 );
		return false;
	}
	
	switch( nSheetDescriptionID )
	{
		case SD_SmartControlValve:
			m_mapSSheetColumnWidth[SD_SmartControlValve][CD_SmartControlValve_FirstColumn] = clTSpread.ColWidthToLogUnits( 4 );
			m_mapSSheetColumnWidth[SD_SmartControlValve][CD_SmartControlValve_CheckBox] = clTSpread.ColWidthToLogUnits( 2 );
			m_mapSSheetColumnWidth[SD_SmartControlValve][CD_SmartControlValve_Name] = clTSpread.ColWidthToLogUnits( 14 );
			m_mapSSheetColumnWidth[SD_SmartControlValve][CD_SmartControlValve_Material] = clTSpread.ColWidthToLogUnits( 12 );
			m_mapSSheetColumnWidth[SD_SmartControlValve][CD_SmartControlValve_Connection] = clTSpread.ColWidthToLogUnits( 14 );
			m_mapSSheetColumnWidth[SD_SmartControlValve][CD_SmartControlValve_PN] = clTSpread.ColWidthToLogUnits( 6 );
			m_mapSSheetColumnWidth[SD_SmartControlValve][CD_SmartControlValve_Size] = clTSpread.ColWidthToLogUnits( 10 );
			m_mapSSheetColumnWidth[SD_SmartControlValve][CD_SmartControlValve_Kvs] = clTSpread.ColWidthToLogUnits( 6 );
			m_mapSSheetColumnWidth[SD_SmartControlValve][CD_SmartControlValve_Qnom] = clTSpread.ColWidthToLogUnits( 8 );
			m_mapSSheetColumnWidth[SD_SmartControlValve][CD_SmartControlValve_DpMin] = clTSpread.ColWidthToLogUnits( 7 );
			// HYS-1939: TA-Smart Dp - 06 - Individual selection: create the datastruct object.
			m_mapSSheetColumnWidth[SD_SmartControlValve][CD_SmartControlValve_DpMax] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_SmartControlValve][CD_SmartControlValve_TemperatureRange] = clTSpread.ColWidthToLogUnits( 8 );
			m_mapSSheetColumnWidth[SD_SmartControlValve][CD_SmartControlValve_MoreInfo] = clTSpread.ColWidthToLogUnits( 6 );
			m_mapSSheetColumnWidth[SD_SmartControlValve][CD_SmartControlValve_Separator] = clTSpread.ColWidthToLogUnits( 1 );
			m_mapSSheetColumnWidth[SD_SmartControlValve][CD_SmartControlValve_PipeSize] = clTSpread.ColWidthToLogUnits( 8 );
			m_mapSSheetColumnWidth[SD_SmartControlValve][CD_SmartControlValve_PipeLinDp] = clTSpread.ColWidthToLogUnits( 8 );
			m_mapSSheetColumnWidth[SD_SmartControlValve][CD_SmartControlValve_PipeV] = clTSpread.ColWidthToLogUnits( 8 );
			m_mapSSheetColumnWidth[SD_SmartControlValve][CD_SmartControlValve_Pointer] = clTSpread.ColWidthToLogUnits( 1 );
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

bool CRViewSSelSmartControlValve::IsSelectionReady( void )
{
	CDB_TAProduct *pSelection = GetCurrentSmartControlValveSelected();

	if( NULL != pSelection )
	{
		return ( false == pSelection->IsDeleted() );
	}

	return false; // No selection available
}

void CRViewSSelSmartControlValve::SetCurrentSmartControlValveSelected( CCellDescriptionProduct *pclCDCurrentSmartControlValveSelected )
{
	// Try to retrieve sheet description linked to smart control valve.
	CSheetDescription *pclSheetDescriptionSmartControlValve = m_ViewDescription.GetFromSheetDescriptionID( SD_SmartControlValve );

	if( NULL != pclSheetDescriptionSmartControlValve )
	{
		pclSheetDescriptionSmartControlValve->SetUserVariable( _SDUV_SELECTEDPRODUCT, (LPARAM)pclCDCurrentSmartControlValveSelected );
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PRIVATE MEMBERS
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

long CRViewSSelSmartControlValve::_FillSmartControlValveRows( CDS_SSelSmartControlValve *pEditedProduct )
{
	if( NULL == m_pclIndSelSmartControlValveParams || NULL == m_pclIndSelSmartControlValveParams->m_pclSelectSmartControlValveList 
			|| NULL == m_pclIndSelSmartControlValveParams->m_pclSelectSmartControlValveList->GetSelectPipeList() || NULL == m_pclIndSelSmartControlValveParams->m_pTADS
			|| NULL == m_pclIndSelSmartControlValveParams->m_pPipeDB )
	{
		ASSERTA_RETURN( 0 );
	}

	// Call 'MultiSpreadBase' method to get a new SSheet.
	CSheetDescription *pclSheetDescriptionSmartControlValve = CreateSSheet( SD_SmartControlValve );

	if( NULL == pclSheetDescriptionSmartControlValve || NULL == pclSheetDescriptionSmartControlValve->GetSSheetPointer() )
	{
		return 0;
	}

	CSSheet *pclSSheet = pclSheetDescriptionSmartControlValve->GetSSheetPointer();

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
	CSelectedValve *pclSelectedValve = m_pclIndSelSmartControlValveParams->m_pclSelectSmartControlValveList->GetFirst<CSelectedValve>();

	if( NULL == pclSelectedValve )
	{
		return 0;
	}

	CDB_SmartControlValve *pclSmartControlValve = dynamic_cast<CDB_SmartControlValve *>( pclSelectedValve->GetpData() );

	if( NULL == pclSmartControlValve )
	{
		return 0;
	}

	bool bValveSettingExist = ( NULL != pclSmartControlValve->GetValveCharacteristic() );
	bool bKvSignalExist = pclSmartControlValve->IsKvSignalEquipped();

	bool bIsUserPressureDrop = m_pclIndSelSmartControlValveParams->m_bDpEnabled;

	// Set max rows.
	pclSSheet->SetMaxRows( RD_SmartControlValve_FirstAvailRow - 1 );
	
	// All cells are static by default and filled with _T("").
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	pclSSheet->FormatStaticText( -1, -1, -1, -1, _T("") );
	
	// Increase row height.
	double RowHeight = 12.75;
	pclSSheet->SetRowHeight( RD_SmartControlValve_FirstRow, RowHeight * 0.5 );
	pclSSheet->SetRowHeight( RD_SmartControlValve_GroupName, RowHeight * 1.5 );
	pclSSheet->SetRowHeight( RD_SmartControlValve_ColName, RowHeight * 2 );
	pclSSheet->SetRowHeight( RD_SmartControlValve_Unit, RowHeight * 1.2 );
	
	// Initialize.
	pclSheetDescriptionSmartControlValve->Init( 1, pclSSheet->GetMaxRows(), 1, CSheetDescription::SD_ParameterMode_Mono );
	pclSheetDescriptionSmartControlValve->AddColumnInPixels( CD_SmartControlValve_FirstColumn, m_mapSSheetColumnWidth[SD_SmartControlValve][CD_SmartControlValve_FirstColumn] );
	pclSheetDescriptionSmartControlValve->AddColumnInPixels( CD_SmartControlValve_CheckBox, m_mapSSheetColumnWidth[SD_SmartControlValve][CD_SmartControlValve_CheckBox] );
	pclSheetDescriptionSmartControlValve->AddColumnInPixels( CD_SmartControlValve_Name, m_mapSSheetColumnWidth[SD_SmartControlValve][CD_SmartControlValve_Name] );
	pclSheetDescriptionSmartControlValve->AddColumnInPixels( CD_SmartControlValve_Material, m_mapSSheetColumnWidth[SD_SmartControlValve][CD_SmartControlValve_Material] );
	pclSheetDescriptionSmartControlValve->AddColumnInPixels( CD_SmartControlValve_Connection, m_mapSSheetColumnWidth[SD_SmartControlValve][CD_SmartControlValve_Connection] );
	pclSheetDescriptionSmartControlValve->AddColumnInPixels( CD_SmartControlValve_PN, m_mapSSheetColumnWidth[SD_SmartControlValve][CD_SmartControlValve_PN] );
	pclSheetDescriptionSmartControlValve->AddColumnInPixels( CD_SmartControlValve_Size, m_mapSSheetColumnWidth[SD_SmartControlValve][CD_SmartControlValve_Size] );
	pclSheetDescriptionSmartControlValve->AddColumnInPixels( CD_SmartControlValve_Kvs, m_mapSSheetColumnWidth[SD_SmartControlValve][CD_SmartControlValve_Kvs] );
	pclSheetDescriptionSmartControlValve->AddColumnInPixels( CD_SmartControlValve_Qnom, m_mapSSheetColumnWidth[SD_SmartControlValve][CD_SmartControlValve_Qnom] );
	pclSheetDescriptionSmartControlValve->AddColumnInPixels( CD_SmartControlValve_DpMin, m_mapSSheetColumnWidth[SD_SmartControlValve][CD_SmartControlValve_DpMin] );
	// HYS-1939: TA-Smart Dp - 06 - Individual selection: create the datastruct object.
	pclSheetDescriptionSmartControlValve->AddColumnInPixels( CD_SmartControlValve_DpMax, m_mapSSheetColumnWidth[SD_SmartControlValve][CD_SmartControlValve_DpMax] );
	pclSheetDescriptionSmartControlValve->AddColumnInPixels( CD_SmartControlValve_TemperatureRange, m_mapSSheetColumnWidth[SD_SmartControlValve][CD_SmartControlValve_TemperatureRange] );
	pclSheetDescriptionSmartControlValve->AddColumnInPixels( CD_SmartControlValve_MoreInfo, m_mapSSheetColumnWidth[SD_SmartControlValve][CD_SmartControlValve_MoreInfo] );
	pclSheetDescriptionSmartControlValve->AddColumnInPixels( CD_SmartControlValve_Separator, m_mapSSheetColumnWidth[SD_SmartControlValve][CD_SmartControlValve_Separator] );
	pclSheetDescriptionSmartControlValve->AddColumnInPixels( CD_SmartControlValve_PipeSize, m_mapSSheetColumnWidth[SD_SmartControlValve][CD_SmartControlValve_PipeSize] );
	pclSheetDescriptionSmartControlValve->AddColumnInPixels( CD_SmartControlValve_PipeLinDp, m_mapSSheetColumnWidth[SD_SmartControlValve][CD_SmartControlValve_PipeLinDp] );
	pclSheetDescriptionSmartControlValve->AddColumnInPixels( CD_SmartControlValve_PipeV, m_mapSSheetColumnWidth[SD_SmartControlValve][CD_SmartControlValve_PipeV] );
	pclSheetDescriptionSmartControlValve->AddColumnInPixels( CD_SmartControlValve_Pointer, m_mapSSheetColumnWidth[SD_SmartControlValve][CD_SmartControlValve_Pointer] );

	// Set in which column parameter must be saved.
	pclSheetDescriptionSmartControlValve->AddParameterColumn( CD_SmartControlValve_Pointer );
	
	// Set the focus column.
	pclSheetDescriptionSmartControlValve->SetActiveColumn( CD_SmartControlValve_Name );

	// Set range for selection.
	pclSheetDescriptionSmartControlValve->SetFocusColumnRange( CD_SmartControlValve_CheckBox, CD_SmartControlValve_MoreInfo );

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

	pclSSheet->SetStaticText( CD_SmartControlValve_Name, RD_SmartControlValve_ColName, IDS_SSHEETSSEL_NAME );
	pclSSheet->SetStaticText( CD_SmartControlValve_Material, RD_SmartControlValve_ColName, IDS_SSHEETSSEL_MATERIAL );
	pclSSheet->SetStaticText( CD_SmartControlValve_Connection, RD_SmartControlValve_ColName, IDS_SSHEETSSEL_CONNECT );
	pclSSheet->SetStaticText( CD_SmartControlValve_PN, RD_SmartControlValve_ColName, IDS_SSHEETSSEL_PN );
	pclSSheet->SetStaticText( CD_SmartControlValve_Size, RD_SmartControlValve_ColName, IDS_SSHEETSSEL_SIZE );

	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();

	if( 0 == pUnitDB->GetDefaultUnitIndex( _C_KVCVCOEFF ) )
	{
		pclSSheet->SetStaticText( CD_SmartControlValve_Kvs, RD_SmartControlValve_ColName, IDS_SSHEETSSELCV_CVKVS );
	}
	else
	{
		pclSSheet->SetStaticText( CD_SmartControlValve_Kvs, RD_SmartControlValve_ColName, IDS_SSHEETSSELCV_CVCV );
	}
	
	pclSSheet->SetStaticText( CD_SmartControlValve_Qnom, RD_SmartControlValve_ColName, IDS_SSHEETSSEL_QNOM );
	pclSSheet->SetStaticText( CD_SmartControlValve_DpMin, RD_SmartControlValve_ColName, IDS_SSHEETSSEL_DPMIN );
	// HYS-1939: TA-Smart Dp - 06 - Individual selection: create the datastruct object.
	pclSSheet->SetStaticText( CD_SmartControlValve_DpMax, RD_SmartControlValve_ColName, IDS_RVIEWSSELCTRL_DPMAX );
	pclSSheet->SetStaticText( CD_SmartControlValve_TemperatureRange, RD_SmartControlValve_ColName, IDS_SSHEETSSEL_COLTEMPRANGE );
	pclSSheet->SetStaticText( CD_SmartControlValve_MoreInfo, RD_SmartControlValve_ColName, IDS_SSHEETSSEL_MORE );

	pclSSheet->SetStaticText( CD_SmartControlValve_Separator, RD_SmartControlValve_ColName, _T("") );

	// Pipes.
	pclSSheet->SetStaticText( CD_SmartControlValve_PipeSize, RD_SmartControlValve_ColName, IDS_SSHEETSSEL_PIPESIZE );
	pclSSheet->SetStaticText( CD_SmartControlValve_PipeLinDp, RD_SmartControlValve_ColName, IDS_SSHEETSSEL_PIPELINDP );
	pclSSheet->SetStaticText( CD_SmartControlValve_PipeV, RD_SmartControlValve_ColName, IDS_SSHEETSSEL_PIPEV );
	//
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Row units.
	pclSSheet->SetStaticText( CD_SmartControlValve_TemperatureRange, RD_SmartControlValve_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_TEMPERATURE ) ).c_str() );
	pclSSheet->SetStaticText( CD_SmartControlValve_PipeLinDp, RD_SmartControlValve_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_LINPRESSDROP ) ).c_str() );
	pclSSheet->SetStaticText( CD_SmartControlValve_PipeV, RD_SmartControlValve_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_VELOCITY ) ).c_str() );
	//
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Draw black line below column header.
	pclSSheet->SetCellBorder( CD_SmartControlValve_CheckBox, RD_SmartControlValve_Unit, CD_SmartControlValve_Separator - 1, RD_SmartControlValve_Unit, true, SS_BORDERTYPE_BOTTOM );
	pclSSheet->SetCellBorder( CD_SmartControlValve_PipeSize, RD_SmartControlValve_Unit, CD_SmartControlValve_Pointer - 1, RD_SmartControlValve_Unit, true, SS_BORDERTYPE_BOTTOM );

	// HYS-1939: TA-Smart Dp - 06 - Individual selection: create the datastruct object.
	if( false == m_pclIndSelSmartControlValveParams->m_bIsDpMaxChecked )
	{
		pclSSheet->ShowCol( CD_SmartControlValve_DpMax, FALSE );
	}

	CSelectPipe selPipe( m_pclIndSelSmartControlValveParams );
	long lRetRow = -1;

	double dRho = m_pclIndSelSmartControlValveParams->m_WC.GetDens();
	double dKinVisc = m_pclIndSelSmartControlValveParams->m_WC.GetKinVisc();
	double dFlow = m_pclIndSelSmartControlValveParams->m_dFlow;

	CDS_TechnicalParameter *pTechParam = m_pclIndSelSmartControlValveParams->m_pTADS->GetpTechParams();
	
	long lRow = RD_SmartControlValve_FirstAvailRow;

	long lSmartControlValveTotalCount = 0;
	long lSmartControlValveNotPriorityCount = 0;
	bool bShowAllPrioritiesShown = false;
	long lShowAllPrioritiesButtonRow = 0;
	bool bAtLeastOneError;
	bool bAtLeastOneWarning;

	pclSheetDescriptionSmartControlValve->SetUserVariable( _SDUV_TOTALPRODUCT_COUNT, 0 );
	pclSheetDescriptionSmartControlValve->SetUserVariable( _SDUV_NOPRIORITYPRODUCT_COUNT, 0 );

	pclSheetDescriptionSmartControlValve->RestartRemarkGenerator();
	
	for( pclSelectedValve = m_pclIndSelSmartControlValveParams->m_pclSelectSmartControlValveList->GetFirst<CSelectedValve>(); NULL != pclSelectedValve; 
			pclSelectedValve = m_pclIndSelSmartControlValveParams->m_pclSelectSmartControlValveList->GetNext<CSelectedValve>() )
	{
		pclSmartControlValve = dynamic_cast<CDB_SmartControlValve *>( pclSelectedValve->GetpData() );

		if( NULL == pclSmartControlValve || NULL == pclSmartControlValve->GetSmartValveCharacteristic() ) 
		{
			continue;
		}

		bAtLeastOneError = false;
		bAtLeastOneWarning = false;
		bool bBest = pclSelectedValve->IsFlagSet(CSelectedBase::eBest );
		
		if( true == pclSelectedValve->IsFlagSet(CSelectedBase::eNotPriority) )
		{
			lSmartControlValveNotPriorityCount++;
			
			if( false == bShowAllPrioritiesShown )
			{
				// Add one empty not selectable row.
				pclSheetDescriptionSmartControlValve->AddRows( 1 );
				pclSheetDescriptionSmartControlValve->RemoveSelectableRow( lRow );

				pclSSheet->SetCellBorder( CD_SmartControlValve_CheckBox, lRow, CD_SmartControlValve_Separator - 1, lRow, true, 
						SS_BORDERTYPE_TOP | SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );

				pclSSheet->SetCellBorder( CD_SmartControlValve_PipeSize, lRow, CD_SmartControlValve_Pointer - 1, lRow, true, 
						SS_BORDERTYPE_TOP | SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );

				lShowAllPrioritiesButtonRow = lRow++;
				bShowAllPrioritiesShown = true;
			}
		}

		// Add this row.
		// 'true' to specify that this row can be selected.
		pclSheetDescriptionSmartControlValve->AddRows( 1, true );

		// First columns will be set at the end!
		
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

		// Temperature range.
		if( true == pclSelectedValve->IsFlagSet( CSelectedBase::eTemperature ) )
		{
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_RED );
			bAtLeastOneError = true;
		}

		pclSSheet->SetStaticText( CD_SmartControlValve_TemperatureRange, lRow, ((CDB_TAProduct *)pclSelectedValve->GetProductIDPtr().MP)->GetTempRange() );
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
		pclSSheet->SetCheckBox( CD_SmartControlValve_CheckBox, lRow, _T(""), false, true);

		// In addition of current flag, we have possibility that valve has the flag 'Not available' or 'Deleted'. In that case, we show valve name in red with "*" around it and
		// symbol '!' or '!!' after.
		if( true == pclSmartControlValve->IsDeleted() )
		{
			pclSheetDescriptionSmartControlValve->WriteTextWithFlags( CString( pclSmartControlValve->GetName() ), CD_SmartControlValve_Name, lRow, 
					CSheetDescription::RemarkFlags::Deleted, _T(""), _RED );
		}
		else if( false == pclSmartControlValve->IsAvailable() )
		{
			pclSheetDescriptionSmartControlValve->WriteTextWithFlags( CString( pclSmartControlValve->GetName() ), CD_SmartControlValve_Name, lRow, 
					CSheetDescription::RemarkFlags::NotAvailable, _T(""), _RED );
		}
		else
		{
			pclSSheet->SetStaticText( CD_SmartControlValve_Name, lRow, pclSmartControlValve->GetName() );
		}

		if( true == bAtLeastOneError )
		{
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );
		}
		else if( true == bBest )
		{
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_GREEN );
		}

		pclSSheet->SetStaticText( CD_SmartControlValve_Material, lRow, pclSmartControlValve->GetBodyMaterial() );
		pclSSheet->SetStaticText( CD_SmartControlValve_Connection, lRow, pclSmartControlValve->GetConnect() );
		pclSSheet->SetStaticText( CD_SmartControlValve_PN, lRow, pclSmartControlValve->GetPN().c_str() );
		pclSSheet->SetStaticText( CD_SmartControlValve_Size, lRow, pclSmartControlValve->GetSize() );
		pclSSheet->SetStaticText( CD_SmartControlValve_Kvs, lRow, WriteCUDouble( _C_KVCVCOEFF, pclSmartControlValve->GetSmartValveCharacteristic()->GetKvs() ) );
		pclSSheet->SetStaticText( CD_SmartControlValve_Qnom, lRow, WriteCUDouble( _U_FLOW, pclSmartControlValve->GetSmartValveCharacteristic()->GetQnom(), true ) );

		// HYS-1914: It's here well the dp min (Kvs is the Kv at full opening).
		double dDpMin = CalcDp( m_pclIndSelSmartControlValveParams->m_dFlow, pclSmartControlValve->GetSmartValveCharacteristic()->GetKvs(), m_pclIndSelSmartControlValveParams->m_WC.GetDens() );
		pclSSheet->SetStaticText( CD_SmartControlValve_DpMin, lRow, WriteCUDouble( _U_DIFFPRESS, dDpMin, true ) );
		
		// HYS-1939: TA-Smart Dp - 06 - Individual selection: create the datastruct object.
		// Set Dp max.
		if( true == m_pclIndSelSmartControlValveParams->m_bIsDpMaxChecked )
		{
			if( pclSmartControlValve->GetDpmax() < m_pclIndSelSmartControlValveParams->m_dDpMax )
			{
				pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_RED );
				bAtLeastOneError = true;
			}

			pclSSheet->SetStaticText( CD_SmartControlValve_DpMax, lRow, WriteCUDouble( _U_DIFFPRESS, pclSmartControlValve->GetDpmax(), true ) );
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );
		}

		// HYS-1660
		pclSSheet->SetPictureCellWithID( IDI_SMV_MOREINFO, CD_SmartControlValve_MoreInfo, lRow, CSSheet::PictureCellType::Icon );

		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );
		// Set pipe size.
		m_pclIndSelSmartControlValveParams->m_pclSelectSmartControlValveList->GetSelectPipeList()->GetMatchingPipe( pclSmartControlValve->GetSizeKey(), selPipe );
		pclSSheet->SetStaticText( CD_SmartControlValve_PipeSize, lRow, selPipe.GetpPipe()->GetName() );
		
		// Set the LinDp to orange if it is above or below the technical parameters limits.
		if( selPipe.GetLinDp() > pTechParam->GetPipeMaxDp() || selPipe.GetLinDp() < pTechParam->GetPipeMinDp())
		{
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_ORANGE );
		}
		pclSSheet->SetStaticText( CD_SmartControlValve_PipeLinDp, lRow, WriteCUDouble( _U_LINPRESSDROP, selPipe.GetLinDp() ) );

		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

		// Set the Velocity to orange if it is above the technical parameter limit.
		// Orange if it is below the dMinVel.
		if( selPipe.GetU() > pTechParam->GetPipeMaxVel() || selPipe.GetU() < pTechParam->GetPipeMinVel() )
		{
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_ORANGE );
		}

		pclSSheet->SetStaticText( CD_SmartControlValve_PipeV, lRow, WriteCUDouble( _U_VELOCITY, selPipe.GetU() ) );
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

		// Save parameter.
		CreateCellDescriptionProduct( pclSheetDescriptionSmartControlValve->GetFirstParameterColumn(), lRow, (LPARAM)pclSmartControlValve, pclSheetDescriptionSmartControlValve );

		if( NULL != pEditedTAP && pEditedTAP == pclSmartControlValve )
		{
			lRetRow = lRow;
		}

		pclSSheet->SetCellBorder( CD_SmartControlValve_CheckBox, lRow, CD_SmartControlValve_Separator - 1, lRow, true, SS_BORDERTYPE_BOTTOM, 
				SS_BORDERSTYLE_SOLID, _GRAY );

		pclSSheet->SetCellBorder( CD_SmartControlValve_PipeSize, lRow, CD_SmartControlValve_Pointer - 1, lRow, true, SS_BORDERTYPE_BOTTOM,
				SS_BORDERSTYLE_SOLID, _GRAY );

		lRow++;
		lSmartControlValveTotalCount++;
	}

	long lLastDataRow = lRow - 1;

	pclSheetDescriptionSmartControlValve->SetUserVariable( _SDUV_TOTALPRODUCT_COUNT, lSmartControlValveTotalCount );
	pclSheetDescriptionSmartControlValve->SetUserVariable( _SDUV_NOPRIORITYPRODUCT_COUNT, lSmartControlValveNotPriorityCount );

	// Now we can create Show/Hide all priorities if needed.
	// Remarks: 2nd condition to avoid to have button if we have only no priority valves.
	if( true == bShowAllPrioritiesShown && lSmartControlValveTotalCount > lSmartControlValveNotPriorityCount )
	{
		// Create Show/Hide all priorities button in regards to current state!
		CCDButtonShowAllPriorities::ButtonState eState = ( true == GetShowAllPrioritiesFlag( SD_SmartControlValve ) ) ? 
				CCDButtonShowAllPriorities::ButtonState::HidePriorities : CCDButtonShowAllPriorities::ButtonState::ShowPriorities;

		CCDButtonShowAllPriorities *pclShowAllButton = CreateShowAllPrioritiesButton( CD_SmartControlValve_FirstColumn, lShowAllPrioritiesButtonRow, 
				bShowAllPrioritiesShown,  eState, lShowAllPrioritiesButtonRow + 1, pclSSheet->GetMaxRows(), pclSheetDescriptionSmartControlValve );

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
	if( lSmartControlValveTotalCount > 2 || ( 2 == lSmartControlValveTotalCount && lSmartControlValveNotPriorityCount != 1 ) )
	{
		vector<long> vecColumnList;
		vecColumnList.push_back( CD_SmartControlValve_PipeSize );
		vecColumnList.push_back( CD_SmartControlValve_PipeLinDp );
		vecColumnList.push_back( CD_SmartControlValve_PipeV );
		pclSSheet->MergeRowsWithSameValues( RD_SmartControlValve_FirstAvailRow, lLastDataRow, vecColumnList );
	}

	pclSSheet->SetCellBorder( CD_SmartControlValve_CheckBox, lLastDataRow, CD_SmartControlValve_Separator - 1, lLastDataRow, true, 
			SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );

	// Needed by pipe merging.
	pclSSheet->SetCellBorder( CD_SmartControlValve_PipeSize, lLastDataRow, CD_SmartControlValve_Pointer - 1, lLastDataRow, true, 
			SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );

	// Add Remarks.
	// Note: these lines must be placed after creating Show/Hide all priorities button to avoid to take in count these lines when defining
	//       range of row to be Show/Hide.
	pclSheetDescriptionSmartControlValve->WriteRemarks( lRow, CD_SmartControlValve_CheckBox, CD_SmartControlValve_Separator );

	// Add the main title.
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)GetTitleBackgroundColor( pclSheetDescriptionSmartControlValve ) );
	pclSheetDescriptionSmartControlValve->SetUserVariable( _SDUV_TITLEFORECOLOR, ( pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor ) ) );
	pclSheetDescriptionSmartControlValve->SetUserVariable( _SDUV_TITLEBACKCOLOR, ( pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor ) ) );

	pclSSheet->AddCellSpanW( CD_SmartControlValve_FirstColumn + 1, RD_SmartControlValve_GroupName, CD_SmartControlValve_Separator - CD_SmartControlValve_FirstColumn - 1, 1 );
	pclSSheet->SetStaticText( CD_SmartControlValve_FirstColumn + 1, RD_SmartControlValve_GroupName, IDS_SSHEETSSELSMARTCONTROLVALVE_VALVEGROUP );
	
	pclSSheet->AddCellSpanW( CD_SmartControlValve_PipeSize, RD_SmartControlValve_GroupName, CD_SmartControlValve_Pointer - CD_SmartControlValve_PipeSize, 1 );
	
	CTable *pclTable = dynamic_cast<CTable *>( m_pclIndSelSmartControlValveParams->m_pPipeDB->Get( m_pclIndSelSmartControlValveParams->m_strPipeSeriesID ).MP );
	ASSERT( NULL != pclTable );

	pclSSheet->SetStaticText( CD_SmartControlValve_PipeSize, RD_SmartControlValve_GroupName, pclTable->GetName() );
	
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

	pclSSheet->SetBool( SSB_REDRAW, TRUE );

	// Set that there is no selection at now.
	SetCurrentSmartControlValveSelected( NULL );

	// Add possibility to change column size.
	// Remark: place this code after any initialization (like adding, removing, resizing, hiding/showing columns).
	CResizingColumnInfo *pclResizingColumnInfo = pclSheetDescriptionSmartControlValve->GetResizingColumnInfo();
	pclResizingColumnInfo->ActivateFeature( true, false, RD_SmartControlValve_FirstAvailRow, lLastDataRow );
	pclResizingColumnInfo->AddRangeColumn( CD_SmartControlValve_Name, CD_SmartControlValve_TemperatureRange, RD_SmartControlValve_ColName, RD_SmartControlValve_Unit );
	pclResizingColumnInfo->AddRangeColumn( CD_SmartControlValve_PipeSize, CD_SmartControlValve_PipeV, RD_SmartControlValve_ColName, RD_SmartControlValve_Unit );

	// Add possibility to click on title group to reset column width.
	CreateTitleGroupButtons( CD_SmartControlValve_CheckBox, CD_SmartControlValve_PipeV, RD_SmartControlValve_GroupName, pclSheetDescriptionSmartControlValve );

	return lRetRow;
}

void CRViewSSelSmartControlValve::_FillAccessoryRows( )
{
	if( NULL == m_pclIndSelSmartControlValveParams || NULL == m_pclIndSelSmartControlValveParams->m_pTADB )
	{
		ASSERT_RETURN;
	}

	// Retrieve the current selected smart control valve.
	CDB_TAProduct *pclCurrentSmartControlValveSelected = GetCurrentSmartControlValveSelected();

	if( NULL == pclCurrentSmartControlValveSelected )
	{
		return;
	}

	// Sanity check.
	CDB_RuledTable *pRuledTable = (CDB_RuledTable *)( pclCurrentSmartControlValveSelected->GetAccessoriesGroupIDPtr().MP );

	if( NULL == pRuledTable )
	{
		return;
	}

	CRank rList;
	int iCount = m_pclIndSelSmartControlValveParams->m_pTADB->GetAccessories( &rList, pRuledTable, m_pclIndSelSmartControlValveParams->m_eFilterSelection );
	
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

	// Get sheet description of smart control valve to retrieve width.
	CSheetDescription *pclSDSmartControlValve = m_ViewDescription.GetFromSheetDescriptionID( SD_SmartControlValve );
	// It's absolutely not normal to have this pointer NULL.
	ASSERT( NULL != pclSDSmartControlValve );
	
	// Take SSheet size and client area size and determine the right width to take in count.
	CRect rectSize = pclSDSmartControlValve->GetSSheetPointer()->GetSheetSizeInPixels();

	// Prepare first column width (to match smart control valve sheet).
	long lFirstColumnWidth = pclSDSmartControlValve->GetSSheetPointer()->GetColWidthInPixelsW( CD_SmartControlValve_FirstColumn );

	// Do some correction in regards to some columns.
	long lTotalWidth = rectSize.Width() - lFirstColumnWidth;
	lTotalWidth -= pclSDSmartControlValve->GetSSheetPointer()->GetColWidthInPixelsW( CD_SmartControlValve_Pointer );

	// Try to create 2 columns in just the middle of smart control valve sheet.
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
	pclSSheet->SetStaticText( CD_Accessory_Left, RD_Accessory_GroupName, IDS_SSHEETSSELSMARVALVE_VALVEACCGROUP );

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

void CRViewSSelSmartControlValve::_ReadAllColumnWidth( CDS_IndSelParameter *pclIndSelParameter )
{
	// Access to the 'RViewSSelSmartControlValve' group.
	CDS_IndSelParameter::CCWWindow *pclCWWindow = pclIndSelParameter->GetWindowColumnWidth( CW_WINDOWID_INDSELSMARTCONTROLVALVE, true );

	// Access to the 'valve' group.
	ResetColumnWidth( SD_SmartControlValve );
	CDS_IndSelParameter::CCWSheet *pclCWSheet = pclCWWindow->GetCWSheet( CW_RVIEWSSELSMARTCONTROLVALVE_SMARTCONTROLVALVE_SHEETID );
	
	// 2019-12-13: Verify also that we have the same number of columns. Because during a development (Between two releases) it can happen
	// that we add columns without incrementing at each time the version. And it's generating bad dimensioning of the column width.
	if( NULL != pclCWSheet && m_mapSSheetColumnWidth[SD_SmartControlValve].size() == pclCWSheet->GetMap().size() )
	{
		short nVersion = pclCWSheet->GetVersion();

		if( CW_RVIEWSSELSMARTCONTROLVALVE_SMARTCONTROLVALVE_VERSION == nVersion )
		{
			m_mapSSheetColumnWidth[SD_SmartControlValve] = pclCWSheet->GetMap();
		}
	}
}

void CRViewSSelSmartControlValve::_WriteAllColumnWidth( CDS_IndSelParameter *pclIndSelParameter )
{
	// Access to the 'RViewSSelBv' group.
	CDS_IndSelParameter::CCWWindow *pclCWWindow = pclIndSelParameter->GetWindowColumnWidth( CW_WINDOWID_INDSELSMARTCONTROLVALVE, true );

	// Write the 'valve' group.
	CDS_IndSelParameter::CCWSheet *pclCWSheet = pclCWWindow->GetCWSheet( CW_RVIEWSSELSMARTCONTROLVALVE_SMARTCONTROLVALVE_SHEETID, true );
	pclCWSheet->SetVersion( CW_RVIEWSSELSMARTCONTROLVALVE_SMARTCONTROLVALVE_VERSION );
	pclCWSheet->GetMap() = m_mapSSheetColumnWidth[SD_SmartControlValve];
}
