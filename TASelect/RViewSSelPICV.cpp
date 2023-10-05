#include "stdafx.h"
#include "afxctl.h"

#include "TASelect.h"
#include "MainFrm.h"
#include "Hydronic.h"

#include "DlgLeftTabSelManager.h"

#include "ProductSelectionParameters.h"
#include "RViewSSelCtrl.h"
#include "RViewSSelPICV.h"

CRViewSSelPICv *pRViewSSelPICv = NULL;

CRViewSSelPICv::CRViewSSelPICv() : CRViewSSelCtrl( CMainFrame::RightViewList::eRVSSelPICv )
{
	m_pclIndSelPIBCVParams = NULL;
	m_pclAdapterRuledTable = NULL;
	pRViewSSelPICv = this;
}

CRViewSSelPICv::~CRViewSSelPICv()
{
	pRViewSSelPICv = NULL;
}

void CRViewSSelPICv::Suggest( CProductSelelectionParameters *pclProductSelectionParameters, LPARAM lpParam )
{
	if( NULL == pclProductSelectionParameters || NULL == dynamic_cast<CIndSelPIBCVParams*>( pclProductSelectionParameters ) )
	{
		return;
	}

	m_pclIndSelPIBCVParams = dynamic_cast<CIndSelPIBCVParams*>( pclProductSelectionParameters );

	CRViewSSelCtrl::Suggest( pclProductSelectionParameters, lpParam );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PROTECTED MEMBERS
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CRViewSSelPICv::OnTextTipFetch( CSheetDescription *pclSheetDescription, long lColumn, long lRow, WORD *pwMultiLine, SHORT *pnTipWidth, 
		TCHAR *pstrTipText, BOOL *pbShowTip )
{
	if( NULL == m_pclIndSelPIBCVParams || NULL == m_pclIndSelPIBCVParams->m_pclSelectPIBCVList || false == m_bInitialised || 
			NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() )
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
	CDB_PIControlValve *pPICv = dynamic_cast<CDB_PIControlValve *>( pTAP );
	CSelectedValve *pSelectedTAP = GetSelectProduct<CSelectedValve>( pTAP, m_pclIndSelPIBCVParams->m_pclSelectPIBCVList );
	CSSheet *pclSSheet = pclSheetDescription->GetSSheetPointer();
	
	if( NULL != pSelectedTAP && SD_ControlValve == pclSheetDescription->GetSheetDescriptionID() )
	{
		if( CD_ControlValve_Preset == lColumn )
		{
			// Check what is the color of the text.
			if( NULL != pclSSheet && _ORANGE == pclSSheet->GetForeColor( lColumn, lRow ) )
			{
				CDB_PICVCharacteristic *pclPIBCValveCharacteristic = pPICv->GetPICVCharacteristic();
				
				if( NULL != pclPIBCValveCharacteristic )
				{
					double dMinRecommendedSetting = pclPIBCValveCharacteristic->GetMinRecSetting();
					double dMinMeasurableSetting = pclPIBCValveCharacteristic->GetMinMeasSetting();

					if( pSelectedTAP->GetH() < dMinRecommendedSetting )
					{
						// HYS-1355: We can have a stroke curve defined.
						if( true == pclPIBCValveCharacteristic->IsStrokeCurveDefined() && ( pSelectedTAP->GetH() < dMinMeasurableSetting ) )
						{
							str = TASApp.LoadLocalizedString( IDS_SSHEETSSEL_SETTINGERROR_TASLIDER );
						}
						else
						{
							CString str1;
							CString str2 = pclPIBCValveCharacteristic->GetSettingString( pclPIBCValveCharacteristic->GetMinRecSetting() );
							str1.Format( _T("%s < %s"), (LPCTSTR)pclSSheet->GetCellText( CD_ControlValve_Preset, lRow ), (LPCTSTR)str2 );
							FormatString( str, IDS_SSHEETSSEL_SETTINGERROR, str1 );
						}
					}
				}
			}
		}
		else if( CD_ControlValve_DpMax == lColumn )
		{
			if( m_pclIndSelPIBCVParams->m_dDpMax > pPICv->GetDpmax() )
			{
				CString str2 = WriteCUDouble( _U_DIFFPRESS, m_pclIndSelPIBCVParams->m_dDpMax, true, 3, 0 );
				CString str3 = WriteCUDouble( _U_DIFFPRESS, pPICv->GetDpmax(), true, 3, 0 );
				FormatString( str, IDS_PRODUCTSELECTION_ERROR_DPMAX, str2, str3 );
			}
		}

		if( true == str.IsEmpty() && NULL != pPICv )
		{
			if( 0 != _tcslen( pPICv->GetComment() ) )
			{
				str = pPICv->GetComment();
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

bool CRViewSSelPICv::IsActuatorFit( CDB_ElectroActuator *pclElectroActuator, CDB_ControlValve *pclSelectedControlValve, 
		bool bDowngradeActuatorFunctionality, bool bAcceptAllFailSafe )
{
	if( NULL == m_pclIndSelPIBCVParams || NULL == pclElectroActuator || NULL == pclSelectedControlValve )
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
		ComboDRPFctChoice = m_pclIndSelPIBCVParams->m_eActuatorDRPFunction;
	}
	else
	{
		ComboDRPFctChoice = CDB_ControlValve::DRPFunction::drpfAll;
	}

	return pclElectroActuator->IsActuatorFit(	m_pclIndSelPIBCVParams->m_strActuatorPowerSupplyID, m_pclIndSelPIBCVParams->m_strActuatorInputSignalID, m_pclIndSelPIBCVParams->m_iActuatorFailSafeFunction,
		ComboDRPFctChoice, CurrentActuatorDRPFct, m_pclIndSelPIBCVParams->m_eCvCtrlType,  bDowngradeActuatorFunctionality );
}

long CRViewSSelPICv::FillControlValveRows( CDB_ControlValve *pEditedControlValve )
{
	if( NULL == m_pclIndSelPIBCVParams || NULL == m_pclIndSelPIBCVParams->m_pclSelectPIBCVList 
			|| NULL == m_pclIndSelPIBCVParams->m_pclSelectPIBCVList->GetSelectPipeList() || NULL == m_pclIndSelPIBCVParams->m_pTADS
			|| NULL == m_pclIndSelPIBCVParams->m_pPipeDB )
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
	
	CSSheet *pclSSheet = pclSheetDescriptionPICv->GetSSheetPointer();

	// Hide unused columns.
	pclSSheet->ShowCol( CD_ControlValve_Kvs, FALSE );
	pclSSheet->ShowCol( CD_ControlValve_DpFullOpening, FALSE );
	pclSSheet->ShowCol( CD_ControlValve_DpHalfOpening, FALSE );
	pclSSheet->ShowCol( CD_ControlValve_DplRange, FALSE );

	if( false == m_pclIndSelPIBCVParams->m_bIsDpMaxChecked )
	{
		pclSSheet->ShowCol( CD_ControlValve_DpMax, FALSE );
	}

	// Variables.
	CSelectPipe selPipe( m_pclIndSelPIBCVParams );
	CDS_TechnicalParameter *pTechParam = m_pclIndSelPIBCVParams->m_pTADS->GetpTechParams();

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

	for( CSelectedValve *pclSelectedPICv = m_pclIndSelPIBCVParams->m_pclSelectPIBCVList->GetFirst<CSelectedValve>(); NULL != pclSelectedPICv; 
			pclSelectedPICv = m_pclIndSelPIBCVParams->m_pclSelectPIBCVList->GetNext<CSelectedValve>() )
	{
		// In case it is a pressure independent control valve.
		CDB_PIControlValve *pPICv = dynamic_cast<CDB_PIControlValve *>( pclSelectedPICv->GetpData() );

		if( NULL == pPICv )
		{
			continue;
		}

		bAtLeastOneError = false;
		bAtLeastOneWarning = false;
		bool bBest = pclSelectedPICv->IsFlagSet(CSelectedBase::eBest );
		
		if( true == pclSelectedPICv->IsFlagSet( CSelectedBase::eNotPriority ) )
		{
			lValveNotPriorityCount++;
			
			if( false == bShowAllPrioritiesShown )
			{
				// Add one empty not selectable row.
				pclSheetDescriptionPICv->AddRows( 1 );
				pclSheetDescriptionPICv->RemoveSelectableRow( lRow );

				pclSSheet->SetCellBorder( CD_ControlValve_CheckBox, lRow, CD_ControlValve_Separator - 1, lRow, true, 
						SS_BORDERTYPE_TOP | SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );

				pclSSheet->SetCellBorder( CD_ControlValve_PipeSize, lRow, CD_ControlValve_Pointer - 1, lRow, true, 
						SS_BORDERTYPE_TOP | SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );

				lShowAllPrioritiesButtonRow = lRow++;
				bShowAllPrioritiesShown = true;
			}
		}

		// Add this row.
		// 'true' to specify that this row can be selected.
		pclSheetDescriptionPICv->AddRows( 1, true );

		// First columns will be set at the end!

		// Add checkbox.
		pclSSheet->SetCheckBox( CD_ControlValve_CheckBox, lRow, _T(""), false, true );

		// Add the picture for a PICVAct-Set.
		if( true == m_pclIndSelPIBCVParams->m_bOnlyForSet && true == pPICv->IsPartOfaSet() )
		{
			CCDButtonBox *pCDButtonBox = CreateCellDescriptionBox( CD_ControlValve_Box, lRow, true, CCDButtonBox::ButtonState::BoxOpened, pclSheetDescriptionPICv );

			if( NULL != pCDButtonBox )
			{
				pCDButtonBox->ApplyInternalChange();
			}

			bAtLeastOnePICVPartOfASet = true;
		}
		
		// Set the presetting first to determine if the red color must be used.
		CString str = L"-";
		double dPresetting = pclSelectedPICv->GetH();
		
		// Remark: to access valve characteristic of the PICV valve, don't call the base class method 'GetValveCharacteristic'.
		//         For 'CDB_PIControlValve', 'CDB_ThermostaticValve' and 'CDB_FlowLimitedControlValve', valve characteristics are now set in respective 
		//         'CDB_PICVCharacteristic', 
		CDB_ValveCharacteristic *pValvChar = (CDB_ValveCharacteristic *)pPICv->GetValveCharDataPointer();
		
		if( NULL != pValvChar && -1.0 != dPresetting )
		{
			str = pValvChar->GetSettingString( dPresetting );
		}

		// If full opening indicator is set after the opening...
		if( -1 != str.Find( _T('*') ) )
		{
			str.Replace( _T('*'), _T(' ') );
			pclSheetDescriptionPICv->WriteTextWithFlags( str, CD_ControlValve_Preset, lRow, CSheetDescription::RemarkFlags::FullOpening );
		}
		else
		{
			if( NULL != pValvChar && -1.0 != dPresetting && dPresetting < pValvChar->GetMinRecSetting() )
			{
				pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_ORANGE );
				bAtLeastOneWarning = true;
			}

			pclSSheet->SetStaticText( CD_ControlValve_Preset, lRow, str );
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );
		}

		// Set the rangeability.
		pclSSheet->SetStaticText( CD_ControlValve_Rangeability, lRow, pPICv->GetStrRangeability().c_str() );

		// Set the leakage rate.
		pclSSheet->SetStaticText( CD_ControlValve_LeakageRate, lRow, WriteCUDouble( _U_NODIM, pPICv->GetLeakageRate() * 100 ) );
				
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

		// Set the Dp min value.
		// HYS-825: Dp min from pPICv instead of pclSelectedPICv
		double dDpMin = pPICv->GetDpmin( m_pclIndSelPIBCVParams->m_dFlow, m_pclIndSelPIBCVParams->m_WC.GetDens() );
		pclSSheet->SetStaticText( CD_ControlValve_Dp, lRow, WriteCUDouble( _U_DIFFPRESS, dDpMin ) );

		// Set the stroke value.
		CDB_CloseOffChar *pCloseOffChar = (CDB_CloseOffChar *)pPICv->GetCloseOffCharIDPtr().MP;
		
		if( NULL != pCloseOffChar )
		{
			if( CDB_CloseOffChar::eOpenType::Linear == pCloseOffChar->GetOpenType() )
			{
				pclSSheet->SetStaticText( CD_ControlValve_Stroke, lRow, WriteCUDouble( _U_DIAMETER, pPICv->GetStroke() ) );
			}
			else
			{
				pclSSheet->SetStaticText( CD_ControlValve_Stroke, lRow, WriteDouble( pPICv->GetStroke(), 0, 0) );
			}
		}
		else
		{
			pclSSheet->SetStaticText( CD_ControlValve_Stroke, lRow, WriteCUDouble( _U_DIAMETER, pPICv->GetStroke() ) ); 
		}

		// Set the characteristic picture.
		CDB_ControlProperties::eCTRLCHAR CtrlChar = pPICv->GetCtrlProp()->GetCvCtrlChar();
		
		if( CDB_ControlProperties::Linear == CtrlChar )
		{
			// Verify the special case of a KTH family product.
			// If it is, add an information to explain that that kind of valve is linear but can works for 3-Points and 
			// proportional control.
			if( 0 == IDcmp( pPICv->GetFamilyID(), _T("FAM_KTH512") ) )
			{
				pclSSheet->SetPictureCellWithID( IDI_CHARACTLINEXCLAM, CD_ControlValve_ImgCharacteristic, lRow, CSSheet::PictureCellType::Icon );
				bRemarkInfoKTH = true;
			}
			else
			{
				pclSSheet->SetPictureCellWithID( IDI_CHARACTLIN, CD_ControlValve_ImgCharacteristic, lRow, CSSheet::PictureCellType::Icon );
			}
		}
		else if( CDB_ControlProperties::EqualPc == CtrlChar )
		{
			pclSSheet->SetPictureCellWithID( IDI_CHARACTEQM, CD_ControlValve_ImgCharacteristic, lRow, CSSheet::PictureCellType::Icon );
		}
		else
		{
			pclSSheet->SetPictureCellWithID( IDI_CHARACTNONE, CD_ControlValve_ImgCharacteristic, lRow, CSSheet::PictureCellType::Icon );
		}

		// Set the push to open/close picture.
		IDPTR IDPtr = pPICv->GetTypeIDPtr();
		CDB_ControlProperties::ePushOrPullToClose PushClose = pPICv->GetCtrlProp()->GetCvPushClose();
		
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

		// Set Dp max.
		if( true == m_pclIndSelPIBCVParams->m_bIsDpMaxChecked )
		{
			if( pPICv->GetDpmax() < m_pclIndSelPIBCVParams->m_dDpMax )
			{
				pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_RED );
				bAtLeastOneError = true;
			}

			pclSSheet->SetStaticText( CD_ControlValve_DpMax, lRow, WriteDouble( CDimValue::SItoCU( _U_DIFFPRESS, pPICv->GetDpmax() ), 3, 0 ) );
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );
		}

		// Set the temperature range.
		if( true == pclSelectedPICv->IsFlagSet( CSelectedBase::eTemperature ) )
		{
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_RED );
			bAtLeastOneError = true;
		}

		pclSSheet->SetStaticText( CD_ControlValve_TemperatureRange, lRow, ( (CDB_TAProduct *)( pclSelectedPICv->GetProductIDPtr().MP ) )->GetTempRange() );
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

		// Now we can set first columns in regards to current status (error, best or normal).
		if( true == bAtLeastOneError )
		{
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_RED );
		}
		else if( true == pclSelectedPICv->IsFlagSet( CSelectedBase::eBest ) )
		{
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_GREEN );
		}
		
		// In addition of current flag, we have possibility that valve has the flag 'Not available' or 'Deleted'. In that case, we show valve name in red with "*" around it and
		// symbol '!' or '!!' after.
		if( true == pPICv->IsDeleted() )
		{
			pclSheetDescriptionPICv->WriteTextWithFlags( CString( pPICv->GetName() ), CD_ControlValve_Name, lRow, 
					CSheetDescription::RemarkFlags::Deleted, _T(""), _RED );
		}
		else if( false == pPICv->IsAvailable() )
		{
			pclSheetDescriptionPICv->WriteTextWithFlags( CString( pPICv->GetName() ), CD_ControlValve_Name, lRow, 
					CSheetDescription::RemarkFlags::NotAvailable, _T(""), _RED );
		}
		else
		{
			pclSSheet->SetStaticText( CD_ControlValve_Name, lRow, pPICv->GetName() );
		}
		
		if( true == bAtLeastOneError )
		{
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );
		}
		else if( true == pclSelectedPICv->IsFlagSet( CSelectedBase::eBest ) )
		{
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_GREEN );
		}
		
		pclSSheet->SetStaticText( CD_ControlValve_Material, lRow, pPICv->GetBodyMaterial() );
		pclSSheet->SetStaticText( CD_ControlValve_Connection, lRow, pPICv->GetConnect() );
		pclSSheet->SetStaticText( CD_ControlValve_Version, lRow, pPICv->GetVersion() );
		pclSSheet->SetStaticText( CD_ControlValve_PN, lRow, pPICv->GetPN().c_str() );
		pclSSheet->SetStaticText( CD_ControlValve_Size, lRow, pPICv->GetSize() );
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

		pclSSheet->SetStaticText( CD_ControlValve_Separator, lRow );

		m_pclIndSelPIBCVParams->m_pclSelectPIBCVList->GetSelectPipeList()->GetMatchingPipe(pPICv->GetSizeKey(),selPipe);
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
		CCellDescriptionProduct *pclCDProduct = CreateCellDescriptionProduct( pclSheetDescriptionPICv->GetFirstParameterColumn(), lRow, (LPARAM)pPICv, pclSheetDescriptionPICv );
		pclCDProduct->SetUserParam( (LPARAM)pclSelectedPICv );
		
		if( NULL != pEditedControlValve && pEditedControlValve == pPICv )
		{
			lRetRow = lRow;
		}

		pclSSheet->SetCellBorder( CD_ControlValve_CheckBox, lRow, CD_ControlValve_Separator- 1, lRow, true, SS_BORDERTYPE_BOTTOM,
				SS_BORDERSTYLE_SOLID, _GRAY );

		pclSSheet->SetCellBorder( CD_ControlValve_PipeSize, lRow, CD_ControlValve_Pointer - 1, lRow, true, SS_BORDERTYPE_BOTTOM,
				SS_BORDERSTYLE_SOLID, _GRAY );

		lRow++;
		lValveTotalCount++;
	}

	long lLastDataRow = lRow - 1;

	// If we are not in selection by package mode OR if there is no valve that belongs to a set...
	if( false == m_pclIndSelPIBCVParams->m_bOnlyForSet || false == bAtLeastOnePICVPartOfASet )
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

		CCDButtonShowAllPriorities* pclShowAllButton = CreateShowAllPrioritiesButton( CD_ControlValve_FirstColumn, lShowAllPrioritiesButtonRow, 
				bShowAllPrioritiesShown,  eState, lShowAllPrioritiesButtonRow + 1, pclSSheet->GetMaxRows(), pclSheetDescriptionPICv );

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
	pclSSheet->SetStaticText( CD_ControlValve_CheckBox, RD_ControlValve_GroupName, IDS_SSHEETSSELCV_PICVGROUP );
	pclSSheet->AddCellSpanW( CD_ControlValve_PipeSize, RD_ControlValve_GroupName, CD_ControlValve_Pointer - CD_ControlValve_PipeSize, 1 );
	
	CTable *pclTable = dynamic_cast<CTable *>( m_pclIndSelPIBCVParams->m_pPipeDB->Get( m_pclIndSelPIBCVParams->m_strPipeSeriesID ).MP );
	ASSERT( NULL != pclTable );

	pclSSheet->SetStaticText( CD_ControlValve_PipeSize, RD_ControlValve_GroupName, pclTable->GetName() );
	
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

	// Add possibility to change column size.
	// Remark: place this code after any initialization (like adding, removing, resizing, hiding/showing columns).
	CResizingColumnInfo* pclResizingColumnInfo = pclSheetDescriptionPICv->GetResizingColumnInfo();
	pclResizingColumnInfo->ActivateFeature( true, false, RD_ControlValve_FirstAvailRow, lLastDataRow );
	pclResizingColumnInfo->AddRangeColumn( CD_ControlValve_Name, CD_ControlValve_TemperatureRange, RD_ControlValve_ColName, RD_ControlValve_Unit );
	pclResizingColumnInfo->AddRangeColumn( CD_ControlValve_PipeSize, CD_ControlValve_PipeV, RD_ControlValve_ColName, RD_ControlValve_Unit );

	// Add possibility to click on title group to reset column width.
	CreateTitleGroupButtons( CD_ControlValve_CheckBox, CD_ControlValve_PipeV, RD_ControlValve_GroupName, pclSheetDescriptionPICv );

	return lRetRow;
}

