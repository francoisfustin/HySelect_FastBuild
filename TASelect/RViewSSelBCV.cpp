#include "stdafx.h"
#include "afxctl.h"

#include "TASelect.h"
#include "MainFrm.h"
#include "Hydronic.h"
#include "DlgLeftTabSelManager.h"

#include "ProductSelectionParameters.h"
#include "RViewSSelCtrl.h"
#include "RViewSSelBCV.h"

CRViewSSelBCv *pRViewSSelBCv = NULL;

CRViewSSelBCv::CRViewSSelBCv() : CRViewSSelCtrl( CMainFrame::RightViewList::eRVSSelBCv )
{
	m_pclIndSelBCVParams = NULL;
	m_pclAdapterRuledTable = NULL;
	pRViewSSelBCv = this;
}

CRViewSSelBCv::~CRViewSSelBCv()
{
	pRViewSSelBCv = NULL;
}

void CRViewSSelBCv::Suggest( CProductSelelectionParameters *pclProductSelectionParameters, LPARAM lpParam )
{
	if( NULL == pclProductSelectionParameters || NULL == dynamic_cast<CIndSelBCVParams*>( pclProductSelectionParameters ) )
	{
		return;
	}

	m_pclIndSelBCVParams = dynamic_cast<CIndSelBCVParams*>( pclProductSelectionParameters );

	CRViewSSelCtrl::Suggest( pclProductSelectionParameters, lpParam );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PROTECTED MEMBERS
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CRViewSSelBCv::OnTextTipFetch( CSheetDescription *pclSheetDescription, long lColumn, long lRow, WORD *pwMultiLine, SHORT *pnTipWidth, 
		TCHAR *pstrTipText, BOOL *pfShowTip )
{
	if( NULL == m_pclIndSelBCVParams || false == m_bInitialised || NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() )
	{
		ASSERTA_RETURN( false );
	}

	// First, we must call base class.
	// Remark: if base class has displayed a tooltip, it's not necessary to go further in this method.
	if( true == CRViewSSelCtrl::OnTextTipFetch( pclSheetDescription, lColumn, lRow, pwMultiLine, pnTipWidth, pstrTipText, pfShowTip ) )
	{
		return true;
	}

	CDB_TAProduct *pTAP = dynamic_cast<CDB_TAProduct *>( (CData *)GetCDProduct( lColumn, lRow, pclSheetDescription ) );
	
	if( NULL == pTAP )
	{
		return false;
	}

	CSelectedValve *pclSelectedBCV = GetSelectProduct<CSelectedValve>( pTAP, m_pclIndSelBCVParams->m_pclSelectBCVList );

	if( NULL == pclSelectedBCV )
	{
		return false;
	}

	CString str;
	bool fReturnValue = false;
	*pfShowTip = false;
	*pwMultiLine = SS_TT_MULTILINE_AUTO;
	
	CDB_ControlValve *pBCV = dynamic_cast<CDB_ControlValve *>( pTAP );
	CSSheet *pclSSheet = pclSheetDescription->GetSSheetPointer();
	CDS_TechnicalParameter *pclTechParam = m_pclIndSelBCVParams->m_pTADS->GetpTechParams();
	
	if( SD_ControlValve == pclSheetDescription->GetSheetDescriptionID() )
	{
		if( CD_ControlValve_Preset == lColumn )
		{
			// Check if flag is set.
			CDB_ValveCharacteristic *pValveCharacteristic = pBCV->GetValveCharacteristic();

			if( true == pclSelectedBCV->IsFlagSet( CSelectedBase::eValveSetting ) && NULL != pValveCharacteristic )
			{
				CString str2 = pValveCharacteristic->GetSettingString( pclSelectedBCV->GetH() );
	
				double dMinRecommendedSetting = pValveCharacteristic->GetMinRecSetting();
				str2 += _T(" < ") + pValveCharacteristic->GetSettingString( dMinRecommendedSetting );
				FormatString( str, IDS_SSHEETSSEL_SETTINGERROR, str2 );
			}
		}
		else if( CD_ControlValve_Dp == lColumn )
		{
			// Check if flag is set.
			if( true == pclSelectedBCV->IsFlagSet( CSelectedBase::eDp ) )
			{
				CString str2 = WriteCUDouble( _U_DIFFPRESS, pclSelectedBCV->GetDp() );
				
				if( pclSelectedBCV->GetDp() < pclTechParam->GetValvMinDp( pTAP->GetTypeIDPtr().ID ) )
				{
					str2 += CString( _T(" < ") ) + WriteCUDouble( _U_DIFFPRESS, pclTechParam->GetValvMinDp( pTAP->GetTypeIDPtr().ID ), true );

					// "Dp is below the minimum value defined in technical parameters ( %1 )"
					FormatString( str, IDS_SSHEETSSEL_DPERRORL, str2 );
				}
				else if( pclSelectedBCV->GetDp() > pclTechParam->VerifyValvMaxDp( pTAP ) ) 
				{
					// Either the max Dp is defined for the valve or we take the max Dp defined in the technical parameters.
					// This is why the message is different in regards to these both cases.
					double dDpMax = pBCV->GetDpmax();

					// IDS_SSHEETSSEL_DPERRORH: "Dp is above the maximum value defined in technical parameters ( %1 )"
					// IDS_SSHEETSSEL_DPERRORH2: "Dp is above the maximum value defined for this valve ( %1 )"
					int iMsgID = ( dDpMax <= 0.0 ) ? IDS_SSHEETSSEL_DPERRORH : IDS_SSHEETSSEL_DPERRORH2;

					str2 += CString( _T(" > ") ) + WriteCUDouble( _U_DIFFPRESS, pclTechParam->VerifyValvMaxDp( pTAP ), true );
					FormatString( str, iMsgID, str2 );
				}
				else
				{
					// User has entered a pressure drop but valve fully opened is already above this value.
					str2 += CString( _T(" > ") ) + WriteCUDouble( _U_DIFFPRESS, m_pclIndSelBCVParams->m_dDp, true );

					// "Pressure drop on valve fully open is already higher than requested Dp value ( %1 )"
					FormatString( str, IDS_SSHEETSSEL_DPERRORNOTF, str2 );
				}
			}
		}
		else if( CD_ControlValve_DpFullOpening == lColumn )
		{
			// Check if flag is set.
			if( true == pclSelectedBCV->IsFlagSet( CSelectedBase::eValveFullODp ) )
			{
				if( pclSelectedBCV->GetDpFullOpen() < pclTechParam->GetValvMinDp() )
				{
					CString str2 = WriteCUDouble( _U_DIFFPRESS, pclSelectedBCV->GetDpFullOpen() );
					str2 += CString( _T(" < ") ) + WriteCUDouble( _U_DIFFPRESS, pclTechParam->GetValvMinDp(), true );

					// "Pressure drop on valve fully open is below the minimum value defined in technical parameters ( %1 )"
					FormatString( str, IDS_SSHEETSSEL_DPFOERROR, str2 );
				}
			}
		}
		else if( CD_ControlValve_DpHalfOpening == lColumn )
		{
			// Check if flag is set.
			if( true == pclSelectedBCV->IsFlagSet( CSelectedBase::eValveHalfODp ) )
			{
				CString str2 = WriteCUDouble( _U_DIFFPRESS, pclSelectedBCV->GetDpHalfOpen() );

				if( pclSelectedBCV->GetDpHalfOpen() < pclTechParam->GetValvMinDp() )
				{
					str2 += CString( _T(" < ") ) + WriteCUDouble( _U_DIFFPRESS, pclTechParam->GetValvMinDp(), true );
					FormatString( str, IDS_SSHEETSSEL_DPHOERRORL, str2 );
				}
				// HYS-1283 : VerifyValvMaxDp give the Dp max of the valve
				else if( pclSelectedBCV->GetDpHalfOpen() > pclTechParam->VerifyValvMaxDp( pTAP ) )
				{
					str2 += CString( _T(" > ") ) + WriteCUDouble( _U_DIFFPRESS, pclTechParam->VerifyValvMaxDp( pTAP ), true );
					FormatString( str, IDS_SSHEETSSEL_DPHOERRORH2, str2 );
				}
			}
		}

		if( true == str.IsEmpty() )
		{
			if( 0 != _tcslen( pTAP->GetComment() ) )
			{
				str = pTAP->GetComment();
			}
		}
	}

	if( false == str.IsEmpty() )
	{
		*pnTipWidth = (SHORT)pclSSheet->GetTipTextWidth( str );
		wcsncpy_s( pstrTipText, SS_TT_TEXTMAX, (LPCTSTR)str, SS_TT_TEXTMAX );
		*pfShowTip = true;
		fReturnValue = true;
	}
	return fReturnValue;
}

bool CRViewSSelBCv::IsActuatorFit( CDB_ElectroActuator *pclElectroActuator, CDB_ControlValve *pclSelectedControlValve, bool bDowngradeActuatorFunctionality, bool bAcceptAllFailSafe )
{
	if( NULL == m_pclIndSelBCVParams || NULL == pclElectroActuator || NULL == pclSelectedControlValve )
	{
		ASSERTA_RETURN( false );
	}

	if( false == pclElectroActuator->IsSelectable( true ) )
	{
		return false;
	}
	
	CDB_ControlValve::DRPFunction CurrentActuatorDRPFct = pclSelectedControlValve->GetCompatibleDRPFunction( (int)pclElectroActuator->GetDefaultReturnPos() );
	CDB_ControlValve::DRPFunction ComboDRPFctChoice;

	if( false == bAcceptAllFailSafe )
	{
		ComboDRPFctChoice = m_pclIndSelBCVParams->m_eActuatorDRPFunction;
	}
	else
	{
		ComboDRPFctChoice = CDB_ControlValve::DRPFunction::drpfAll;
	}

	return pclElectroActuator->IsActuatorFit( m_pclIndSelBCVParams->m_strActuatorPowerSupplyID, m_pclIndSelBCVParams->m_strActuatorInputSignalID, m_pclIndSelBCVParams->m_iActuatorFailSafeFunction,
		ComboDRPFctChoice, CurrentActuatorDRPFct, m_pclIndSelBCVParams->m_eCvCtrlType, bDowngradeActuatorFunctionality );
}

long CRViewSSelBCv::FillControlValveRows( CDB_ControlValve *pEditedControlValve )
{
	if( NULL == m_pclIndSelBCVParams || NULL == m_pclIndSelBCVParams->m_pclSelectBCVList 
			|| NULL == m_pclIndSelBCVParams->m_pclSelectBCVList->GetSelectPipeList() || NULL == m_pclIndSelBCVParams->m_pTADS
			|| NULL == m_pclIndSelBCVParams->m_pPipeDB )
	{
		ASSERTA_RETURN( -1 );
	}

	// Call first the bass class to initialize sheet, header and so on.
	if( -1 == CRViewSSelCtrl::FillControlValveRows( pEditedControlValve ) )
	{
		return -1;
	}

	// Retrieve sheet description created in base class.
	CSheetDescription *pclSheetDescriptionBCV = m_ViewDescription.GetFromSheetDescriptionID( SD_ControlValve );

	if( NULL == pclSheetDescriptionBCV || NULL == pclSheetDescriptionBCV->GetSSheetPointer() )
	{
		return -1;
	}
	
	CSSheet *pclSSheet = pclSheetDescriptionBCV->GetSSheetPointer();

	// Hide unused columns.
	pclSSheet->ShowCol( CD_ControlValve_Kvs, FALSE );
	pclSSheet->ShowCol( CD_ControlValve_DpMax, FALSE );

	double dRho = m_pclIndSelBCVParams->m_WC.GetDens();
	double dKinVisc = m_pclIndSelBCVParams->m_WC.GetKinVisc();
	double dFlow = m_pclIndSelBCVParams->m_dFlow;

	CDS_TechnicalParameter *pTechParam = m_pclIndSelBCVParams->m_pTADS->GetpTechParams();
	
	long lRetRow = -1;
	long lRow = RD_ControlValve_FirstAvailRow;
	
	CSelectPipe selPipe( m_pclIndSelBCVParams );
	long lValveTotalCount = 0;
	long lValveNotPriorityCount = 0;
	bool fShowAllPrioritiesShown = false;
	long lShowAllPrioritiesButtonRow = 0;
	bool bAtLeastOneError = false;
	bool bAtLeastOneWarning = false;
	BOOL bShowColumnImgPushClose = FALSE;
	BOOL bShowColumnImgChar = FALSE;
	BOOL bShowColumnDpFullOpen = FALSE;

	pclSheetDescriptionBCV->SetUserVariable( _SDUV_TOTALPRODUCT_COUNT, 0 );
	pclSheetDescriptionBCV->SetUserVariable( _SDUV_NOPRIORITYPRODUCT_COUNT, 0 );
	
	bool bAtLeastOneBCVPartOfASet = false;
	pclSheetDescriptionBCV->RestartRemarkGenerator();
	
	for( CSelectedValve *pclSelectedValve = m_pclIndSelBCVParams->m_pclSelectBCVList->GetFirst<CSelectedValve>(); NULL != pclSelectedValve; pclSelectedValve = m_pclIndSelBCVParams->m_pclSelectBCVList->GetNext<CSelectedValve>() )
	{
		CDB_ControlValve *pclBalancingControlValve = dynamic_cast<CDB_ControlValve *>( pclSelectedValve->GetpData() );

		if( NULL == pclBalancingControlValve ) 
		{
			continue;
		}

		bAtLeastOneError = false;
		bAtLeastOneWarning = false;
		
		if( true == pclSelectedValve->IsFlagSet( CSelectedBase::eNotPriority ) )
		{
			lValveNotPriorityCount++;

			if( false == fShowAllPrioritiesShown )
			{
				// Add one empty not selectable row.
				pclSheetDescriptionBCV->AddRows( 1 );
				pclSheetDescriptionBCV->RemoveSelectableRow( lRow );

				pclSSheet->SetCellBorder( CD_ControlValve_CheckBox, lRow, CD_ControlValve_Separator - 1, lRow, true, SS_BORDERTYPE_TOP | SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );
				pclSSheet->SetCellBorder( CD_ControlValve_PipeSize, lRow, CD_ControlValve_Pointer - 1, lRow, true, SS_BORDERTYPE_TOP | SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );

				lShowAllPrioritiesButtonRow = lRow++;
				fShowAllPrioritiesShown = true;
			}
		}

		// Add this row.
		// 'true' to specify that this row can be selected.
		pclSheetDescriptionBCV->AddRows( 1, true );

		// First columns will be set at the end!

		// Add checkbox.
		pclSSheet->SetCheckBox(CD_ControlValve_CheckBox, lRow, _T(""), false, true);

		// Add the picture for a BCVAct-Set.
		if( true == m_pclIndSelBCVParams->m_bOnlyForSet && true == pclBalancingControlValve->IsPartOfaSet() )
		{
			CCDButtonBox *pCDButtonBox = CreateCellDescriptionBox( CD_ControlValve_Box, lRow, true, CCDButtonBox::ButtonState::BoxOpened, pclSheetDescriptionBCV );

			if( NULL != pCDButtonBox )
			{
				pCDButtonBox->ApplyInternalChange();
			}

			bAtLeastOneBCVPartOfASet = true;
		}

		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

		// Set the rangeability.
		pclSSheet->SetStaticText( CD_ControlValve_Rangeability, lRow, pclBalancingControlValve->GetStrRangeability().c_str() );

		// Set the leakage rate.
		pclSSheet->SetStaticText( CD_ControlValve_LeakageRate, lRow, WriteCUDouble( _U_NODIM, pclBalancingControlValve->GetLeakageRate() * 100.0 ) );

		pclSSheet->SetStaticText( CD_ControlValve_Stroke, lRow, pclBalancingControlValve->GetStrokeString() );

		// Set the characteristic picture.
		int iImageID = pclBalancingControlValve->GetCharacteristicImageID();
		pclSSheet->SetPictureCellWithID( iImageID, CD_ControlValve_ImgCharacteristic, lRow, CSSheet::PictureCellType::Icon );
		bShowColumnImgChar = ( IDI_CHARACTNONE == iImageID ) ? FALSE : TRUE;

		// Set the Push to open/close picture.
		iImageID = pclBalancingControlValve->GetPushPullCloseImageID();
		
		if( -1 != iImageID )
		{
			pclSSheet->SetPictureCellWithID( iImageID, CD_ControlValve_ImgPushClose, lRow, CSSheet::PictureCellType::Icon );
			bShowColumnImgPushClose = TRUE;
		}
		else
		{
			pclSSheet->ShowCol( CD_ControlValve_ImgSeparator, FALSE );
			pclSSheet->ShowCol( CD_ControlValve_ImgPushClose, FALSE );
		}

		// Set the hand wheel presetting.
		CString str = _T("-");
		CDB_ValveCharacteristic *pclValveCharacteristic = pclBalancingControlValve->GetValveCharacteristic();

		if( NULL != pclValveCharacteristic )
		{
			str = pclValveCharacteristic->GetSettingString( pclSelectedValve->GetH() );
		}

		if( true == pclSelectedValve->IsFlagSet( CSelectedBase::eValveMaxSetting ) )
		{
			pclSheetDescriptionBCV->WriteTextWithFlags( str, CD_ControlValve_Preset, lRow, CSheetDescription::RemarkFlags::FullOpening );
		}
		else
		{
			if( true == pclSelectedValve->IsFlagSet( CSelectedBase::eValveSetting ) )
			{
				pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_ORANGE );
				bAtLeastOneWarning = true;
			}
			
			pclSSheet->SetStaticText( CD_ControlValve_Preset, lRow, str );
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );
		}
		
		// Set Dp.
		if( true == pclSelectedValve->IsFlagSet( CSelectedBase::eDp ) )
		{
			if( pclSelectedValve->GetDp() > pTechParam->VerifyValvMaxDp( pclBalancingControlValve ) )
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
		
		pclSSheet->SetStaticText( CD_ControlValve_Dp, lRow, WriteCUDouble( _U_DIFFPRESS, pclSelectedValve->GetDp() ) );
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

		// If Dp unknown.
		if( false == m_pclIndSelBCVParams->m_bDpEnabled || -1.0 == m_pclIndSelBCVParams->m_dDp )
		{
			if( true == pclSelectedValve->IsFlagSet( CSelectedBase::eValveFullODp ) )
			{
				pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_ORANGE );
				bAtLeastOneWarning = true;
			}				
			
			pclSSheet->SetStaticText( CD_ControlValve_DpFullOpening, lRow, WriteCUDouble( _U_DIFFPRESS, pclSelectedValve->GetDpFullOpen() ) );
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

			if( true == pclSelectedValve->IsFlagSet( CSelectedBase::eValveHalfODp ) )
			{
				pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_ORANGE );
				bAtLeastOneWarning = true;
			}
			
			pclSSheet->SetStaticText( CD_ControlValve_DpHalfOpening, lRow, WriteCUDouble( _U_DIFFPRESS, pclSelectedValve->GetDpHalfOpen() ) );
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );
			
			bShowColumnDpFullOpen = TRUE;
		}
		else
		{	
			// Hide these two columns.
			pclSSheet->ShowCol( CD_ControlValve_DpFullOpening, FALSE );
			pclSSheet->ShowCol( CD_ControlValve_DpHalfOpening, FALSE );
		}

		// Temperature range.
		if( true == pclSelectedValve->IsFlagSet( CSelectedBase::eTemperature ) )
		{
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_RED );
			bAtLeastOneError = true;
		}
		
		pclSSheet->SetStaticText( CD_ControlValve_TemperatureRange, lRow, pclBalancingControlValve->GetTempRange() );
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

		pclSSheet->SetStaticText( CD_ControlValve_Name, lRow, pclBalancingControlValve->GetName() );

		if( true == bAtLeastOneError )
		{
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );
		}
		else if( true == pclSelectedValve->IsFlagSet( CSelectedBase::eBest ) )
		{
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_GREEN );
		}

		pclSSheet->SetStaticText( CD_ControlValve_Material, lRow, pclBalancingControlValve->GetBodyMaterial() );
		pclSSheet->SetStaticText( CD_ControlValve_Connection, lRow, pclBalancingControlValve->GetConnect() );
		pclSSheet->SetStaticText( CD_ControlValve_Version, lRow, pclBalancingControlValve->GetVersion() );
		pclSSheet->SetStaticText( CD_ControlValve_Size, lRow, pclBalancingControlValve->GetSize() );
		pclSSheet->SetStaticText( CD_ControlValve_PN, lRow, pclBalancingControlValve->GetPN().c_str() );

		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

		// Set pipe size.
		m_pclIndSelBCVParams->m_pclSelectBCVList->GetSelectPipeList()->GetMatchingPipe( pclBalancingControlValve->GetSizeKey(), selPipe );
		pclSSheet->SetStaticText( CD_ControlValve_PipeSize, lRow, selPipe.GetpPipe()->GetName() );
		
		// Set the LinDp to orange if it is above or below the technical parameters limits.
		if( selPipe.GetLinDp() > pTechParam->GetPipeMaxDp() || selPipe.GetLinDp() < pTechParam->GetPipeMinDp())
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
		CCellDescriptionProduct *pclCDProduct = CreateCellDescriptionProduct( pclSheetDescriptionBCV->GetFirstParameterColumn(), lRow, (LPARAM)pclBalancingControlValve, pclSheetDescriptionBCV );
		pclCDProduct->SetUserParam( (LPARAM)pclSelectedValve );
		
		if( NULL != pEditedControlValve && pEditedControlValve == pclBalancingControlValve )
		{
			lRetRow = lRow;
		}

		pclSSheet->SetCellBorder( CD_ControlValve_CheckBox, lRow, CD_ControlValve_Separator - 1, lRow, true, SS_BORDERTYPE_BOTTOM,SS_BORDERSTYLE_SOLID, _GRAY );
		pclSSheet->SetCellBorder( CD_ControlValve_PipeSize, lRow, CD_ControlValve_Pointer - 1, lRow, true, SS_BORDERTYPE_BOTTOM,SS_BORDERSTYLE_SOLID, _GRAY );

		lRow++;
		lValveTotalCount++;
	}

	long lLastDataRow = lRow - 1;

	// Hide some un-needed columns
	pclSSheet->ShowCol( CD_ControlValve_ImgCharacteristic, bShowColumnImgChar );

	pclSSheet->ShowCol( CD_ControlValve_ImgSeparator, bShowColumnImgPushClose && bShowColumnImgChar);
	pclSSheet->ShowCol( CD_ControlValve_ImgPushClose, bShowColumnImgPushClose );

	pclSSheet->ShowCol( CD_ControlValve_DpFullOpening, bShowColumnDpFullOpen );
	pclSSheet->ShowCol( CD_ControlValve_DpHalfOpening, bShowColumnDpFullOpen );
	pclSSheet->ShowCol( CD_ControlValve_DplRange, FALSE );

	// If we are not in selection by package mode OR if there is no valve that belongs to a set...
	if( false == m_pclIndSelBCVParams->m_bOnlyForSet || false == bAtLeastOneBCVPartOfASet )
	{
		pclSSheet->ShowCol( CD_ControlValve_Box, FALSE );
	}

	pclSheetDescriptionBCV->SetUserVariable( _SDUV_TOTALPRODUCT_COUNT, lValveTotalCount );
	pclSheetDescriptionBCV->SetUserVariable( _SDUV_NOPRIORITYPRODUCT_COUNT, lValveNotPriorityCount );

	// Now we can create Show/All priorities if needed.
	// Remarks: 2nd condition to avoid to have button if we have only no priority valves.
	if( true == fShowAllPrioritiesShown && lValveTotalCount > lValveNotPriorityCount )
	{
		// Create Show/Hide all priorities button in regards to current state!
		CCDButtonShowAllPriorities::ButtonState eState = ( true == GetShowAllPrioritiesFlag( SD_ControlValve ) ) ? 
				CCDButtonShowAllPriorities::ButtonState::HidePriorities : CCDButtonShowAllPriorities::ButtonState::ShowPriorities;

		CCDButtonShowAllPriorities *pclShowAllButton = CreateShowAllPrioritiesButton( CD_ControlValve_FirstColumn, lShowAllPrioritiesButtonRow, fShowAllPrioritiesShown, 
				eState, lShowAllPrioritiesButtonRow + 1, pclSSheet->GetMaxRows(), pclSheetDescriptionBCV );

		// Here we must call 'ApplyInternalChange' to show or hide rows in regards to the current state!
		if( NULL != pclShowAllButton )
		{
			pclShowAllButton->ApplyInternalChange( );
		}
	}
	else
	{
		if( 0 != lShowAllPrioritiesButtonRow )
		{
			pclSSheet->ShowRow( lShowAllPrioritiesButtonRow, FALSE );
		}
	}

	// Try to merge only if there is more than one BCV.
	if( lValveTotalCount > 2 || ( 2 == lValveTotalCount && lValveNotPriorityCount != 1 ) )
	{
		vector<long> vecColumnList;
		vecColumnList.push_back( CD_ControlValve_PipeSize );
		vecColumnList.push_back( CD_ControlValve_PipeLinDp );
		vecColumnList.push_back( CD_ControlValve_PipeV );
		pclSSheet->MergeRowsWithSameValues( RD_ControlValve_FirstAvailRow, lLastDataRow, vecColumnList );
	}
	
	pclSSheet->SetCellBorder( CD_ControlValve_CheckBox, lLastDataRow, CD_ControlValve_Separator - 1, lLastDataRow, true, SS_BORDERTYPE_BOTTOM,SS_BORDERSTYLE_SOLID, _BLACK );

	// Needed by pipe merging 
	pclSSheet->SetCellBorder( CD_ControlValve_PipeSize, lLastDataRow, CD_ControlValve_Pointer - 1, lLastDataRow, true, SS_BORDERTYPE_BOTTOM,SS_BORDERSTYLE_SOLID, _BLACK );

	// Add Remarks.
	// Note: these lines must be placed after creating Show/Hide all priorities button to avoid to take in count these lines when defining
	//       range of row to be Show/Hide.
	pclSheetDescriptionBCV->WriteRemarks( lRow, CD_ControlValve_CheckBox, CD_ControlValve_Separator );

	// Add the Main Title
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)GetTitleBackgroundColor( pclSheetDescriptionBCV ) );

	pclSheetDescriptionBCV->SetUserVariable( _SDUV_TITLEFORECOLOR, ( pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor ) ) );
	pclSheetDescriptionBCV->SetUserVariable( _SDUV_TITLEBACKCOLOR, ( pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor ) ) );

	pclSSheet->AddCellSpanW( CD_ControlValve_CheckBox, RD_ControlValve_GroupName, CD_ControlValve_Separator - CD_ControlValve_CheckBox, 1 );
	pclSSheet->SetStaticText(CD_ControlValve_CheckBox, RD_ControlValve_GroupName, IDS_SSHEETSSELBCV_BCVGROUP );
	pclSSheet->AddCellSpanW( CD_ControlValve_PipeSize, RD_ControlValve_GroupName, CD_ControlValve_Pointer - CD_ControlValve_PipeSize, 1 );
	
	CTable *pclTable = dynamic_cast<CTable *>( m_pclIndSelBCVParams->m_pPipeDB->Get( m_pclIndSelBCVParams->m_strPipeSeriesID ).MP );
	ASSERT( NULL != pclTable );

	pclSSheet->SetStaticText( CD_ControlValve_PipeSize, RD_ControlValve_GroupName, pclTable->GetName() );
	
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

	// Add possibility to change column size.
	// Remark: place this code after any initialization (like adding, removing, resizing, hiding/showing columns).
	CResizingColumnInfo* pclResizingColumnInfo = pclSheetDescriptionBCV->GetResizingColumnInfo();
	pclResizingColumnInfo->ActivateFeature( true, false, RD_ControlValve_FirstAvailRow, lLastDataRow );
	pclResizingColumnInfo->AddRangeColumn( CD_ControlValve_Name, CD_ControlValve_TemperatureRange, RD_ControlValve_ColName, RD_ControlValve_Unit );
	pclResizingColumnInfo->AddRangeColumn( CD_ControlValve_PipeSize, CD_ControlValve_PipeV, RD_ControlValve_ColName, RD_ControlValve_Unit );

	// Add possibility to click on title group to reset column width.
	CreateTitleGroupButtons( CD_ControlValve_Name, CD_ControlValve_PipeV, RD_ControlValve_GroupName, pclSheetDescriptionBCV );

	return lRetRow;
}

