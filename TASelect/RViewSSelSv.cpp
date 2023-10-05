#include "stdafx.h"
#include "afxctl.h"

#include "TASelect.h"
#include "MainFrm.h"

#include "DlgLeftTabSelManager.h"
#include "ProductSelectionParameters.h"
#include "RViewSSelSv.h"

CRViewSSelSv *pRViewSSelSv = NULL;

CRViewSSelSv::CRViewSSelSv() : CRViewSSelSS( CMainFrame::RightViewList::eRVSSelSv, false )
{
	m_pclIndSelSVParams = NULL;
	m_lShutOffValveSelectedRow = -1;
	m_lActuatorSelectedRow = 0;
	m_pCDBExpandCollapseRows = NULL;
	m_pCDBExpandCollapseGroupAccessory = NULL;
	m_pCDBExpandCollapseRowsActuator = NULL;
	m_pCDBExpandCollapseGroupActuatorAcc = NULL;
	m_pCDBShowAllPriorities = NULL;
	pRViewSSelSv = this;
}

CRViewSSelSv::~CRViewSSelSv()
{
	pRViewSSelSv = NULL;
}

CDB_TAProduct *CRViewSSelSv::GetCurrentShutOffValveSelected( void )
{
	CDB_TAProduct *pclCurrentShutoffValveSelected = NULL;
	CSheetDescription *pclSheetDescriptionSv = m_ViewDescription.GetFromSheetDescriptionID( SD_ShutOffValve );

	if( NULL != pclSheetDescriptionSv )
	{
		// Retrieve the current selected shut-off valve.
		CCellDescriptionProduct *pclCDCurrentShutoffValveSelected = NULL;
		LPARAM lpPointer = NULL;

		if( true == pclSheetDescriptionSv->GetUserVariable( _SDUV_SELECTEDPRODUCT, lpPointer ) )
		{
			pclCDCurrentShutoffValveSelected = ( CCellDescriptionProduct * )lpPointer;
		}

		if( NULL != pclCDCurrentShutoffValveSelected && NULL != pclCDCurrentShutoffValveSelected->GetProduct() )
		{
			pclCurrentShutoffValveSelected = dynamic_cast<CDB_TAProduct *>( (CData *)pclCDCurrentShutoffValveSelected->GetProduct() );
		}
	}

	return pclCurrentShutoffValveSelected;
}

CDB_Actuator *CRViewSSelSv::GetCurrentActuatorSelected( void )
{
	CDB_Actuator *pclCurrentActuatorSelected = NULL;
	CSheetDescription *pclSheetDescriptionActuator = m_ViewDescription.GetFromSheetDescriptionID( SD_ShutOffValveActuator );

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
			pclCurrentActuatorSelected = dynamic_cast<CDB_Actuator *>( (CData *)pclCDCurrentActuatorSelected->GetProduct() );
		}
	}

	return pclCurrentActuatorSelected;
}

void CRViewSSelSv::Reset()
{
	m_lShutOffValveSelectedRow = -1;
	m_lActuatorSelectedRow = -1;
	m_vecShutOffValveAccessoryList.clear();
	m_vecActuatorAccessories.clear();
	m_pCDBExpandCollapseRows = NULL;
	m_pCDBExpandCollapseGroupAccessory = NULL;
	m_pCDBExpandCollapseRowsActuator = NULL;
	m_pCDBExpandCollapseGroupActuatorAcc = NULL;
	m_pCDBShowAllPriorities = NULL;
	CRViewSSelSS::Reset();
}

