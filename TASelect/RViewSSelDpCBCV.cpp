#include "stdafx.h"
#include "afxctl.h"

#include "TASelect.h"
#include "MainFrm.h"
#include "Hydronic.h"

#include "DlgLeftTabSelManager.h"
#include "DlgInfoSSelDpCBCV.h"
#include "DlgSelectionComboHelper.h"

#include "RViewSSelSS.h"
#include "RViewSSelCtrl.h"
#include "RViewSSelDpCBCV.h"

CRViewSSelDpCBCV *pRViewSSelDpCBCV = NULL;

CRViewSSelDpCBCV::CRViewSSelDpCBCV() : CRViewSSelCtrl( CMainFrame::RightViewList::eRVSSelDpCBCV )
{
	m_pclIndSelDpCBCVParams = NULL;
	m_pclDlgInfoSSelDpCBCV = NULL;
	m_pclAdapterRuledTable = NULL;
	m_vecSvAccessoryList.clear();
	m_pCDBExpandCollapseRowsSv = NULL;
	m_pCDBExpandCollapseGroupSvAccessory = NULL;
	m_pCDBShowAllPrioritiesSv = NULL;
	m_lSVSelectedRow = -1;
	pRViewSSelDpCBCV = this;
}

CRViewSSelDpCBCV::~CRViewSSelDpCBCV()
{
	pRViewSSelDpCBCV = NULL;
}

void CRViewSSelDpCBCV::UpdateDpCBCVPictureAndInfos( void )
{
	if( NULL == m_pclIndSelDpCBCVParams || NULL == m_pclIndSelDpCBCVParams->m_pclSelectDpCBCVList )
	{
		ASSERT_RETURN;
	}

	if( NULL == m_pclDlgInfoSSelDpCBCV || NULL == m_pclDlgInfoSSelDpCBCV->GetSafeHwnd() )
	{
		return;
	}

		// Check current combined Dp controller, control and balancing valve selected.
	CDB_DpCBCValve *pclDpCBCValveSelected = dynamic_cast<CDB_DpCBCValve*>( GetCurrentControlValveSelected() );

	if( NULL == pclDpCBCValveSelected )
	{
		return;
	}

	CSelectedValve *pSelectedTAP = GetSelectProduct<CSelectedValve>( pclDpCBCValveSelected, m_pclIndSelDpCBCVParams->m_pclSelectDpCBCVList );

	if( NULL == pSelectedTAP )
	{
		return;
	}
	
	bool bValidRange = ( false == pSelectedTAP->IsFlagSet( CSelectedBase::eValveDpToLarge ) && false == pSelectedTAP->IsFlagSet( CSelectedBase::eValveDpToSmall ) );
	double dFlow = m_pclIndSelDpCBCVParams->m_dFlow;
	double dRho = m_pclIndSelDpCBCVParams->m_WC.GetDens();
	
	m_pclDlgInfoSSelDpCBCV->UpdateInfoDpCBCV( (CDB_DpCBCValve*)pclDpCBCValveSelected, pSelectedTAP->GetDpMin(), bValidRange, dFlow, dRho );
	m_pclDlgInfoSSelDpCBCV->Invalidate();
	m_pclDlgInfoSSelDpCBCV->UpdateWindow();
}

void CRViewSSelDpCBCV::UpdateSVInfos( void )
{
	if( NULL == m_pclIndSelDpCBCVParams || NULL == m_pclIndSelDpCBCVParams->m_pclSelectDpCBCVList || 
			NULL == m_pclDlgInfoSSelDpCBCV || NULL == m_pclDlgInfoSSelDpCBCV->GetSafeHwnd() )
	{
		return;
	}

	UpdateDpCBCVPictureAndInfos();

	// Check current shutoff valve selected.
	CDB_ShutoffValve *pclShutoffValveSelected = _GetCurrentShutoffValveSelected();

	if( NULL == pclShutoffValveSelected )
	{
		return;
	}

	CSelectShutoffList *pclSelectShutoffList = m_pclIndSelDpCBCVParams->m_pclSelectDpCBCVList->GetSvSelected();
	CSelectedValve *pSelectedTAP = GetSelectProduct<CSelectedValve>( pclShutoffValveSelected, pclSelectShutoffList );

	if( NULL == pSelectedTAP )
	{
		return;
	}

	m_pclDlgInfoSSelDpCBCV->UpdateInfoSTS( pclShutoffValveSelected, pSelectedTAP->GetDp() );
	m_pclDlgInfoSSelDpCBCV->Invalidate();
	m_pclDlgInfoSSelDpCBCV->UpdateWindow();
}

void CRViewSSelDpCBCV::Reset( void )
{
	m_vecSvAccessoryList.clear();
	m_pCDBExpandCollapseRowsSv = NULL;
	m_pCDBExpandCollapseGroupSvAccessory = NULL;
	m_pCDBShowAllPrioritiesSv = NULL;
	m_lSVSelectedRow = -1;

	CRViewSSelCtrl::Reset();
}

void CRViewSSelDpCBCV::OnNewDocument( CDS_IndSelParameter *pclIndSelParameter )
{
	CRViewSSelCtrl::OnNewDocument( pclIndSelParameter );
	_ReadAllColumnWidth( pclIndSelParameter );
}

void CRViewSSelDpCBCV::SaveSelectionParameters( CDS_IndSelParameter *pclIndSelParameter )
{
	CRViewSSelCtrl::SaveSelectionParameters( pclIndSelParameter );
	_WriteAllColumnWidth( pclIndSelParameter );
}

void CRViewSSelDpCBCV::Suggest( CProductSelelectionParameters *pclProductSelectionParameters, LPARAM lpParam )
{
	if( NULL == pclProductSelectionParameters || NULL == dynamic_cast<CIndSelDpCBCVParams*>( pclProductSelectionParameters ) 
			|| NULL == lpParam )
	{
		return;
	}

	m_pclIndSelDpCBCVParams = dynamic_cast<CIndSelDpCBCVParams*>( pclProductSelectionParameters );
	m_pclDlgInfoSSelDpCBCV = (CDlgInfoSSelDpCBCV *)lpParam;

	CRViewSSelCtrl::Suggest( pclProductSelectionParameters, lpParam );
}

