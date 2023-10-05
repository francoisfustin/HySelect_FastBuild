#include "stdafx.h"
#include "afxctl.h"

#include "TASelect.h"
#include "MainFrm.h"
#include "Hydronic.h"
#include "DlgLeftTabSelManager.h"

#include "ProductSelectionParameters.h"
#include "RViewSSelCtrl.h"
#include "RViewSSelCV.h"

CRViewSSelCv *pRViewSSelCv = NULL;
CRViewSSelCv::CRViewSSelCv() : CRViewSSelCtrl( CMainFrame::RightViewList::eRVSSelCv )
{
	m_pclIndSelCVParams = NULL;
	pRViewSSelCv = this;
}

CRViewSSelCv::~CRViewSSelCv()
{
	pRViewSSelCv = NULL;
}

void CRViewSSelCv::Suggest( CProductSelelectionParameters *pclProductSelectionParameters, LPARAM lpParam )
{
	if( NULL == pclProductSelectionParameters || NULL == dynamic_cast<CIndSelCVParams*>( pclProductSelectionParameters ) )
	{
		ASSERT_RETURN;
	}

	m_pclIndSelCVParams = dynamic_cast<CIndSelCVParams*>( pclProductSelectionParameters );

	CRViewSSelCtrl::Suggest( pclProductSelectionParameters, lpParam );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PROTECTED MEMBERS
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CRViewSSelCv::OnTextTipFetch( CSheetDescription *pclSheetDescription, long lColumn, long lRow, WORD *pwMultiLine, SHORT *pnTipWidth, 
		TCHAR *pstrTipText, BOOL *pbShowTip )
{
	if( NULL == m_pclIndSelCVParams || NULL == m_pclIndSelCVParams->m_pclSelectCVList )
	{
		ASSERTA_RETURN( false );
	}

	if( false == m_bInitialised || NULL == dynamic_cast<CSelectCVList*>( m_pclIndSelCVParams->m_pclSelectCVList )
			|| NULL == pclSheetDescription || NULL == pclSheetDescription->GetSSheetPointer() )
	{
		return false;
	}

	// First, we must call base class.
	// Remark: if base class has displayed a tooltip, it's not necessary to go further in this method.
	if( true == CRViewSSelCtrl::OnTextTipFetch( pclSheetDescription, lColumn, lRow, pwMultiLine, pnTipWidth, pstrTipText, pbShowTip ) )
	{
		return true;
	}

	CDB_TAProduct *pTAP = dynamic_cast<CDB_TAProduct *>( (CData *)GetCDProduct( lColumn, lRow, pclSheetDescription ) );

	if( NULL == pTAP )
	{
		return false;
	}

	CSelectCVList *pSelectCVList = dynamic_cast<CSelectCVList*>( m_pclIndSelCVParams->m_pclSelectCVList );
	CSelectedValve *pclSelectedCv = GetSelectProduct<CSelectedValve>( pTAP, pSelectCVList );

	if( NULL == pclSelectedCv )
	{
		return false;
	}

	CString str;
	bool bReturnValue = false;
	*pbShowTip = false;
	*pwMultiLine = SS_TT_MULTILINE_AUTO;
	
	CDB_ControlValve *pCV = dynamic_cast<CDB_ControlValve *>( pTAP );
	CSSheet *pclSSheet = pclSheetDescription->GetSSheetPointer();
	CDS_TechnicalParameter *pclTechParam = m_pclIndSelCVParams->m_pTADS->GetpTechParams();
	
	if( SD_ControlValve == pclSheetDescription->GetSheetDescriptionID() )
	{
		if( CD_ControlValve_Kvs == lColumn )
		{
			// Check what is the color of the text.
			if( NULL != pclSSheet && _ORANGE == pclSSheet->GetForeColor( lColumn, lRow ) )
			{
				CString str2 = WriteCUDouble( _C_KVCVCOEFF, pCV->GetKvs() );
				str2 += CString( _T(" > ") ) + WriteCUDouble( _C_KVCVCOEFF, m_pclIndSelCVParams->m_dKvs );
				FormatString( str, IDS_SSHEETSSELCV_KVSERROR, str2 );
			}
		}
		
		if( CD_ControlValve_Dp == lColumn && NULL != pclSSheet && _ORANGE == pclSSheet->GetForeColor( lColumn, lRow ) )
		{
			double dRho = m_pclIndSelCVParams->m_WC.GetDens();
			double dFlow = m_pclIndSelCVParams->m_dFlow;
			double dKvs = pCV->GetKvs();
			double dDp = CalcDp( dFlow, dKvs, dRho );
			
			if( dDp > 0.0 )
			{
				double dMinDpTechParam;

				if( CDB_ControlProperties::eCvOnOff == m_pclIndSelCVParams->m_eCvCtrlType )
				{
					dMinDpTechParam = pclTechParam->GetCVMinDpOnoff();
				}
				else
				{
					dMinDpTechParam = pclTechParam->GetCVMinDpProp();
				}

				double dMinDpMinCv = pSelectCVList->GetMinDpMinCv();
				bool bBest = pclSelectedCv->IsFlagSet( CSelectedBase::eBest );

				if( dDp < dMinDpTechParam )
				{
					CDB_ControlProperties *pControlProperties = pCV->GetCtrlProp();

					if( NULL != pControlProperties )
					{
						double dMinDpTechParam = 0.0;

						if( CDB_ControlProperties::eCvOnOff == pControlProperties->GetCtrlType() )
						{
							dMinDpTechParam = pclTechParam->GetCVMinDpOnoff();
						}
						else
						{
							dMinDpTechParam = pclTechParam->GetCVMinDpProp();
						}

						CString str2 = WriteCUDouble( _U_DIFFPRESS, dDp );
						str2 += CString( _T(" < ") ) + WriteCUDouble( _U_DIFFPRESS, dMinDpTechParam, true );

						// "Dp is below the minimum value defined in technical parameters ( %1 )"
						FormatString( str, IDS_SSHEETSSEL_DPERRORL, str2 );
					}
				}
				else if( dDp < m_pclIndSelCVParams->m_dDp )
				{
					CString str2 = WriteCUDouble( _U_DIFFPRESS, dDp );
					str2 += CString( _T(" < ") ) + WriteCUDouble( _U_DIFFPRESS, m_pclIndSelCVParams->m_dDp, true );
					FormatString( str, IDS_SSHEETSSELCV_DPERRORL, str2 );
				}
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

	if( false == str.IsEmpty() )
	{
		*pnTipWidth = (SHORT)pclSSheet->GetTipTextWidth( str );
		wcsncpy_s( pstrTipText, SS_TT_TEXTMAX, (LPCTSTR)str, SS_TT_TEXTMAX );
		*pbShowTip = true;
		bReturnValue = true;
	}
	return bReturnValue;
}

bool CRViewSSelCv::IsActuatorFit( CDB_ElectroActuator *pclElectroActuator, CDB_ControlValve *pclSelectedControlValve, bool bDowngradeActuatorFunctionality, 
		bool bAcceptAllFailSafe )
{
	if( NULL == m_pclIndSelCVParams )
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
		ComboDRPFctChoice = m_pclIndSelCVParams->m_eActuatorDRPFunction;
	}
	else
	{
		ComboDRPFctChoice = CDB_ControlValve::DRPFunction::drpfAll;
	}

	return pclElectroActuator->IsActuatorFit( m_pclIndSelCVParams->m_strActuatorPowerSupplyID, m_pclIndSelCVParams->m_strActuatorInputSignalID, m_pclIndSelCVParams->m_iActuatorFailSafeFunction,
		ComboDRPFctChoice, CurrentActuatorDRPFct, m_pclIndSelCVParams->m_eCvCtrlType, bDowngradeActuatorFunctionality );
}

long CRViewSSelCv::FillControlValveRows( CDB_ControlValve *pEditedControlValve )
{
	if( NULL == m_pclIndSelCVParams || NULL == m_pclIndSelCVParams->m_pclSelectCVList 
			|| NULL == m_pclIndSelCVParams->m_pclSelectCVList->GetSelectPipeList() || NULL == m_pclIndSelCVParams->m_pTADS
			|| NULL == m_pclIndSelCVParams->m_pPipeDB )
	{
		ASSERTA_RETURN( -1 );
	}

	// Call first the bass class to initialize sheet, header and so on.
	if( -1 == CRViewSSelCtrl::FillControlValveRows( pEditedControlValve ) )
	{
		return -1;
	}

	// Retrieve sheet description created in base class.
	CSheetDescription *pclSheetDescriptionCv = m_ViewDescription.GetFromSheetDescriptionID( SD_ControlValve );

	if( NULL == pclSheetDescriptionCv || NULL == pclSheetDescriptionCv->GetSSheetPointer() )
	{
		return -1;
	}
	
	CSSheet *pclSSheet = pclSheetDescriptionCv->GetSSheetPointer();

	// Hide unused columns.
	pclSSheet->ShowCol( CD_ControlValve_Preset, FALSE );
	pclSSheet->ShowCol( CD_ControlValve_DpFullOpening, FALSE );
	pclSSheet->ShowCol( CD_ControlValve_DpHalfOpening, FALSE );
	pclSSheet->ShowCol( CD_ControlValve_DplRange, FALSE );
	pclSSheet->ShowCol( CD_ControlValve_DpMax, FALSE );

	// Variables.
	double dMinDpTechParam = DBL_MAX;
	double dRho = m_pclIndSelCVParams->m_WC.GetDens();
	double dFlow = m_pclIndSelCVParams->m_dFlow;

	CDS_TechnicalParameter *pTechParam = m_pclIndSelCVParams->m_pTADS->GetpTechParams();
	
	if( CDB_ControlProperties::eCvOnOff == m_pclIndSelCVParams->m_eCvCtrlType )
	{
		dMinDpTechParam = pTechParam->GetCVMinDpOnoff();
	}
	else
	{
		dMinDpTechParam = pTechParam->GetCVMinDpProp();
	}
	
	long lRetRow = -1;
	long lRow = RD_ControlValve_FirstAvailRow;

	CSelectPipe selPipe( m_pclIndSelCVParams );
	CDB_ControlValve *pCv = NULL;

	// Retrieve the lowest Dp equal or above the Dp input by user. This (or these) one(s) will be drawn in green.
	// Remark: only if user has input a valid dp!
	double dMinDpMinCv = m_pclIndSelCVParams->m_pclSelectCVList->GetMinDpMinCv();
	
	// Fill-in the cells.
	long lValveTotalCount = 0;
	long lValveNotPriorityCount = 0;
	bool bShowAllPrioritiesShown = false;
	long bShowAllPrioritiesButtonRow = 0;
	bool bAtLeastOneError;
	bool bAtLeastOneWarning;
	bool bAtLeastOneGreen;
	
	pclSheetDescriptionCv->SetUserVariable( _SDUV_TOTALPRODUCT_COUNT, 0 );
	
	bool bAtLeastOneCVPartOfASet = false;
	pclSheetDescriptionCv->RestartRemarkGenerator();
	
	for( CSelectedValve *pclSelectedValve = m_pclIndSelCVParams->m_pclSelectCVList->GetFirst<CSelectedValve>(); NULL != pclSelectedValve; 
			pclSelectedValve = m_pclIndSelCVParams->m_pclSelectCVList->GetNext<CSelectedValve>() )
	{
		CDB_ControlValve *pclControlValve = dynamic_cast<CDB_ControlValve*>( pclSelectedValve->GetpData() );	
		
		if( NULL == pclControlValve ) 
		{
			continue;
		}

		bool bBest = pclSelectedValve->IsFlagSet( CSelectedBase::eBest );
		bAtLeastOneError = false;
		bAtLeastOneWarning = false;
		bAtLeastOneGreen = false;
		double dKvs = pclControlValve->GetKvs();
		double dDp = CalcDp( dFlow, dKvs, dRho );

		if( true == pclSelectedValve->IsFlagSet(CSelectedBase::eNotPriority) )
		{
			lValveNotPriorityCount++;

			if( false == bShowAllPrioritiesShown )
			{
				// Add one empty not selectable row.
				pclSheetDescriptionCv->AddRows( 1 );
				pclSheetDescriptionCv->RemoveSelectableRow( lRow );

				pclSSheet->SetCellBorder( CD_ControlValve_CheckBox, lRow, CD_ControlValve_Separator - 1, lRow, true, 
						SS_BORDERTYPE_TOP | SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );

				pclSSheet->SetCellBorder( CD_ControlValve_PipeSize, lRow, CD_ControlValve_Pointer - 1, lRow, true, 
						SS_BORDERTYPE_TOP | SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, _BLACK );

				bShowAllPrioritiesButtonRow = lRow++;
				bShowAllPrioritiesShown = true;
			}
		}

		// Add this row.
		// 'true' to specify that this row can be selected.
		pclSheetDescriptionCv->AddRows( 1, true );

		// Return to standard color.
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

		// add checkbox
		pclSSheet->SetCheckBox( CD_ControlValve_CheckBox, lRow, _T(""), false, true );

		// Add the picture for a CvAct-Set.
		if( true == m_pclIndSelCVParams->m_bOnlyForSet && true == pclControlValve->IsPartOfaSet() )
		{
			CCDButtonBox *pCDButtonBox = CreateCellDescriptionBox( CD_ControlValve_Box, lRow, true, CCDButtonBox::ButtonState::BoxOpened, pclSheetDescriptionCv );

			if( NULL != pCDButtonBox )
			{
				pCDButtonBox->ApplyInternalChange();
			}

			bAtLeastOneCVPartOfASet = true;
		}

		// Set the color for the Kvs value.
		if( m_pclIndSelCVParams->m_dKvs > 0.0 )
		{
			if( pclControlValve->GetKvs() == m_pclIndSelCVParams->m_dKvs )
			{
				pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_GREEN );
				bAtLeastOneGreen = true;
			}
			else if( pclControlValve->GetKvs() > m_pclIndSelCVParams->m_dKvs )
			{
				pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_ORANGE );
				bAtLeastOneWarning = true;
			}
		}

		// Set the Kvs value.
		pclSSheet->SetStaticText( CD_ControlValve_Kvs, lRow, WriteCUDouble( _C_KVCVCOEFF, pclControlValve->GetKvs() ) );
		
		// Return to standard color.
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );
		
		// Set the color for the Dp value.
		if( m_pclIndSelCVParams->m_dDp > 0 )
		{
			// Remark: 'dMinDpMinCv' is the Dp that is nearest (from above) of the Dp wanted by the user.
			if( dDp == dMinDpMinCv )
			{
				// Dp computed on the current valve is the lowest possible value just equal or bigger the Dp value input by user...
				pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_GREEN );
				bAtLeastOneGreen = true;
			}
			else if( dDp < m_pclIndSelCVParams->m_dDp )
			{
				// Dp computed is lower than the minimal admissible Dp value.
				pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_ORANGE );
				bAtLeastOneWarning = true;
			}
		}

		if( dDp < dMinDpTechParam ) 
		{
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_ORANGE );
			bAtLeastOneWarning = true;
		}
		
		// Set the Dp value.
		pclSSheet->SetStaticText( CD_ControlValve_Dp, lRow, WriteCUDouble( _U_DIFFPRESS, dDp ) );
		
		// Return to standard color.
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

		pclSSheet->SetStaticText( CD_ControlValve_Rangeability, lRow, pclControlValve->GetStrRangeability().c_str() ) ;

		pclSSheet->SetStaticText( CD_ControlValve_LeakageRate, lRow, WriteCUDouble( _U_NODIM, pclControlValve->GetLeakageRate() * 100 ) );
	
		if( CDB_CloseOffChar::eOpenType::Linear == ( (CDB_CloseOffChar *)pclControlValve->GetCloseOffCharIDPtr().MP )->GetOpenType() )
		{
			pclSSheet->SetStaticText( CD_ControlValve_Stroke, lRow, WriteCUDouble( _U_DIAMETER, pclControlValve->GetStroke() ) ); 
		}
		else
		{
			pclSSheet->SetStaticText( CD_ControlValve_Stroke, lRow, WriteDouble( pclControlValve->GetStroke(), 0, 0 ) );
		}
		
		// Set the characteristic picture.
		CDB_ControlProperties::eCTRLCHAR CtrlChar = pclControlValve->GetCtrlProp()->GetCvCtrlChar();
		
		if( CDB_ControlProperties::eCTRLCHAR::Linear == CtrlChar )
		{
			pclSSheet->SetPictureCellWithID( IDI_CHARACTLIN, CD_ControlValve_ImgCharacteristic, lRow, CSSheet::PictureCellType::Icon );
		}
		else if( CDB_ControlProperties::eCTRLCHAR::EqualPc == CtrlChar )
		{
			pclSSheet->SetPictureCellWithID( IDI_CHARACTEQM, CD_ControlValve_ImgCharacteristic, lRow, CSSheet::PictureCellType::Icon );
		}
		else
		{
			pclSSheet->SetPictureCellWithID( IDI_CHARACTNONE, CD_ControlValve_ImgCharacteristic, lRow, CSSheet::PictureCellType::Icon );
		}
		
		// Set the Push to open/close picture.
		IDPTR IDPtr = pclControlValve->GetTypeIDPtr();
		CDB_ControlProperties::ePushOrPullToClose PushClose = pclControlValve->GetCtrlProp()->GetCvPushClose();
		
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
		if( true == pclSelectedValve->IsFlagSet( CSelectedBase::eTemperature ) )
		{
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_RED );
			bAtLeastOneError = true;
		}
		
		pclSSheet->SetStaticText( CD_ControlValve_TemperatureRange, lRow, ( (CDB_TAProduct *)pclSelectedValve->GetProductIDPtr().MP )->GetTempRange() );
		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

		// Now we can set first columns in regards to current status (error, best or normal).
		if( true == bAtLeastOneError )
		{
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_RED );
		}
		else if( true == bAtLeastOneGreen || true == pclSelectedValve->IsFlagSet( CSelectedBase::eBest ) )
		{
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_GREEN );
		}
		
		// In addition of current flag, we have possibility that valve has the flag 'Not available' or 'Deleted'. In that case, we show valve name in red with "*" around it and
		// symbol '!' or '!!' after.
		if( true == pclControlValve->IsDeleted() )
		{
			pclSheetDescriptionCv->WriteTextWithFlags( CString( pclControlValve->GetName() ), CD_ControlValve_Name, lRow, 
					CSheetDescription::RemarkFlags::Deleted, _T(""), _RED );
		}
		else if( false == pclControlValve->IsAvailable() )
		{
			pclSheetDescriptionCv->WriteTextWithFlags( CString( pclControlValve->GetName() ), CD_ControlValve_Name, lRow, 
					CSheetDescription::RemarkFlags::NotAvailable, _T(""), _RED );
		}
		else
		{
			pclSSheet->SetStaticText( CD_ControlValve_Name, lRow, pclControlValve->GetName() );
		}

		if( true == bAtLeastOneError )
		{
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );
		}
		else if( true == bAtLeastOneGreen || true == pclSelectedValve->IsFlagSet( CSelectedBase::eBest ) )
		{
			pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_GREEN );
		}
		
		pclSSheet->SetStaticText( CD_ControlValve_Material, lRow, pclControlValve->GetBodyMaterial() );
		pclSSheet->SetStaticText( CD_ControlValve_Connection, lRow, pclControlValve->GetConnect() );
		pclSSheet->SetStaticText( CD_ControlValve_Version, lRow, pclControlValve->GetVersion() );
		pclSSheet->SetStaticText( CD_ControlValve_Size, lRow, pclControlValve->GetSize() );
		pclSSheet->SetStaticText( CD_ControlValve_PN, lRow, pclControlValve->GetPN().c_str() );

		pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor, (LPARAM)_BLACK );

		pclSSheet->SetStaticText( CD_ControlValve_Separator, lRow );

		m_pclIndSelCVParams->m_pclSelectCVList->GetSelectPipeList()->GetMatchingPipe( pclControlValve->GetSizeKey(), selPipe );
		pclSSheet->SetStaticText( CD_ControlValve_PipeSize, lRow, selPipe.GetpPipe()->GetName() );       
		
		// Set the LinDp to orange if it is above the technical parameter limit.
		// Orange if it is below the dMinLinDp.
		if( ( selPipe.GetLinDp() > pTechParam->GetPipeMaxDp() ) || ( selPipe.GetLinDp() < pTechParam->GetPipeMinDp() ) )
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
		CCellDescriptionProduct *pclCDProduct = CreateCellDescriptionProduct( pclSheetDescriptionCv->GetFirstParameterColumn(), lRow, (LPARAM)pclControlValve, pclSheetDescriptionCv );
		pclCDProduct->SetUserParam( (LPARAM)pclSelectedValve );

		if( NULL != pEditedControlValve && pEditedControlValve == pclControlValve )
		{
			lRetRow = lRow;
		}

		pclSSheet->SetCellBorder( CD_ControlValve_CheckBox, lRow, CD_ControlValve_Separator - 1, lRow, true, SS_BORDERTYPE_BOTTOM,SS_BORDERSTYLE_SOLID, _GRAY );
		pclSSheet->SetCellBorder( CD_ControlValve_PipeSize, lRow, CD_ControlValve_Pointer - 1, lRow, true, SS_BORDERTYPE_BOTTOM,SS_BORDERSTYLE_SOLID, _GRAY );

		lRow++;
		lValveTotalCount++;
	}

	long lLastDataRow = lRow - 1;

	pclSheetDescriptionCv->SetUserVariable( _SDUV_TOTALPRODUCT_COUNT, lValveTotalCount );
	
	// Try to merge only if there is more than one control valve.
	if( lValveTotalCount > 1 )
	{
		vector<long> vecColumnList;
		vecColumnList.push_back( CD_ControlValve_PipeSize );
		vecColumnList.push_back( CD_ControlValve_PipeLinDp );
		vecColumnList.push_back( CD_ControlValve_PipeV );
		pclSSheet->MergeRowsWithSameValues( RD_ControlValve_FirstAvailRow, lLastDataRow, vecColumnList );
	}

	// If we are not in selection by package mode OR if there is no valve that belongs to a set...
	if( false == m_pclIndSelCVParams->m_bOnlyForSet || false == bAtLeastOneCVPartOfASet )
	{
		pclSSheet->ShowCol( CD_ControlValve_Box, FALSE );
	}

	pclSSheet->SetCellBorder( CD_ControlValve_CheckBox, lLastDataRow, CD_ControlValve_Separator - 1, lLastDataRow, true, 
			SS_BORDERTYPE_BOTTOM,SS_BORDERSTYLE_SOLID, _BLACK );

	// Needed by pipe merging.
	pclSSheet->SetCellBorder( CD_ControlValve_PipeSize, lLastDataRow, CD_ControlValve_Pointer - 1, lLastDataRow, true, 
			SS_BORDERTYPE_BOTTOM,SS_BORDERSTYLE_SOLID, _BLACK );

	// Add Remarks.
	// Note: these lines must be placed after creating Show/Hide all priorities button to avoid to take in count these lines when defining
	//       range of row to be Show/Hide.
	lRow = pclSheetDescriptionCv->WriteRemarks( lRow, CD_ControlValve_CheckBox, CD_ControlValve_Separator );
	
	pclSheetDescriptionCv->SetUserVariable( _SDUV_TITLEFORECOLOR, ( pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontForeColor ) ) );
	pclSheetDescriptionCv->SetUserVariable( _SDUV_TITLEBACKCOLOR, ( pclSSheet->GetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor ) ) );

	// Now we can create Show/Hide all priorities if needed.
	// Remarks: 2nd condition to avoid to have button if we have only no priority valves.
	if( true == bShowAllPrioritiesShown && lValveTotalCount > lValveNotPriorityCount )
	{
		// Create Show/Hide all priorities button in regards to current state!
		CCDButtonShowAllPriorities::ButtonState eState = ( true == GetShowAllPrioritiesFlag( SD_ControlValve ) ) ? 
				CCDButtonShowAllPriorities::ButtonState::HidePriorities : CCDButtonShowAllPriorities::ButtonState::ShowPriorities;
		
		CCDButtonShowAllPriorities *pclShowAllButton = CreateShowAllPrioritiesButton( CD_ControlValve_FirstColumn, bShowAllPrioritiesButtonRow, 
				bShowAllPrioritiesShown, eState, bShowAllPrioritiesButtonRow + 1, pclSSheet->GetMaxRows(), pclSheetDescriptionCv );

		// Here we must call 'ApplyInternalChange' to show or hide rows in regards to the current state!
		if( NULL != pclShowAllButton )
		{
			pclShowAllButton->ApplyInternalChange( );
		}
	}
	else
	{
		if( bShowAllPrioritiesButtonRow != 0 )
		{
			pclSSheet->ShowRow( bShowAllPrioritiesButtonRow, false );
		}
	}

	// Add the main title.
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, (LPARAM)GetTitleBackgroundColor( pclSheetDescriptionCv ) );
	pclSSheet->AddCellSpanW( CD_ControlValve_CheckBox, RD_ControlValve_GroupName, CD_ControlValve_Separator - CD_ControlValve_CheckBox, 1 );
	pclSSheet->SetStaticText( CD_ControlValve_CheckBox, RD_ControlValve_GroupName, IDS_SSHEETSSELCV_CVGROUP );
	pclSSheet->AddCellSpanW( CD_ControlValve_PipeSize, RD_ControlValve_GroupName, CD_ControlValve_Pointer - CD_ControlValve_PipeSize, 1 );
	
	CTable *pclTable = dynamic_cast<CTable *>( m_pclIndSelCVParams->m_pPipeDB->Get( m_pclIndSelCVParams->m_strPipeSeriesID ).MP );
	ASSERT( NULL != pclTable );

	pclSSheet->SetStaticText( CD_ControlValve_PipeSize, RD_ControlValve_GroupName, pclTable->GetName() );
	
	pclSSheet->SetTextPattern( CSSheet::_SSTextPattern::UserStaticColumn );
	pclSSheet->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBold, (LPARAM)FALSE );

	// Add possibility to change column size.
	// Remark: place this code after any initialization (like adding, removing, resizing, hiding/showing columns).
	CResizingColumnInfo *pclResizingColumnInfo = pclSheetDescriptionCv->GetResizingColumnInfo();
	pclResizingColumnInfo->ActivateFeature( true, false, RD_ControlValve_FirstAvailRow, lLastDataRow );
	pclResizingColumnInfo->AddRangeColumn( CD_ControlValve_Name, CD_ControlValve_TemperatureRange, RD_ControlValve_ColName, RD_ControlValve_Unit );
	pclResizingColumnInfo->AddRangeColumn( CD_ControlValve_PipeSize, CD_ControlValve_PipeV, RD_ControlValve_ColName, RD_ControlValve_Unit );

	// Add possibility to click on title group to reset column width.
	CreateTitleGroupButtons( CD_ControlValve_CheckBox, CD_ControlValve_PipeV, RD_ControlValve_GroupName, pclSheetDescriptionCv );

	return lRetRow;
}