void CRViewSSelBCv::GetAdapterList( CDB_ControlValve *pclSelectedControlValve, CRank *pclAdapterList, CDB_RuledTableBase **ppclRuledTable )
{
	if( NULL == m_pclIndSelBCVParams || NULL == m_pclIndSelBCVParams->m_pTADB )
	{
		ASSERT_RETURN;
	}

	m_pclIndSelBCVParams->m_pTADB->GetAdapterList( pclSelectedControlValve, pclAdapterList, ppclRuledTable, m_bDowngradeActuatorFunctionality,
			m_pclIndSelBCVParams->m_strActuatorPowerSupplyID, m_pclIndSelBCVParams->m_strActuatorInputSignalID, m_pclIndSelBCVParams->m_iActuatorFailSafeFunction,
			m_pclIndSelBCVParams->m_eActuatorDRPFunction, m_pclIndSelBCVParams->m_eCvCtrlType, m_pclIndSelBCVParams->m_bOnlyForSet, 
			m_pclIndSelBCVParams->m_eFilterSelection );
}

void CRViewSSelBCv::GetSetAccessoryList( CDB_ControlValve *pclSelectedControlValve, CDB_Actuator *pclSeletedActuator, CRank *pclSetAccessoryList, 
		CDB_RuledTableBase **ppclRuledTable )
{
	if( NULL == m_pclIndSelBCVParams || NULL == m_pclIndSelBCVParams->m_pTADB )
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

	CDB_Set *pBCVActSet = pActSetTab->FindCompatibleSet( pclSelectedControlValve->GetIDPtr().ID, pclSeletedActuator->GetIDPtr().ID );
	
	if( NULL != pBCVActSet )
	{
		*ppclRuledTable = dynamic_cast<CDB_RuledTable *>( pBCVActSet->GetAccGroupIDPtr().MP );
		
		if( NULL != *ppclRuledTable )
		{	
			m_pclIndSelBCVParams->m_pTADB->GetAccessories( pclSetAccessoryList, *ppclRuledTable, m_pclIndSelBCVParams->m_eFilterSelection );
		}
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PRIVATE MEMBERS
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CRViewSSelBCv::_SetRemarkFullOpening( CSheetDescription *pclSheetDescriptionBCV, long lRow )
{
	if( NULL == pclSheetDescriptionBCV || NULL == pclSheetDescriptionBCV->GetSSheetPointer() )
	{
		return;
	}

	CSSheet *pclSSheet = pclSheetDescriptionBCV->GetSSheetPointer();
	
	// 'false' to specify that this row can't be selected.
	pclSheetDescriptionBCV->AddRows( 1, false );

	CString str = _T("* ");
	str += TASApp.LoadLocalizedString( IDS_FULLOPENING );

	// Span the cell and set the static text.
	pclSSheet->AddCellSpanW( CD_ControlValve_Name, lRow, CD_ControlValve_Separator - CD_ControlValve_Name, 1 );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_LEFT );
	pclSSheet->SetStaticText( CD_ControlValve_Name, lRow, str );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_StyleHorizontalAlign, (LPARAM)SSS_ALIGN_CENTER );
}