void CRViewSSelDpCBCV::FillInSelected( CDS_SelProd *pSelectedProductToFill )
{
	if( NULL == m_pclIndSelDpCBCVParams )
	{
		ASSERT_RETURN;
	}

	CDS_SSelDpCBCV *pSelectedDpCBCVToFill = dynamic_cast<CDS_SSelDpCBCV *>( pSelectedProductToFill );

	if( NULL == pSelectedDpCBCVToFill )
	{
		return;
	}

	// Fill data HMin for valve.
	CDB_DpCBCValve *pclCurrentDpCBCValveSelected = dynamic_cast<CDB_DpCBCValve*>( GetCurrentControlValveSelected() );

	if( NULL != pclCurrentDpCBCValveSelected && NULL != m_pclIndSelDpCBCVParams->m_pclSelectDpCBCVList )
	{
		// Search valve in CSelectList to set the correct H min.
		for( CSelectedValve *pclSelectedValve = m_pclIndSelDpCBCVParams->m_pclSelectDpCBCVList->GetFirst<CSelectedValve>(); NULL != pclSelectedValve;
				pclSelectedValve = m_pclIndSelDpCBCVParams->m_pclSelectDpCBCVList->GetNext<CSelectedValve>() )
		{
			if( pclSelectedValve->GetpData() == pclCurrentDpCBCValveSelected )	// Product found
			{
				pSelectedDpCBCVToFill->SetHMin( pclSelectedValve->GetHMin() );
				break;
			}
		}
	}

	bool bIsEditionMode = false;
	// Clear previous shutoff valve accessories if we are not in edition mode.
	CAccessoryList *pclSVAccessoryToFill = pSelectedDpCBCVToFill->GetSVAccessoryList();
	if( false == m_pclIndSelDpCBCVParams->m_bEditModeRunning )
	{
		pclSVAccessoryToFill->Clear();
	}
	else
	{
		bIsEditionMode = true;
	}

	// Retrieve the current selected shut-off valve if exist.
	CDB_TAProduct *pclCurrentShutoffValveSelected = _GetCurrentShutoffValveSelected();

	if( NULL != pclCurrentShutoffValveSelected )
	{
		pSelectedDpCBCVToFill->SetSVIDPtr( pclCurrentShutoffValveSelected->GetIDPtr() );
		// HYS-987: If we are not in Edition mode we fill the accessory list. In edition mode we juste update accessory list
		if( false == bIsEditionMode )
		{
			// Retrieve selected accessory and add it.
			for( vecCDCAccessoryListIter vecIter = m_vecSvAccessoryList.begin(); vecIter != m_vecSvAccessoryList.end(); vecIter++ )
			{
				CCDBCheckboxAccessory *pCDBCheckboxAccessory = *vecIter;

				if( NULL != pCDBCheckboxAccessory && true == pCDBCheckboxAccessory->GetCheckStatus() && NULL != pCDBCheckboxAccessory->GetAccessoryPointer() )
				{
					pclSVAccessoryToFill->Add( pCDBCheckboxAccessory->GetAccessoryPointer()->GetIDPtr(), CAccessoryList::_AT_Accessory,
						pCDBCheckboxAccessory->GetRuledTable() );
				}
			}
		}
		else
		{
			UpdateAccessoryList( m_vecSvAccessoryList, pclSVAccessoryToFill, CAccessoryList::_AT_Accessory );
		}
	}

	// Base class now.
	CRViewSSelCtrl::FillInSelected( pSelectedProductToFill );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PROTECTED MEMBERS
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CRViewSSelDpCBCV::OnTextTipFetch( CSheetDescription *pclSheetDescription, long lColumn, long lRow, WORD *pwMultiLine, SHORT *pnTipWidth, 
		TCHAR *pstrTipText, BOOL *pbShowTip )
{
	if( NULL == m_pclIndSelDpCBCVParams || NULL == m_pclIndSelDpCBCVParams->m_pclSelectDpCBCVList || false == m_bInitialised 
			|| NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() )
	{
		ASSERTA_RETURN( false );
	}

	// First, we must call base class.
	// Remark: if base class has displayed a tooltip, it's not necessary to go further in this method.
	if( true == CRViewSSelCtrl::OnTextTipFetch( pclSheetDescription, lColumn, lRow, pwMultiLine, pnTipWidth, pstrTipText, pbShowTip ) )
	{
		return true;
	}

	CString str;
	bool bReturnValue = false;
	*pbShowTip = false;
	*pwMultiLine = SS_TT_MULTILINE_AUTO;
	
	CDB_TAProduct *pTAP = dynamic_cast<CDB_TAProduct *>( (CData *)GetCDProduct( lColumn, lRow, pclSheetDescription ) );
	CDB_DpCBCValve *pclDpcBCV = dynamic_cast<CDB_DpCBCValve *>( pTAP );
	CSelectDpCBCVList *pSelectDpCBCVList = m_pclIndSelDpCBCVParams->m_pclSelectDpCBCVList;
	CSelectedValve *pSelectedTAP = GetSelectProduct<CSelectedValve>( pTAP, pSelectDpCBCVList );
	CSSheet *pclSSheet = pclSheetDescription->GetSSheetPointer();

	double dFlow = m_pclIndSelDpCBCVParams->m_dFlow;
	double dRho = m_pclIndSelDpCBCVParams->m_WC.GetDens();
	
	if( NULL != pSelectedTAP && SD_ControlValve == pclSheetDescription->GetSheetDescriptionID() )
	{
		if( CD_ControlValve_Preset == lColumn )
		{
			// Check what is the color of the text.
			if( NULL != pclSSheet && _ORANGE == pclSSheet->GetForeColor( lColumn, lRow ) )
			{
				CDB_ValveCharacteristic *pValveChar = pclDpcBCV->GetValveCharacteristic();
				
				if( NULL != pValveChar )
				{
					CString str1;
					CString str2 = pValveChar->GetSettingString( pValveChar->GetMinRecSetting() );
					str1.Format( _T("%s < %s"), (LPCTSTR)pclSSheet->GetCellText( CD_ControlValve_Preset, lRow ), (LPCTSTR)str2 );
					FormatString( str, IDS_SSHEETSSEL_SETTINGERROR, str1 );
				}
			}
		}
		else if( CD_ControlValve_DplRange == lColumn )
		{
			if( NULL != pSelectedTAP )
			{
				CString str2 = WriteCUDouble( _U_DIFFPRESS, m_pclIndSelDpCBCVParams->m_dDpToStabilize, false, 3, 0 );

				if( true == pSelectedTAP->IsFlagSet( CSelectedBase::eValveDpToLarge ) && -1.0 != pclDpcBCV->GetDplmax( dFlow, dRho ) )
				{
					str2 += CString( _T(" > ") ) + WriteCUDouble( _U_DIFFPRESS, pclDpcBCV->GetDplmax( dFlow, dRho ), true, 3, 0 );
					FormatString( str, IDS_SSHEETSSELDPC_DPLRANGEERRORH, str2 );
				}
				else if( true == pSelectedTAP->IsFlagSet( CSelectedBase::eValveDpToSmall ) && -1.0 != pclDpcBCV->GetDplmin() )
				{
					str2 += CString( _T(" < ") ) + WriteCUDouble( _U_DIFFPRESS, pclDpcBCV->GetDplmin(), true, 3, 0 );
					FormatString( str, IDS_SSHEETSSELDPC_DPLRANGEERRORL, str2 );
				}
			}
		}

		if( true == str.IsEmpty() && NULL != pclDpcBCV )
		{
			if( 0 != _tcslen( pclDpcBCV->GetComment() ) )
			{
				str = pclDpcBCV->GetComment();
			}
		}
	}

	if( false == str.IsEmpty() )
	{
		*pnTipWidth = (SHORT)pclSSheet->GetTipTextWidth( str );
		wcsncpy_s( pstrTipText, SS_TT_TEXTMAX, (LPCTSTR)str, SS_TT_TEXTMAX );
		*pbShowTip = true;
		bReturnValue = true;
	}
	return bReturnValue;
}

bool CRViewSSelDpCBCV::OnClickProduct( CSheetDescription *pclSheetDescription, CCellDescriptionProduct *pclCellDescriptionProduct, long lColumn, long lRow )
{
	if( NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() || NULL == pclCellDescriptionProduct )
	{
		return false;
	}

 	bool bNeedRefresh = true;
	CDB_ShutoffValve *pclShutoffValve = dynamic_cast<CDB_ShutoffValve *>( ( CData * )pclCellDescriptionProduct->GetProduct() );

	if( NULL == pclShutoffValve )
	{
		bNeedRefresh = CRViewSSelCtrl::OnClickProduct( pclSheetDescription, pclCellDescriptionProduct, lColumn, lRow );

		// If user has well clicked on a combined Dp controller, control and balancing valve...
		if( NULL != dynamic_cast<CDB_ControlValve *>( ( CData * )pclCellDescriptionProduct->GetProduct() ) )
		{
			UpdateDpCBCVPictureAndInfos();
		}
	}
	else
	{
		_ClickOnShutoffValve( pclSheetDescription, pclCellDescriptionProduct, lColumn, lRow );
	}
	
	return bNeedRefresh;

}

bool CRViewSSelDpCBCV::ResetColumnWidth( short nSheetDescriptionID )
{
	if( false == CRViewSSelCtrl::ResetColumnWidth( nSheetDescriptionID ) )
	{
		return false;
	}

	TSpread clTSpread;

	if( FALSE == clTSpread.Create( WS_CHILD, CRect( 0, 0, 0, 0 ), this, 0 ) )
	{
		ASSERTA_RETURN( false );
	}

	switch( nSheetDescriptionID )
	{
		case SD_ShutoffValve:
			m_mapSSheetColumnWidth[SD_ShutoffValve][CD_ShutoffValve_FirstColumn] = clTSpread.ColWidthToLogUnits( 4 );
			m_mapSSheetColumnWidth[SD_ShutoffValve][CD_ShutoffValve_CheckBox] = clTSpread.ColWidthToLogUnits( 2 );
			m_mapSSheetColumnWidth[SD_ShutoffValve][CD_ShutoffValve_Name] = clTSpread.ColWidthToLogUnits( 14 );
			m_mapSSheetColumnWidth[SD_ShutoffValve][CD_ShutoffValve_Material] = clTSpread.ColWidthToLogUnits( 12 );
			m_mapSSheetColumnWidth[SD_ShutoffValve][CD_ShutoffValve_Connection] = clTSpread.ColWidthToLogUnits( 14 );
			m_mapSSheetColumnWidth[SD_ShutoffValve][CD_ShutoffValve_Version] = clTSpread.ColWidthToLogUnits( 12 );
			m_mapSSheetColumnWidth[SD_ShutoffValve][CD_ShutoffValve_PN] = clTSpread.ColWidthToLogUnits( 6 );
			m_mapSSheetColumnWidth[SD_ShutoffValve][CD_ShutoffValve_Size] = clTSpread.ColWidthToLogUnits( 10 );
			m_mapSSheetColumnWidth[SD_ShutoffValve][CD_ShutoffValve_Dp] = clTSpread.ColWidthToLogUnits( 7 );
			m_mapSSheetColumnWidth[SD_ShutoffValve][CD_ShutoffValve_TemperatureRange] = clTSpread.ColWidthToLogUnits( 8 );
			m_mapSSheetColumnWidth[SD_ShutoffValve][CD_ShutoffValve_Separator] = clTSpread.ColWidthToLogUnits( 1 );
			m_mapSSheetColumnWidth[SD_ShutoffValve][CD_ShutoffValve_PipeSize] = clTSpread.ColWidthToLogUnits( 8 );
			m_mapSSheetColumnWidth[SD_ShutoffValve][CD_ShutoffValve_PipeLinDp] = clTSpread.ColWidthToLogUnits( 8 );
			m_mapSSheetColumnWidth[SD_ShutoffValve][CD_ShutoffValve_PipeV] = clTSpread.ColWidthToLogUnits( 8 );
			m_mapSSheetColumnWidth[SD_ShutoffValve][CD_ShutoffValve_Pointer] = clTSpread.ColWidthToLogUnits( 1 );
			break;

		case SD_SVAccessory:
			// No yet!
			break;
	}

	if( clTSpread.GetSafeHwnd() != NULL )
	{
		clTSpread.DestroyWindow();
	}

	return true;
}

bool CRViewSSelDpCBCV::IsSelectionReady( void )
{
	bool bReady = false;
	CDB_DpCBCValve *pclSelectedDpCBCValve = dynamic_cast<CDB_DpCBCValve*>( GetCurrentControlValveSelected() );

	if( NULL != pclSelectedDpCBCValve )
	{
		bReady = ( false == pclSelectedDpCBCValve->IsDeleted() );

		// If user choice to use STS.
		if( NULL != m_pclIndSelDpCBCVParams && true == m_pclIndSelDpCBCVParams->m_bIsWithSTSChecked && NULL == _GetCurrentShutoffValveSelected() )
		{
			bReady = false;
		}
	}

	return bReady;
}

bool CRViewSSelDpCBCV::IsActuatorFit( CDB_ElectroActuator *pclElectroActuator, CDB_ControlValve *pclSelectedControlValve, bool bDowngradeActuatorFunctionality, 
		bool bAcceptAllFailSafe )
{
	if( NULL == m_pclIndSelDpCBCVParams )
	{
		ASSERTA_RETURN( false );
	}

	if( NULL == pclElectroActuator || NULL == pclSelectedControlValve )
	{
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
		ComboDRPFctChoice = m_pclIndSelDpCBCVParams->m_eActuatorDRPFunction;
	}
	else
	{
		ComboDRPFctChoice = CDB_ControlValve::DRPFunction::drpfAll;
	}

	return pclElectroActuator->IsActuatorFit( m_pclIndSelDpCBCVParams->m_strActuatorPowerSupplyID, m_pclIndSelDpCBCVParams->m_strActuatorInputSignalID, m_pclIndSelDpCBCVParams->m_iActuatorFailSafeFunction,
		ComboDRPFctChoice, CurrentActuatorDRPFct, m_pclIndSelDpCBCVParams->m_eCvCtrlType, bDowngradeActuatorFunctionality );
}

long CRViewSSelDpCBCV::FillControlValveRows( CDB_ControlValve *pEditedControlValve )
{
	if( NULL == m_pclIndSelDpCBCVParams || NULL == m_pclIndSelDpCBCVParams->m_pclSelectDpCBCVList
			|| NULL == m_pclIndSelDpCBCVParams->m_pclSelectDpCBCVList->GetSelectPipeList() || NULL == m_pclIndSelDpCBCVParams->m_pTADS
			|| NULL == m_pclIndSelDpCBCVParams->m_pTADB )
	{
		ASSERTA_RETURN( -1 );
	}

	// Call first the bass class to initialize sheet, header and so on.
	if( -1 == CRViewSSelCtrl::FillControlValveRows( pEditedControlValve ) )
	{
		return -1;
	}

	// Retrieve sheet description created in base class.
	CSheetDescription *pclSheetDescriptionPICv = m_ViewDescription.GetFromSheetDescriptionID( SD_ControlValve );

	if( NULL == pclSheetDescriptionPICv || NULL == pclSheetDescriptionPICv->GetSSheetPointer() )
	{
		return -1;
	}

	CSelectDpCBCVList *pclSelectedDpCBCVList = m_pclIndSelDpCBCVParams->m_pclSelectDpCBCVList;
	CSSheet *pclSSheet = pclSheetDescriptionPICv->GetSSheetPointer();

	// Hide unused columns.
	pclSSheet->ShowCol( CD_ControlValve_Kvs, FALSE );
	pclSSheet->ShowCol( CD_ControlValve_DpFullOpening, FALSE );
	pclSSheet->ShowCol( CD_ControlValve_DpHalfOpening, FALSE );
	pclSSheet->ShowCol( CD_ControlValve_DpMax, FALSE );

	// Variables.
	CSelectPipe selPipe( m_pclIndSelDpCBCVParams );
	CDS_TechnicalParameter *pTechParam = m_pclIndSelDpCBCVParams->m_pTADS->GetpTechParams();

	// Change the column name from "Dp" to "Dp min."
	pclSSheet->SetStaticText( CD_ControlValve_Dp, RD_ControlValve_ColName, IDS_SSHEETSSELCV_DPMIN );

	long lRetRow = -1;
	long lRow = RD_ControlValve_FirstAvailRow;

	long lValveTotalCount = 0;
	long lValveNotPriorityCount = 0;
	bool bShowAllPrioritiesShown = false;
	long lShowAllPrioritiesButtonRow = 0;
	bool bAtLeastOneError;
	bool bAtLeastOneWarning;
	bool bRemarkInfoKTH = false;

	bool bAtLeastOnePICVPartOfASet = false;

	pclSheetDescriptionPICv->SetUserVariable( _SDUV_TOTALPRODUCT_COUNT, 0 );
	pclSheetDescriptionPICv->SetUserVariable( _SDUV_NOPRIORITYPRODUCT_COUNT, 0 );
	pclSheetDescriptionPICv->RestartRemarkGenerator();
	
	for( CSelectedValve *pclSelectedDpCBCV = pclSelectedDpCBCVList->GetFirst<CSelectedValve>(); NULL != pclSelectedDpCBCV; 
			pclSelectedDpCBCV = pclSelectedDpCBCVList->GetNext<CSelectedValve>() )
	{
		// In case it is a combined Dp controller and balancing & control valve.
		CDB_DpCBCValve *pclDpCBCV = dynamic_cast<CDB_DpCBCValve *>( pclSelectedDpCBCV->GetpData() );
		
		if( NULL == pclDpCBCV )
		{
			continue;
		}

		bAtLeastOneError = false;
		bAtLeastOneWarning = false;
		bool bBest = pclSelectedDpCBCV->IsFlagSet(CSelectedBase::eBest );
		
		if( true == pclSelectedDpCBCV->IsFlagSet( CSelectedBase::eNotPriority ) )
		{
			lValveNotPriorityCount++;
			
			if( false == bShowAllPrioritiesShown )
			{
				// Add one empty not selectable row
				pclSheetDescriptionPICv->AddRows( 1 );
				pclSheetDescriptionPICv->RemoveSelectableRow( lRow );

				pclSSheet->SetCellBorder( CD_ControlValve_CheckBox, lRow, CD_ControlValve_Separator - 1, lRow, true, SS_BORDERTYPE_TOP | SS_BORDERTYPE_BOTTOM, 
						SS_BORDERSTYLE_SOLID, _BLACK );

				pclSSheet->SetCellBorder( CD_ControlValve_PipeSize, lRow, CD_ControlValve_Pointer - 1, lRow, true, SS_BORDERTYPE_TOP | SS_BORDERTYPE_BOTTOM, 
						SS_BORDERSTYLE_SOLID, _BLACK );

				lShowAllPrioritiesButtonRow = lRow++;
				bShowAllPrioritiesShown = true;
			}
		}

		// Add this row.
		// 'true' to specify that this row can be selected.
		pclSheetDescriptionPICv->AddRows( 1, true );

		// First columns will be set at the end!

		// Add checkbox.
		pclSSheet->SetCheckBox( CD_Actuator_CheckBox, lRow, _T(""), false, true );

		// Add the picture for a set.
		if( true == m_pclIndSelDpCBCVParams->m_bOnlyForSet && true == pclDpCBCV->IsPartOfaSet() )
		{
			CCDButtonBox *pCDButtonBox = CreateCellDescriptionBox( CD_ControlValve_Box, lRow, true, CCDButtonBox::ButtonState::BoxOpened, pclSheetDescriptionPICv );
			
			if( NULL != pCDButtonBox )
			{
				pCDButtonBox->ApplyInternalChange();
			}
			
			bAtLeastOnePICVPartOfASet = true;
		}
		
		// Set the presetting first to determine if the red color must be used.
		CString str = _T("-");
		double dPresetting = pclSelectedDpCBCV->GetH();
		
		CDB_ValveCharacteristic *pValvChar = (CDB_ValveCharacteristic *)pclDpCBCV->GetValveCharDataPointer();
		
		if( NULL != pValvChar && -1.0 != dPresetting )
		{
			str = pValvChar->GetSettingString( dPresetting );
		}

		// If full opening indicator is set after the opening...
		if( str.Find( _T('*') ) != -1 )
		{
			str.Replace( _T('*'), _T(' ') );
			pclSheetDescriptionPICv->WriteTextWithFlags( str, CD_ControlValve_Preset, lRow, CSheetDescription::RemarkFlags::FullOpening );
		}
		else
		{
			pclSSheet->SetStaticText( CD_ControlValve_Preset, lRow, str );
		}

		// Set the rangeability.
		pclSSheet->SetStaticText( CD_ControlValve_Rangeability, lRow, pclDpCBCV->GetStrRangeability().c_str() );

		// Set the leakage rate.
		pclSSheet->SetStaticText( CD_ControlValve_LeakageRate, lRow, WriteCUDouble( _U_NODIM, pclDpCBCV->GetLeakageRate() * 100 ) );
				
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

		// Set the Dp min value.
		
		// Get pressure drop over the pressure part of the valve.
		double dDpp = pclDpCBCV->GetDppmin( m_pclIndSelDpCBCVParams->m_dFlow, m_pclIndSelDpCBCVParams->m_WC.GetDens() );
		
		// Get pressure drop over the control part of the valve.
		double dDpc = pclDpCBCV->GetDpc( m_pclIndSelDpCBCVParams->m_dFlow, m_pclIndSelDpCBCVParams->m_dDpToStabilize );

		double dDpMin = dDpp + dDpc;
		pclSSheet->SetStaticText( CD_ControlValve_Dp, lRow, WriteCUDouble( _U_DIFFPRESS, dDpMin ) );

		// Set DplRange.
		str = pclDpCBCV->GetFormatedDplRange( m_pclIndSelDpCBCVParams->m_dFlow, m_pclIndSelDpCBCVParams->m_WC.GetDens(), false ).c_str();

		if( ( true == pclSelectedDpCBCV->IsFlagSet( CSelectedBase::eValveDpToLarge ) || true == pclSelectedDpCBCV->IsFlagSet( CSelectedBase::eValveDpToSmall ) ) )
		{
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, ( LPARAM )_RED );
			bAtLeastOneError = true;
		}

		pclSSheet->SetStaticText( CD_ControlValve_DplRange, lRow, str );
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, ( LPARAM )_BLACK );

		// Set the stroke value.
		CDB_CloseOffChar *pCloseOffChar = (CDB_CloseOffChar *)( pclDpCBCV->GetCloseOffCharIDPtr().MP );
		
		if( NULL != pCloseOffChar )
		{
			if( CDB_CloseOffChar::eOpenType::Linear == pCloseOffChar->GetOpenType() )
			{
				pclSSheet->SetStaticText( CD_ControlValve_Stroke, lRow, WriteCUDouble( _U_DIAMETER, pclDpCBCV->GetStroke() ) );
			}
			else
			{
				pclSSheet->SetStaticText( CD_ControlValve_Stroke, lRow, WriteDouble( pclDpCBCV->GetStroke(), 0, 0) );
			}
		}
		else
		{
			pclSSheet->SetStaticText( CD_ControlValve_Stroke, lRow, WriteCUDouble( _U_DIAMETER, pclDpCBCV->GetStroke() ) ); 
		}

		// Set the characteristic picture.
		CDB_ControlProperties::eCTRLCHAR eCtrlChar = pclDpCBCV->GetCtrlProp()->GetCvCtrlChar();
		
		if( CDB_ControlProperties::Linear == eCtrlChar )
		{
			// Verify the special case of a KTH family product.
			// If it is, add an information to explain that that kind of valve is linear but can works for 3-Points and 
			// proportional control.
			if( 0 == IDcmp( pclDpCBCV->GetFamilyID(), _T("FAM_KTH512") ) )
			{
				pclSSheet->SetPictureCellWithID( IDI_CHARACTLINEXCLAM, CD_ControlValve_ImgCharacteristic, lRow, CSSheet::PictureCellType::Icon );
				bRemarkInfoKTH = true;
			}
			else
			{
				pclSSheet->SetPictureCellWithID( IDI_CHARACTLIN, CD_ControlValve_ImgCharacteristic, lRow, CSSheet::PictureCellType::Icon );
			}
		}
		else if( CDB_ControlProperties::EqualPc == eCtrlChar )
		{
			pclSSheet->SetPictureCellWithID( IDI_CHARACTEQM, CD_ControlValve_ImgCharacteristic, lRow, CSSheet::PictureCellType::Icon );
		}
		else
		{
			pclSSheet->SetPictureCellWithID( IDI_CHARACTNONE, CD_ControlValve_ImgCharacteristic, lRow, CSSheet::PictureCellType::Icon );
		}

		// Set the Push to open/close picture.
		IDPTR IDPtr = pclDpCBCV->GetTypeIDPtr();
		CDB_ControlProperties::ePushOrPullToClose PushClose = pclDpCBCV->GetCtrlProp()->GetCvPushClose();

		if( CDB_ControlProperties::PushToClose == PushClose )
		{
			pclSSheet->SetPictureCellWithID( IDI_PUSHCLOSE, CD_ControlValve_ImgPushClose, lRow, CSSheet::PictureCellType::Icon );
		}
		else if( CDB_ControlProperties::PullToClose == PushClose )
		{
			pclSSheet->SetPictureCellWithID( IDI_PUSHOPEN, CD_ControlValve_ImgPushClose, lRow, CSSheet::PictureCellType::Icon );
		}
		else if( CDB_ControlProperties::Undef == PushClose )
		{
			pclSSheet->ShowCol( CD_ControlValve_ImgSeparator, FALSE );
			pclSSheet->ShowCol( CD_ControlValve_ImgPushClose, FALSE );
		}

		// Set the temperature range.
		if( true == pclSelectedDpCBCV->IsFlagSet( CSelectedBase::eTemperature ) )
		{
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_RED );
			bAtLeastOneError = true;
		}

		pclSSheet->SetStaticText( CD_ControlValve_TemperatureRange, lRow, ( (CDB_TAProduct *)( pclSelectedDpCBCV->GetProductIDPtr().MP ) )->GetTempRange() );
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

		// Now we can set first columns in regards to current status (error, best or normal).
		if( true == bAtLeastOneError )
		{
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_RED );
		}
		else if( true == pclSelectedDpCBCV->IsFlagSet( CSelectedBase::eBest ) )
		{
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_GREEN );
		}
		
		// In addition of current flag, we have possibility that valve has the flag 'Not available' or 'Deleted'. In that case, we show valve name in red with "*" around it and
		// symbol '!' or '!!' after.
		if( true == pclDpCBCV->IsDeleted() )
		{
			pclSheetDescriptionPICv->WriteTextWithFlags( CString( pclDpCBCV->GetName() ), CD_ControlValve_Name, lRow, 
					CSheetDescription::RemarkFlags::Deleted, _T(""), _RED );
		}
		else if( false == pclDpCBCV->IsAvailable() )
		{
			pclSheetDescriptionPICv->WriteTextWithFlags( CString( pclDpCBCV->GetName() ), CD_ControlValve_Name, lRow, 
					CSheetDescription::RemarkFlags::NotAvailable, _T(""), _RED );
		}
		else
		{
			pclSSheet->SetStaticText( CD_ControlValve_Name, lRow, pclDpCBCV->GetName() );
		}
		
		if( true == bAtLeastOneError )
		{
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );
		}
		else if( true == pclSelectedDpCBCV->IsFlagSet( CSelectedBase::eBest ) )
		{
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_GREEN );
		}

		pclSSheet->SetStaticText( CD_ControlValve_Material, lRow, pclDpCBCV->GetBodyMaterial() );
		pclSSheet->SetStaticText( CD_ControlValve_Connection, lRow, pclDpCBCV->GetConnect() );
		pclSSheet->SetStaticText( CD_ControlValve_Version, lRow, pclDpCBCV->GetVersion() );
		pclSSheet->SetStaticText( CD_ControlValve_PN, lRow, pclDpCBCV->GetPN().c_str() );
		pclSSheet->SetStaticText( CD_ControlValve_Size, lRow, pclDpCBCV->GetSize() );
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

		pclSSheet->SetStaticText( CD_ControlValve_Separator, lRow );

		m_pclIndSelDpCBCVParams->m_pclSelectDpCBCVList->GetSelectPipeList()->GetMatchingPipe( pclDpCBCV->GetSizeKey(), selPipe );
		pclSSheet->SetStaticText( CD_ControlValve_PipeSize, lRow, selPipe.GetpPipe()->GetName() );
		
		// Set the LinDp to orange if it is above or below the technical parameters limits.
		if( selPipe.GetLinDp() > pTechParam->GetPipeMaxDp() || selPipe.GetLinDp() < pTechParam->GetPipeMinDp() )
		{
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_ORANGE );
		}
		
		pclSSheet->SetStaticText( CD_ControlValve_PipeLinDp, lRow, WriteCUDouble( _U_LINPRESSDROP, selPipe.GetLinDp() ) );
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

		// Set the Velocity to orange if it is above the technical parameter limit.
		// Orange if it is below the dMinVel.
		if( selPipe.GetU() > pTechParam->GetPipeMaxVel() || selPipe.GetU() < pTechParam->GetPipeMinVel() )
		{
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_ORANGE );
		}
		
		pclSSheet->SetStaticText( CD_ControlValve_PipeV, lRow, WriteCUDouble( _U_VELOCITY, selPipe.GetU() ) );
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

		// Save parameter.
		CCellDescriptionProduct *pclCDProduct = CreateCellDescriptionProduct( pclSheetDescriptionPICv->GetFirstParameterColumn(), lRow, (LPARAM)pclDpCBCV, pclSheetDescriptionPICv );
		pclCDProduct->SetUserParam( (LPARAM)pclSelectedDpCBCV );

		if( NULL != pEditedControlValve && pEditedControlValve == pclDpCBCV )
		{
			lRetRow = lRow;
		}

		pclSSheet->SetCellBorder( CD_ControlValve_CheckBox, lRow, CD_ControlValve_Separator- 1, lRow, true, SS_BORDERTYPE_BOTTOM,SS_BORDERSTYLE_SOLID, _GRAY );
		pclSSheet->SetCellBorder( CD_ControlValve_PipeSize, lRow, CD_ControlValve_Pointer - 1, lRow, true, SS_BORDERTYPE_BOTTOM,SS_BORDERSTYLE_SOLID, _GRAY );

		lRow++;
		lValveTotalCount++;
	}

	long lLastDataRow = lRow - 1;

	// If we are not in selection by package mode OR if there is no valve that belongs to a set...
	if( false == m_pclIndSelDpCBCVParams->m_bOnlyForSet || false == bAtLeastOnePICVPartOfASet )
	{
		pclSSheet->ShowCol( CD_ControlValve_Box, FALSE );
	}

	pclSheetDescriptionPICv->SetUserVariable( _SDUV_TOTALPRODUCT_COUNT, lValveTotalCount );
	pclSheetDescriptionPICv->SetUserVariable( _SDUV_NOPRIORITYPRODUCT_COUNT, lValveNotPriorityCount );

	// Now we can create Show/All priorities if needed.
	// Remarks: 2nd condition to avoid to have button if we have only no priority valves.
	if( true == bShowAllPrioritiesShown && lValveTotalCount > lValveNotPriorityCount )
	{
		// Create Show/Hide all priorities button in regards to current state!
		CCDButtonShowAllPriorities::ButtonState eState = ( true == GetShowAllPrioritiesFlag( SD_ControlValve ) ) ? 
				CCDButtonShowAllPriorities::ButtonState::HidePriorities : CCDButtonShowAllPriorities::ButtonState::ShowPriorities;
		
		CCDButtonShowAllPriorities *pclShowAllButton = CreateShowAllPrioritiesButton( CD_ControlValve_FirstColumn, lShowAllPrioritiesButtonRow, 
				bShowAllPrioritiesShown, eState, lShowAllPrioritiesButtonRow + 1, pclSSheet->GetMaxRows(), pclSheetDescriptionPICv );

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
			pclSSheet->ShowRow( lShowAllPrioritiesButtonRow, FALSE );
		}
	}

	// Try to merge only if there is more than one PICV.
	if( lValveTotalCount > 1 )
	{
		vector<long> vecColumnList;
		vecColumnList.push_back( CD_ControlValve_PipeSize );
		vecColumnList.push_back( CD_ControlValve_PipeLinDp );
		vecColumnList.push_back( CD_ControlValve_PipeV );
		pclSSheet->MergeRowsWithSameValues( RD_ControlValve_FirstAvailRow, lLastDataRow, vecColumnList );
	}

	pclSSheet->SetCellBorder( CD_ControlValve_CheckBox, lLastDataRow, CD_ControlValve_Separator - 1, lLastDataRow, true, SS_BORDERTYPE_BOTTOM,
			SS_BORDERSTYLE_SOLID, _BLACK );

	// Needed by pipe merging.
	pclSSheet->SetCellBorder( CD_ControlValve_PipeSize, lLastDataRow, CD_ControlValve_Pointer- 1, lLastDataRow, true, SS_BORDERTYPE_BOTTOM,
			SS_BORDERSTYLE_SOLID, _BLACK );

	// Add remark for KTH family.
	// This loop suppose the KTH came from the TabCDialogSSelPICv.
	if( true == bRemarkInfoKTH )
	{
		lRow = _SetRemarksKTH( pclSheetDescriptionPICv, lRow );
	}

	// Add Remarks.
	// Note: these lines must be placed after creating Show/Hide all priorities button to avoid to take in count these lines when defining
	//       range of row to be Show/Hide.
	lRow = pclSheetDescriptionPICv->WriteRemarks( lRow, CD_ControlValve_CheckBox, CD_ControlValve_Separator );
		
	// Add the main title.
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)GetTitleBackgroundColor( pclSheetDescriptionPICv ) );
	
	pclSheetDescriptionPICv->SetUserVariable( _SDUV_TITLEFORECOLOR, ( pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor ) ) );
	pclSheetDescriptionPICv->SetUserVariable( _SDUV_TITLEBACKCOLOR, ( pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor ) ) );

	pclSSheet->AddCellSpanW( CD_ControlValve_CheckBox, RD_ControlValve_GroupName, CD_ControlValve_Separator - CD_ControlValve_CheckBox, 1 );
	pclSSheet->SetStaticText( CD_ControlValve_CheckBox, RD_ControlValve_GroupName, IDS_SSHEETSSELCV_DPCBCVGROUP );
	pclSSheet->AddCellSpanW( CD_ControlValve_PipeSize, RD_ControlValve_GroupName, CD_ControlValve_Pointer - CD_ControlValve_PipeSize, 1 );
	
	CTable *pclTable = dynamic_cast<CTable *>( m_pclIndSelDpCBCVParams->m_pPipeDB->Get( m_pclIndSelDpCBCVParams->m_strPipeSeriesID ).MP );
	ASSERT( NULL != pclTable );

	pclSSheet->SetStaticText( CD_ControlValve_PipeSize, RD_ControlValve_GroupName, pclTable->GetName() );
	
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

	// Add possibility to change column size.
	// Remark: place this code after any initialization (like adding, removing, resizing, hiding/showing columns).
	CResizingColumnInfo *pclResizingColumnInfo = pclSheetDescriptionPICv->GetResizingColumnInfo();
	pclResizingColumnInfo->ActivateFeature( true, false, RD_ControlValve_FirstAvailRow, lLastDataRow );
	pclResizingColumnInfo->AddRangeColumn( CD_ControlValve_Name, CD_ControlValve_TemperatureRange, RD_ControlValve_ColName, RD_ControlValve_Unit );
	pclResizingColumnInfo->AddRangeColumn( CD_ControlValve_PipeSize, CD_ControlValve_PipeV, RD_ControlValve_ColName, RD_ControlValve_Unit );

	// Add possibility to click on title group to reset column width.
	CreateTitleGroupButtons( CD_ControlValve_CheckBox, CD_ControlValve_PipeV, RD_ControlValve_GroupName, pclSheetDescriptionPICv );

	return lRetRow;
}

