#include "stdafx.h"
#include <float.h>
#include <math.h>

#include "TASelect.h"
#include "MainFrm.h"
#include "Global.h"
#include "Select.h"

#include "DlgNoDevSeparatorFound.h"
#include "DlgConfSel.h"
#include "DlgTechParam.h"
#include "DlgWaterChar.h"

#include "DlgLeftTabSelManager.h"

#include "RViewSSelSS.h"
#include "RViewSSelSeparator.h"

#include "DlgSelectionBase.h"
#include "DlgIndSelBase.h"
#include "DlgIndSelSeparator.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CDlgIndSelSeparator::CDlgIndSelSeparator( CWnd *pParent )
	: CDlgIndSelBase( m_clIndSelSeparatorParams, CDlgIndSelSeparator::IDD, pParent )
{
	m_clIndSelSeparatorParams.m_eProductSubCategory = ProductSubCategory::PSC_PM_Separator;
	m_clIndSelSeparatorParams.m_eProductSelectionMode = ProductSelectionMode::ProductSelectionMode_Individual;
}

void CDlgIndSelSeparator::SaveSelectionParameters()
{
	CDlgIndSelBase::SaveSelectionParameters();
	CDS_IndSelParameter *pclIndSelParameter = m_clIndSelSeparatorParams.m_pTADS->GetpIndSelParameter();

	if( NULL == pclIndSelParameter )
	{
		return;
	}

	pclIndSelParameter->SetSepRadioFlowPowerDT( (int)m_clIndSelSeparatorParams.m_eFlowOrPowerDTMode );
	pclIndSelParameter->SetSeparatorTypeID( m_clIndSelSeparatorParams.m_strComboTypeID );
	pclIndSelParameter->SetSeparatorFamilyID( m_clIndSelSeparatorParams.m_strComboFamilyID );
	pclIndSelParameter->SetSeparatorConnectID( m_clIndSelSeparatorParams.m_strComboConnectID );
	pclIndSelParameter->SetSeparatorVersionID( m_clIndSelSeparatorParams.m_strComboVersionID );

	if( NULL != pRViewSSelSeparator )
	{
		pRViewSSelSeparator->SaveSelectionParameters( pclIndSelParameter );
	}
}

void CDlgIndSelSeparator::ChangeApplicationType( ProjectType eProductSelectionApplicationType )
{
	CDlgIndSelBase::ChangeApplicationType( eProductSelectionApplicationType );

	if( NULL != pRViewSSelSeparator )
	{
		pRViewSSelSeparator->Reset();
	}
}

void CDlgIndSelSeparator::OnVerifyFluidCharacteristics( bool &bEnable, CString &strMsg, CDlgWaterChar *pclDlgWaterChar )
{
	_VerifyFluidCharacteristicsHelper( bEnable, strMsg, pclDlgWaterChar );
}