void CRViewSSelSv::Suggest( CProductSelelectionParameters *pclProductSelectionParameters, LPARAM lpParam )
{
	CRViewSSelSS::Suggest( pclProductSelectionParameters, lpParam );

	if( NULL == pclProductSelectionParameters || NULL == dynamic_cast<CIndSelSVParams*>( pclProductSelectionParameters ) )
	{
		return;
	}

	m_pclIndSelSVParams = dynamic_cast<CIndSelSVParams*>( pclProductSelectionParameters );

	// To remove all current displayed sheets.
	Reset();

	BeginWaitCursor();
	CWnd::SetRedraw( FALSE );

	long lRowSelected = -1;

	CDS_SSelSv *pSelectedShutOffValve = NULL;

	// If we are in edition mode...
	if( NULL != m_pclIndSelSVParams->m_SelIDPtr.MP )
	{
		pSelectedShutOffValve = reinterpret_cast<CDS_SSelSv *>( m_pclIndSelSVParams->m_SelIDPtr.MP );

		if( NULL == pSelectedShutOffValve )
		{
			ASSERT( 0 );
		}

		lRowSelected = _FillShutOffValveRows( pSelectedShutOffValve );
	}
	else
	{
		lRowSelected = _FillShutOffValveRows();
	}

	// Verify if sheet description has been well created.
	CSheetDescription *pclSheetDescription = m_ViewDescription.GetFromSheetDescriptionID( SD_ShutOffValve );

	if( NULL != pclSheetDescription && NULL != pclSheetDescription->GetSSheetPointer() )
	{
		CSSheet *pclSSheet = pclSheetDescription->GetSSheetPointer();

		if( lRowSelected > -1 && NULL != pSelectedShutOffValve )
		{
			CCellDescriptionProduct *pclCDProduct = FindCDProduct( lRowSelected, (LPARAM)( pSelectedShutOffValve->GetProductAs<CDB_TAProduct>() ), pclSheetDescription );

			if( NULL != pclCDProduct && NULL != pclCDProduct->GetProduct() )
			{
				// If we are in edition mode we simulate a click on the product.
				OnClickProduct( pclSheetDescription, pclCDProduct, pclSheetDescription->GetActiveColumn(), lRowSelected );

				// Allow to check if we need to change the 'Show all priorities' button or not.
				CheckShowAllPrioritiesButtonState( pclSheetDescription, lRowSelected );

				// Verify accessories.
				CAccessoryList *pclAccessoryList = pSelectedShutOffValve->GetAccessoryList();
				CAccessoryList::AccessoryItem rAccessoryItem = pclAccessoryList->GetFirst( CAccessoryList::_AT_Accessory );

				while( rAccessoryItem.IDPtr.MP != NULL )
				{
					// Remark: 'true' because accessories in the 'pclCvAccessoryList' are those that are selected!
					VerifyCheckboxAccessories( (CDB_Product *)rAccessoryItem.IDPtr.MP, true, &m_vecShutOffValveAccessoryList );
					rAccessoryItem = pclAccessoryList->GetNext( CAccessoryList::_AT_Accessory );
				}

				// Verify if user has selected an actuator.
				IDPTR ActuatorIDPtr = pSelectedShutOffValve->GetActrIDPtr();
				CSheetDescription *pclSheetDescriptionActuator = m_ViewDescription.GetFromSheetDescriptionID( SD_ShutOffValveActuator );

				if( _NULL_IDPTR != ActuatorIDPtr && NULL != ActuatorIDPtr.MP && NULL != pclSheetDescriptionActuator )
				{
					CDB_Actuator *pEditedActuator = dynamic_cast<CDB_Actuator *>( ActuatorIDPtr.MP );

					if( NULL != pEditedActuator )
					{
						// Find row number where is the	actuator.
						long lActuatorRowSelected = _GetRowOfEditedActuator( pclSheetDescriptionActuator, pEditedActuator );

						if( lActuatorRowSelected != -1 )
						{
							CCellDescriptionProduct *pclCDActuator = FindCDProduct( lActuatorRowSelected, (LPARAM)pEditedActuator, pclSheetDescriptionActuator );

							if( NULL != pclCDActuator && NULL != pclCDActuator->GetProduct() )
							{
								// Simulate a click on the edited actuator.
								OnClickProduct( pclSheetDescriptionActuator, pclCDActuator, pclSheetDescriptionActuator->GetActiveColumn(), lActuatorRowSelected );

								// Verify accessories on actuator.
								CAccessoryList *pclActuatorAccessoryList = pSelectedShutOffValve->GetActuatorAccessoryList();
								rAccessoryItem = pclActuatorAccessoryList->GetFirst();

								while( rAccessoryItem.IDPtr.MP != NULL )
								{
									VerifyCheckboxAccessories( (CDB_Product *)rAccessoryItem.IDPtr.MP, true, &m_vecActuatorAccessories );
									rAccessoryItem = pclActuatorAccessoryList->GetNext();
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
			PrepareAndSetNewFocus( pclSheetDescription, CD_ShutoffValve_Name, RD_ShutoffValve_FirstAvailRow, 0 );
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

void CRViewSSelSv::FillInSelected( CDS_SelProd *pSelectedProductToFill )
{
	if( NULL == m_pclIndSelSVParams || NULL == m_pclIndSelSVParams->m_pclSelectSVList 
			|| NULL == m_pclIndSelSVParams->m_pclSelectSVList->GetSelectPipeList() )
	{
		ASSERT_RETURN;
	}

	CDS_SSelSv *pSelectedSvToFill = dynamic_cast<CDS_SSelSv *>( pSelectedProductToFill );

	if( NULL == pSelectedSvToFill )
	{
		return;
	}

	// Clear previous selected actuator.
	pSelectedSvToFill->SetActrIDPtr( _NULL_IDPTR );

	// Clear previous selected accessories on the shut-off valve.
	CAccessoryList *pclSvAccessoryListToFill = pSelectedSvToFill->GetAccessoryList();

	// Clear previous selected accessories on the actuator.
	CAccessoryList *pclActuatorAccessoryListToFill = pSelectedSvToFill->GetActuatorAccessoryList();
	bool bIsEditionMode = false;

	if( false == m_pclIndSelSVParams->m_bEditModeRunning )
	{
		pclSvAccessoryListToFill->Clear();
		pclActuatorAccessoryListToFill->Clear();
	}
	else
	{
		bIsEditionMode = true;
	}

	// Retrieve the current selected shut-off valve if exist.
	CDB_TAProduct *pclCurrentShutoffValveSelected = GetCurrentShutOffValveSelected();

	if( NULL != pclCurrentShutoffValveSelected )
	{
		pSelectedSvToFill->SetProductIDPtr( pclCurrentShutoffValveSelected->GetIDPtr() );

		// Search shut-off valve in CSelectList to set the correct opening.
		for( CSelectedValve *pclSelectedValve = m_pclIndSelSVParams->m_pclSelectSVList->GetFirst<CSelectedValve>(); NULL != pclSelectedValve;
				pclSelectedValve = m_pclIndSelSVParams->m_pclSelectSVList->GetNext<CSelectedValve>() )
		{
			if( pclSelectedValve->GetpData() == pclCurrentShutoffValveSelected )	// Product found
			{
				break;
			}
		}

		if( false == bIsEditionMode )
		{
			// Retrieve selected accessory and add it.
			for( vecCDCAccessoryListIter vecIter = m_vecShutOffValveAccessoryList.begin(); vecIter != m_vecShutOffValveAccessoryList.end(); vecIter++ )
			{
				CCDBCheckboxAccessory *pCDBCheckboxAccessory = *vecIter;

				if( NULL != pCDBCheckboxAccessory && true == pCDBCheckboxAccessory->GetCheckStatus() && NULL != pCDBCheckboxAccessory->GetAccessoryPointer() )
				{
					pclSvAccessoryListToFill->Add( pCDBCheckboxAccessory->GetAccessoryPointer()->GetIDPtr(), CAccessoryList::_AT_Accessory,
						pCDBCheckboxAccessory->GetRuledTable() );
				}
			}
		}
		else
		{
			UpdateAccessoryList( m_vecShutOffValveAccessoryList, pclSvAccessoryListToFill, CAccessoryList::_AT_Accessory );
		}

		// Selected pipe informations.
		m_pclIndSelSVParams->m_pclSelectSVList->GetSelectPipeList()->GetMatchingPipe( pclCurrentShutoffValveSelected->GetSizeKey(), 
				*pSelectedSvToFill->GetpSelPipe() );
	}

	// Fill data for actuator.
	CDB_Actuator *pclCurrentSelectedActuator = GetCurrentActuatorSelected();

	if( NULL != pclCurrentSelectedActuator )
	{
		pSelectedSvToFill->SetActrIDPtr( pclCurrentSelectedActuator->GetIDPtr() );

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
			UpdateAccessoryList( m_vecActuatorAccessories, pclActuatorAccessoryListToFill, CAccessoryList::_AT_Accessory );
		}
	}
}

void CRViewSSelSv::OnNewDocument( CDS_IndSelParameter *pclIndSelParameter )
{
	_ReadAllColumnWidth( pclIndSelParameter );
}

void CRViewSSelSv::SaveSelectionParameters( CDS_IndSelParameter *pclIndSelParameter )
{
	_WriteAllColumnWidth( pclIndSelParameter );
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PROTECTED MEMBERS
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CRViewSSelSv::OnTextTipFetch( CSheetDescription *pclSheetDescription, long lColumn, long lRow, WORD *pwMultiLine, SHORT *pnTipWidth, 
		TCHAR *pstrTipText, BOOL *pbShowTip )
{
	if( NULL == m_pclIndSelSVParams || NULL == m_pclIndSelSVParams->m_pclSelectSVList )
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
	CDB_ShutoffValve *pclShutoffValve = dynamic_cast<CDB_ShutoffValve *>( pTAP );
	CSelectedValve *pclSelectedSv = GetSelectProduct<CSelectedValve>( pTAP, m_pclIndSelSVParams->m_pclSelectSVList );
	CSSheet *pclSSheet = pclSheetDescription->GetSSheetPointer();
	CDS_TechnicalParameter *pclTechParam = m_pclIndSelSVParams->m_pTADS->GetpTechParams();

	switch( pclSheetDescription->GetSheetDescriptionID() )
	{
		// Check if mouse cursor has passed over a valve.
		case SD_ShutOffValve:

			if( NULL != pclSelectedSv )
			{
				if( CD_ShutoffValve_Dp == lColumn )
				{
					// TODO
					/*
					// Check if flag is set.
					if( true == pclSelectedSv->IsFlagSet( CSelectedBase::eDp ) )
					{
						// See 'Select.h' for description of errors.
						CString str2 = WriteCUDouble( _U_DIFFPRESS, pclSelectedSv->GetDp() );
						int iFlagError = pclSelectedSv->GetFlagError( CSelectedBase::eDp );

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
								double dDpMax = pclShutoffValve->GetDpmax();

								// IDS_SSHEETSSEL_DPERRORH: "Dp is above the maximum value defined in technical parameters ( %1 )"
								// IDS_SSHEETSSEL_DPERRORH2: "Dp is above the maximum value defined for this valve ( %1 )"
								int iMsgID = ( dDpMax <= 0.0 ) ? IDS_SSHEETSSEL_DPERRORH : IDS_SSHEETSSEL_DPERRORH2;

								str2 += CString( _T(" > ") ) + WriteCUDouble( _U_DIFFPRESS, pclTechParam->VerifyValvMaxDp( pclShutoffValve ), true );
								FormatString( str, iMsgID, str2 );
								break;
							}

							case CSelectList::BvFlagError::DpToReachTooLow:
								str2 += CString( _T(" > ") ) + WriteCUDouble( _U_DIFFPRESS, m_rBvNeededData.m_dDp, true );

								// "Pressure drop on valve fully open is already higher than requested Dp value ( %1 )"
								FormatString( str, IDS_SSHEETSSEL_DPERRORNOTF, str2 );
								break;

							case CSelectList::BvFlagError::DpQOTooLow:
								str2 = WriteCUDouble( _U_DIFFPRESS, pclSelectedSv->GetDpQuarterOpen() );
								str2 += CString( _T(" < ") ) + WriteCUDouble( _U_DIFFPRESS, pclTechParam->GetValvMinDp(), true );

								// "Pressure drop on valve at quarter opening is below the minimum valve pressure drop defined in technical parameters ( %1 )"
								FormatString( str, IDS_SSHEETSSELBV_DPQOTOOLOWERROR, str2 );
								break;

							case CSelectList::BvFlagError::DpFOTooHigh:
								str2 = WriteCUDouble( _U_DIFFPRESS, pclSelectedSv->GetDpFullOpen() );
								str2 += CString( _T(" > ") ) + WriteCUDouble( _U_DIFFPRESS, pclTechParam->GetValvMinDp(), true );

								// "Pressure drop on valve fully open is above the minimum valve pressure drop defined in technical parameters ( %1 )"
								FormatString( str, IDS_SSHEETSSELBV_DPFOTOOLOWERROR, str2 );
								break;
						}
					}
					*/
				}
				else if( CD_ShutoffValve_TemperatureRange == lColumn )
				{
					if( true == pclSelectedSv->IsFlagSet( CSelectedBase::eTemperature ) )
					{
						FormatString( str, IDS_SSHEETSSEL_TEMPERROR, pclShutoffValve->GetTempRange() );
					}
				}
				else if( CD_ShutoffValve_PipeLinDp == lColumn )
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
				else if( CD_ShutoffValve_PipeV == lColumn )
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

				if( true == str.IsEmpty() && NULL != pclShutoffValve )
				{
					if( 0 != _tcslen( pclShutoffValve->GetComment() ) )
					{
						str = pclShutoffValve->GetComment();
					}
				}
			}

			break;
		//HYS-726: show tooltip error
		case SD_ShutOffValveActuator:
			if( ( CD_Actuator_Name == lColumn || CD_Actuator_MaxTemp == lColumn ) && _RED == pclSSheet->GetForeColor( lColumn, lRow ) )
			{
				str = TASApp.LoadLocalizedString( IDS_SSHEETACT_TT_MAXTEMPERROR );
			}
			break;

		case SD_ShutOffValveAccessory:
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

bool CRViewSSelSv::OnClickProduct( CSheetDescription *pclSheetDescriptionSv, CCellDescriptionProduct *pclCellDescriptionProduct, long lColumn, long lRow )
{
	if( NULL == pclSheetDescriptionSv || NULL == pclSheetDescriptionSv->GetSSheetPointer() || NULL == pclCellDescriptionProduct )
	{
		ASSERTA_RETURN( false );
	}

	bool bNeedRefresh = true;
	CSSheet *pclSSheet = pclSheetDescriptionSv->GetSSheetPointer();
	pclSSheet->SetBool( SSB_REDRAW, FALSE );

	CDB_ShutoffValve *pclShutoffValve = dynamic_cast<CDB_ShutoffValve *>( (CData *)pclCellDescriptionProduct->GetProduct() );
	CDB_Actuator *pclActuator = dynamic_cast<CDB_Actuator *>( (CData *)pclCellDescriptionProduct->GetProduct() );

	if( NULL != pclShutoffValve )
	{
		// User has clicked on a control valve.
		_ClickOnShutOffValve( pclSheetDescriptionSv, pclShutoffValve, pclCellDescriptionProduct, lColumn, lRow );
	}
	else if( NULL != pclActuator )
	{
		// User has clicked on an actuator.
		_ClickOnActuator( pclSheetDescriptionSv, pclActuator, pclCellDescriptionProduct, lColumn, lRow );
	}
	else
	{
		bNeedRefresh = false;
	}

	pclSSheet->SetBool( SSB_REDRAW, TRUE );
	return bNeedRefresh;
}

bool CRViewSSelSv::ResetColumnWidth( short nSheetDescriptionID )
{
	TSpread clTSpread;

	if( FALSE == clTSpread.Create( WS_CHILD, CRect( 0, 0, 0, 0 ), this, 0 ) )
	{
		ASSERTA_RETURN( false );
	}

	switch( nSheetDescriptionID )
	{
		case SD_ShutOffValve:
			m_mapSSheetColumnWidth[SD_ShutOffValve][CD_ShutoffValve_FirstColumn] = clTSpread.ColWidthToLogUnits( 4 );
			m_mapSSheetColumnWidth[SD_ShutOffValve][CD_ShutoffValve_Box] = clTSpread.ColWidthToLogUnits( 2 );
			m_mapSSheetColumnWidth[SD_ShutOffValve][CD_ShutoffValve_CheckBox] = clTSpread.ColWidthToLogUnits( 2 );
			m_mapSSheetColumnWidth[SD_ShutOffValve][CD_ShutoffValve_Name] = clTSpread.ColWidthToLogUnits( 14 );
			m_mapSSheetColumnWidth[SD_ShutOffValve][CD_ShutoffValve_Material] = clTSpread.ColWidthToLogUnits( 12 );
			m_mapSSheetColumnWidth[SD_ShutOffValve][CD_ShutoffValve_Connection] = clTSpread.ColWidthToLogUnits( 14 );
			m_mapSSheetColumnWidth[SD_ShutOffValve][CD_ShutoffValve_Version] = clTSpread.ColWidthToLogUnits( 12 );
			m_mapSSheetColumnWidth[SD_ShutOffValve][CD_ShutoffValve_PN] = clTSpread.ColWidthToLogUnits( 6 );
			m_mapSSheetColumnWidth[SD_ShutOffValve][CD_ShutoffValve_Size] = clTSpread.ColWidthToLogUnits( 10 );
			m_mapSSheetColumnWidth[SD_ShutOffValve][CD_ShutoffValve_Dp] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_ShutOffValve][CD_ShutoffValve_TemperatureRange] = clTSpread.ColWidthToLogUnits( 8 );
			m_mapSSheetColumnWidth[SD_ShutOffValve][CD_ShutoffValve_Separator] = clTSpread.ColWidthToLogUnits( 1 );
			m_mapSSheetColumnWidth[SD_ShutOffValve][CD_ShutoffValve_PipeSize] = clTSpread.ColWidthToLogUnits( 8 );
			m_mapSSheetColumnWidth[SD_ShutOffValve][CD_ShutoffValve_PipeLinDp] = clTSpread.ColWidthToLogUnits( 8 );
			m_mapSSheetColumnWidth[SD_ShutOffValve][CD_ShutoffValve_PipeV] = clTSpread.ColWidthToLogUnits( 8 );
			m_mapSSheetColumnWidth[SD_ShutOffValve][CD_ShutoffValve_Pointer] = clTSpread.ColWidthToLogUnits( 1 );
			break;

		case SD_ShutOffValveActuator:
			m_mapSSheetColumnWidth[SD_ShutOffValveActuator][CD_Actuator_FirstColumn] = clTSpread.ColWidthToLogUnits( 4 );
			m_mapSSheetColumnWidth[SD_ShutOffValveActuator][CD_Actuator_Box] = clTSpread.ColWidthToLogUnits( 2 );
			m_mapSSheetColumnWidth[SD_ShutOffValveActuator][CD_Actuator_CheckBox] = clTSpread.ColWidthToLogUnits( 2 );
			m_mapSSheetColumnWidth[SD_ShutOffValveActuator][CD_Actuator_Name] = clTSpread.ColWidthToLogUnits( 30 );
			m_mapSSheetColumnWidth[SD_ShutOffValveActuator][CD_Actuator_CloseOffValue] = clTSpread.ColWidthToLogUnits( 15 );
			m_mapSSheetColumnWidth[SD_ShutOffValveActuator][CD_Actuator_ActuatingTime] = clTSpread.ColWidthToLogUnits( 10 );
			m_mapSSheetColumnWidth[SD_ShutOffValveActuator][CD_Actuator_IP] = clTSpread.ColWidthToLogUnits( 10 );
			m_mapSSheetColumnWidth[SD_ShutOffValveActuator][CD_Actuator_PowSupply] = clTSpread.ColWidthToLogUnits( 20 );
			m_mapSSheetColumnWidth[SD_ShutOffValveActuator][CD_Actuator_InputSig] = clTSpread.ColWidthToLogUnits( 20 );
			m_mapSSheetColumnWidth[SD_ShutOffValveActuator][CD_Actuator_OutputSig] = clTSpread.ColWidthToLogUnits( 20 );
			m_mapSSheetColumnWidth[SD_ShutOffValveActuator][CD_Actuator_RelayType] = clTSpread.ColWidthToLogUnits( 20 );
			m_mapSSheetColumnWidth[SD_ShutOffValveActuator][CD_Actuator_FailSafe] = clTSpread.ColWidthToLogUnits( 10 );
			m_mapSSheetColumnWidth[SD_ShutOffValveActuator][CD_Actuator_DefaultReturnPos] = clTSpread.ColWidthToLogUnits( 15 );
			m_mapSSheetColumnWidth[SD_ShutOffValveActuator][CD_Actuator_MaxTemp] = clTSpread.ColWidthToLogUnits( 8 );
			m_mapSSheetColumnWidth[SD_ShutOffValveActuator][CD_Actuator_Pointer] = clTSpread.ColWidthToLogUnits( 1 );
			// No yet!
			break;
	}

	if( clTSpread.GetSafeHwnd() != NULL )
	{
		clTSpread.DestroyWindow();
	}

	return true;
}

bool CRViewSSelSv::IsSelectionReady( void )
{
	CDB_TAProduct *pSelection = GetCurrentShutOffValveSelected();

	if( NULL != pSelection )
	{
		return ( false == pSelection->IsDeleted() );
	}

	return false; // No selection available
}

void CRViewSSelSv::SetCurrentShutoffValveSelected( CCellDescriptionProduct *pclCDCurrentShutoffValveSelected )
{
	// Try to retrieve sheet description linked to shut-off valve.
	CSheetDescription *pclSheetDescriptionSv = m_ViewDescription.GetFromSheetDescriptionID( SD_ShutOffValve );

	if( NULL != pclSheetDescriptionSv )
	{
		pclSheetDescriptionSv->SetUserVariable( _SDUV_SELECTEDPRODUCT, (LPARAM)pclCDCurrentShutoffValveSelected );
	}
}

void CRViewSSelSv::SetCurrentActuatorSelected( CCellDescriptionProduct *pclCDCurrentActuatorSelected )
{
	// Try to retrieve sheet description linked to actuator.
	CSheetDescription *pclSheetDescriptionActuator = m_ViewDescription.GetFromSheetDescriptionID( SD_ShutOffValveActuator );

	if( NULL != pclSheetDescriptionActuator )
	{
		pclSheetDescriptionActuator->SetUserVariable( _SDUV_SELECTEDPRODUCT, (LPARAM)pclCDCurrentActuatorSelected );
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PRIVATE MEMBERS
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CRViewSSelSv::_ClickOnShutOffValve( CSheetDescription *pclSheetDescriptionSv, CDB_ShutoffValve *pShutOffValveClicked, 
			CCellDescriptionProduct *pclCellDescriptionProduct, long lColumn, long lRow )
{
	CSSheet *pclSSheet = pclSheetDescriptionSv->GetSSheetPointer();

	m_lActuatorSelectedRow = 0;
	m_pCDBExpandCollapseGroupAccessory = NULL;
	m_pCDBExpandCollapseRowsActuator = NULL;
	m_pCDBExpandCollapseGroupActuatorAcc = NULL;

	// By default clear accessories list.
	m_vecShutOffValveAccessoryList.clear();
	m_vecActuatorAccessories.clear();

	SetCurrentActuatorSelected( NULL );

	// Memorize previous focus row.
	long lCurrentFocusedRow = GetCurrentFocusedRow();

	// To be sure to clean all previous data concerning focus.
	KillCurrentFocus();

	// Unselect row where current selection is set (yellow light).
	// Remark: 'KillCurrentFocus' unselect only rows where focus is on (blue light).
	pclSSheet->UnSelectMultipleRows();

	LPARAM lShutoffValveCount;
	pclSheetDescriptionSv->GetUserVariable( _SDUV_TOTALPRODUCT_COUNT, lShutoffValveCount );

	// Retrieve the current selected shut-off valve if exist.
	CDB_TAProduct *pclCurrentShutoffValveSelected = GetCurrentShutOffValveSelected();

	// If there is already one shut-off valve selected and user clicks on the current one...
	// Remark: 'm_pCDBExpandCollapseRows' is not created if there is only one shut-off valve. Thus we need to check first if there is only one shut-off valve.
	//         Otherwise we have to check if user has clicked on the current one.
	if( NULL != pclCurrentShutoffValveSelected 
			&& ( ( 1 == lShutoffValveCount ) 
				|| ( NULL != m_pCDBExpandCollapseRows && lRow == m_pCDBExpandCollapseRows->GetCellPosition().y ) ) )
	{
		// Reset current product selected.
		SetCurrentShutoffValveSelected( NULL );

		// Unselect checkbox.
		pclSSheet->SetCheckBox( CD_ShutoffValve_CheckBox, lRow, _T(""), false, true );

		// Delete Expand/Collapse rows button if exist.
		// Remark: 'm_pCDBExpandCollapseRows' is set to NULL in method!
		DeleteExpandCollapseRowsButton( m_pCDBExpandCollapseRows, pclSheetDescriptionSv );

		// Set focus on shut-off valve currently selected.
		PrepareAndSetNewFocus( pclSheetDescriptionSv, pclSheetDescriptionSv->GetActiveColumn(), lRow, 0 );

		// Remove all sheets after control valve.
		m_ViewDescription.RemoveAllSheetAfter( SD_ShutOffValve );

		m_lShutOffValveSelectedRow = -1;

		// Notify a change in the selection.
		SelectionHasChanged( IsSelectionReady() );
	}
	else
	{
		// If a shut-off valve is already selected...
		if( NULL != pclCurrentShutoffValveSelected )
		{
			// Unselect checkbox.
			pclSSheet->SetCheckBox(CD_ShutoffValve_CheckBox, m_lShutOffValveSelectedRow, _T(""), false, true);
			m_lShutOffValveSelectedRow = -1;

			// Remove all sheets after shut-off valve.
			m_ViewDescription.RemoveAllSheetAfter( SD_ShutOffValve );
		}

		// Save new shut-off valve selection.
		SetCurrentShutoffValveSelected( pclCellDescriptionProduct );

		// Delete Expand/Collapse rows button if exist.
		if( NULL != m_pCDBExpandCollapseRows )
		{
			// Remark: 'm_pCDBExpandCollapseRows' is set to NULL in method!
			DeleteExpandCollapseRowsButton( m_pCDBExpandCollapseRows, pclSheetDescriptionSv );
		}

		// Create Expand/Collapse rows button if needed...
		LPARAM lValveTotalCount;
		pclSheetDescriptionSv->GetUserVariable( _SDUV_TOTALPRODUCT_COUNT, lValveTotalCount );

		if( lValveTotalCount > 1 )
		{
			m_pCDBExpandCollapseRows = CreateExpandCollapseRowsButton( CD_ShutoffValve_FirstColumn, lRow, true, 
					CCDButtonExpandCollapseRows::ButtonState::CollapseRow, pclSheetDescriptionSv->GetFirstSelectableRow(), 
					pclSheetDescriptionSv->GetLastSelectableRow( false ), pclSheetDescriptionSv );

			// Show button.
			if( NULL != m_pCDBExpandCollapseRows )
			{
				m_pCDBExpandCollapseRows->SetShowStatus( true );
			}
		}

		// Check the checkbox.
		pclSSheet->SetCheckBox( CD_ShutoffValve_CheckBox, lRow, _T(""), true, true );
		
		// Select shut-off valve (just highlight background).
		pclSSheet->SelectOneRow( lRow, pclSheetDescriptionSv->GetSelectionFrom(), pclSheetDescriptionSv->GetSelectionTo() );

		// Fill corresponding accessories.
		_FillShutOffAccessoryRows();

		// Fill actuators available for the current shut-off valve.
		_FillShutOffActuatorRows();

		// Memorize control valve row selected.
		m_lShutOffValveSelectedRow = lRow;

		// Try to set the focus on the next table but do not lose the focus if no other table exist.
		CSheetDescription *pclNextSheetDescription = NULL;
		long lNewFocusedRow;
		bool bShiftPressed;

		if( DNFERRC_FocusOnNextGroupOK == DefineNextFocusedRow( pclSheetDescriptionSv, CD_ShutoffValve_FirstColumn, lRow, false, lNewFocusedRow,
				pclNextSheetDescription, bShiftPressed ) )
		{
			long lNewFocusedCol = pclNextSheetDescription->GetFirstParameterColumn();
			PrepareAndSetNewFocus( pclNextSheetDescription, lNewFocusedCol, lNewFocusedRow );
		}
		else
		{
			// Because we have changed current selection background in yellow, if there is no other sheet following current one, we
			// must set focus on current selection.
			PrepareAndSetNewFocus( pclSheetDescriptionSv, lColumn, lRow );
		}

		// Notify a change in the selection.
		SelectionHasChanged( IsSelectionReady() );
	}
}

long CRViewSSelSv::_FillShutOffValveRows( CDS_SSelSv *pEditedProduct )
{
	if( NULL == m_pclIndSelSVParams || NULL == m_pclIndSelSVParams->m_pclSelectSVList 
			|| NULL == m_pclIndSelSVParams->m_pclSelectSVList->GetSelectPipeList() || NULL == m_pclIndSelSVParams->m_pTADS
			|| NULL == m_pclIndSelSVParams->m_pPipeDB )
	{
		ASSERTA_RETURN( 0 );
	}

	// Call 'MultiSpreadBase' method to get a new SSheet.
	CSheetDescription *pclSheetDescriptionSv = CreateSSheet( SD_ShutOffValve );

	if( NULL == pclSheetDescriptionSv || NULL == pclSheetDescriptionSv->GetSSheetPointer() )
	{
		return 0;
	}

	CSSheet *pclSSheet = pclSheetDescriptionSv->GetSSheetPointer();

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
	CSelectedValve *pclSelectedValve = m_pclIndSelSVParams->m_pclSelectSVList->GetFirst<CSelectedValve>();

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

	// Set max rows.
	pclSSheet->SetMaxRows( RD_ShutoffValve_FirstAvailRow - 1 );

	// All cells are static by default and filled with _T("").
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	pclSSheet->FormatStaticText( -1, -1, -1, -1, _T( "" ) );

	// Increase row height.
	double RowHeight = 12.75;
	pclSSheet->SetRowHeight( RD_ShutoffValve_FirstRow, RowHeight * 0.5 );
	pclSSheet->SetRowHeight( RD_ShutoffValve_GroupName, RowHeight * 1.5 );
	pclSSheet->SetRowHeight( RD_ShutoffValve_ColName, RowHeight * 2 );
	pclSSheet->SetRowHeight( RD_ShutoffValve_Unit, RowHeight * 1.2 );

	// Initialize.
	pclSheetDescriptionSv->Init( 1, pclSSheet->GetMaxRows(), 1, CSheetDescription::SD_ParameterMode_Mono );
	pclSheetDescriptionSv->AddColumnInPixels( CD_ShutoffValve_FirstColumn, m_mapSSheetColumnWidth[SD_ShutOffValve][CD_ShutoffValve_FirstColumn] );
	pclSheetDescriptionSv->AddColumnInPixels( CD_ShutoffValve_Box, m_mapSSheetColumnWidth[SD_ShutOffValve][CD_ShutoffValve_Box] );
	pclSheetDescriptionSv->AddColumnInPixels( CD_ShutoffValve_CheckBox, m_mapSSheetColumnWidth[SD_ShutOffValve][CD_ShutoffValve_CheckBox] );
	pclSheetDescriptionSv->AddColumnInPixels( CD_ShutoffValve_Name, m_mapSSheetColumnWidth[SD_ShutOffValve][CD_ShutoffValve_Name] );
	pclSheetDescriptionSv->AddColumnInPixels( CD_ShutoffValve_Material, m_mapSSheetColumnWidth[SD_ShutOffValve][CD_ShutoffValve_Material] );
	pclSheetDescriptionSv->AddColumnInPixels( CD_ShutoffValve_Connection, m_mapSSheetColumnWidth[SD_ShutOffValve][CD_ShutoffValve_Connection] );
	pclSheetDescriptionSv->AddColumnInPixels( CD_ShutoffValve_Version, m_mapSSheetColumnWidth[SD_ShutOffValve][CD_ShutoffValve_Version] );
	pclSheetDescriptionSv->AddColumnInPixels( CD_ShutoffValve_PN, m_mapSSheetColumnWidth[SD_ShutOffValve][CD_ShutoffValve_PN] );
	pclSheetDescriptionSv->AddColumnInPixels( CD_ShutoffValve_Size, m_mapSSheetColumnWidth[SD_ShutOffValve][CD_ShutoffValve_Size] );
	pclSheetDescriptionSv->AddColumnInPixels( CD_ShutoffValve_Dp, m_mapSSheetColumnWidth[SD_ShutOffValve][CD_ShutoffValve_Dp] );
	pclSheetDescriptionSv->AddColumnInPixels( CD_ShutoffValve_TemperatureRange, m_mapSSheetColumnWidth[SD_ShutOffValve][CD_ShutoffValve_TemperatureRange] );
	pclSheetDescriptionSv->AddColumnInPixels( CD_ShutoffValve_Separator, m_mapSSheetColumnWidth[SD_ShutOffValve][CD_ShutoffValve_Separator] );
	pclSheetDescriptionSv->AddColumnInPixels( CD_ShutoffValve_PipeSize, m_mapSSheetColumnWidth[SD_ShutOffValve][CD_ShutoffValve_PipeSize] );
	pclSheetDescriptionSv->AddColumnInPixels( CD_ShutoffValve_PipeLinDp, m_mapSSheetColumnWidth[SD_ShutOffValve][CD_ShutoffValve_PipeLinDp] );
	pclSheetDescriptionSv->AddColumnInPixels( CD_ShutoffValve_PipeV, m_mapSSheetColumnWidth[SD_ShutOffValve][CD_ShutoffValve_PipeV] );
	pclSheetDescriptionSv->AddColumnInPixels( CD_ShutoffValve_Pointer, m_mapSSheetColumnWidth[SD_ShutOffValve][CD_ShutoffValve_Pointer] );

	// Set in which column parameter must be saved.
	pclSheetDescriptionSv->AddParameterColumn( CD_ShutoffValve_Pointer );

	// Set the focus column.
	pclSheetDescriptionSv->SetActiveColumn( CD_ShutoffValve_Name );

	// Set range for selection.
	pclSheetDescriptionSv->SetFocusColumnRange( CD_ShutoffValve_CheckBox, CD_ShutoffValve_TemperatureRange );

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

	pclSSheet->SetStaticText( CD_ShutoffValve_Name, RD_ShutoffValve_ColName, IDS_SSHEETSSEL_NAME );
	pclSSheet->SetStaticText( CD_ShutoffValve_Material, RD_ShutoffValve_ColName, IDS_SSHEETSSEL_MATERIAL );
	pclSSheet->SetStaticText( CD_ShutoffValve_Connection, RD_ShutoffValve_ColName, IDS_SSHEETSSEL_CONNECT );
	pclSSheet->SetStaticText( CD_ShutoffValve_Version, RD_ShutoffValve_ColName, IDS_SSHEETSSEL_VERSION );
	pclSSheet->SetStaticText( CD_ShutoffValve_PN, RD_ShutoffValve_ColName, IDS_SSHEETSSEL_PN );
	pclSSheet->SetStaticText( CD_ShutoffValve_Size, RD_ShutoffValve_ColName, IDS_SSHEETSSEL_SIZE );
	pclSSheet->SetStaticText( CD_ShutoffValve_Dp, RD_ShutoffValve_ColName, IDS_SSHEETSSEL_DP );
	pclSSheet->SetStaticText( CD_ShutoffValve_TemperatureRange, RD_ShutoffValve_ColName, IDS_SSHEETSSEL_COLTEMPRANGE );

	pclSSheet->SetStaticText( CD_ShutoffValve_Separator, RD_ShutoffValve_ColName, _T( "" ) );

	// Pipes.
	pclSSheet->SetStaticText( CD_ShutoffValve_PipeSize, RD_ShutoffValve_ColName, IDS_SSHEETSSEL_PIPESIZE );
	pclSSheet->SetStaticText( CD_ShutoffValve_PipeLinDp, RD_ShutoffValve_ColName, IDS_SSHEETSSEL_PIPELINDP );
	pclSSheet->SetStaticText( CD_ShutoffValve_PipeV, RD_ShutoffValve_ColName, IDS_SSHEETSSEL_PIPEV );
	//
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Row units.
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();
	pclSSheet->SetStaticText( CD_ShutoffValve_Dp, RD_ShutoffValve_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ) ).c_str() );
	pclSSheet->SetStaticText( CD_ShutoffValve_TemperatureRange, RD_ShutoffValve_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_TEMPERATURE ) ).c_str() );

	// Units.
	pclSSheet->SetStaticText( CD_ShutoffValve_PipeLinDp, RD_ShutoffValve_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_LINPRESSDROP ) ).c_str() );
	pclSSheet->SetStaticText( CD_ShutoffValve_PipeV, RD_ShutoffValve_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_VELOCITY ) ).c_str() );
	//
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Draw black line below column header.
	pclSSheet->SetCellBorder( CD_ShutoffValve_CheckBox, RD_ShutoffValve_Unit, CD_ShutoffValve_Separator - 1, RD_ShutoffValve_Unit, true, 
			SS_BORDERTYPE_BOTTOM );

	pclSSheet->SetCellBorder( CD_ShutoffValve_PipeSize, RD_ShutoffValve_Unit, CD_ShutoffValve_Pointer - 1, RD_ShutoffValve_Unit, true,
			 SS_BORDERTYPE_BOTTOM );

	CSelectPipe selPipe( m_pclIndSelSVParams );
	long lRetRow = -1;

	CDS_TechnicalParameter *pTechParam = m_pclIndSelSVParams->m_pTADS->GetpTechParams();

	long lRow = RD_ShutoffValve_FirstAvailRow;

	long lValveTotalCount = 0;
	long lValveNotPriorityCount = 0;
	bool bShowAllPrioritiesShown = false;
	long lShowAllPrioritiesButtonRow = 0;
	bool bAtLeastOneError;
	bool bAtLeastOneWarning;

	pclSheetDescriptionSv->SetUserVariable( _SDUV_TOTALPRODUCT_COUNT, 0 );
	pclSheetDescriptionSv->SetUserVariable( _SDUV_NOPRIORITYPRODUCT_COUNT, 0 );

	pclSheetDescriptionSv->RestartRemarkGenerator();

	for( pclSelectedValve = m_pclIndSelSVParams->m_pclSelectSVList->GetFirst<CSelectedValve>(); NULL != pclSelectedValve;
			pclSelectedValve = m_pclIndSelSVParams->m_pclSelectSVList->GetNext<CSelectedValve>() )
	{
		pTAP = dynamic_cast<CDB_TAProduct *>( pclSelectedValve->GetpData() );

		if( NULL == pTAP )
		{
			continue;
		}

		bAtLeastOneError = false;
		bAtLeastOneWarning = false;
		bool bBest = pclSelectedValve->IsFlagSet( CSelectedBase::eBest );

		if( true == pclSelectedValve->IsFlagSet( CSelectedBase::eNotPriority ) )
		{
			lValveNotPriorityCount++;

			if( false == bShowAllPrioritiesShown )
			{
				// Add one empty not selectable row.
				pclSheetDescriptionSv->AddRows( 1 );
				pclSheetDescriptionSv->RemoveSelectableRow( lRow );

				pclSSheet->SetCellBorder( CD_ShutoffValve_CheckBox, lRow, CD_ShutoffValve_Separator - 1, lRow, true, SS_BORDERTYPE_TOP | SS_BORDERTYPE_BOTTOM,
						SS_BORDERSTYLE_SOLID, _BLACK );

				pclSSheet->SetCellBorder( CD_ShutoffValve_PipeSize, lRow, CD_ShutoffValve_Pointer - 1, lRow, true, SS_BORDERTYPE_TOP | SS_BORDERTYPE_BOTTOM,
						SS_BORDERSTYLE_SOLID, _BLACK );

				lShowAllPrioritiesButtonRow = lRow++;
				bShowAllPrioritiesShown = true;
			}
		}

		// Add this row.
		// 'true' to specify that this row can be selected.
		pclSheetDescriptionSv->AddRows( 1, true );

		// First columns will be set at the end!

		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

		// Add checkbox.
		pclSSheet->SetCheckBox(CD_ShutoffValve_CheckBox, lRow, _T(""), false, true);

		// Set Dp.
		if( true == pclSelectedValve->IsFlagSet( CSelectedBase::eDp ) )
		{
			// TODO: verify that it is well this function (VerifyValvMaxDp) to call !!
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

		pclSSheet->SetStaticText( CD_ShutoffValve_Dp, lRow, WriteCUDouble( _U_DIFFPRESS, pclSelectedValve->GetDp() ) );
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

		// Temperature range.
		if( true == pclSelectedValve->IsFlagSet( CSelectedBase::eTemperature ) )
		{
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_RED );
			bAtLeastOneError = true;
		}

		pclSSheet->SetStaticText( CD_ShutoffValve_TemperatureRange, lRow, ( ( CDB_TAProduct * )pclSelectedValve->GetProductIDPtr().MP )->GetTempRange() );
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

		// In addition of current flag, we have possibility that valve has the flag 'Not available' or 'Deleted'. In that case, we show valve name in red with "*" around it and
		// symbol '!' or '!!' after.
		if( true == pTAP->IsDeleted() )
		{
			pclSheetDescriptionSv->WriteTextWithFlags( CString( pTAP->GetName() ), CD_ShutoffValve_Name, lRow, CSheetDescription::RemarkFlags::Deleted, _T(""), _RED );
		}
		else if( false == pTAP->IsAvailable() )
		{
			pclSheetDescriptionSv->WriteTextWithFlags( CString( pTAP->GetName() ), CD_ShutoffValve_Name, lRow, CSheetDescription::RemarkFlags::NotAvailable, _T(""), _RED );
		}
		else
		{
			pclSSheet->SetStaticText( CD_ShutoffValve_Name, lRow, pTAP->GetName() );
		}

		if( true == bAtLeastOneError )
		{
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );
		}
		else if( true == bBest )
		{
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_GREEN );
		}

		pclSSheet->SetStaticText( CD_ShutoffValve_Material, lRow, pTAP->GetBodyMaterial() );
		pclSSheet->SetStaticText( CD_ShutoffValve_Connection, lRow, pTAP->GetConnect() );
		pclSSheet->SetStaticText( CD_ShutoffValve_Version, lRow, pTAP->GetVersion() );
		pclSSheet->SetStaticText( CD_ShutoffValve_PN, lRow, pTAP->GetPN().c_str() );
		pclSSheet->SetStaticText( CD_ShutoffValve_Size, lRow, pTAP->GetSize() );

		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

		// Set pipe size.
		m_pclIndSelSVParams->m_pclSelectSVList->GetSelectPipeList()->GetMatchingPipe( pTAP->GetSizeKey(), selPipe );
		pclSSheet->SetStaticText( CD_ShutoffValve_PipeSize, lRow, selPipe.GetpPipe()->GetName() );

		// Set the LinDp to orange if it is above or below the technical parameters limits.
		if( selPipe.GetLinDp() > pTechParam->GetPipeMaxDp() || selPipe.GetLinDp() < pTechParam->GetPipeMinDp() )
		{
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_ORANGE );
		}

		pclSSheet->SetStaticText( CD_ShutoffValve_PipeLinDp, lRow, WriteCUDouble( _U_LINPRESSDROP, selPipe.GetLinDp() ) );
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

		// Set the velocity to orange if it is above the technical parameter limit.
		// Orange if it is below the dMinVel.
		if( selPipe.GetU() > pTechParam->GetPipeMaxVel() || selPipe.GetU() < pTechParam->GetPipeMinVel() )
		{
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_ORANGE );
		}

		pclSSheet->SetStaticText( CD_ShutoffValve_PipeV, lRow, WriteCUDouble( _U_VELOCITY, selPipe.GetU() ) );
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

		// Save parameter.
		CreateCellDescriptionProduct( pclSheetDescriptionSv->GetFirstParameterColumn(), lRow, (LPARAM)pTAP, pclSheetDescriptionSv );

		if( NULL != pEditedTAP && pEditedTAP == pTAP )
		{
			lRetRow = lRow;
		}

		pclSSheet->SetCellBorder( CD_ShutoffValve_CheckBox, lRow, CD_ShutoffValve_Separator - 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _GRAY );
		pclSSheet->SetCellBorder( CD_ShutoffValve_PipeSize, lRow, CD_ShutoffValve_Pointer - 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _GRAY );

		lRow++;
		lValveTotalCount++;
	}

	long lLastDataRow = lRow - 1;

	// Hide columns corresponding to user selected combos.
	pclSSheet->ShowCol( CD_ShutoffValve_Box, FALSE );
	pclSSheet->ShowCol( CD_ShutoffValve_Material, ( true == m_pclIndSelSVParams->m_strComboMaterialID.IsEmpty() ) ? TRUE : FALSE );
	pclSSheet->ShowCol( CD_ShutoffValve_Connection, ( true == m_pclIndSelSVParams->m_strComboConnectID.IsEmpty() ) ? TRUE : FALSE );
	pclSSheet->ShowCol( CD_ShutoffValve_Version, ( true == m_pclIndSelSVParams->m_strComboVersionID.IsEmpty() ) ? TRUE : FALSE );
	pclSSheet->ShowCol( CD_ShutoffValve_PN, ( true == m_pclIndSelSVParams->m_strComboPNID.IsEmpty() ) ? TRUE : FALSE );

	pclSheetDescriptionSv->SetUserVariable( _SDUV_TOTALPRODUCT_COUNT, lValveTotalCount );
	pclSheetDescriptionSv->SetUserVariable( _SDUV_NOPRIORITYPRODUCT_COUNT, lValveNotPriorityCount );

	// Now we can create Show/Hide all priorities if needed.
	// Remarks: 2nd condition to avoid to have button if we have only no priority valves.
	if( true == bShowAllPrioritiesShown && lValveTotalCount > lValveNotPriorityCount )
	{
		// Create Show/Hide all priorities button in regards to current state!
		CCDButtonShowAllPriorities::ButtonState eState = ( true == GetShowAllPrioritiesFlag( SD_ShutOffValve ) ) ? 
				CCDButtonShowAllPriorities::ButtonState::HidePriorities : CCDButtonShowAllPriorities::ButtonState::ShowPriorities;
		
		CCDButtonShowAllPriorities *pclShowAllButton = CreateShowAllPrioritiesButton( CD_ShutoffValve_FirstColumn, lShowAllPrioritiesButtonRow,
				bShowAllPrioritiesShown, eState, lShowAllPrioritiesButtonRow + 1, pclSSheet->GetMaxRows(), pclSheetDescriptionSv );

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

	// Try to merge only if there is more than one shut-off valve.
	if( lValveTotalCount > 2 || ( 2 == lValveTotalCount && lValveNotPriorityCount != 1 ) )
	{
		vector<long> vecColumnList;
		vecColumnList.push_back( CD_ShutoffValve_PipeSize );
		vecColumnList.push_back( CD_ShutoffValve_PipeLinDp );
		vecColumnList.push_back( CD_ShutoffValve_PipeV );
		pclSSheet->MergeRowsWithSameValues( RD_ShutoffValve_FirstAvailRow, lLastDataRow, vecColumnList );
	}

	pclSSheet->SetCellBorder( CD_ShutoffValve_CheckBox, lLastDataRow, CD_ShutoffValve_Separator - 1, lLastDataRow, true, SS_BORDERTYPE_BOTTOM, 
			SS_BORDERSTYLE_SOLID, _BLACK );

	// Needed by pipe merging.
	pclSSheet->SetCellBorder( CD_ShutoffValve_PipeSize, lLastDataRow, CD_ShutoffValve_Pointer - 1, lLastDataRow, true, SS_BORDERTYPE_BOTTOM,
			SS_BORDERSTYLE_SOLID, _BLACK );

	// Add Remarks.
	// Note: these lines must be placed after creating Show/Hide all priorities button to avoid to take in count these lines when defining
	//       range of row to be Show/Hide.
	pclSheetDescriptionSv->WriteRemarks( lRow, CD_ShutoffValve_CheckBox, CD_ShutoffValve_Separator );

	// Add the main title.
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)GetTitleBackgroundColor( pclSheetDescriptionSv ) );
	pclSheetDescriptionSv->SetUserVariable( _SDUV_TITLEFORECOLOR, ( pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor ) ) );
	pclSheetDescriptionSv->SetUserVariable( _SDUV_TITLEBACKCOLOR, ( pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor ) ) );

	pclSSheet->AddCellSpanW( CD_ShutoffValve_CheckBox, RD_ShutoffValve_GroupName, CD_ShutoffValve_Separator - CD_ShutoffValve_CheckBox, 1 );
	pclSSheet->SetStaticText( CD_ShutoffValve_CheckBox, RD_ShutoffValve_GroupName, IDS_SSHEETSSELSV_VALVEGROUP );

	pclSSheet->AddCellSpanW( CD_ShutoffValve_PipeSize, RD_ShutoffValve_GroupName, CD_ShutoffValve_Pointer - CD_ShutoffValve_PipeSize, 1 );

	CTable *pclTable = dynamic_cast<CTable *>( m_pclIndSelSVParams->m_pPipeDB->Get( m_pclIndSelSVParams->m_strPipeSeriesID ).MP );
	ASSERT( NULL != pclTable );

	pclSSheet->SetStaticText( CD_ShutoffValve_PipeSize, RD_ShutoffValve_GroupName, pclTable->GetName() );

	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

	pclSSheet->SetBool( SSB_REDRAW, TRUE );

	// Set that there is no selection at now.
	SetCurrentShutoffValveSelected( NULL );

	// Add possibility to change column size.
	// Remark: place this code after any initialization (like adding, removing, resizing, hiding/showing columns).
	CResizingColumnInfo *pclResizingColumnInfo = pclSheetDescriptionSv->GetResizingColumnInfo();
	pclResizingColumnInfo->ActivateFeature( true, false, RD_ShutoffValve_FirstAvailRow, lLastDataRow );
	pclResizingColumnInfo->AddRangeColumn( CD_ShutoffValve_Name, CD_ShutoffValve_TemperatureRange, RD_ShutoffValve_ColName, RD_ShutoffValve_Unit );
	pclResizingColumnInfo->AddRangeColumn( CD_ShutoffValve_PipeSize, CD_ShutoffValve_PipeV, RD_ShutoffValve_ColName, RD_ShutoffValve_Unit );

	// Add possibility to click on title group to reset column width.
	CreateTitleGroupButtons( CD_ShutoffValve_CheckBox, CD_ShutoffValve_PipeV, RD_ShutoffValve_GroupName, pclSheetDescriptionSv );

	return lRetRow;
}