void CRViewSSelDpCBCV::GetAdapterList( CDB_ControlValve *pclSelectedControlValve, CRank *pclAdapterList, CDB_RuledTableBase **ppclRuledTable )
{
	if( NULL == m_pclIndSelDpCBCVParams || NULL == m_pclIndSelDpCBCVParams->m_pTADB )
	{
		ASSERT_RETURN;
	}

	m_pclIndSelDpCBCVParams->m_pTADB->GetAdapterList( pclSelectedControlValve, pclAdapterList, ppclRuledTable, m_bDowngradeActuatorFunctionality,
			m_pclIndSelDpCBCVParams->m_strActuatorPowerSupplyID, m_pclIndSelDpCBCVParams->m_strActuatorInputSignalID, m_pclIndSelDpCBCVParams->m_iActuatorFailSafeFunction,
			m_pclIndSelDpCBCVParams->m_eActuatorDRPFunction, m_pclIndSelDpCBCVParams->m_eCvCtrlType, m_pclIndSelDpCBCVParams->m_bOnlyForSet, 
			m_pclIndSelDpCBCVParams->m_eFilterSelection );
}

void CRViewSSelDpCBCV::GetSetAccessoryList( CDB_ControlValve *pclSelectedControlValve, CDB_Actuator *pclSeletedActuator, CRank *pclSetAccessoryList, 
		CDB_RuledTableBase **ppclRuledTable )
{
	if( NULL == m_pclIndSelDpCBCVParams || NULL == m_pclIndSelDpCBCVParams->m_pTADB )
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

	CDB_Set *pPICVActSet = pActSetTab->FindCompatibleSet( pclSelectedControlValve->GetIDPtr().ID, pclSeletedActuator->GetIDPtr().ID );
	
	if( NULL != pPICVActSet )
	{
		*ppclRuledTable = dynamic_cast<CDB_RuledTable *>( pPICVActSet->GetAccGroupIDPtr().MP );
	
		if( NULL != *ppclRuledTable )
		{	
			m_pclIndSelDpCBCVParams->m_pTADB->GetAccessories( pclSetAccessoryList, *ppclRuledTable, m_pclIndSelDpCBCVParams->m_eFilterSelection );
		}
	}
}