void CRViewSSelPICv::GetAdapterList( CDB_ControlValve *pclSelectedControlValve, CRank *pclAdapterList, CDB_RuledTableBase **ppclRuledTable )
{
	if( NULL == m_pclIndSelPIBCVParams || NULL == m_pclIndSelPIBCVParams->m_pTADB )
	{
		ASSERT_RETURN;
	}

	m_pclIndSelPIBCVParams->m_pTADB->GetAdapterList( pclSelectedControlValve, pclAdapterList, ppclRuledTable, m_bDowngradeActuatorFunctionality,
			m_pclIndSelPIBCVParams->m_strActuatorPowerSupplyID, m_pclIndSelPIBCVParams->m_strActuatorInputSignalID, m_pclIndSelPIBCVParams->m_iActuatorFailSafeFunction,
			m_pclIndSelPIBCVParams->m_eActuatorDRPFunction,  m_pclIndSelPIBCVParams->m_eCvCtrlType, m_pclIndSelPIBCVParams->m_bOnlyForSet, 
			m_pclIndSelPIBCVParams->m_eFilterSelection );
}

void CRViewSSelPICv::GetSetAccessoryList( CDB_ControlValve *pclSelectedControlValve, CDB_Actuator *pclSeletedActuator, CRank *pclSetAccessoryList, 
		CDB_RuledTableBase **ppclRuledTable )
{
	if( NULL == m_pclIndSelPIBCVParams || NULL == m_pclIndSelPIBCVParams->m_pTADB )
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
			m_pclIndSelPIBCVParams->m_pTADB->GetAccessories( pclSetAccessoryList, *ppclRuledTable, m_pclIndSelPIBCVParams->m_eFilterSelection );
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PRIVATE MEMBERS
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

long CRViewSSelPICv::_SetRemarksKTH( CSheetDescription *pclSheetDescriptionPICv, long lRow )
{
	if( NULL == m_pclIndSelPIBCVParams || NULL == pclSheetDescriptionPICv || NULL == pclSheetDescriptionPICv->GetSSheetPointer() )
	{
		return lRow;
	}
	
	CSSheet *pclSSheet = pclSheetDescriptionPICv->GetSSheetPointer();
	
	CString str = _T("! ");
	
	// Add remark for KTH family.
	if( CDB_ControlProperties::eCvProportional == m_pclIndSelPIBCVParams->m_eCvCtrlType )
	{
		str += TASApp.LoadLocalizedString( IDS_INFOKTHPROP );
	}
	else if( CDB_ControlProperties::eCv3point == m_pclIndSelPIBCVParams->m_eCvCtrlType )
	{
		str += TASApp.LoadLocalizedString( IDS_INFOKTH3P );
	}
	else if( CDB_ControlProperties::eCvOnOff == m_pclIndSelPIBCVParams->m_eCvCtrlType )
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