void CRViewSSelSv::_FillShutOffAccessoryRows( )
{
	if( NULL == m_pclIndSelSVParams || NULL == m_pclIndSelSVParams->m_pTADB )
	{
		ASSERT_RETURN;
	}

	// Retrieve the current selected shut-off valve.
	CDB_TAProduct *pclCurrentShutoffValveSelected = GetCurrentShutOffValveSelected();

	if( NULL == pclCurrentShutoffValveSelected )
	{
		return;
	}

	CDB_RuledTable *pclRuledTable = ( CDB_RuledTable * )( pclCurrentShutoffValveSelected->GetAccessoriesGroupIDPtr().MP );

	if( NULL == pclRuledTable )
	{
		return;
	}

	CRank rList;
	m_pclIndSelSVParams->m_pTADB->GetAccessories( &rList, pclRuledTable, m_pclIndSelSVParams->m_eFilterSelection );
	_FillRowsAcc( IDS_SSHEETSSELSV_VALVEACCGROUP, true, &rList, pclRuledTable );
}

void CRViewSSelSv::_FillShutOffActuatorRows()
{
	if( NULL == m_pclIndSelSVParams || NULL == m_pclIndSelSVParams->m_pclSelectSVList )
	{
		ASSERT_RETURN;
	}

	CDB_ShutoffValve *pclSelectedShutOffValve = dynamic_cast<CDB_ShutoffValve *>( GetCurrentShutOffValveSelected() );

	if( NULL == pclSelectedShutOffValve )
	{
		return;
	}

	CRank clActuatorList;
	_GetActuatorList( pclSelectedShutOffValve, &clActuatorList );

	if( 0 == clActuatorList.GetCount() )
	{
		return;
	}

	// Call 'MultiSpreadBase' method to get a new SSheet.
	CSheetDescription *pclSheetDescriptionActuator = CreateSSheet( SD_ShutOffValveActuator );

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
	pclSheetDescriptionActuator->AddColumnInPixels( CD_Actuator_FirstColumn, m_mapSSheetColumnWidth[SD_ShutOffValveActuator][CD_Actuator_FirstColumn] );
	pclSheetDescriptionActuator->AddColumnInPixels( CD_Actuator_Box, m_mapSSheetColumnWidth[SD_ShutOffValveActuator][CD_Actuator_Box] );
	pclSheetDescriptionActuator->AddColumnInPixels( CD_Actuator_CheckBox, m_mapSSheetColumnWidth[SD_ShutOffValveActuator][CD_Actuator_CheckBox] );
	pclSheetDescriptionActuator->AddColumnInPixels( CD_Actuator_Name, m_mapSSheetColumnWidth[SD_ShutOffValveActuator][CD_Actuator_Name] );
	pclSheetDescriptionActuator->AddColumnInPixels( CD_Actuator_CloseOffValue, m_mapSSheetColumnWidth[SD_ShutOffValveActuator][CD_Actuator_CloseOffValue] );
	pclSheetDescriptionActuator->AddColumnInPixels( CD_Actuator_ActuatingTime, m_mapSSheetColumnWidth[SD_ShutOffValveActuator][CD_Actuator_ActuatingTime] );
	pclSheetDescriptionActuator->AddColumnInPixels( CD_Actuator_IP, m_mapSSheetColumnWidth[SD_ShutOffValveActuator][CD_Actuator_IP] );
	pclSheetDescriptionActuator->AddColumnInPixels( CD_Actuator_PowSupply, m_mapSSheetColumnWidth[SD_ShutOffValveActuator][CD_Actuator_PowSupply] );
	pclSheetDescriptionActuator->AddColumnInPixels( CD_Actuator_InputSig, m_mapSSheetColumnWidth[SD_ShutOffValveActuator][CD_Actuator_InputSig] );
	pclSheetDescriptionActuator->AddColumnInPixels( CD_Actuator_OutputSig, m_mapSSheetColumnWidth[SD_ShutOffValveActuator][CD_Actuator_OutputSig] );
	pclSheetDescriptionActuator->AddColumnInPixels( CD_Actuator_RelayType, m_mapSSheetColumnWidth[SD_ShutOffValveActuator][CD_Actuator_RelayType] );
	pclSheetDescriptionActuator->AddColumnInPixels( CD_Actuator_FailSafe, m_mapSSheetColumnWidth[SD_ShutOffValveActuator][CD_Actuator_FailSafe] );
	pclSheetDescriptionActuator->AddColumnInPixels( CD_Actuator_DefaultReturnPos, m_mapSSheetColumnWidth[SD_ShutOffValveActuator][CD_Actuator_DefaultReturnPos] );
	pclSheetDescriptionActuator->AddColumnInPixels( CD_Actuator_MaxTemp, m_mapSSheetColumnWidth[SD_ShutOffValveActuator][CD_Actuator_MaxTemp] );
	pclSheetDescriptionActuator->AddColumnInPixels( CD_Actuator_Pointer, m_mapSSheetColumnWidth[SD_ShutOffValveActuator][CD_Actuator_Pointer] );

	// Show or hide box column.
	pclSSheet->ShowCol( CD_Actuator_Box, FALSE );

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
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)GetTitleBackgroundColor( pclSheetDescriptionActuator ) );
	
	pclSheetDescriptionActuator->SetUserVariable( _SDUV_TITLEFORECOLOR,
			( pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor ) ) );
	
	pclSheetDescriptionActuator->SetUserVariable( _SDUV_TITLEBACKCOLOR,
			( pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor ) ) );

	pclSSheet->AddCellSpanW( CD_Actuator_CheckBox, RD_Actuator_GroupName, CD_Actuator_Pointer - CD_Actuator_CheckBox, 1 );
	pclSSheet->SetStaticText( CD_Actuator_CheckBox, RD_Actuator_GroupName, IDS_SSHEETSSELSV_ACTRGROUP );

	// Columns title.
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

	pclSSheet->SetStaticText( CD_Actuator_Name, RD_Actuator_ColName, IDS_SSHEETSSELSV_ACTRNAME );

	pclSSheet->ShowCol( CD_Actuator_CloseOffValue, FALSE );
	
	pclSSheet->SetStaticText( CD_Actuator_ActuatingTime, RD_Actuator_ColName, IDS_SSHEETSSELSV_ACTUATINGTIME );
	pclSSheet->SetStaticText( CD_Actuator_IP, RD_Actuator_ColName, IDS_SSHEETSSELSV_ACTRIP );
	pclSSheet->SetStaticText( CD_Actuator_PowSupply, RD_Actuator_ColName, IDS_SSHEETSSELSV_ACTRPOWERSUP );
	pclSSheet->SetStaticText( CD_Actuator_InputSig, RD_Actuator_ColName, IDS_SSHEETSSELSV_ACTRINPUTSIG );
	pclSSheet->SetStaticText( CD_Actuator_OutputSig, RD_Actuator_ColName, IDS_SSHEETSSELSV_ACTROUTPUTSIG );
	pclSSheet->SetStaticText( CD_Actuator_RelayType, RD_Actuator_ColName, IDS_SSHEETSSELSV_ACTRRELAYTYPE );
	pclSSheet->SetStaticText( CD_Actuator_FailSafe, RD_Actuator_ColName, IDS_SSHEETSSELSV_ACTRFAILSAFE );
	pclSSheet->SetStaticText( CD_Actuator_DefaultReturnPos, RD_Actuator_ColName, IDS_SSHEETSSEL_ACTRDRP );
	//HYS-726	
	pclSSheet->SetStaticText( CD_Actuator_MaxTemp, RD_Actuator_ColName, IDS_SSHEETSSELCV_ACTRMAXTEMP );
	pclSSheet->SetStaticText( CD_Actuator_MaxTemp, RD_Actuator_Unit, IDS_UNITDEGREECELSIUS );
	// Unit.
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();
	pclSSheet->SetStaticText( CD_Actuator_ActuatingTime, RD_Actuator_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_TIME ) ).c_str() );

	pclSSheet->SetCellBorder( CD_Actuator_CheckBox, RD_Actuator_Unit, CD_Actuator_Pointer - 1, RD_Actuator_Unit,
			true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );

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

		// Add this row.
		// 'true' to specify that this row can be selected.
		pclSheetDescriptionActuator->AddRows( 1, true );

		// Add checkbox.
		pclSSheet->SetCheckBox( CD_Actuator_CheckBox, lRow, _T(""), false, true );
		//HYS-726
		if( m_pclIndSelSVParams->m_WC.GetTemp() > pElectroActuator->GetTmax() )
		{
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_RED );
			pclSSheet->SetStaticText( CD_Actuator_Name, lRow, pElectroActuator->GetName() );
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );
		}
		else
		{
			pclSSheet->SetStaticText( CD_Actuator_Name, lRow, pElectroActuator->GetName() );
		}

		// Compute Actuating time.
		// Remark: actuating Time is given in s/m or in s/deg; stroke is given in m or in deg.
		str = WriteDouble( pElectroActuator->GetDefaultActuatingTime(), 2 );
		pclSSheet->SetStaticText( CD_Actuator_ActuatingTime, lRow, str );

		CString str = pElectroActuator->GetIPxxFull();
		pclSSheet->SetStaticText( CD_Actuator_IP, lRow, str );
		pclSSheet->SetStaticText( CD_Actuator_PowSupply, lRow, pElectroActuator->GetPowerSupplyStr() );
		str.Empty();


		pclSSheet->SetStaticText( CD_Actuator_InputSig, lRow, pElectroActuator->GetInOutSignalsStr( true ) );
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

		pclSSheet->SetStaticText( CD_Actuator_OutputSig, lRow, pElectroActuator->GetInOutSignalsStr( false ) );

		pclSSheet->SetStaticText( CD_Actuator_RelayType, lRow, pElectroActuator->GetRelayStr() );

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
		str.Empty();

		pclSSheet->SetStaticText( CD_Actuator_DefaultReturnPos, lRow, CString( pElectroActuator->GetDefaultReturnPosStr( pElectroActuator->GetDefaultReturnPos() ).c_str() ) );
		//HYS-726
		CString sTmaxMedia = ( DBL_MAX == pElectroActuator->GetTmax() ) ? _T("-") : WriteDouble( pElectroActuator->GetTmax(), 3 );

		if( m_pclIndSelSVParams->m_WC.GetTemp() > pElectroActuator->GetTmax() )
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
		CreateCellDescriptionProduct( pclSheetDescriptionActuator->GetFirstParameterColumn(), lRow, (LPARAM)pElectroActuator, pclSheetDescriptionActuator );

		lRow++;
		lActuatorCount++;
	}

	long lLastDataRow = lRow - 1;

	// Save the total actuator number.
	pclSheetDescriptionActuator->SetUserVariable( _SDUV_TOTALACTUATOR_COUNT, lActuatorCount );

	// Set that there is no selection at now.
	SetCurrentActuatorSelected( NULL );

	pclSSheet->SetCellBorder( CD_Actuator_CheckBox, lLastDataRow, CD_Actuator_Pointer - 1, lLastDataRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );

	// Add possibility to change column size.
	// Remark: place this code after initialization is finished (like adding, removing, resizing, hiding/showing columns).
	CResizingColumnInfo *pclResizingColumnInfo = pclSheetDescriptionActuator->GetResizingColumnInfo();
	pclResizingColumnInfo->ActivateFeature( true, false, RD_Actuator_FirstAvailRow, lLastDataRow );
	pclResizingColumnInfo->AddRangeColumn( CD_Actuator_Name, CD_Actuator_MaxTemp, RD_Actuator_ColName, RD_Actuator_Unit );

	// Add possibility to click on title group to reset column width.
	CreateTitleGroupButtons( CD_Actuator_CheckBox, CD_Actuator_MaxTemp, RD_Actuator_GroupName, pclSheetDescriptionActuator );
}