void CRViewSSelDpCBCV::RemoveSheetDescriptions( UINT uiFromSheetDescriptionID )
{
	CSheetDescription *pBottomSheetDescription = m_ViewDescription.GetBottomSheetDescription();

	while( NULL != pBottomSheetDescription && uiFromSheetDescriptionID != pBottomSheetDescription->GetSheetDescriptionID() )
	{
		switch( uiFromSheetDescriptionID )
		{
			case SD_ControlValve:

				// Other sheets will be removed in the base class.
				if( pBottomSheetDescription->GetSheetDescriptionID() >= SD_DpCBCValveFirst 
						&& pBottomSheetDescription->GetSheetDescriptionID() < SD_DpCBCValveLast )
				{
					m_ViewDescription.RemoveOneSheetDescription( pBottomSheetDescription->GetSheetDescriptionID() );
					m_pCDBExpandCollapseRowsSv = NULL;
					m_pCDBExpandCollapseGroupSvAccessory = NULL;
					m_pCDBShowAllPrioritiesSv = NULL;
					pBottomSheetDescription = m_ViewDescription.GetBottomSheetDescription();
				}
				else
				{
					pBottomSheetDescription = m_ViewDescription.GetPrevSheetDescription( pBottomSheetDescription );
				}

				break;

			case SD_Actuator:

				// Don't remove shutoff valves and its accessories.
				pBottomSheetDescription = m_ViewDescription.GetPrevSheetDescription( pBottomSheetDescription );
				break;

			case SD_ShutoffValve:

				if( SD_SVAccessory == pBottomSheetDescription->GetSheetDescriptionID() )
				{
					m_ViewDescription.RemoveOneSheetDescription( pBottomSheetDescription->GetSheetDescriptionID() );
					pBottomSheetDescription = m_ViewDescription.GetBottomSheetDescription();
				}
				else
				{
					pBottomSheetDescription = m_ViewDescription.GetPrevSheetDescription( pBottomSheetDescription );
				}

				break;
		}
	}

	// Base class.
	CRViewSSelCtrl::RemoveSheetDescriptions( uiFromSheetDescriptionID );
}