void CDlgIndSelSeparator::ActivateLeftTabDialog( void )
{
	if( NULL == m_pclIndSelParams || NULL == m_pclIndSelParams->m_pTADS || NULL == m_pclIndSelParams->m_pTADS->GetpIndSelParameter() )
	{
		ASSERT_RETURN;
	}

	CDlgIndSelBase::ActivateLeftTabDialog();

	// HYS-1263 : Added to update application type if it is necessary
	CDS_TechnicalParameter *pTechP = m_clIndSelSeparatorParams.m_pTADS->GetpTechParams();
	ASSERT( NULL != pTechP );

	ProjectType eCurrentPSApplicationType = pTechP->GetProductSelectionApplicationType();

	if( eCurrentPSApplicationType != m_clIndSelSeparatorParams.m_eApplicationType )
	{
		ChangeApplicationType( eCurrentPSApplicationType );
	}

	if( this == pDlgLeftTabSelManager->GetCurrentLeftTabDialog() )
	{
		_VerifyFluidCharacteristics( false );
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PROTECTED METHODS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP( CDlgIndSelSeparator, CDlgIndSelBase )
	ON_CBN_SELCHANGE( IDC_COMBOTYPE, OnCbnSelChangeType )
	ON_CBN_SELCHANGE( IDC_COMBOFAMILY, OnCbnSelChangeFamily )
	ON_CBN_SELCHANGE( IDC_COMBOCONNECT, OnCbnSelChangeConnect )
	ON_CBN_SELCHANGE( IDC_COMBOVERSION, OnCbnSelChangeVersion )
	ON_MESSAGE( WM_USER_MODIFYSELECTEDSEPARATOR, OnModifySelectedSeparator )
END_MESSAGE_MAP()

void CDlgIndSelSeparator::DoDataExchange( CDataExchange* pDX )
{
	CDlgIndSelBase::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_COMBOTYPE, m_ComboType );
	DDX_Control( pDX, IDC_COMBOFAMILY, m_ComboFamily );
	DDX_Control( pDX, IDC_COMBOCONNECT, m_ComboConnect );
	DDX_Control( pDX, IDC_COMBOVERSION, m_ComboVersion );
	DDX_Control( pDX, IDC_GROUPSEPARATOR, m_GroupSeparator );

	// 'CDlgIndSelBase' variables.
	DDX_Control( pDX, IDC_EDITFLOW, m_clExtEditFlow );
	DDX_Control( pDX, IDC_EDITPOWER, m_clExtEditPower );
	DDX_Control( pDX, IDC_EDITDT, m_clExtEditDT );
	DDX_Control( pDX, IDC_GROUPQ, m_clGroupQ );
	DDX_Radio( pDX, IDC_RADIOFLOW, m_iRadioFlowPowerDT );

	DDX_Control( pDX, IDC_BUTTONSUGGEST, m_clButtonSuggest );
}

BOOL CDlgIndSelSeparator::OnInitDialog() 
{
	CDlgIndSelBase::OnInitDialog();
	
	// Initialization of flow and power static text is done in the 'CDlgSelectionBase' base class.
	CString str;
	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELSEPARATOR_DT );
	GetDlgItem( IDC_STATICDT )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELSEPARATOR_BUTTONSUGGEST );
	GetDlgItem( IDC_BUTTONSUGGEST )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELSEPARATOR_BUTTONSELECT );
	GetDlgItem( IDC_BUTTONSELECT )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELSEPARATOR_BUTTONCANCEL );
	GetDlgItem( IDC_BUTTONCANCEL )->SetWindowText( str);
	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELSEPARATOR_STATICTYPE );
	GetDlgItem( IDC_STATICTYPE )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELSEPARATOR_STATICFAMILY );
	GetDlgItem( IDC_STATICFAMILY )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELSEPARATOR_STATICCONNECT );
	GetDlgItem( IDC_STATICCONNECT )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_TABCDIALOGSSELSEPARATOR_STATICVERSION );
	GetDlgItem( IDC_STATICVERSION )->SetWindowText( str );

	str.Empty();

	// Set proper style and add icons the separator groups.
	CImageList *pclImgListGroupBox = TASApp.GetpRCImageManager()->GetImageList( CRCImageManager::ILN_GroupBox );
	ASSERT ( NULL != pclImgListGroupBox );

	if( NULL != pclImgListGroupBox )
	{
		m_GroupSeparator.SetImageList( pclImgListGroupBox, CRCImageManager::ILGB_Separator );
	}

	CImageList *pclImgListGroupBoxGrayed = TASApp.GetpRCImageManager()->GetImageList( CRCImageManager::ILN_GroupBoxGrayed );
	ASSERT ( NULL != pclImgListGroupBoxGrayed );

	if( NULL != pclImgListGroupBoxGrayed )
	{
		m_GroupSeparator.SetImageList( pclImgListGroupBoxGrayed, CRCImageManager::ILGBG_Separator, true );
	}

	m_GroupSeparator.SetInOffice2007Mainframe( true );

	// PAY ATTENTION: if it's feature is reactivated, there is some changes to do in 'OnLButtonDown' (ask to FF).
// 	m_GroupValve.SetExpandCollapseMode( true );
// 	m_GroupValve.SetNotificationHandler( this );
	
	if( NULL != pclImgListGroupBox )
	{
		GetpGroupQ()->SetImageList( pclImgListGroupBox, CRCImageManager::ILGB_Q );
	}

	if( NULL != pclImgListGroupBoxGrayed )
	{
		GetpGroupQ()->SetImageList( pclImgListGroupBoxGrayed, CRCImageManager::ILGBG_Q, true );
	}

	GetpGroupQ()->SetInOffice2007Mainframe( true );

	return TRUE;
}

void CDlgIndSelSeparator::OnCbnSelChangeType() 
{
	m_clIndSelSeparatorParams.m_strComboTypeID = m_ComboType.GetCBCurSelIDPtr().ID;
	_FillComboFamily();
	OnCbnSelChangeFamily();
}

void CDlgIndSelSeparator::OnCbnSelChangeFamily() 
{
	m_clIndSelSeparatorParams.m_strComboFamilyID = m_ComboFamily.GetCBCurSelIDPtr().ID;
	_FillComboConnect()	;
	OnCbnSelChangeConnect();
}

void CDlgIndSelSeparator::OnCbnSelChangeConnect() 
{
	m_clIndSelSeparatorParams.m_strComboConnectID = m_ComboConnect.GetCBCurSelIDPtr().ID;
	_FillComboVersion();
	OnCbnSelChangeVersion();
}

void CDlgIndSelSeparator::OnCbnSelChangeVersion() 
{
	m_clIndSelSeparatorParams.m_strComboVersionID = m_ComboVersion.GetCBCurSelIDPtr().ID;
	ClearAll();
}