void CRViewSSelSv::_ClickOnActuator( CSheetDescription *pclSheetDescriptionActuator, CDB_Actuator *pActuatorClicked,
		CCellDescriptionProduct *pclCellDescriptionProduct, long lColumn, long lRow, bool bExternalCall )
{
	if( NULL == m_pclIndSelSVParams )
	{
		ASSERT_RETURN;
	}

	// Check control valve selected.
	CDB_ShutoffValve *pclShutOffValveSelected = dynamic_cast<CDB_ShutoffValve*>( GetCurrentShutOffValveSelected() );

	if( NULL == pclShutOffValveSelected )
	{
		ASSERT_RETURN;
	}

	CSSheet *pclSSheet = pclSheetDescriptionActuator->GetSSheetPointer();

	LPARAM lActuatorCount;
	pclSheetDescriptionActuator->GetUserVariable( _SDUV_TOTALACTUATOR_COUNT, lActuatorCount );

	m_vecActuatorAccessories.clear();
	m_pCDBExpandCollapseGroupActuatorAcc = NULL;

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
		m_ViewDescription.RemoveAllSheetAfter( SD_ShutOffValveActuator );

		// uncheck checkbox
		pclSSheet->SetCheckBox(CD_Actuator_CheckBox, lRow, _T(""), false, true);

		// Set focus on actuator currently selected.
		PrepareAndSetNewFocus( pclSheetDescriptionActuator, pclSheetDescriptionActuator->GetActiveColumn(), lRow, 0 );

		m_lActuatorSelectedRow = 0;

		// Notify a change in the selection.
		SelectionHasChanged( IsSelectionReady() );
	}
	else
	{
		// If an actuator is already selected...
		if( NULL != pclCurrentActuatorSelected )
		{
			// Remove all sheet after the current except if it's the sheet for adapter.
			m_ViewDescription.RemoveAllSheetAfter( SD_ShutOffValveActuator );

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

		// Select actuator (just highlight background).
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

		// Notify a change in the selection.
		SelectionHasChanged( IsSelectionReady() );
	}
}