void CRViewSSelDpCBCV::FillOtherProducts( CSelectedValve *pSelectedTAP )
{
	if( NULL == m_pclIndSelDpCBCVParams )
	{
		ASSERT_RETURN;
	}

	if( true == m_pclIndSelDpCBCVParams->m_bIsWithSTSChecked )
	{
		_FillShutoffValveRows( pSelectedTAP );
	}
}

void CRViewSSelDpCBCV::SuggestOtherProducts( CDS_SSelCtrl *pSelectedControlValve )
{
	if( NULL == pSelectedControlValve || NULL == dynamic_cast<CDS_SSelDpCBCV*>( pSelectedControlValve) )
	{
		return;
	}

	CDS_SSelDpCBCV *pSelectedDpCBCValve = dynamic_cast<CDS_SSelDpCBCV*>( pSelectedControlValve);
	CDB_ShutoffValve *pclShutoffValve = dynamic_cast<CDB_ShutoffValve*>( pSelectedDpCBCValve->GetSVIDPtr().MP );
	CSheetDescription *pclSheetDescriptionSv = m_ViewDescription.GetFromSheetDescriptionID( SD_ShutoffValve );

	if( NULL != pclShutoffValve && NULL != pclSheetDescriptionSv )
	{
		// Find row number where is the shutoff valve.
		long lSVRowSelected = _GetRowOfEditedSV( pclSheetDescriptionSv, pclShutoffValve );

		if( -1 != lSVRowSelected )
		{
			CCellDescriptionProduct *pclCDShutoffValve = FindCDProduct( lSVRowSelected, ( LPARAM )pclShutoffValve, pclSheetDescriptionSv );

			if( NULL != pclCDShutoffValve && NULL != pclCDShutoffValve->GetProduct() )
			{
				// Simulate a click on the edited shutoff valve.
				_ClickOnShutoffValve( pclSheetDescriptionSv, pclCDShutoffValve, pclSheetDescriptionSv->GetActiveColumn(), lSVRowSelected );

				// Allow to check if we need to change the 'Show all priorities' button or not.
				CheckShowAllPrioritiesButtonState( pclSheetDescriptionSv, lSVRowSelected );

				// Verify accessories on shutoff valve.
				CAccessoryList *pclSvAccessoryList = pSelectedDpCBCValve->GetSVAccessoryList();

				if( pclSvAccessoryList->GetCount() > 0 )
				{
					CAccessoryList::AccessoryItem rAccessoryItem = pclSvAccessoryList->GetFirst();

					while( NULL != rAccessoryItem.IDPtr.MP )
					{
						VerifyCheckboxAccessories( (CDB_Product * )rAccessoryItem.IDPtr.MP, true, &m_vecSvAccessoryList );
						rAccessoryItem = pclSvAccessoryList->GetNext();
					}
				}
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PRIVATE MEMBERS
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

long CRViewSSelDpCBCV::_SetRemarksKTH( CSheetDescription *pclSheetDescriptionPICv, long lRow )
{
	if( NULL == m_pclIndSelDpCBCVParams )
	{
		ASSERTA_RETURN( lRow );
	}

	if( NULL == pclSheetDescriptionPICv || NULL == pclSheetDescriptionPICv->GetSSheetPointer() )
	{
		return lRow;
	}

	CSSheet *pclSSheet = pclSheetDescriptionPICv->GetSSheetPointer();
	CString str = _T("! ");
	
	// Add remark for KTH family.
	if( CDB_ControlProperties::eCvProportional == m_pclIndSelDpCBCVParams->m_eCvCtrlType )
	{
		str += TASApp.LoadLocalizedString( IDS_INFOKTHPROP );
	}
	else if( CDB_ControlProperties::eCv3point == m_pclIndSelDpCBCVParams->m_eCvCtrlType )
	{
		str += TASApp.LoadLocalizedString( IDS_INFOKTH3P );
	}
	else if( CDB_ControlProperties::eCvOnOff == m_pclIndSelDpCBCVParams->m_eCvCtrlType )
	{
		return lRow;
	}
	
	// +1 to set remarks.
	// 'false' to specify that this row can't be selected.
	pclSheetDescriptionPICv->AddRows( 1, false );

	// Span the cell and set the static text.
	pclSSheet->AddCellSpanW( CD_ControlValve_Name, lRow, CD_ControlValve_Separator- CD_ControlValve_Name, 1 );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_LEFT );
	pclSSheet->SetStaticText( CD_ControlValve_Name, lRow, str );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_CENTER );
	lRow++;
	
	return lRow;
}

bool CRViewSSelDpCBCV::_ClickOnShutoffValve( CSheetDescription *pclSheetDescriptionSv, CCellDescriptionProduct *pclCellDescriptionProduct, 
		long lColumn, long lRow )
{
	if( NULL == pclSheetDescriptionSv || NULL == pclSheetDescriptionSv->GetSSheetPointer() || NULL == pclCellDescriptionProduct )
	{
		return false;
	}

	CDB_TAProduct *pclShutoffValve = dynamic_cast<CDB_TAProduct *>( ( CData * )pclCellDescriptionProduct->GetProduct() );

	if( NULL == pclShutoffValve )
	{
		return false;
	}

	CSSheet *pclSSheet = pclSheetDescriptionSv->GetSSheetPointer();

	// Memorize previous focus row.
	long lCurrentFocusedRow = GetCurrentFocusedRow();

	// To be sure to clean all previous data concerning focus.
	KillCurrentFocus();

	// Unselect row where current selection is set (yellow light).
	// Remark: 'KillCurrentFocus' unselect only rows where focus is on (blue light).
	pclSSheet->UnSelectMultipleRows();

	// By default clear accessories list of the shutoff valve.
	m_vecSvAccessoryList.clear();
	m_pCDBExpandCollapseGroupSvAccessory = NULL;

	LPARAM lShutoffValveCount;
	pclSheetDescriptionSv->GetUserVariable( _SDUV_TOTALPRODUCT_COUNT, lShutoffValveCount );

	// Retrieve the current selected shut-off valve if exist.
	CDB_TAProduct *pclCurrentShutoffValveSelected = _GetCurrentShutoffValveSelected();

	// If there is already one shut-off valve selected and user clicks on the current one...
	// Remark: 'm_pCDBExpandCollapseRowsSv' is not created if there is only one shut-off valve. Thus we need to check first if there is only one shut-off valve.
	//         Otherwise we have to check if user has clicked on the current one.
	if( NULL != pclCurrentShutoffValveSelected && ( ( 1 == lShutoffValveCount ) || ( NULL != m_pCDBExpandCollapseRowsSv
			&& lRow == m_pCDBExpandCollapseRowsSv->GetCellPosition().y ) ) )
	{
		// Reset current product selected.
		_SetCurrentShutoffValveSelected( NULL );

		// Uncheck checkbox.
		pclSSheet->SetCheckBox( CD_ShutoffValve_CheckBox, lRow, _T(""), false, true );
		m_lSVSelectedRow = -1;

		// Delete Expand/Collapse rows button if exist.
		// Remark: 'm_pCDBExpandCollapseRows' is set to NULL in method!
		DeleteExpandCollapseRowsButton( m_pCDBExpandCollapseRowsSv, pclSheetDescriptionSv );

		// Show Show/Hide all priorities button if exist.
		if( NULL != m_pCDBShowAllPrioritiesSv )
		{
			m_pCDBShowAllPrioritiesSv->ApplyInternalChange();
		}

		// Set focus on shut-off valve currently selected.
		PrepareAndSetNewFocus( pclSheetDescriptionSv, pclSheetDescriptionSv->GetActiveColumn(), lRow, 0 );

		// Remove all sheets after shut-off valve.
		RemoveSheetDescriptions( SD_ShutoffValve );

		UpdateSVInfos();

		// Notify a change in the selection.
		SelectionHasChanged( IsSelectionReady() );
	}
	else
	{
		// If a shut-off valve is already selected...
		if( NULL != pclCurrentShutoffValveSelected )
		{
			// Remove all sheets after shut-off valve.
			RemoveSheetDescriptions( SD_ShutoffValve );
		}

		// Save new shut-off valve selection.
		_SetCurrentShutoffValveSelected( pclCellDescriptionProduct );

		// Unselect previous selection.
		if( m_lSVSelectedRow > -1 )
		{
			pclSSheet->SetCheckBox( CD_ShutoffValve_CheckBox, m_lSVSelectedRow, _T(""), false, true );
		}

		// Check checkbox.
		pclSSheet->SetCheckBox( CD_ShutoffValve_CheckBox, lRow, _T(""), true, true );
		m_lSVSelectedRow = lRow;

		// Update DlgInfoSSelDpCBCV.
		UpdateSVInfos();

		// Delete Expand/Collapse rows button if exist.
		if( NULL != m_pCDBExpandCollapseRowsSv )
		{
			// Remark: 'm_pCDBExpandCollapseRowsSv' is set to NULL in method!
			DeleteExpandCollapseRowsButton( m_pCDBExpandCollapseRowsSv, pclSheetDescriptionSv );
		}

		// Create Expand/Collapse rows button if needed...
		LPARAM lValveTotalCount;
		pclSheetDescriptionSv->GetUserVariable( _SDUV_TOTALPRODUCT_COUNT, lValveTotalCount );

		if( lValveTotalCount > 1 )
		{
			m_pCDBExpandCollapseRowsSv = CreateExpandCollapseRowsButton( CD_ShutoffValve_FirstColumn, lRow, true, 
					CCDButtonExpandCollapseRows::ButtonState::CollapseRow, pclSheetDescriptionSv->GetFirstSelectableRow(), 
					pclSheetDescriptionSv->GetLastSelectableRow( false ), pclSheetDescriptionSv );

			// Show button.
			if( NULL != m_pCDBExpandCollapseRowsSv )
			{
				m_pCDBExpandCollapseRowsSv->SetShowStatus( true );
			}
		}

		// Select shut-off valve (just highlight background).
		pclSSheet->SelectOneRow( lRow, pclSheetDescriptionSv->GetSelectionFrom(), pclSheetDescriptionSv->GetSelectionTo() );

		// Fill corresponding accessories.
		_FillSVAccessoryRows();

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

	return true;
}

void CRViewSSelDpCBCV::_FillShutoffValveRows( CSelectedValve *pSelectedTAPDpCBCV )
{
	if( NULL == m_pclIndSelDpCBCVParams || NULL == m_pclIndSelDpCBCVParams->m_pclSelectDpCBCVList || NULL == pSelectedTAPDpCBCV )
	{
		ASSERT_RETURN;
	}

	// Try to retrieve combined Dp controller, control and balancing valve selected list.
	CSelectDpCBCVList *pclDpCBCVList = m_pclIndSelDpCBCVParams->m_pclSelectDpCBCVList;
	CSelectShutoffList *pSelectedSvList = NULL;

	// Retrieve shutoff valve list linked to current combined Dp controller, control and balancing valve.
	if( pclDpCBCVList->SelectShutoffValve( m_pclIndSelDpCBCVParams, pSelectedTAPDpCBCV ) > 0 )
	{
		pSelectedSvList = m_pclIndSelDpCBCVParams->m_pclSelectDpCBCVList->GetSvSelected();
	}

	// If no balancing valve exist...
	if( NULL == pSelectedSvList || 0 == pSelectedSvList->GetCount() )
	{
		return;
	}

	// Call 'MultiSpreadBase' method to get a new SSheet.
	CSheetDescription *pclSheetDescriptionSV = CreateSSheet( SD_ShutoffValve );

	if( NULL == pclSheetDescriptionSV || NULL == pclSheetDescriptionSV->GetSSheetPointer() )
	{
		return;
	}

	CSSheet *pclSSheet = pclSheetDescriptionSV->GetSSheetPointer();

	pclSSheet->SetBool( SSB_REDRAW, FALSE );

	pclSSheet->Init();
	pclSSheet->SetBool( SSB_ALLOWDRAGDROP, FALSE );
	pclSSheet->SetBool( SSB_HORZSCROLLBAR, FALSE );
	pclSSheet->SetBool( SSB_VERTSCROLLBAR, FALSE );
	pclSSheet->SetBool( SSB_MOVEACTIVEONFOCUS, FALSE );
	pclSSheet->SetTextTip( SS_TT_STATUS_FLOATING, 250, NULL, _LIGHTYELLOW, _TAH_BLACK );

	// Initialize DpC sheet and fill header.
	_InitAndFillSVHeader( pclSheetDescriptionSV, pclSSheet );

	long lRetRow = -1;
	long lRow = RD_ShutoffValve_FirstAvailRow;

	long lValveTotalCount = 0;
	long lValveNotPriorityCount = 0;
	bool bShowAllPrioritiesShown = false;
	long lShowAllPrioritiesButtonRow = 0;

	pclSheetDescriptionSV->SetUserVariable( _SDUV_TOTALPRODUCT_COUNT, 0 );
	pclSheetDescriptionSV->SetUserVariable( _SDUV_NOPRIORITYPRODUCT_COUNT, 0 );

	pclSheetDescriptionSV->RestartRemarkGenerator();

	for( CSelectedValve *pclSelectedShutoffValve = pSelectedSvList->GetFirst<CSelectedValve>(); NULL != pclSelectedShutoffValve;
			pclSelectedShutoffValve = pSelectedSvList->GetNext<CSelectedValve>() )
	{
		CDB_ShutoffValve *pclShutoffValve = dynamic_cast<CDB_ShutoffValve *>( pclSelectedShutoffValve->GetpData() );

		if( NULL == pclShutoffValve )
		{
			continue;
		}

		if( true == pclSelectedShutoffValve->IsFlagSet( CSelectedBase::eNotPriority ) )
		{
			lValveNotPriorityCount++;

			if( false == bShowAllPrioritiesShown )
			{
				// Add one empty not selectable row.
				pclSheetDescriptionSV->AddRows( 1 );
				pclSheetDescriptionSV->RemoveSelectableRow( lRow );

				pclSSheet->SetCellBorder( CD_ShutoffValve_CheckBox, lRow, CD_ShutoffValve_Separator - 1, lRow, true, SS_BORDERTYPE_TOP | SS_BORDERTYPE_BOTTOM,
						SS_BORDERSTYLE_SOLID, _BLACK );

				pclSSheet->SetCellBorder( CD_ShutoffValve_PipeSize, lRow, CD_ShutoffValve_Pointer - 1, lRow, true, SS_BORDERTYPE_TOP | SS_BORDERTYPE_BOTTOM,
						SS_BORDERSTYLE_SOLID, _BLACK );

				lShowAllPrioritiesButtonRow = lRow++;
				bShowAllPrioritiesShown = true;
			}
		}

		long lRetVal = _FillOneSVRow( pclSheetDescriptionSV, pclSSheet, pclSelectedShutoffValve, lRow );

		if( lRetVal > 0 )
		{
			lRetRow = lRetVal;
		}

		lRow++;
		lValveTotalCount++;
	}

	long lLastDataRow = lRow - 1;

	pclSheetDescriptionSV->SetUserVariable( _SDUV_TOTALPRODUCT_COUNT, lValveTotalCount );
	pclSheetDescriptionSV->SetUserVariable( _SDUV_NOPRIORITYPRODUCT_COUNT, lValveNotPriorityCount );

	// Now we can create Show/Hide all priorities if needed.
	// Remarks: 2nd condition to avoid to have button if we have only no priority valves.
	if( true == bShowAllPrioritiesShown && lValveTotalCount > lValveNotPriorityCount )
	{
		// Create Show/Hide all priorities button in regards to current state!
		CCDButtonShowAllPriorities::ButtonState eState = ( true == GetShowAllPrioritiesFlag( SD_ShutoffValve ) ) ? 
				CCDButtonShowAllPriorities::ButtonState::HidePriorities : CCDButtonShowAllPriorities::ButtonState::ShowPriorities;
		
		CCDButtonShowAllPriorities *pclShowAllButton = CreateShowAllPrioritiesButton( CD_ShutoffValve_FirstColumn, lShowAllPrioritiesButtonRow, 
				bShowAllPrioritiesShown, eState, lShowAllPrioritiesButtonRow + 1, pclSSheet->GetMaxRows(), pclSheetDescriptionSV );

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
	lRow = pclSheetDescriptionSV->WriteRemarks( lRow, CD_ShutoffValve_CheckBox, CD_ShutoffValve_Separator );

	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, ( LPARAM )FALSE );

	pclSSheet->SetBool( SSB_REDRAW, TRUE );

	// Set that there is no selection at now.
	_SetCurrentShutoffValveSelected( NULL );

	// Add possibility to change column size.
	// Remark: place this code after any initialization (like adding, removing, resizing, hiding/showing columns).
	CResizingColumnInfo *pclResizingColumnInfo = pclSheetDescriptionSV->GetResizingColumnInfo();
	pclResizingColumnInfo->ActivateFeature( true, false, RD_ShutoffValve_FirstAvailRow, lLastDataRow );
	pclResizingColumnInfo->AddRangeColumn( CD_ShutoffValve_Name, CD_ShutoffValve_TemperatureRange, RD_ShutoffValve_ColName, RD_ShutoffValve_Unit );
	pclResizingColumnInfo->AddRangeColumn( CD_ShutoffValve_PipeSize, CD_ShutoffValve_PipeV, RD_ShutoffValve_ColName, RD_ShutoffValve_Unit );

	// Add possibility to click on title group to reset column width.
	CreateTitleGroupButtons( CD_ShutoffValve_CheckBox, CD_ShutoffValve_PipeV, RD_ShutoffValve_GroupName, pclSheetDescriptionSV );

	return;
}

void CRViewSSelDpCBCV::_InitAndFillSVHeader( CSheetDescription *pclSheetDescriptionSV, CSSheet *pclSSheet )
{
	if( NULL == m_pclIndSelDpCBCVParams || NULL == m_pclIndSelDpCBCVParams->m_pPipeDB )
	{
		ASSERT_RETURN;
	}

	// Set max rows.
	pclSSheet->SetMaxRows( RD_ShutoffValve_FirstAvailRow - 1 );

	// All cells are static by default and filled with _T("").
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	pclSSheet->FormatStaticText( -1, -1, -1, -1, _T( "" ) );

	// Increase row height.
	double dRowHeight = 12.75;
	pclSSheet->SetRowHeight( RD_ShutoffValve_FirstRow, dRowHeight * 0.5 );
	pclSSheet->SetRowHeight( RD_ShutoffValve_GroupName, dRowHeight * 1.5 );
	pclSSheet->SetRowHeight( RD_ShutoffValve_ColName, dRowHeight * 2 );
	pclSSheet->SetRowHeight( RD_ShutoffValve_Unit, dRowHeight * 1.2 );

	// Initialize.
	pclSheetDescriptionSV->Init( 1, pclSSheet->GetMaxRows(), 1, CSheetDescription::SD_ParameterMode_Mono );
	pclSheetDescriptionSV->AddColumnInPixels( CD_ShutoffValve_FirstColumn, m_mapSSheetColumnWidth[SD_ShutoffValve][CD_ShutoffValve_FirstColumn] );
	pclSheetDescriptionSV->AddColumnInPixels( CD_ShutoffValve_CheckBox, m_mapSSheetColumnWidth[SD_ShutoffValve][CD_ShutoffValve_CheckBox] );
	pclSheetDescriptionSV->AddColumnInPixels( CD_ShutoffValve_Name, m_mapSSheetColumnWidth[SD_ShutoffValve][CD_ShutoffValve_Name] );
	pclSheetDescriptionSV->AddColumnInPixels( CD_ShutoffValve_Material, m_mapSSheetColumnWidth[SD_ShutoffValve][CD_ShutoffValve_Material] );
	pclSheetDescriptionSV->AddColumnInPixels( CD_ShutoffValve_Connection, m_mapSSheetColumnWidth[SD_ShutoffValve][CD_ShutoffValve_Connection] );
	pclSheetDescriptionSV->AddColumnInPixels( CD_ShutoffValve_Version, m_mapSSheetColumnWidth[SD_ShutoffValve][CD_ShutoffValve_Version] );
	pclSheetDescriptionSV->AddColumnInPixels( CD_ShutoffValve_PN, m_mapSSheetColumnWidth[SD_ShutoffValve][CD_ShutoffValve_PN] );
	pclSheetDescriptionSV->AddColumnInPixels( CD_ShutoffValve_Size, m_mapSSheetColumnWidth[SD_ShutoffValve][CD_ShutoffValve_Size] );
	pclSheetDescriptionSV->AddColumnInPixels( CD_ShutoffValve_Dp, m_mapSSheetColumnWidth[SD_ShutoffValve][CD_ShutoffValve_Dp] );
	pclSheetDescriptionSV->AddColumnInPixels( CD_ShutoffValve_TemperatureRange, m_mapSSheetColumnWidth[SD_ShutoffValve][CD_ShutoffValve_TemperatureRange] );
	pclSheetDescriptionSV->AddColumnInPixels( CD_ShutoffValve_Separator, m_mapSSheetColumnWidth[SD_ShutoffValve][CD_ShutoffValve_Separator] );
	pclSheetDescriptionSV->AddColumnInPixels( CD_ShutoffValve_PipeSize, m_mapSSheetColumnWidth[SD_ShutoffValve][CD_ShutoffValve_PipeSize] );
	pclSheetDescriptionSV->AddColumnInPixels( CD_ShutoffValve_PipeLinDp, m_mapSSheetColumnWidth[SD_ShutoffValve][CD_ShutoffValve_PipeLinDp] );
	pclSheetDescriptionSV->AddColumnInPixels( CD_ShutoffValve_PipeV, m_mapSSheetColumnWidth[SD_ShutoffValve][CD_ShutoffValve_PipeV] );
	pclSheetDescriptionSV->AddColumnInPixels( CD_ShutoffValve_Pointer, m_mapSSheetColumnWidth[SD_ShutoffValve][CD_ShutoffValve_Pointer] );

	// Set in which column parameter must be saved.
	pclSheetDescriptionSV->AddParameterColumn( CD_ShutoffValve_Pointer );

	// Set the focus column.
	pclSheetDescriptionSV->SetActiveColumn( CD_ShutoffValve_Name );

	// Set range for selection.
	pclSheetDescriptionSV->SetFocusColumnRange( CD_ShutoffValve_CheckBox, CD_ShutoffValve_TemperatureRange );

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Row name.

	// Add the main title.
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, ( LPARAM )GetTitleBackgroundColor( pclSheetDescriptionSV ) );
	pclSheetDescriptionSV->SetUserVariable( _SDUV_TITLEFORECOLOR, ( pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor ) ) );
	pclSheetDescriptionSV->SetUserVariable( _SDUV_TITLEBACKCOLOR, ( pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor ) ) );

	pclSSheet->AddCellSpanW( CD_ShutoffValve_CheckBox, RD_ShutoffValve_GroupName, CD_ShutoffValve_Separator - CD_ShutoffValve_CheckBox, 1 );
	pclSSheet->SetStaticText( CD_ShutoffValve_CheckBox, RD_ShutoffValve_GroupName, IDS_SSHEETSSELSV_VALVEGROUP );

	pclSSheet->AddCellSpanW( CD_ShutoffValve_PipeSize, RD_ShutoffValve_GroupName, CD_ShutoffValve_Pointer - CD_ShutoffValve_PipeSize, 1 );

	CTable *pclTable = dynamic_cast<CTable *>( m_pclIndSelDpCBCVParams->m_pPipeDB->Get( m_pclIndSelDpCBCVParams->m_strPipeSeriesID ).MP );
	ASSERT( NULL != pclTable );

	pclSSheet->SetStaticText( CD_ShutoffValve_PipeSize, RD_ShutoffValve_GroupName, pclTable->GetName() );

	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, ( LPARAM )FALSE );

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
	pclSSheet->SetStaticText( CD_ShutoffValve_PipeLinDp, RD_ShutoffValve_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_LINPRESSDROP ) ).c_str() );
	pclSSheet->SetStaticText( CD_ShutoffValve_PipeV, RD_ShutoffValve_Unit, GetNameOf( pUnitDB->GetDefaultUnit( _U_VELOCITY ) ).c_str() );
	//
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Draw black line below column header.
	pclSSheet->SetCellBorder( CD_ShutoffValve_CheckBox, RD_ShutoffValve_Unit, CD_ShutoffValve_Separator - 1, RD_ShutoffValve_Unit, true,	
			SS_BORDERTYPE_BOTTOM );
	
	pclSSheet->SetCellBorder( CD_ShutoffValve_PipeSize, RD_ShutoffValve_Unit, CD_ShutoffValve_Pointer - 1, RD_ShutoffValve_Unit, true,
			SS_BORDERTYPE_BOTTOM );
}