LRESULT CDlgIndSelSeparator::OnModifySelectedSeparator( WPARAM wParam, LPARAM lParam )
{
	if( NULL == TASApp.GetpTADS() || NULL == TASApp.GetpTADS()->GetpTechParams() )
	{
		ASSERTA_RETURN( -1 );
	}

	CDS_SSelAirVentSeparator *pclSelectedSeparator = dynamic_cast<CDS_SSelAirVentSeparator *>( (CData *)lParam );
	ASSERT( NULL != pclSelectedSeparator );

	if( NULL == pclSelectedSeparator || NULL == pDlgLTtabctrl )
	{
		return 0;
	}

	m_clIndSelSeparatorParams.m_SelIDPtr = pclSelectedSeparator->GetIDPtr();
	m_bInitialised = false;

	pDlgLTtabctrl->SetActiveTab( CMyMFCTabCtrl::TabIndex::etiSSel );

	// If we are not in the good selection mode, we must first disable it.
	if( ProductSelectionMode_Individual != pMainFrame->GetCurrentProductSelectionMode() )
	{
		pMainFrame->ChangeProductSelectionMode( ProductSelectionMode_Individual );
	}

	// HYS-1242: SetRedraw to false to avoid too much refresh in ClickRibbonProductSelGallery. 
	pMainFrame->SetRedraw( FALSE );
	// Select the right item in the gallery box.
	pMainFrame->ClickRibbonProductSelGallery( ProductSubCategory::PSC_PM_Separator );

	// Select 'Heating, 'Cooling' or 'Solar'.
	m_clIndSelSeparatorParams.m_eApplicationType = pclSelectedSeparator->GetpSelectedInfos()->GetApplicationType();
	pMainFrame->ChangeRbnProductSelectionApplicationType( m_clIndSelSeparatorParams.m_eApplicationType );

	// Transfer water characteristics.
	m_clIndSelSeparatorParams.m_WC = *( pclSelectedSeparator->GetpSelectedInfos()->GetpWCData() );

	// Copy water characteristic and temperature data in the technical parameters.
	OnModifySelectedProductUpdateTechData( pclSelectedSeparator->GetpSelectedInfos() );

	// HYS-1242: To refresh the window in InitializeRibbonRichEditBoxesSSel
	pMainFrame->SetRedraw( TRUE );
	pMainFrame->InitializeRibbonRichEditBoxesSSel();

	// Remark: The 'm_clIndSelSeparatorParams.m_strPipeSeriesID' and 'm_clIndSelSeparatorParams.m_strPipeID' are both updated
	//         by calling the 'SelectCurrentComboPipes' method.
	SelectCurrentComboPipes( pclSelectedSeparator->GetPipeSeriesID(), pclSelectedSeparator->GetPipeID() );

	// Remark: Corresponding combo variables in 'm_clIndSelSeparatorParams' are updated in each of this following methods.
	_FillComboType( pclSelectedSeparator->GetTypeID() );
	_FillComboFamily( pclSelectedSeparator->GetFamilyID() );
	_FillComboConnect( pclSelectedSeparator->GetConnectID() );
	_FillComboVersion( pclSelectedSeparator->GetVersionID() );

	m_iRadioFlowPowerDT = ( CDS_SelProd::efdFlow == pclSelectedSeparator->GetFlowDef() ) ? Radio_Flow : Radio_PowerDT;
	
	// 'FALSE' to change the radio state in the dialog with the content of the 'm_iRadioFlowPowerDT' variable.	
	UpdateData( FALSE );
	
	// Remark: 'm_clIndSelSeparatorParams.m_eFlowOrPowerDTMode' is updated in the following method in regards to the 'm_iRadioFlowPowerDT' variable.
	CDlgIndSelBase::UpdateFlowOrPowerDTState();

	if( CDS_SelProd::efdFlow == m_clIndSelSeparatorParams.m_eFlowOrPowerDTMode )
	{
		m_clIndSelSeparatorParams.m_dFlow = pclSelectedSeparator->GetQ();
		UpdateFlowFieldValue();
	}
	else
	{
		m_clIndSelSeparatorParams.m_dPower = pclSelectedSeparator->GetPower();
		UpdatePowerFieldValue();

		m_clIndSelSeparatorParams.m_dDT = pclSelectedSeparator->GetDT();
		UpdateDTFieldValue();
	}

	m_bInitialised = true;

	// By default now we automatically enable apply button.
	EnableSelectButton( true );
	
	// Remark: the 'm_pclIndSelParams->m_bEditModeRunning' variable is updated by calling the 'SetModificationMode' method.
	SetModificationMode( true );
	PostWMCommandToControl( GetpBtnSuggest() );

	return 0;
}