void CRViewSSelSv::_GetActuatorList( CDB_ShutoffValve *pclSelectedShutOffValve, CRank *pclActuatorList )
{
	if( NULL == m_pclIndSelSVParams || NULL == m_pclIndSelSVParams->m_pTADB || NULL == pclSelectedShutOffValve || NULL == pclActuatorList )
	{
		ASSERT_RETURN;
	}

	pclActuatorList->PurgeAll();

	// Retrieve the actuator group on the selected control valve.
	CTable *pclActuatorGroup = ( CTable * )( pclSelectedShutOffValve->GetActuatorGroupIDPtr().MP );

	if( NULL == pclActuatorGroup )
	{
		return;
	}

	// Retrieve list of all actuators in this group.
	CRank rList( false );
	m_pclIndSelSVParams->m_pTADB->GetActuator( &rList, pclActuatorGroup, L"", L"", -1, CDB_ElectroActuator::DefaultReturnPosition::Undefined, 
			m_pclIndSelSVParams->m_eFilterSelection );

	CString str;
	LPARAM lparam;

	for( BOOL bContinue = rList.GetFirst( str, lparam ); TRUE == bContinue; bContinue = rList.GetNext( str, lparam ) )
	{
		CDB_ElectroActuator *pclElectroActuator = dynamic_cast<CDB_ElectroActuator *>( (CData *)lparam );

		if( NULL == pclElectroActuator )
		{
			continue;
		}

		pclActuatorList->Add( pclElectroActuator->GetName(), pclElectroActuator->GetOrderKey(), (LPARAM)pclElectroActuator, false );
	}
}