long CRViewSSelDpCBCV::_FillOneSVRow( CSheetDescription *pclSheetDescriptionSV, CSSheet *pclSSheet, CSelectedValve *pSelectedTAP, long lRow )
{
	if( NULL == m_pclIndSelDpCBCVParams || NULL == m_pclIndSelDpCBCVParams->m_pclSelectDpCBCVList 
			|| NULL == m_pclIndSelDpCBCVParams->m_pclSelectDpCBCVList->GetSelectPipeList() )
	{
		ASSERTA_RETURN( -1 );
	}

	CDS_TechnicalParameter *pTechParam = m_pclIndSelDpCBCVParams->m_pTADS->GetpTechParams();

	CDB_ShutoffValve *pclShutoffValve = dynamic_cast<CDB_ShutoffValve *>( pSelectedTAP->GetpData() );

	if( NULL == pclShutoffValve )
	{
		return -1;
	}

	long lRetRow = 0;
	bool bAtLeastOneError = false;
	bool bAtLeastOneWarning = false;
	bool bBest = pSelectedTAP->IsFlagSet( CSelectedBase::eBest );

	// Add this row.
	// 'true' to specify that this row can be selected.
	pclSheetDescriptionSV->AddRows( 1, true );

	// First columns will be set at the end!

	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, ( LPARAM )_BLACK );

	// Set Dp.
	if( true == pSelectedTAP->IsFlagSet( CSelectedBase::eDp ) )
	{
		// TODO: verify that it is well this function (VerifyValvMaxDp) to call !!
		if( pSelectedTAP->GetDp() > pTechParam->VerifyValvMaxDp( pclShutoffValve ) )
		{
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, ( LPARAM )_RED );
			bAtLeastOneError = true;
		}
		else
		{
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, ( LPARAM )_ORANGE );
			bAtLeastOneWarning = true;
		}
	}

	pclSSheet->SetStaticText( CD_ShutoffValve_Dp, lRow, WriteCUDouble( _U_DIFFPRESS, pSelectedTAP->GetDp() ) );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, ( LPARAM )_BLACK );

	// Temperature range.
	if( true == pSelectedTAP->IsFlagSet( CSelectedBase::eTemperature ) )
	{
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, ( LPARAM )_RED );
		bAtLeastOneError = true;
	}

	pclSSheet->SetStaticText( CD_ShutoffValve_TemperatureRange, lRow, ( ( CDB_TAProduct * )pSelectedTAP->GetProductIDPtr().MP )->GetTempRange() );
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

	// Add checkbox.
	pclSSheet->SetCheckBox( CD_ShutoffValve_CheckBox, lRow, _T(""), false, true );

	// In addition of current flag, we have possibility that valve has the flag 'Not available' or 'Deleted'. In that case, we show valve name 
	// in red with "*" around it and symbol '!' or '!!' after.
	if( true == pclShutoffValve->IsDeleted() )
	{
		pclSheetDescriptionSV->WriteTextWithFlags( CString( pclShutoffValve->GetName() ), CD_ShutoffValve_Name, lRow, 
				CSheetDescription::RemarkFlags::Deleted, _T(""), _RED );
	}
	else if( false == pclShutoffValve->IsAvailable() )
	{
		pclSheetDescriptionSV->WriteTextWithFlags( CString( pclShutoffValve->GetName() ), CD_ShutoffValve_Name, lRow, 
				CSheetDescription::RemarkFlags::NotAvailable, _T(""), _RED );
	}
	else
	{
		pclSSheet->SetStaticText( CD_ShutoffValve_Name, lRow, pclShutoffValve->GetName() );
	}

	if( true == bAtLeastOneError )
	{
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, ( LPARAM )_BLACK );
	}
	else if( true == bBest )
	{
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, ( LPARAM )_GREEN );
	}

	pclSSheet->SetStaticText( CD_ShutoffValve_Material, lRow, pclShutoffValve->GetBodyMaterial() );
	pclSSheet->SetStaticText( CD_ShutoffValve_Connection, lRow, pclShutoffValve->GetConnect() );
	pclSSheet->SetStaticText( CD_ShutoffValve_Version, lRow, pclShutoffValve->GetVersion() );
	pclSSheet->SetStaticText( CD_ShutoffValve_PN, lRow, pclShutoffValve->GetPN().c_str() );
	pclSSheet->SetStaticText( CD_ShutoffValve_Size, lRow, pclShutoffValve->GetSize() );

	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, ( LPARAM )_BLACK );

	// Set pipe size.
	CSelectPipe selPipe( m_pclIndSelDpCBCVParams );
	m_pclIndSelDpCBCVParams->m_pclSelectDpCBCVList->GetSelectPipeList()->GetMatchingPipe( pclShutoffValve->GetSizeKey(), selPipe );
	pclSSheet->SetStaticText( CD_ShutoffValve_PipeSize, lRow, selPipe.GetpPipe()->GetName() );

	// Set the LinDp to orange if it is above or below the technical parameters limits.
	if( selPipe.GetLinDp() > pTechParam->GetPipeMaxDp() || selPipe.GetLinDp() < pTechParam->GetPipeMinDp() )
	{
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, ( LPARAM )_ORANGE );
	}

	pclSSheet->SetStaticText( CD_ShutoffValve_PipeLinDp, lRow, WriteCUDouble( _U_LINPRESSDROP, selPipe.GetLinDp() ) );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, ( LPARAM )_BLACK );

	// Set the velocity to orange if it is above the technical parameter limit.
	// Orange if it is below the dMinVel.
	if( selPipe.GetU() > pTechParam->GetPipeMaxVel() || selPipe.GetU() < pTechParam->GetPipeMinVel() )
	{
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, ( LPARAM )_ORANGE );
	}

	pclSSheet->SetStaticText( CD_ShutoffValve_PipeV, lRow, WriteCUDouble( _U_VELOCITY, selPipe.GetU() ) );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, ( LPARAM )_BLACK );

	// Save parameter.
	CreateCellDescriptionProduct( pclSheetDescriptionSV->GetFirstParameterColumn(), lRow, ( LPARAM )pclShutoffValve, pclSheetDescriptionSV );

	pclSSheet->SetCellBorder( CD_ShutoffValve_CheckBox, lRow, CD_ShutoffValve_Separator - 1, lRow, true, SS_BORDERTYPE_BOTTOM, 
			SS_BORDERSTYLE_SOLID, _GRAY );

	pclSSheet->SetCellBorder( CD_ShutoffValve_PipeSize, lRow, CD_ShutoffValve_Pointer - 1, lRow, true, SS_BORDERTYPE_BOTTOM, 
			SS_BORDERSTYLE_SOLID, _GRAY );

	return 0;
}