void CDlgIndSelSeparator::OnBnClickedSuggest() 
{
	ClearAll();
	
 	if( true == _VerifySubmittedValues() )
 	{
		m_clIndSelSeparatorParams.m_SeparatorList.PurgeAll();

		m_clIndSelSeparatorParams.m_pTADB->GetSeparatorList( 
				&m_clIndSelSeparatorParams.m_SeparatorList, 
				(LPCTSTR)m_clIndSelSeparatorParams.m_strComboTypeID, 
				(LPCTSTR)m_clIndSelSeparatorParams.m_strComboFamilyID,
				(LPCTSTR)m_clIndSelSeparatorParams.m_strComboConnectID,
				(LPCTSTR)m_clIndSelSeparatorParams.m_strComboVersionID,
				m_clIndSelSeparatorParams.m_eFilterSelection );
		
		_SuggestIndividualSelection();
	}
}

void CDlgIndSelSeparator::OnBnClickedSelect() 
{
	// Verify the dialog is active.
	if( FALSE == this->IsWindowVisible() )
	{
		return;
	}

	_SelectIndividualSelection();
}

LRESULT CDlgIndSelSeparator::OnNewDocument( WPARAM wParam, LPARAM lParam )
{
	CDlgIndSelBase::OnNewDocument( wParam, lParam );

	if( false == TASApp.IsAirVentSepDisplayed() )
	{
		return 0;
	}
	
	// Get last selected parameters.
	CString str1, str2, str3;
	CDS_IndSelParameter *pclIndSelParameter = m_clIndSelSeparatorParams.m_pTADS->GetpIndSelParameter();

	if( NULL == pclIndSelParameter )
	{
		return -1;
	}

	m_iRadioFlowPowerDT = ( CDS_SelProd::efdFlow == (CDS_SelProd::eFlowDef)pclIndSelParameter->GetSepRadioFlowPowerDT() ) ? Radio_Flow : Radio_PowerDT;

	// 'FALSE' to change the radio state in the dialog with the content of the 'm_iRadioFlowPowerDT' variable.
	UpdateData( FALSE );
	
	// Remark: 'm_clIndSelSeparatorParams.m_eFlowOrPowerDTMode' is updated in the following method in regards to the 'm_iRadioFlowPowerDT' variable.
	CDlgIndSelBase::UpdateFlowOrPowerDTState();

	// Remark: Corresponding combo variables in 'm_clIndSelBVParams' are updated in each of this following methods.
	_FillComboType( pclIndSelParameter->GetSeparatorTypeID() );
	_FillComboFamily( pclIndSelParameter->GetSeparatorFamilyID() );
	_FillComboConnect( pclIndSelParameter->GetSeparatorConnectID() );
	_FillComboVersion( pclIndSelParameter->GetSeparatorVersionID() );

	// Fill the flow and Dp unit static controls and update water TCHAR. strings.
	OnUnitChange();

	m_bInitialised = true;

	if( NULL != pRViewSSelSeparator )
	{
		pRViewSSelSeparator->OnNewDocument( pclIndSelParameter );
	}

	_VerifyFluidCharacteristics( false );

	return 0;
}

LRESULT CDlgIndSelSeparator::OnPipeChange( WPARAM wParam, LPARAM lParam )
{
	if( WMUserPipeChange::WM_UPC_ForProductSelection != (WMUserPipeChange)wParam )
	{
		return 0;
	}

	CDlgIndSelBase::OnPipeChange( wParam, lParam );
	
	if( NULL != pRViewSSelSeparator )
	{
		pRViewSSelSeparator->Reset();
	}
	
	return 0;
}

LRESULT CDlgIndSelSeparator::OnTechParChange( WPARAM wParam, LPARAM lParam )
{
	CDlgIndSelBase::OnTechParChange( wParam, lParam );

	if( NULL != pRViewSSelSeparator )
	{
		pRViewSSelSeparator->Reset();
	}

	return 0;
}

LRESULT CDlgIndSelSeparator::OnUnitChange( WPARAM wParam, LPARAM lParam )
{
	CDlgIndSelBase::OnUnitChange( wParam, lParam );

	if( NULL != pRViewSSelSeparator )
	{
		pRViewSSelSeparator->Reset();
	}

	return 0;
}

LRESULT CDlgIndSelSeparator::OnWaterChange( WPARAM wParam, LPARAM lParam )
{
	// HYS-1693: Not for changeOver application type
	if( WMUserWaterCharWParam::WM_UWC_WP_ForProductSel != ( (WMUserWaterCharWParam)wParam & WMUserWaterCharWParam::WM_UWC_WP_ForProductSel ) 
		|| WMUserWaterCharLParam::WM_UWC_LWP_ChangeForChangeOver == (WMUserWaterCharLParam)lParam
		|| WMUserWaterCharLParam::WM_UWC_LWP_NoChangeForChangeOver == (WMUserWaterCharLParam)lParam )
	{
		return 0;
	}

	CDlgIndSelBase::OnWaterChange( wParam, lParam );

	if( NULL != pRViewSSelSeparator )
	{
		pRViewSSelSeparator->Reset();
	}

	_VerifyFluidCharacteristics( false );

	return 0;
}