void CRViewSSelSv::_GetActuatorAccessoryList( CDB_Actuator *pclSeletedActuator, CRank *pclActuatorAccessoryList, CDB_RuledTableBase **ppclRuledTable )
{
	if( NULL == m_pclIndSelSVParams || NULL == m_pclIndSelSVParams->m_pTADB )
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

	m_pclIndSelSVParams->m_pTADB->GetAccessories( pclActuatorAccessoryList, *ppclRuledTable, m_pclIndSelSVParams->m_eFilterSelection );
}

void CRViewSSelSv::_FillActuatorAccessoryRows()
{
	if( NULL == m_pclIndSelSVParams )
	{
		ASSERT_RETURN;
	}

	// Check current shut-off valve selected.
	CDB_ShutoffValve *pclCurrentShutOffValveSelected = dynamic_cast<CDB_ShutoffValve *>( GetCurrentShutOffValveSelected() );

	if( NULL == pclCurrentShutOffValveSelected )
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
	_GetActuatorAccessoryList( pclCurrentActuatorSelected, &rActuatorAccessoryList, &pclRuledTable );

	if( rActuatorAccessoryList.GetCount() > 0 )
	{
		_FillRowsAcc( IDS_SSHEETSSELSV_ACTRACCGROUP, false, &rActuatorAccessoryList, pclRuledTable );
	}
}