void CRViewSSelDpCBCV::_FillSVAccessoryRows( )
{
	if( NULL == m_pclIndSelDpCBCVParams || NULL == m_pclIndSelDpCBCVParams->m_pTADB )
	{
		ASSERT_RETURN;
	}

	// Retrieve the current selected shutoff valve.
	CDB_TAProduct *pclCurrentShutoffValveSelected = _GetCurrentShutoffValveSelected();

	if( NULL == pclCurrentShutoffValveSelected )
	{
		return;
	}

	CDB_RuledTable *pRuledTable = ( CDB_RuledTable * )( pclCurrentShutoffValveSelected->GetAccessoriesGroupIDPtr().MP );

	if( NULL == pRuledTable )
	{
		return;
	}

	CRank rList;
	int iCount = m_pclIndSelDpCBCVParams->m_pTADB->GetAccessories( &rList, pRuledTable, m_pclIndSelDpCBCVParams->m_eFilterSelection );

	if( 0 == iCount )
	{
		return;
	}

	// Call 'MultiSpreadBase' method to get a new SSheet.
	CSheetDescription *pclSDAccessory = CreateSSheet( SD_SVAccessory );

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
	pclSSheet->FormatStaticText( -1, -1, -1, -1, _T( "" ) );

	// Get sheet description of shut-off valve to retrieve width.
	CSheetDescription *pclSDShutoffValve = m_ViewDescription.GetFromSheetDescriptionID( SD_ShutoffValve );
	// It's absolutely not normal to have this pointer NULL.
	ASSERT( NULL != pclSDShutoffValve );

	// Take SSheet size and client area size and determine the right width to take in count.
	CRect rectSize = pclSDShutoffValve->GetSSheetPointer()->GetSheetSizeInPixels();

	// Prepare first column width (to match shut-off valve sheet).
	long lFirstColumnWidth = pclSDShutoffValve->GetSSheetPointer()->GetColWidthInPixelsW( CD_ShutoffValve_FirstColumn );

	// Do some correction in regards to some columns.
	long lTotalWidth = rectSize.Width() - lFirstColumnWidth;
	lTotalWidth -= pclSDShutoffValve->GetSSheetPointer()->GetColWidthInPixelsW( CD_ShutoffValve_Pointer );

	// Try to create 2 columns in just the middle of balancing valve sheet.
	long lLeftWidth = ( long )( lTotalWidth / 2 );
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
	double RowHeight = 12.75;
	pclSSheet->SetRowHeight( RD_Accessory_FirstRow, RowHeight * 0.5 );
	pclSSheet->SetRowHeight( RD_Accessory_GroupName, RowHeight * 1.5 );

	// Set title.
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, ( LPARAM )GetTitleBackgroundColor( pclSDAccessory ) );

	pclSDAccessory->SetUserVariable( _SDUV_TITLEFORECOLOR, ( pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor ) ) );
	pclSDAccessory->SetUserVariable( _SDUV_TITLEBACKCOLOR, ( pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor ) ) );

	pclSSheet->AddCellSpanW( CD_Accessory_Left, RD_Accessory_GroupName, CD_Accessory_LastColumn - CD_Accessory_Left, 1 );
	pclSSheet->SetStaticText( CD_Accessory_Left, RD_Accessory_GroupName, IDS_SSHEETSSELSV_VALVEACCGROUP );

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
		CDB_Product *pclAccessory = (CDB_Product * )lparam;
		VERIFY( NULL != pclAccessory );
		//VERIFY( 0 != pclAccessory->IsClass( CLASS( CDB_Product ) ) );

		if( false == pclAccessory->IsAnAccessory() )
		{
			continue;
		}

		// Create checkbox accessory.
		CString strName = _T("");

		if( true == pRuledTable->IsByPair( pclAccessory->GetIDPtr().ID ) )
		{
			strName += _T("2x ");
		}

		strName += pclAccessory->GetName();

		CCDBCheckboxAccessory *pCheckbox = CreateCheckboxAccessory( lLeftOrRight, lRow, false, true, strName, pclAccessory, pRuledTable,
				&m_vecSvAccessoryList, pclSDAccessory );

		if( NULL != pCheckbox )
		{
			pCheckbox->ApplyInternalChange();
		}

		// Description.
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleEllipses, ( LPARAM )TRUE );
		pclSSheet->SetStaticText( lLeftOrRight, lRow + 1, pclAccessory->GetComment() );
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

		if( FALSE == bContinue )
		{
			lRow++;
		}
	}

	VerifyCheckboxAccessories( NULL, false, &m_vecSvAccessoryList );

	m_pCDBExpandCollapseGroupSvAccessory = CreateExpandCollapseGroupButton( CD_Accessory_FirstColumn, RD_Accessory_GroupName, true,
			CCDButtonExpandCollapseGroup::ButtonState::CollapseRow, pclSDAccessory->GetFirstSelectableRow(), pclSDAccessory->GetLastSelectableRow( false ), 
			pclSDAccessory );

	// Show button.
	if( NULL != m_pCDBExpandCollapseGroupSvAccessory )
	{
		m_pCDBExpandCollapseGroupSvAccessory->SetShowStatus( true );
	}

	pclSSheet->SetCellBorder( CD_Accessory_Left, lRow, CD_Accessory_LastColumn - 1, lRow, true, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );

	pclSSheet->SetBool( SSB_REDRAW, TRUE );
}