CRViewSSelSS *CDlgIndSelSeparator::GetLinkedRightViewSSel( void ) 
{ 
	return pRViewSSelSeparator;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CDlgIndSelSeparator::_SuggestIndividualSelection()
{
	if( NULL == pRViewSSelSeparator )
	{
		return false;
	}

	if( NULL != m_clIndSelSeparatorParams.m_pclSelectSeparatorList )
	{
		delete m_clIndSelSeparatorParams.m_pclSelectSeparatorList;
	}

	m_clIndSelSeparatorParams.m_pclSelectSeparatorList = new CSelectSeparatorList();
	
	if( NULL == m_clIndSelSeparatorParams.m_pclSelectSeparatorList || NULL == m_clIndSelSeparatorParams.m_pclSelectSeparatorList->GetSelectPipeList() )
	{
		return false;
	}

	m_clIndSelSeparatorParams.m_pclSelectSeparatorList->GetSelectPipeList()->SelectPipes( &m_clIndSelSeparatorParams, m_clIndSelSeparatorParams.m_dFlow );

	bool bSizeShiftProblem = false;
	bool bBestFound = false;

	int iDevFound = m_clIndSelSeparatorParams.m_pclSelectSeparatorList->SelectSeparator( &m_clIndSelSeparatorParams, &bSizeShiftProblem, bBestFound );

	if( 0 != iDevFound )
	{
		VerifyModificationMode();
		pRViewSSelSeparator->Suggest( &m_clIndSelSeparatorParams );
	}
	else
	{
		m_clIndSelSeparatorParams.m_SeparatorList.PurgeAll();

		m_clIndSelSeparatorParams.m_pTADB->GetSeparatorList( &m_clIndSelSeparatorParams.m_SeparatorList, (LPCTSTR)m_clIndSelSeparatorParams.m_strComboTypeID, 
				_T(""), _T(""), _T(""), m_clIndSelSeparatorParams.m_eFilterSelection );

		iDevFound = m_clIndSelSeparatorParams.m_pclSelectSeparatorList->SelectSeparator( &m_clIndSelSeparatorParams, &bSizeShiftProblem, bBestFound );
		
		if( iDevFound > 0 )
		{
			// HYS-697: Temporary solution because it is not practical to pass through 2 lists to go in the 'CDlgNoDevSeparatorFound' dialog.
			m_clIndSelSeparatorParams.m_SeparatorList.PurgeAll();
			double dKey = 0.0;

			for( CSelectedSeparator *pclSelectedSeparator = m_clIndSelSeparatorParams.m_pclSelectSeparatorList->GetFirst<CSelectedSeparator>(); NULL != pclSelectedSeparator;
				pclSelectedSeparator = m_clIndSelSeparatorParams.m_pclSelectSeparatorList->GetNext<CSelectedSeparator>() )
			{
				CDB_TAProduct *pTAP = dynamic_cast<CDB_TAProduct *>( pclSelectedSeparator->GetpData() );

				if( NULL == pTAP )
				{
					continue;
				}

				m_clIndSelSeparatorParams.m_SeparatorList.Add( pTAP->GetIDPtr().ID, dKey++, (LPARAM)pTAP->GetIDPtr().MP );
			}

			// Launch the no device found dialog box if something is found.
			// Remark: Combos in this dialog will be filled thanks to the new alternative separators found and saved in the 'm_clIndSelSeparatorParams.m_SeparatorList' variable.
			CDlgNoDevSeparatorFoundParams clNoDevSeparatorParams = CDlgNoDevSeparatorFoundParams( &m_clIndSelSeparatorParams.m_SeparatorList, 
					m_clIndSelSeparatorParams.m_strComboTypeID, m_clIndSelSeparatorParams.m_strComboFamilyID, m_clIndSelSeparatorParams.m_strComboConnectID, 
					m_clIndSelSeparatorParams.m_strComboVersionID );

			CDlgNoDevSeparatorFound dlg( &m_clIndSelSeparatorParams, &clNoDevSeparatorParams );

			if( IDOK == dlg.DoModal() )
			{
				clNoDevSeparatorParams = *dlg.GetNoDevSeparatorFoundParams();

				_LaunchSuggestion( clNoDevSeparatorParams.m_strTypeID, clNoDevSeparatorParams.m_strFamilyID, clNoDevSeparatorParams.m_strConnectID, 
						clNoDevSeparatorParams.m_strVersionID );
			}
		}
		else if( true == bSizeShiftProblem )
		{
			// In that case valves exist but not with the parameters specified.
			// Problem of SizeShift.
			if( IDYES == TASApp.AfxLocalizeMessageBox( AFXMSG_SIZE_SHIFT_PROB, MB_YESNO | MB_ICONQUESTION, 0 ) )
			{
				CDlgTechParam *pdlg = new CDlgTechParam( CDlgTechParam::PageGeneralDeviceSizes );

				if( IDOK != pdlg->DoModal() )
				{
					// Set the focus on the first available edit control.
					SetFocusOnControl( CDlgSelectionBase::SetFocusWhere::First | CDlgSelectionBase::SetFocusWhere::Edit );
				}

				delete pdlg;
			}
		}
		else
		{
			// No device found message.
			TASApp.AfxLocalizeMessageBox( AFXMSG_NO_DEV_FOUND );
		}
	}

	return true;
}

bool CDlgIndSelSeparator::_SelectIndividualSelection()
{
	try
	{
		if( NULL == m_clIndSelSeparatorParams.m_pTADS )
		{
			HYSELECT_THROW( _T("Internal error: 'm_clIndSelSeparatorParams.m_pTADS' argument can't be NULL.") );
		}
		else if( NULL == m_clIndSelSeparatorParams.m_pTADS->GetpTechParams() )
		{
			HYSELECT_THROW( _T("Internal error: 'm_clIndSelSeparatorParams.m_pTADS->GetpTechParams()' argument can't be NULL.") );
		}

		// Verify the dialog is active.
		if( NULL == pRViewSSelSeparator )
		{
			HYSELECT_THROW( _T("Internal error: The air vent & separator right view is not created.") );
		}
		else if( FALSE == this->IsWindowVisible() )
		{
			HYSELECT_THROW( _T("Internal error: The air vent & separator right view is not visible.") );
		}

		// Verify pointer validity; user can delete object after entering in edition.
		if( _T('\0') != *m_clIndSelSeparatorParams.m_SelIDPtr.ID )
		{
			m_clIndSelSeparatorParams.m_SelIDPtr = m_clIndSelSeparatorParams.m_pTADS->Get( m_clIndSelSeparatorParams.m_SelIDPtr.ID );
		}
	
		// Create selected object and initialize it.
		CDS_SSelAirVentSeparator *pSelSeparator = dynamic_cast<CDS_SSelAirVentSeparator *>( m_clIndSelSeparatorParams.m_SelIDPtr.MP );

		if( NULL == pSelSeparator )
		{
			IDPTR IDPtr;
			m_clIndSelSeparatorParams.m_pTADS->CreateObject( IDPtr, CLASS( CDS_SSelAirVentSeparator ) );
			pSelSeparator = (CDS_SSelAirVentSeparator *)( IDPtr.MP );
		}
	
		if( m_clIndSelSeparatorParams.m_dFlow <= 0.0 ) 
		{
			HYSELECT_THROW( _T("Internal error: Flow '%f' can't be lower or equal to 0."), m_clIndSelSeparatorParams.m_dFlow );
		}
	
		pSelSeparator->SetQ( m_clIndSelSeparatorParams.m_dFlow );

		if( CDS_SelProd::efdFlow == m_clIndSelSeparatorParams.m_eFlowOrPowerDTMode )
		{
			pSelSeparator->SetFlowDef( CDS_SelProd::efdFlow );
			pSelSeparator->SetPower( 0.0 );
			pSelSeparator->SetDT( 0.0 );
		}
		else
		{
			pSelSeparator->SetFlowDef( CDS_SelProd::efdPower );
			pSelSeparator->SetPower( m_clIndSelSeparatorParams.m_dPower );
			pSelSeparator->SetDT( m_clIndSelSeparatorParams.m_dDT );
		}

		pSelSeparator->SetTypeID( m_clIndSelSeparatorParams.m_strComboTypeID );
		pSelSeparator->SetFamilyID( m_clIndSelSeparatorParams.m_strComboFamilyID );
		pSelSeparator->SetConnectID( m_clIndSelSeparatorParams.m_strComboConnectID );
		pSelSeparator->SetVersionID( m_clIndSelSeparatorParams.m_strComboVersionID );
		pSelSeparator->SetPipeSeriesID( m_clIndSelSeparatorParams.m_strPipeSeriesID );
		pSelSeparator->SetPipeID( m_clIndSelSeparatorParams.m_strPipeID );

		*pSelSeparator->GetpSelectedInfos()->GetpWCData() = m_clIndSelSeparatorParams.m_WC;
	
		// We save here the real DT in the ribbon to be able to restore it when we edit a selection.
		// Remark: the DT in the ribbon is never used in the computing. It is the DT in the left panel that is used when
		//         user chooses the Power/DT mode input.
		pSelSeparator->GetpSelectedInfos()->SetDT( m_clIndSelSeparatorParams.m_pTADS->GetpTechParams()->GetDefaultISDT() );

		// For the same reason, we also save the application type.
		pSelSeparator->GetpSelectedInfos()->SetApplicationType( m_clIndSelSeparatorParams.m_eApplicationType );

		pRViewSSelSeparator->FillInSelected( pSelSeparator );

		CDlgConfSel dlg( &m_clIndSelSeparatorParams );
		IDPTR IDPtr = pSelSeparator->GetIDPtr();
		dlg.Display( pSelSeparator );

		if( IDOK == dlg.DoModal() )
		{
			CTable *pTab = dynamic_cast<CTable *>( m_clIndSelSeparatorParams.m_pTADS->Get( _T("AIRVENTSEP_TAB") ).MP );
			
			if( NULL == pTab )
			{
				HYSELECT_THROW( _T("Internal error: Can't retrieve the 'AIRVENTSEP_TAB' table from the datastruct.") );
			}
		
			if( _T('\0') == *m_clIndSelSeparatorParams.m_SelIDPtr.ID )
			{
				pTab->Insert( IDPtr );
				m_clIndSelSeparatorParams.m_pTADS->SetNewIndex( CLASS( CDS_SSelAirVentSeparator ) );
			}

			// It's only in case user has clicked 'OK' button that we can set modification mode to 'false'. Otherwise if user
			// has clicked 'Cancel' button, we must stay in edition mode and not activate 'Select' button.
			if( true == m_clIndSelSeparatorParams.m_bEditModeRunning )
			{
				SetModificationMode( false );
				PostWMCommandToControl( GetpBtnSuggest() );
			}

			// Enable 'Select' button.
			EnableSelectButton( true );
		}
		else
		{
			if( _T('\0') == *m_clIndSelSeparatorParams.m_SelIDPtr.ID )
			{
				m_clIndSelSeparatorParams.m_pTADS->DeleteObject( IDPtr );
			}
		}

		if( NULL != dlg.GetSafeHwnd() )
		{
			dlg.DestroyWindow();
		}

		return true;
	}
	catch( CHySelectException &clHySelectException )
	{
		clHySelectException.AddMessage( _T("Error in 'CDlgIndSelSeparator::_SelectIndividualSelection'."), __LINE__, __FILE__ );
		throw;
	}
}

void CDlgIndSelSeparator::_FillComboType( CString strTypeID )
{
	if( NULL == m_clIndSelSeparatorParams.m_pTADB )
	{
		ASSERT_RETURN;
	}

	CRankEx TypeList;
	m_clIndSelSeparatorParams.m_pTADB->GetSeparatorTypeList( &TypeList, m_clIndSelSeparatorParams.m_eFilterSelection );
	m_ComboType.FillInCombo( &TypeList, strTypeID, m_clIndSelSeparatorParams.GetComboTypeAllID() );
	m_clIndSelSeparatorParams.m_strComboTypeID = m_ComboType.GetCBCurSelIDPtr().ID;

	CString str( _T("") );

	if( CB_ERR != m_ComboType.GetCurSel() )
	{
		m_ComboType.GetLBText( m_ComboType.GetCurSel(), str );

		if( NULL != m_ToolTip.GetSafeHwnd() )
		{
			m_ToolTip.UpdateTipText( (LPCTSTR)str, &m_ComboType );
		}
	}
}

void CDlgIndSelSeparator::_FillComboFamily( CString strFamilyID )
{
	if( NULL == m_clIndSelSeparatorParams.m_pTADB )
	{
		ASSERT_RETURN;
	}

	// Fill family according type.
	CRankEx FamilyList;
	m_clIndSelSeparatorParams.m_pTADB->GetSeparatorFamilyList( &FamilyList, (LPCTSTR)m_clIndSelSeparatorParams.m_strComboTypeID, 
			m_clIndSelSeparatorParams.m_eFilterSelection );
	
	m_ComboFamily.FillInCombo( &FamilyList, strFamilyID, m_clIndSelSeparatorParams.GetComboFamilyAllID() );
	m_clIndSelSeparatorParams.m_strComboFamilyID = m_ComboFamily.GetCBCurSelIDPtr().ID;
}

void CDlgIndSelSeparator::_FillComboConnect( CString strConnectID )
{
	if( NULL == m_clIndSelSeparatorParams.m_pTADB )
	{
		ASSERT_RETURN;
	}

	// Fill connection according type and family.
	CRankEx ConnectionList;

	m_clIndSelSeparatorParams.m_pTADB->GetSeparatorConnList( &ConnectionList, (LPCTSTR)m_clIndSelSeparatorParams.m_strComboTypeID, 
			(LPCTSTR)m_clIndSelSeparatorParams.m_strComboFamilyID, m_clIndSelSeparatorParams.m_eFilterSelection );
	
	m_ComboConnect.FillInCombo( &ConnectionList, strConnectID, m_clIndSelSeparatorParams.GetComboConnectAllID() );
	m_clIndSelSeparatorParams.m_strComboConnectID = m_ComboConnect.GetCBCurSelIDPtr().ID;
}

void CDlgIndSelSeparator::_FillComboVersion( CString strVersionID )
{
	if( NULL == m_clIndSelSeparatorParams.m_pTADB )
	{
		ASSERT_RETURN;
	}

	// Fill the version combo according to selected type, family and connection.
	CRankEx VersionList;

	m_clIndSelSeparatorParams.m_pTADB->GetSeparatorVersList( &VersionList, (LPCTSTR)m_clIndSelSeparatorParams.m_strComboTypeID, 
			(LPCTSTR)m_clIndSelSeparatorParams.m_strComboFamilyID, (LPCTSTR)m_clIndSelSeparatorParams.m_strComboConnectID, 
			m_clIndSelSeparatorParams.m_eFilterSelection );
	
	m_ComboVersion.FillInCombo( &VersionList, strVersionID, m_clIndSelSeparatorParams.GetComboVersionAllID() );
	m_clIndSelSeparatorParams.m_strComboVersionID = m_ComboVersion.GetCBCurSelIDPtr().ID;
}

void CDlgIndSelSeparator::_LaunchSuggestion( CString strTypeID, CString strFamilyID, CString strConnectID, CString strVersionID )
{
	_FillComboType( strTypeID );
	_FillComboFamily( strFamilyID );
	_FillComboConnect( strConnectID );
	_FillComboVersion( strVersionID );

	PostWMCommandToControl( GetpBtnSuggest() );
}

bool CDlgIndSelSeparator::_VerifySubmittedValues()
{
	bool bValid = true;

	if( m_clIndSelSeparatorParams.m_dFlow <= 0.0 )
	{
		TASApp.AfxLocalizeMessageBox( AFXMSG_NO_SUGGESTION_AT_ZERO_FLOW );
		GetDlgItem( IDC_EDITFLOW )->SetFocus();
		bValid = false;
		GetDlgItem( IDC_BUTTONSUGGEST )->EnableWindow( true );
	}

	return bValid;
}

void CDlgIndSelSeparator::_VerifyFluidCharacteristics( bool bShowErrorMsg )
{
	bool bEnable = true;
	CString strMsg = _T("");
	_VerifyFluidCharacteristicsHelper( bEnable, strMsg );

	if( true == bEnable )
	{
		m_clButtonSuggest.EnableWindow( TRUE );
		m_clButtonSuggest.ModifyStyle( BS_OWNERDRAW, 0 );
		m_clButtonSuggest.SetTextColor( _BLACK );
		m_clButtonSuggest.ResetDrawBorder();

		if( NULL != m_ToolTip.GetSafeHwnd() )
		{
			m_ToolTip.DelTool( &m_clButtonSuggest );
		}
	}
	else
	{
		m_clButtonSuggest.EnableWindow( FALSE );
		m_clButtonSuggest.ModifyStyle( 0, BS_OWNERDRAW );
		m_clButtonSuggest.SetTextColor( _RED );
		m_clButtonSuggest.SetBackColor( RGB( 204, 204, 204 ) );
		m_clButtonSuggest.SetDrawBorder( true, _RED );

		if( NULL != m_ToolTip.GetSafeHwnd() )
		{
			m_ToolTip.AddToolWindow( &m_clButtonSuggest, strMsg );
		}

		if( true == bShowErrorMsg )
		{
			AfxMessageBox( strMsg );
		}
	}
}

void CDlgIndSelSeparator::_VerifyFluidCharacteristicsHelper( bool &bEnable, CString &strMsg, CDlgWaterChar *pclDlgWaterChar )
{
	CString strAdditiveFamilyID = _T("");
	
	if( NULL != pclDlgWaterChar )
	{
		strAdditiveFamilyID = pclDlgWaterChar->GetAdditiveFamilyID();
	}
	else
	{
		strAdditiveFamilyID = m_clIndSelSeparatorParams.m_WC.GetAdditFamID();
	}

	bEnable = true;
	strMsg = _T("");

	// HYS-1041: Disable suggest button when the additive is not a Glycol.
	// HYS-1111: 2019-06-05: Christian Thesing and Norbert Ramser allow alcohol for PAG products.
	if( 0 != StringCompare( strAdditiveFamilyID, _T("GLYCOL_ADDIT") ) 
			&& 0 != StringCompare( strAdditiveFamilyID, _T("ALCOHOL_ADDIT") )
			&& 0 != StringCompare( strAdditiveFamilyID, _T("WATER_ADDIT") )
			&& 0 != StringCompare( strAdditiveFamilyID, _T("OTHER_ADDIT") ) )
	{
		strMsg = TASApp.LoadLocalizedString( AFXMSG_BAD_ADDITIVE );
		bEnable = false;
	}
}