void CRViewSSelSv::_FillRowsAcc( int iTitleID, bool bForCtrl, CRank *pclList, CDB_RuledTableBase *pclRuledTable )
{
	if( NULL == m_pclIndSelSVParams )
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
		case IDS_SSHEETSSELSV_VALVEACCGROUP:
			iSSheetID = SD_ShutOffValveAccessory;
			pvecAccessories = &m_vecShutOffValveAccessoryList;
			ppclGroupButton = &m_pCDBExpandCollapseGroupAccessory;
			break;

		case IDS_SSHEETSSELSV_ACTRACCGROUP:
			iSSheetID = SD_ShutOffValveActuatorAccessory;
			pvecAccessories = &m_vecActuatorAccessories;
			ppclGroupButton = &m_pCDBExpandCollapseGroupActuatorAcc;
			break;
	}

	// Call 'MultiSpreadBase' method to get a new SSheet.
	CSheetDescription *pclSheetDescription = CreateSSheet( iSSheetID );

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
		case SD_ShutOffValve:
			lFirstColumnWidth = pclPreviousSheetDescription->GetSSheetPointer()->GetColWidthInPixelsW( CD_ShutoffValve_FirstColumn );
			lLastColumnWidth = pclPreviousSheetDescription->GetSSheetPointer()->GetColWidthInPixelsW( CD_ShutoffValve_Pointer );
			break;

		case SD_ShutOffValveAccessory:
		case SD_ShutOffValveActuatorAccessory:
			lFirstColumnWidth = pclPreviousSheetDescription->GetSSheetPointer()->GetColWidthInPixelsW( CD_Accessory_FirstColumn );
			lLastColumnWidth = pclPreviousSheetDescription->GetSSheetPointer()->GetColWidthInPixelsW( CD_Accessory_LastColumn );
			break;

		case SD_ShutOffValveActuator:
			lFirstColumnWidth = pclPreviousSheetDescription->GetSSheetPointer()->GetColWidthInPixelsW( CD_Actuator_FirstColumn );
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
		CDB_Product *pAccessory = (CDB_Product * )lparam;
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

		CCDBCheckboxAccessory *pCheckbox = CreateCheckboxAccessory( lLeftOrRight, lRow, false, true, strName, pAccessory, pclRuledTable,
				pvecAccessories, pclSheetDescription );

		if( NULL != pCheckbox )
		{
			pCheckbox->ApplyInternalChange();
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

long CRViewSSelSv::_GetRowOfEditedActuator( CSheetDescription *pclSheetDescriptionActuator, CDB_Actuator *pActuator )
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

void CRViewSSelSv::_ReadAllColumnWidth( CDS_IndSelParameter *pclIndSelParameter )
{
	if( NULL == pclIndSelParameter )
	{
		ASSERT_RETURN;
	}

	std::map<UINT, short> mapSDIDVersion;
	mapSDIDVersion[CW_RVIEWSSELSV_SHEETID_SHUTOFFVALVE] = CW_RVIEWSSELSV_SHUTOFFVALVE_VERSION;
	mapSDIDVersion[CW_RVIEWSSELSV_SHEETID_ACTUATOR] = CW_RVIEWSSELSV_ACTUATOR_VERSION;

	// Container window sheet ID to sheetdescription of this rightview.
	std::map<UINT, short> mapCWtoRW;
	mapCWtoRW[CW_RVIEWSSELSV_SHEETID_SHUTOFFVALVE] = SD_ShutOffValve;
	mapCWtoRW[CW_RVIEWSSELSV_SHEETID_ACTUATOR] = SD_ShutOffValveActuator;

	// By default and before reading registry saved column width force reset column width for all sheets.
	for( auto &iter : mapCWtoRW )
	{
		ResetColumnWidth( iter.second );
	}

	// Access to the 'RViewSSelSv' group.
	CDS_IndSelParameter::CCWWindow *pclCWWindow = pclIndSelParameter->GetWindowColumnWidth( CW_WINDOWID_INDSELSV, true );

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

void CRViewSSelSv::_WriteAllColumnWidth( CDS_IndSelParameter *pclIndSelParameter )
{
	if( NULL == pclIndSelParameter )
	{
		ASSERT_RETURN;
	}

	std::map<UINT, short> mapSDIDVersion;
	mapSDIDVersion[CW_RVIEWSSELSV_SHEETID_SHUTOFFVALVE] = CW_RVIEWSSELSV_SHUTOFFVALVE_VERSION;
	mapSDIDVersion[CW_RVIEWSSELSV_SHEETID_ACTUATOR] = CW_RVIEWSSELSV_ACTUATOR_VERSION;

	// Container window sheet ID to sheetdescription of this rightview.
	std::map<UINT, short> mapCWtoRW;
	mapCWtoRW[CW_RVIEWSSELSV_SHEETID_SHUTOFFVALVE] = SD_ShutOffValve;
	mapCWtoRW[CW_RVIEWSSELSV_SHEETID_ACTUATOR] = SD_ShutOffValveActuator;

	// Access to the 'RViewSSelSv' group.
	CDS_IndSelParameter::CCWWindow *pclCWWindow = pclIndSelParameter->GetWindowColumnWidth( CW_WINDOWID_INDSELSV, true );

	for( auto &iter : mapSDIDVersion )
	{
		CDS_IndSelParameter::CCWSheet *pclCWSheet = pclCWWindow->GetCWSheet( iter.first, true );
		pclCWSheet->SetVersion( iter.second );
		pclCWSheet->GetMap() = m_mapSSheetColumnWidth[mapCWtoRW[iter.first]];
	}
}