CDB_ShutoffValve *CRViewSSelDpCBCV::_GetCurrentShutoffValveSelected()
{
	CDB_ShutoffValve *pclCurrentShutoffValveSelected = NULL;

	CSheetDescription *pclSheetDescriptionSV = m_ViewDescription.GetFromSheetDescriptionID( SD_ShutoffValve );

	if( NULL != pclSheetDescriptionSV )
	{
		// Retrieve the current selected shut-off valve.
		CCellDescriptionProduct *pclCDCurrentShutoffValveSelected = NULL;
		LPARAM lpPointer = NULL;

		if( true == pclSheetDescriptionSV->GetUserVariable( _SDUV_SELECTEDPRODUCT, lpPointer ) )
		{
			pclCDCurrentShutoffValveSelected = (CCellDescriptionProduct *)lpPointer;
		}

		if( NULL != pclCDCurrentShutoffValveSelected && NULL != pclCDCurrentShutoffValveSelected->GetProduct() )
		{
			pclCurrentShutoffValveSelected = dynamic_cast<CDB_ShutoffValve*>( (CData *)pclCDCurrentShutoffValveSelected->GetProduct() );
		}
	}

	return pclCurrentShutoffValveSelected;
}

void CRViewSSelDpCBCV::_SetCurrentShutoffValveSelected( CCellDescriptionProduct *pclCDCurrentShutoffValveSelected )
{
	// Try to retrieve sheet description linked to shut-off valve.
	CSheetDescription *pclSheetDescriptionSV = m_ViewDescription.GetFromSheetDescriptionID( SD_ShutoffValve );

	if( NULL != pclSheetDescriptionSV )
	{
		pclSheetDescriptionSV->SetUserVariable( _SDUV_SELECTEDPRODUCT, ( LPARAM )pclCDCurrentShutoffValveSelected );
	}
}

long CRViewSSelDpCBCV::_GetRowOfEditedSV( CSheetDescription *pclSheetDescriptionSV, CDB_TAProduct *pEditedTAP )
{
	if( NULL == pclSheetDescriptionSV || NULL == pEditedTAP )
	{
		return -1;
	}

	// Retrieve list of all products in 'pclSheetDescription'.
	CSheetDescription::vecCellDescription vecCellDescriptionList;
	pclSheetDescriptionSV->GetCellDescriptionList( vecCellDescriptionList, RVSCellDescription::CD_Product );

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

void CRViewSSelDpCBCV::_ReadAllColumnWidth( CDS_IndSelParameter *pclIndSelParameter )
{
	// Access to the 'RViewSSelDpCBCV' group.
	CDS_IndSelParameter::CCWWindow *pclCWWindow = pclIndSelParameter->GetWindowColumnWidth( CW_WINDOWID_INDSELDPCBCV, true );

	// Access to the 'Shutoff valve' group.
	ResetColumnWidth( SD_ShutoffValve );
	CDS_IndSelParameter::CCWSheet *pclCWSheet = pclCWWindow->GetCWSheet( CW_RVIEWSSELDPCBCV_SHEETID_SHUTOFFVALVE );

	if( NULL != pclCWSheet && m_mapSSheetColumnWidth[SD_ShutoffValve].size() == pclCWSheet->GetMap().size() )
	{
		short nVersion = pclCWSheet->GetVersion();

		if( nVersion == CW_RVIEWSSELDPCBCV_SHUTOFFVALVE_VERSION )
		{
			m_mapSSheetColumnWidth[SD_ShutoffValve] = pclCWSheet->GetMap();
		}
	}
}

void CRViewSSelDpCBCV::_WriteAllColumnWidth( CDS_IndSelParameter *pclIndSelParameter )
{
	// Access to the 'RViewSSelDpCBCV' group.
	CDS_IndSelParameter::CCWWindow *pclCWWindow = pclIndSelParameter->GetWindowColumnWidth( CW_WINDOWID_INDSELDPCBCV, true );

	// Write the 'Shutoff valve' group.
	CDS_IndSelParameter::CCWSheet *pclCWSheet = pclCWWindow->GetCWSheet( CW_RVIEWSSELDPCBCV_SHEETID_SHUTOFFVALVE, true );
	pclCWSheet->SetVersion( CW_RVIEWSSELDPCBCV_SHUTOFFVALVE_VERSION );
	pclCWSheet->GetMap() = m_mapSSheetColumnWidth[SD_ShutoffValve];
}
