#include "stdafx.h"
#include <float.h>
#include <math.h>

#include "TASelect.h"
#include "MainFrm.h"
#include "Global.h"
#include "SelectPM.h"

#include "DlgNoDevFound.h"
#include "DlgConfSel.h"

#include "DlgLeftTabSelManager.h"

#include "RViewSSelSS.h"
#include "RViewSSelPM.h"

#include "DlgSelectionBase.h"
#include "DlgIndSelBase.h"
#include "DlgIndSelPM.h"
#include "DlgWizardSelPM.h"

#include "DlgIndSelPMPanels.h"

// TO REMOVE
#include "DlgPMGraphsOutput.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CDlgIndSelPressureMaintenance::CDlgIndSelPressureMaintenance( CWnd *pParent )
	: CDlgSelectionBase( m_clIndSelPMParams, CDlgIndSelPressureMaintenance::IDD, pParent )
{
	m_clIndSelPMParams.m_eProductSubCategory = ProductSubCategory::PSC_PM_ExpansionVessel;
	m_clIndSelPMParams.m_eProductSelectionMode = ProductSelectionMode::ProductSelectionMode_Individual;

	m_rInterfacePMPanels.m_pclParent = this;
	m_rInterfacePMPanels.m_pfnClearAll = &CDlgSelectionBase::ClearAll;
	m_rInterfacePMPanels.m_pfnGetLinkedRightView = &CDlgIndSelPressureMaintenance::GetLinkedRightViewSSel;
	m_eCurrentPanel = CDlgIndSelPMPanelBase::ePanelLast;
}

CDlgIndSelPressureMaintenance::~CDlgIndSelPressureMaintenance()
{
	for( int i = 0; i < ( int )m_mapPanels.size(); i++ )
	{
		if( NULL != m_mapPanels[i] )
		{
			delete m_mapPanels[i];
			m_mapPanels[i] = NULL;
		}
	}
}

void CDlgIndSelPressureMaintenance::SetApplicationBackground( COLORREF cBackColor )
{
	CDlgSelectionBase::SetApplicationBackground( cBackColor );

	for( auto &iter : m_mapPanels )
	{
		iter.second->SetBackgroundColor( cBackColor );
	}
}

void CDlgIndSelPressureMaintenance::ClickOnSuggestButton( void )
{
	if( WS_DISABLED != ( WS_DISABLED & m_clButtonSuggest.GetStyle() ) )
	{
		m_clButtonSuggest.SetFocus();
		OnBnClickedSuggest();
	}
}

void CDlgIndSelPressureMaintenance::EnableSelectButton( bool bEnabled )
{
	if( NULL != GetDlgItem( IDC_BUTTONSELECT ) && NULL != GetDlgItem( IDC_BUTTONSELECT )->GetSafeHwnd() )
	{
		GetDlgItem( IDC_BUTTONSELECT )->EnableWindow( ( true == bEnabled ) ? TRUE : FALSE );
	}
}

bool CDlgIndSelPressureMaintenance::IsButtonSelectEnabled( void )
{
	if( NULL == GetDlgItem( IDC_BUTTONSELECT ) || NULL == GetDlgItem( IDC_BUTTONSELECT )->GetSafeHwnd() )
	{
		return false;
	}

	if( FALSE == GetDlgItem( IDC_BUTTONSELECT )->IsWindowEnabled() )
	{
		return false;
	}

	return true;
}

void CDlgIndSelPressureMaintenance::VerifyModificationMode()
{
	// Object still exist?
	m_clIndSelPMParams.m_SelIDPtr = m_clIndSelPMParams.m_pTADS->Get( m_clIndSelPMParams.m_SelIDPtr.ID );

	if( NULL == m_clIndSelPMParams.m_SelIDPtr.MP )
	{
		if( NULL != GetDlgItem( IDC_BUTTONSELECT ) && NULL != GetDlgItem( IDC_BUTTONSELECT )->GetSafeHwnd() )
		{
			GetDlgItem( IDC_BUTTONSELECT )->SetWindowText( TASApp.LoadLocalizedString( IDS_TABCDIALOGSSEL_BUTTONSELECT ) );
			GetDlgItem( IDC_BUTTONSELECT )->EnableWindow( FALSE );
		}

		if( NULL != GetDlgItem( IDC_BUTTONCANCEL ) && NULL != GetDlgItem( IDC_BUTTONCANCEL )->GetSafeHwnd() )
		{
			GetDlgItem( IDC_BUTTONCANCEL )->ShowWindow( SW_HIDE );
		}

		m_clIndSelPMParams.m_bEditModeRunning = false;
	}
	else
	{
		if( NULL != GetDlgItem( IDC_BUTTONSELECT ) && NULL != GetDlgItem( IDC_BUTTONSELECT )->GetSafeHwnd() )
		{
			GetDlgItem( IDC_BUTTONSELECT )->SetWindowText( TASApp.LoadLocalizedString( IDS_TABCDIALOGSSEL_APPLYMODIFICATION ) );
		}

		if( NULL != GetDlgItem( IDC_BUTTONCANCEL ) && NULL != GetDlgItem( IDC_BUTTONCANCEL )->GetSafeHwnd() )
		{
			GetDlgItem( IDC_BUTTONCANCEL )->ShowWindow( SW_SHOW );
			GetDlgItem( IDC_BUTTONCANCEL )->EnableWindow( TRUE );
			GetDlgItem( IDC_BUTTONCANCEL )->SetWindowText( TASApp.LoadLocalizedString( IDS_CANCEL ) );
		}

		m_clIndSelPMParams.m_bEditModeRunning = true;
	}
}

void CDlgIndSelPressureMaintenance::SetNorm( CString strNormID )
{
	for( int iLoop = 0; iLoop < m_ComboNorm.GetCount(); iLoop++ )
	{
        CDB_StringID *pStrID = dynamic_cast< CDB_StringID *>( (CData *)m_ComboNorm.GetItemData( iLoop ) );

		if( NULL != pStrID && strNormID == CString( pStrID->GetIDPtr().ID ) )
		{
			m_ComboNorm.SetCurSel( iLoop );
			m_clIndSelPMParams.m_pclSelectPMList->GetpclPMInputUser()->SetNormID( pStrID->GetIDPtr().ID );
			_ChangeComboPressMaintType();
			break;
		}
	}
}

CString CDlgIndSelPressureMaintenance::GetDlgCaption()
{
	if( NULL == m_clIndSelPMParams.m_pTADS || NULL == m_clIndSelPMParams.m_pTADS->GetpTechParams() )
	{
		ASSERTA_RETURN( _T("" ) );
	}

	CDS_TechnicalParameter *pTechP = m_clIndSelPMParams.m_pTADS->GetpTechParams();
	CString strCaption = TASApp.LoadLocalizedString( IDS_LEFTTABTITLE_PRESSURISATION ) + _T(" - ");
	strCaption += TASApp.LoadLocalizedString( pTechP->GetProductSelectionApplicationTypeIDS( pTechP->GetProductSelectionApplicationType() ) );
	return strCaption;
}

void CDlgIndSelPressureMaintenance::SaveSelectionParameters()
{
	if( NULL == m_clIndSelPMParams.m_pclSelectPMList || NULL == m_clIndSelPMParams.m_pclSelectPMList->GetpclPMInputUser() 
			|| NULL == m_clIndSelPMParams.m_pclSelectPMList->GetpclPMInputUser()->GetPMWQSelectionPreferences() )
	{
		ASSERT_RETURN;
	}

	CDlgSelectionBase::SaveSelectionParameters();
	CDS_IndSelParameter *pclIndSelParameter = m_clIndSelPMParams.m_pTADS->GetpIndSelParameter();

	if( NULL == pclIndSelParameter )
	{
		return;
	}

	// Save parameters.
	CPMInputUser *pclPMInputUser = m_clIndSelPMParams.m_pclSelectPMList->GetpclPMInputUser();
	CPMWQPrefs *pclPMWQPrefs = pclPMInputUser->GetPMWQSelectionPreferences();

	pclIndSelParameter->SetPMApplicationType( pclPMInputUser->GetApplicationType() );
	pclIndSelParameter->SetPMPressMaintTypeID( pclPMInputUser->GetPressureMaintenanceTypeID() );
	pclIndSelParameter->SetPMWaterMakeUpTypeID( pclPMInputUser->GetWaterMakeUpTypeID() );
	pclIndSelParameter->SetPMDegassingChecked( pclPMInputUser->GetDegassingChecked() );
	pclIndSelParameter->SetPMWaterMakeupBreakTank( pclPMWQPrefs->IsChecked( CPMWQPrefs::WaterMakeupBreakTank ) );
	pclIndSelParameter->SetPMNorm( m_ComboNorm.GetCBCurSelIDPtr().ID );

	// Save parameters for each panels.
	for( std::map<int, CDlgIndSelPMPanelBase *>::iterator It = m_mapPanels.begin(); m_mapPanels.end() != It; ++It )
	{
		( It->second )->SaveSelectionParameters();
	}

	// HYS-1058: Save also the pressurisation maintenance and water quality preferences.
	pclIndSelParameter->SetPMWQPrefs( pclPMWQPrefs );

	// Save parameters for the right view.
	if( NULL != pRViewSSelPM )
	{
		pRViewSSelPM->SaveSelectionParameters( pclIndSelParameter );
	}
}

void CDlgIndSelPressureMaintenance::LeaveLeftTabDialog()
{
	// Set that we use variables in 'CPMSelectionHelper' with the wizard selection mode.
	if( NULL != pDlgLeftTabSelManager )
	{
		pDlgLeftTabSelManager->GetPMSelectionHelper()->SetLastSelectionMode( CPMSelectionHelper::LSM_Individual );
	}
}

void CDlgIndSelPressureMaintenance::ActivateLeftTabDialog()
{
	if( NULL == pDlgLeftTabSelManager || NULL == m_clIndSelPMParams.m_pclSelectPMList 
			|| NULL == m_clIndSelPMParams.m_pclSelectPMList->GetpclPMInputUser() 
			|| NULL == m_clIndSelPMParams.m_pTADS || NULL == m_clIndSelPMParams.m_pTADS->GetpIndSelParameter() )
	{
		ASSERT_RETURN;
	}

	CDlgSelectionBase::ActivateLeftTabDialog();

	// Verify if we have the good panel in regards to the current product selection application type set in the ribbon.
	if( true == m_clIndSelPMParams.m_pclSelectPMList->GetpclPMInputUser()->IsPressurisationSystemExist() )
	{
		CDS_TechnicalParameter *pTechP = m_clIndSelPMParams.m_pTADS->GetpTechParams();
		ASSERT( NULL != pTechP );

		ProjectType eCurrentPSApplicationType = pTechP->GetProductSelectionApplicationType();

		if( ( ProjectType::Cooling == eCurrentPSApplicationType && CDlgIndSelPMPanelBase::ePanelCooling != m_eCurrentPanel && CDlgIndSelPMPanelBase::ePanelCoolingSWKI != m_eCurrentPanel )
				|| ( ProjectType::Heating == eCurrentPSApplicationType && CDlgIndSelPMPanelBase::ePanelHeating != m_eCurrentPanel && CDlgIndSelPMPanelBase::ePanelHeatingSWKI != m_eCurrentPanel )
				|| ( ProjectType::Solar == eCurrentPSApplicationType && CDlgIndSelPMPanelBase::ePanelSolar != m_eCurrentPanel && CDlgIndSelPMPanelBase::ePanelSolarSWKI != m_eCurrentPanel ) )
		{
			ChangeApplicationType( eCurrentPSApplicationType );
		}
	}

	if( CPMSelectionHelper::LSM_Wizard == pDlgLeftTabSelManager->GetPMSelectionHelper()->GetLastSelectionMode() )
	{
		_UpdateWithWizardSelPMState();
		// HYS-1537 : To copy all preferences in Heatin, Cooling and Solar mode
		UpdateWithWizardAllPrefInAllMode();
	}

	// Second parameters: do not display popup with error message if exist.
	if( NULL != m_clIndSelPMParams.m_pclSelectPMList && NULL != m_clIndSelPMParams.m_pclSelectPMList->GetpclPMInputUser() )
	{
		m_mapPanels[m_eCurrentPanel]->VerifyPMInputUser( m_clIndSelPMParams.m_pclSelectPMList->GetpclPMInputUser(), false );
	}

	if( this == pDlgLeftTabSelManager->GetCurrentLeftTabDialog() )
	{
		_VerifyFluidCharacteristics( false );
	}
}

void CDlgIndSelPressureMaintenance::ChangeApplicationType( ProjectType eProjectType )
{
	CDlgSelectionBase::ChangeApplicationType( eProjectType );
	
	if( NULL == pDlgLeftTabSelManager || NULL == m_clIndSelPMParams.m_pclSelectPMList 
			|| NULL == m_clIndSelPMParams.m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERT_RETURN;
	}

	m_clIndSelPMParams.m_pclSelectPMList = pDlgLeftTabSelManager->GetPMSelectionHelper()->GetSelectPMList( ProductSelectionMode_Individual, eProjectType );

	// Copy also water characteristic in 'CPMInputUser'.
	*( m_clIndSelPMParams.m_pclSelectPMList->GetpclPMInputUser()->GetpWC() ) = m_clIndSelPMParams.m_WC;

	bool bRedraw = false;

	if( TRUE == IsWindowVisible() && NULL != pDlgLTtabctrl )
	{
		CString str = GetDlgCaption();
		pDlgLTtabctrl->SetWindowText( str );
		bRedraw = true;
	}

	_VerifyPMWQPrefs();

	// First parameter: redraw or not, second parameter: do not display popup with error messages if exist.
	_UpdateAllFields( bRedraw, false );

	// Allow us to know in which mode we are when going in the wizard mode.
	m_clIndSelPMParams.m_pTADS->GetpIndSelParameter()->SetPMApplicationType( eProjectType );

	if( NULL != pRViewSSelPM )
	{
		pRViewSSelPM->Reset();
	}
	
	// Verify now fluid characteristic and temperatures.
	_VerifyFluidCharacteristics( false );
}

void CDlgIndSelPressureMaintenance::OnRViewSSelSelectProduct( void )
{
	// Verify the dialog is active.
	if( FALSE == this->IsWindowVisible() )
	{
		return;
	}

	OnBnClickedSelect();
}

void CDlgIndSelPressureMaintenance::OnRViewSSelSelectionChanged( bool bSelectionActive )
{
	// Verify the dialog is active.
	if( FALSE == this->IsWindowVisible() )
	{
		return;
	}

	EnableSelectButton( bSelectionActive );
}

void CDlgIndSelPressureMaintenance::OnRViewSSelLostFocusWithTabKey( bool bShiftPressed )
{
	// !!!! Does not intentionally call the base class !!!

	// Verify the dialog is active.
	if( FALSE == this->IsWindowVisible() )
	{
		return;
	}

	// Reset the focus on the left tab.
	if( NULL != pDlgLeftTabSelManager )
	{
		pDlgLeftTabSelManager->SetFocus();
	}

	if( false == bShiftPressed )
	{
		// Set the focus on the first available edit control.
		SetFocus();
		SetFocusOnControl( SetFocusWhere::First );
	}
	else
	{
		// SHIFT + TAB -> must set the focus on the 'CDlgInSelPMPanels' and set the focus on the last control.
		m_mapPanels[m_eCurrentPanel]->SetFocus();
		m_mapPanels[m_eCurrentPanel]->SetFocusOnControl( CDlgSelectionTabHelper::SetFocusWhere::Last );
	}
}

void CDlgIndSelPressureMaintenance::OnRViewSSelKeyboardEvent( int iKeyboardVirtualKey )
{
	// Verify the dialog is active.
	if( FALSE == this->IsWindowVisible() )
	{
		return;
	}

	switch( iKeyboardVirtualKey )
	{
		case VK_RETURN:
			if( true == IsButtonSelectEnabled() )
			{
				PostWMCommandToControl( GetDlgItem( IDC_BUTTONSELECT ) );
			}
			else
			{
				PostWMCommandToControl( &m_clButtonSuggest );
			}

			break;

		case VK_ESCAPE:
			// Set the focus on the first available edit control.
			SetFocusOnControl( CDlgSelectionBase::SetFocusWhere::First | CDlgSelectionBase::SetFocusWhere::Edit );
			break;
	}
}

void CDlgIndSelPressureMaintenance::OnVerifyFluidCharacteristics( bool &bEnable, CString &strMsg, CDlgWaterChar *pclDlgWaterChar )
{
	_VerifyFluidCharacteristicsHelper( bEnable, strMsg, pclDlgWaterChar );
}

bool CDlgIndSelPressureMaintenance::ResetRightView()
{
	bool bReturn = false;

	if( NULL != pRViewSSelPM )
	{
		if( false == pRViewSSelPM->IsEmpty() )
		{
			pRViewSSelPM->Reset();
		}

		pRViewSSelPM->RegisterNotificationHandler( this, CRViewSSelSS::INotificationHandler::NH_All );
		bReturn = true;
	}

	return bReturn;
}

CMainFrame::RightViewList CDlgIndSelPressureMaintenance::GetRViewID()
{
	return CMainFrame::eRVSSelPresureMaintenance;
}

bool CDlgIndSelPressureMaintenance::LeftTabKillFocus( bool bNext )
{
	// !!! Does not intentionally call the base class !!!

	bool bReturn = false;

	if( true == bNext )
	{
		// TAB -> must set the focus on the 'CDlgInSelPMPanels' and set the focus on the first control.
		m_mapPanels[m_eCurrentPanel]->SetFocus();
		m_mapPanels[m_eCurrentPanel]->SetFocusOnControl( CDlgSelectionTabHelper::SetFocusWhere::First );
		bReturn = true;
	}
	else
	{
		// SHIFT + TAB -> go to the top control only if right view is not empty.
		if( NULL != pRViewSSelPM && TRUE == pRViewSSelPM->IsWindowVisible() && false == pRViewSSelPM->IsEmpty() )
		{
			// Set the focus on the right view.
			pRViewSSelPM->SetFocus();
			// Set focus on the appropriate group in the right view.
			pRViewSSelPM->SetFocusW( false );
			bReturn = true;
		}
		else
		{
			// Focus must be set on the last control of the 'CDlgIndSelPMPanels'.
			m_mapPanels[m_eCurrentPanel]->SetFocus();
			m_mapPanels[m_eCurrentPanel]->SetFocusOnControl( CDlgSelectionTabHelper::SetFocusWhere::Last );
			bReturn = true;
		}
	}

	return bReturn;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PROTECTED METHODS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP( CDlgIndSelPressureMaintenance, CDlgSelectionBase )
	ON_MESSAGE( WM_USER_MODIFYSELECTEDPM, OnModifySelectedPMProduct )
	ON_MESSAGE( WM_USER_PMWQSELECTIONPREFSCHANGE, OnPMWQSelectionPrefsChange )
	ON_BN_CLICKED( IDC_BUTTONSUGGEST, OnBnClickedSuggest )
	ON_BN_CLICKED( IDC_BUTTONSELECT, OnBnClickedSelect )
	ON_BN_CLICKED( IDC_BUTTONRESET, OnBnClickedReset )
	ON_BN_CLICKED( IDC_BUTTONCANCEL, OnBnClickedCancel )
	ON_CBN_SELCHANGE( IDC_CB_PMTYPE, OnCbnSelChangePMType )
	ON_CBN_SELCHANGE( IDC_CB_WATERMAKEUP, OnCbnSelChangeWaterMakeUpType )
	ON_BN_CLICKED( IDC_CHECKDEGASSING, OnClickedCheckDegassing )
	ON_CBN_SELCHANGE( IDC_COMBO_NORM2, OnCbnSelChangeNorm )
END_MESSAGE_MAP()

void CDlgIndSelPressureMaintenance::DoDataExchange( CDataExchange *pDX )
{
	CDlgSelectionBase::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_CB_PMTYPE, m_ComboPMType );
	DDX_Control( pDX, IDC_BUTTONSUGGEST, m_clButtonSuggest );
	DDX_Control( pDX, IDC_CB_WATERMAKEUP, m_ComboWaterMakeUpType );
	DDX_Control( pDX, IDC_CHECKDEGASSING, m_DegassingCheckbox );
	DDX_Control( pDX, IDC_COMBO_NORM2, m_ComboNorm );
}

BOOL CDlgIndSelPressureMaintenance::OnInitDialog()
{
	CDlgSelectionBase::OnInitDialog();

	GetDlgItem( IDC_STATIC_PMTYPE )->SetWindowText( TASApp.LoadLocalizedString( IDS_PMGENTAB_PMTYPE ) );
	GetDlgItem( IDC_STATIC_OPTIONALFCT )->SetWindowText( TASApp.LoadLocalizedString( IDS_PMGENTAB_GROUPOPTIONALFCT ) );
	GetDlgItem( IDC_STATIC_WATERMAKEUP )->SetWindowText( TASApp.LoadLocalizedString( IDS_PMGENTAB_WATERMAKEUP ) );
	GetDlgItem( IDC_CHECKDEGASSING )->SetWindowText( TASApp.LoadLocalizedString( IDS_PMGENTAB_CHECKDEGASSING ) );
	GetDlgItem( IDC_STATIC_NORM )->SetWindowText( TASApp.LoadLocalizedString( IDS_PMHEATING_NORM ) );
	m_clButtonSuggest.SetWindowText( TASApp.LoadLocalizedString( IDS_PMGENTAB_BUTTONSUGGEST ) );
	GetDlgItem( IDC_BUTTONSELECT )->SetWindowText( TASApp.LoadLocalizedString( IDS_PMGENTAB_BUTTONSELECT ) );
	GetDlgItem( IDC_BUTTONRESET )->SetWindowText( TASApp.LoadLocalizedString( IDS_PMGENTAB_BUTTONRESET ) );
	GetDlgItem( IDC_BUTTONCANCEL )->SetWindowText( TASApp.LoadLocalizedString( IDS_PMGENTAB_BUTTONCANCEL ) );

	// Create all panels.
	_CreateAllPanels();

	for( auto &iter : m_mapPanels )
	{
		iter.second->SetInterface( ( LPARAM )&m_rInterfacePMPanels );
	}

	InitToolTips();

	return TRUE;
}

LRESULT CDlgIndSelPressureMaintenance::OnPMWQSelectionPrefsChange( WPARAM wParam, LPARAM lParam )
{
	if( NULL == m_clIndSelPMParams.m_pclSelectPMList || NULL == m_clIndSelPMParams.m_pclSelectPMList->GetpclPMInputUser() 
		|| NULL == m_clIndSelPMParams.m_pclSelectPMList->GetpclPMInputUser()->GetPMWQSelectionPreferences() )
	{
		return TRUE;
	}

	CPMWQPrefs *pclPMWQPrefs = m_clIndSelPMParams.m_pclSelectPMList->GetpclPMInputUser()->GetPMWQSelectionPreferences();

	if( CMainFrame::ChangeFromRibbonOneDevice == wParam )
	{
		pclPMWQPrefs->SetChecked( CPMWQPrefs::GeneralCombinedInOneDevice, ( 0 == lParam ) ? false : true );
		_VerifySuggestButton();
	}

	// Disable the 'Select' (or 'Apply' if in edition mode) button.
	EnableSelectButton( false );

	if( NULL != pRViewSSelPM )
	{
		pRViewSSelPM->Reset();
	}

	return TRUE;
}

LRESULT CDlgIndSelPressureMaintenance::OnModifySelectedPMProduct( WPARAM wParam, LPARAM lParam )
{
	if( NULL == pDlgLeftTabSelManager || NULL == m_pclProdSelParam || NULL == m_pclProdSelParam->m_pTADS 
			|| NULL == m_pclProdSelParam->m_pTADS->GetpTechParams() )
	{
		ASSERTA_RETURN( 0 );
	}

	CDS_SSelPMaint *pclSSelPMaint = dynamic_cast<CDS_SSelPMaint *>( (CData *)lParam );
	ASSERT( NULL != pclSSelPMaint );

	if( NULL == pclSSelPMaint || NULL == pDlgLTtabctrl )
	{
		return 0;
	}

	CPMInputUser *pclPMInputUser = pclSSelPMaint->GetpInputUser();

	if( NULL == pclPMInputUser )
	{
		return 0;
	}

	// Verify that Application type is available in the current database.
	if( pclPMInputUser->GetApplicationType() != ( m_clIndSelPMParams.m_pTADS->GetpTechParams()->GetAvailablePMPanels() & pclPMInputUser->GetApplicationType() ) )
	{
		return 0;
	}

	m_bInitialised = false;

	// Do this before calling 'pDlgLTtabctrl->SetActiveTab' below. Because when the 'ActivateLeftTabDialog' base class will be called,
	// it needs the current application type to set the tab title.
	m_clIndSelPMParams.m_pTADS->GetpTechParams()->SetProductSelectionApplicationType( pclPMInputUser->GetApplicationType() );

	// This methods will fire an active tab event and the 'ActivateLeftTabDialog' method will be called.
	pDlgLTtabctrl->SetActiveTab( CMyMFCTabCtrl::TabIndex::etiSSel );

	// If we are not in the good selection mode, we must first disable it.
	if( ProductSelectionMode_Individual != pMainFrame->GetCurrentProductSelectionMode() )
	{
		pMainFrame->ChangeProductSelectionMode( ProductSelectionMode_Individual );
	}

	// HYS-1242: SetRedraw to false to avoid too much refresh in ClickRibbonProductSelGallery. 
	pMainFrame->SetRedraw( FALSE );
	// Select the right item in the gallery box.
	pMainFrame->ClickRibbonProductSelGallery( ProductSubCategory::PSC_PM_ExpansionVessel );

	// Select 'Heating, 'Cooling' or 'Solar'.
	pMainFrame->ChangeRbnProductSelectionApplicationType( pclPMInputUser->GetApplicationType() );

	m_clIndSelPMParams.m_SelIDPtr = pclSSelPMaint->GetIDPtr();

	// Because we call the 'ClickRibbonSSelApplicationType' above, the application type is already known.
	CPMInputUser *pclGlobalPMInputUser = pDlgLeftTabSelManager->GetPMSelectionHelper()->GetPMInputUser( ProductSelectionMode_Individual, pclPMInputUser->GetApplicationType() );

	if( NULL != pclGlobalPMInputUser )
	{
		pclGlobalPMInputUser->CopyFrom( pclPMInputUser );
	}

	m_clIndSelPMParams.m_pclSelectPMList = pDlgLeftTabSelManager->GetPMSelectionHelper()->GetSelectPMList( ProductSelectionMode_Individual, pclPMInputUser->GetApplicationType() );
	m_clIndSelPMParams.m_WC = *m_clIndSelPMParams.m_pclSelectPMList->GetpclPMInputUser()->GetpWC();

	// Copy water characteristic and temperature data in the technical parameters.
	OnModifySelectedProductUpdateTechData( pclSSelPMaint->GetpSelectedInfos() );

	// HYS-1242: To refresh the window in InitializeRibbonRichEditBoxesSSel
	pMainFrame->SetRedraw( TRUE );
	// Remark: Water characteristics has been already copied above with 'pclGlobalPMInputUser->CopyFrom( pclPMInputUser )'.
	pMainFrame->InitializeRibbonRichEditBoxesSSel();

	_VerifyPMWQPrefs();

	// First parameter: redraw, second parameter: display popup with error messages if exist.
	_UpdateAllFields( true, true );

	m_bInitialised = true;

	// By default now we automatically enable apply button.
	EnableSelectButton( true );

	// Remark: the 'm_pclIndSelParams->m_bEditModeRunning' variable is updated by calling the 'SetModificationMode' method.
	SetModificationMode( true );
	PostWMCommandToControl( &m_clButtonSuggest );

	return 0;
}

void CDlgIndSelPressureMaintenance::OnBnClickedSuggest()
{
	if( NULL == pRViewSSelPM )
	{
		return;
	}
	
	ClearAll();

	if( NULL == m_clIndSelPMParams.m_pclSelectPMList || NULL == m_clIndSelPMParams.m_pclSelectPMList->GetpclPMInputUser() )
	{
		return;
	}
	
	// Second parameters: display popup with error message if exist.
	if( false == m_mapPanels[m_eCurrentPanel]->VerifyPMInputUser( m_clIndSelPMParams.m_pclSelectPMList->GetpclPMInputUser(), true ) )
	{
		return;
	}

	int iDevFound = m_clIndSelPMParams.m_pclSelectPMList->Select();

	if( 0 != iDevFound )
	{
		pRViewSSelPM->Suggest( &m_clIndSelPMParams );
	}
}

void CDlgIndSelPressureMaintenance::OnBnClickedSelect()
{
	try
	{
		// Verify the dialog is active.
		if( FALSE == this->IsWindowVisible() || NULL == m_clIndSelPMParams.m_pclSelectPMList
				|| NULL == m_clIndSelPMParams.m_pclSelectPMList->GetpclPMInputUser() )
		{
			return;
		}

		// HYS-1427: Verify PAZ accessory on Transfero TI if we are in EN 12952 norm.
		if( false == _VerifyPAZAccessory() )
		{
			return;
		}
		
		if( false == _VerifyComCubeDMLConnectAccessory() )
		{
			return;
		}

		// Verify pointer validity; user can delete object after entering in edition.
		if( _T('\0') != *m_clIndSelPMParams.m_SelIDPtr.ID )
		{
			m_clIndSelPMParams.m_SelIDPtr = m_clIndSelPMParams.m_pTADS->Get( m_clIndSelPMParams.m_SelIDPtr.ID );
		}

		CDS_SSelPMaint *pclSSelPMaint = dynamic_cast<CDS_SSelPMaint *>( m_clIndSelPMParams.m_SelIDPtr.MP );

		// If object doesn't exist, create a new one and initialize it.
		if( NULL == pclSSelPMaint )
		{
			IDPTR IDPtr;
			m_clIndSelPMParams.m_pTADS->CreateObject( IDPtr, CLASS( CDS_SSelPMaint ) );
			pclSSelPMaint = (CDS_SSelPMaint *)( IDPtr.MP );
		}

		if( NULL == pclSSelPMaint )
		{
			return;
		}

		// Must be done before calling 'FillInSelected' because this method uses some variables in it.
		pclSSelPMaint->GetpInputUser()->CopyFrom( m_clIndSelPMParams.m_pclSelectPMList->GetpclPMInputUser() );

		*pclSSelPMaint->GetpSelectedInfos()->GetpWCData() = m_clIndSelPMParams.m_WC;

		// HYS-1054: Registrer DT value in pSelectedInfos
		pclSSelPMaint->GetpSelectedInfos()->SetDT( m_clIndSelPMParams.m_pTADS->GetpTechParams()->GetDefaultISDT() );

		if( NULL != pRViewSSelPM )
		{
			pRViewSSelPM->FillInSelected( pclSSelPMaint );
		}

		// This variable must be set after the call to the 'FillInSelected' method because this one reset all.
		pclSSelPMaint->SetSelectionMode( ProductSelectionMode_Individual );

		CDlgConfSel dlg( &m_clIndSelPMParams );
		IDPTR IDPtr = pclSSelPMaint->GetIDPtr();
		dlg.Display( pclSSelPMaint );

		if( IDOK == dlg.DoModal() )
		{
			CTable *pTab = dynamic_cast<CTable *>( m_clIndSelPMParams.m_pTADS->Get( _T("PRESSMAINT_TAB") ).MP );
			
			if( NULL == pTab )
			{
				HYSELECT_THROW( _T("Internal error: Can't retrieve the 'PRESSMAINT_TAB' table from the datastruct.") );
			}

			if( _T('\0') == *m_clIndSelPMParams.m_SelIDPtr.ID )
			{
				pTab->Insert( IDPtr );
			}

			m_clIndSelPMParams.m_pTADS->SetNewIndex( CLASS( CDS_SSelPMaint ) );

			// It's only in case user has clicked 'OK' button that we can set modification mode to 'false'. Otherwise if user
			// has clicked 'Cancel' button, we must stay in edition mode and not activate 'Select' button.
			if( true == m_clIndSelPMParams.m_bEditModeRunning )
			{
				SetModificationMode( false );
				PostWMCommandToControl( &m_clButtonSuggest );
			}

			// Enable 'Select' button.
			EnableSelectButton( true );
		}
		else
		{
			if( _T('\0') == *m_clIndSelPMParams.m_SelIDPtr.ID )
			{
				m_clIndSelPMParams.m_pTADS->DeleteObject( IDPtr );
			}
		}

		if( NULL != dlg.GetSafeHwnd() )
		{
			dlg.DestroyWindow();
		}
	}
	catch( CHySelectException &clHySelectException )
	{
		clHySelectException.AddMessage( _T("Error in 'CDlgIndSelPressureMaintenance::OnBnClickedSelect'."), __LINE__, __FILE__ );
		throw;
	}
}

void CDlgIndSelPressureMaintenance::OnBnClickedReset()
{
	ClearAll();

	// Disable the 'Select' (or 'Apply' if in edition mode) button.
	EnableSelectButton( false );

	if( true == m_clIndSelPMParams.m_bEditModeRunning )
	{
		SetModificationMode( false );
	}

	if( NULL != pDlgLeftTabSelManager )
	{
		pDlgLeftTabSelManager->GetPMSelectionHelper()->Reset( ProductSelectionMode_Individual );
	}

	_VerifyPMWQPrefs();
	_UpdateAllFields( true, false );
}

void CDlgIndSelPressureMaintenance::OnBnClickedCancel()
{
	// User cancel edition of an individual selection.
	SetModificationMode( false );
	PostWMCommandToControl( &m_clButtonSuggest );
}

void CDlgIndSelPressureMaintenance::OnCbnSelChangePMType()
{
	SetRedraw( FALSE );
	
	ClearAll();

	// Disable the 'Select' (or 'Apply' if in edition mode) button.
	EnableSelectButton( false );

	_ChangeComboPressMaintType();

	_VerifySuggestButton();
	_VerifyPMWQPrefs();

	// Notify the current panel.
	_UpdateAllFields( true, false );

	SetRedraw( TRUE );
	Invalidate();
	UpdateWindow();
}

void CDlgIndSelPressureMaintenance::OnCbnSelChangeWaterMakeUpType()
{
	SetRedraw( FALSE );

	ClearAll();

	// Disable the 'Select' (or 'Apply' if in edition mode) button.
	EnableSelectButton( false );

	_ChangeComboWaterMakeUpType();

	_VerifySuggestButton();
	_VerifyPMWQPrefs();

	SetRedraw( TRUE );
	Invalidate();
	UpdateWindow();
}

void CDlgIndSelPressureMaintenance::OnClickedCheckDegassing()
{
	SetRedraw( FALSE );
	ClearAll();

	// Disable the 'Select' (or 'Apply' if in edition mode) button.
	EnableSelectButton( false );

	_ChangeButtonDegassing();

	_VerifySuggestButton();
	_VerifyPMWQPrefs();

	SetRedraw( TRUE );
	Invalidate();
	UpdateWindow();
}

void CDlgIndSelPressureMaintenance::OnCbnSelChangeNorm()
{
	m_clIndSelPMParams.m_pclSelectPMList->GetpclPMInputUser()->SetNormID( _GetNormID() );
	_UpdateAllFields( true, false );

	m_mapPanels[m_eCurrentPanel]->OnNormChanged();

	_VerifyFluidCharacteristics( false );
}

LRESULT CDlgIndSelPressureMaintenance::OnNewDocument( WPARAM wParam, LPARAM lParam )
{
	if( NULL == pDlgLeftTabSelManager )
	{
		ASSERTA_RETURN( -1 );
	}

	CDlgSelectionBase::OnNewDocument( wParam, lParam );

	CDS_IndSelParameter *pclIndSelParameter = m_clIndSelPMParams.m_pTADS->GetpIndSelParameter();

	if( NULL == pclIndSelParameter )
	{
		ASSERTA_RETURN( -1 );
	}

	// Retrieve the last application type used by the user.
	ProjectType eProjectType = pclIndSelParameter->GetPMApplicationType();

	// HYS-1350: We reset all to avoid previous errors.
	pDlgLeftTabSelManager->GetPMSelectionHelper()->Reset( ProductSelectionMode_Individual, eProjectType );

	// Now retrieve variables for the current application type.
	m_clIndSelPMParams.m_pclSelectPMList = pDlgLeftTabSelManager->GetPMSelectionHelper()->GetSelectPMList( ProductSelectionMode_Individual, eProjectType );
	
	if( NULL == m_clIndSelPMParams.m_pclSelectPMList )
	{
		ASSERTA_RETURN( -1 );
	}

	// Update variables with last user choices.
	m_clIndSelPMParams.m_pclSelectPMList->GetpclPMInputUser()->SetPressureMaintenanceTypeID( pclIndSelParameter->GetPMPressMaintTypeID() );
	m_clIndSelPMParams.m_pclSelectPMList->GetpclPMInputUser()->SetWaterMakeUpTypeID( pclIndSelParameter->GetPMWaterMakeUpTypeID() );
	m_clIndSelPMParams.m_pclSelectPMList->GetpclPMInputUser()->SetDegassingChecked( pclIndSelParameter->GetPMDegassingChecked() );
	m_clIndSelPMParams.m_pclSelectPMList->GetpclPMInputUser()->SetNormID( pclIndSelParameter->GetPMNorm() );
	m_clIndSelPMParams.m_pclSelectPMList->GetpclPMInputUser()->SetPressOn( pclIndSelParameter->GetPMPressOn() );

	// Copy also water characteristic in 'CPMInputUser'.
	*( m_clIndSelPMParams.m_pclSelectPMList->GetpclPMInputUser()->GetpWC() ) = m_clIndSelPMParams.m_WC;

	m_bInitialised = true;

	InitToolTips();

	if( NULL != pRViewSSelPM )
	{
		pRViewSSelPM->SetParent( this );
		pRViewSSelPM->OnNewDocument( pclIndSelParameter );
	}

	// HYS-1058: Read also the pressurisation maintenance and water quality preferences.
	m_clIndSelPMParams.m_pclSelectPMList->GetpclPMInputUser()->GetPMWQSelectionPreferences()->CopyFrom( pclIndSelParameter->GetpPMWQPrefs() );

	_VerifyPMWQPrefs();

	for( auto &iter : m_mapPanels )
	{
		iter.second->OnNewDocument();
	}

	// First parameter: No redraw, second parameter: do not display popup with error messages if exist.
	_UpdateAllFields( false, false );

	return 0;
}

void CDlgIndSelPressureMaintenance::InitToolTips( CWnd *pWnd )
{
	if( NULL != m_ToolTip.GetSafeHwnd() && NULL != m_clIndSelPMParams.m_pclSelectPMList 
			&& NULL != m_clIndSelPMParams.m_pclSelectPMList->GetpclPMInputUser() 
			&& Solar == m_clIndSelPMParams.m_pclSelectPMList->GetpclPMInputUser()->GetApplicationType() )
	{
		CString TTstr;
		TTstr = TASApp.LoadLocalizedString( IDS_PM_SOLAR_WARNING_TECBOX );
		m_ToolTip.AddToolWindow( &m_ComboPMType, TTstr );
	}
}

LRESULT CDlgIndSelPressureMaintenance::OnPipeChange( WPARAM wParam, LPARAM lParam )
{
	if( WMUserPipeChange::WM_UPC_ForProductSelection != ( WMUserPipeChange )wParam )
	{
		return 0;
	}

	CDlgSelectionBase::OnPipeChange( wParam, lParam );

	// Disable the 'Select' (or 'Apply' if in edition mode) button.
	EnableSelectButton( false );

	if( NULL != pRViewSSelPM )
	{
		pRViewSSelPM->Reset();
	}

	return 0;
}

LRESULT CDlgIndSelPressureMaintenance::OnTechParChange( WPARAM wParam, LPARAM lParam )
{
	CDlgSelectionBase::OnTechParChange( wParam, lParam );

	_VerifyPMWQPrefs();

    // First parameter: redraw or not, second parameter: do not popup with error messages if exist.
	_UpdateAllFields( ( TRUE == IsWindowVisible() ) ? true : false, false );

	// Disable the 'Select' (or 'Apply' if in edition mode) button.
	EnableSelectButton( false );

	if( NULL != pRViewSSelPM )
	{
		pRViewSSelPM->Reset();
	}

	return 0;
}

LRESULT CDlgIndSelPressureMaintenance::OnUnitChange( WPARAM wParam, LPARAM lParam )
{
	CDlgSelectionBase::OnUnitChange( wParam, lParam );

	// Disable the 'Select' (or 'Apply' if in edition mode) button.
	EnableSelectButton( false );

	if( NULL != pRViewSSelPM )
	{
		pRViewSSelPM->Reset();
	}

	return 0;
}

LRESULT CDlgIndSelPressureMaintenance::OnWaterChange( WPARAM wParam, LPARAM lParam )
{
	if( WMUserWaterCharWParam::WM_UWC_WP_ForProductSel != ( ( WMUserWaterCharWParam )wParam & WMUserWaterCharWParam::WM_UWC_WP_ForProductSel ) 
			|| NULL == m_clIndSelPMParams.m_pclSelectPMList || NULL == m_clIndSelPMParams.m_pclSelectPMList->GetpclPMInputUser() 
			|| NULL == m_clIndSelPMParams.m_pTADS || NULL == m_clIndSelPMParams.m_pTADS->GetpTechParams() )
	{
		return 0;
	}

	// HYS-1693: Not for changeOver application type
	if( WMUserWaterCharLParam::WM_UWC_LWP_ChangeForChangeOver == (WMUserWaterCharLParam)lParam
		|| WMUserWaterCharLParam::WM_UWC_LWP_NoChangeForChangeOver == (WMUserWaterCharLParam)lParam )
	{
		return 0;
	}

	CDlgSelectionBase::OnWaterChange( wParam, lParam );

	// HYS-1380: Get the m_pclSelectPMList from PMSelecionHelper to update m_clIndSelPMParams.m_pclSelectPMList. It is necessary when we 
	// change this on the anywhere else.
	CDS_TechnicalParameter *pTechParams = m_clIndSelPMParams.m_pTADS->GetpTechParams();
	m_clIndSelPMParams.m_pclSelectPMList = pDlgLeftTabSelManager->GetPMSelectionHelper()->GetSelectPMList( ProductSelectionMode_Individual, pTechParams->GetProductSelectionApplicationType() );

	CPMInputUser *pclPMInputUser = m_clIndSelPMParams.m_pclSelectPMList->GetpclPMInputUser();

	switch( pclPMInputUser->GetApplicationType() )
	{
		case ProjectType::Heating:
			pclPMInputUser->SetSupplyTemperature( pTechParams->GetDefaultISHeatingTps() );
			pclPMInputUser->SetReturnTemperature( pTechParams->GetDefaultISHeatingTps() - pTechParams->GetDefaultISHeatingDT() );
			break;

		case ProjectType::Solar:
			pclPMInputUser->SetSupplyTemperature( pTechParams->GetDefaultISSolarTps() );
			pclPMInputUser->SetReturnTemperature( pTechParams->GetDefaultISSolarTps() - pTechParams->GetDefaultISSolarDT() );
			break;

		case ProjectType::Cooling:
			pclPMInputUser->SetSupplyTemperature( pTechParams->GetDefaultISCoolingTps() );
			pclPMInputUser->SetReturnTemperature( pTechParams->GetDefaultISCoolingTps() + pTechParams->GetDefaultISCoolingDT() );
			
			if( false == pclPMInputUser->GetCheckMinTemperature() )
			{
				pclPMInputUser->SetMinTemperature( m_clIndSelPMParams.m_WC.GetTfreez() );
			}
			
			break;
	}

	// Copy also water characteristic in 'CPMInputUser'.
	*( m_clIndSelPMParams.m_pclSelectPMList->GetpclPMInputUser()->GetpWC() ) = m_clIndSelPMParams.m_WC;

	_VerifyPMWQPrefs();

	// First parameter: redraw or not, second parameter: do not popup with error messages if exist.
	_UpdateAllFields( ( TRUE == IsWindowVisible() ) ? true : false, false );
		
	// Disable the 'Select' (or 'Apply' if in edition mode) button.
	EnableSelectButton( false );

	if( NULL != pRViewSSelPM )
	{
		pRViewSSelPM->Reset();
	}

	return 0;
}

CRViewSSelSS *CDlgIndSelPressureMaintenance::GetLinkedRightViewSSel( void )
{
	return pRViewSSelPM;
}

void CDlgIndSelPressureMaintenance::SetModificationMode( bool bFlag )
{
	if( false == bFlag )
	{
		m_clIndSelPMParams.m_SelIDPtr = _NULL_IDPTR;
	}

	VerifyModificationMode();
}

void CDlgIndSelPressureMaintenance::_CreateAllPanels()
{
	// Create a new panel instance for heating.
	m_mapPanels[CDlgIndSelPMPanelBase::ePanelHeating] = new CDlgIndSelPMPanelHeating( &m_clIndSelPMParams, this );
	
	if( NULL == m_mapPanels[CDlgIndSelPMPanelBase::ePanelHeating] )
	{
		ASSERT_RETURN;
	}

	m_mapPanels[CDlgIndSelPMPanelBase::ePanelHeating]->Create( IDD_DLGINDSELPRESSMAINTHEATING, this );

	if( NULL == m_mapPanels[CDlgIndSelPMPanelBase::ePanelHeating]->GetSafeHwnd() )
	{
		ASSERT_RETURN;
	}

	// Create a new panel instance for heating SWKI.
	m_mapPanels[CDlgIndSelPMPanelBase::ePanelHeatingSWKI] = new CDlgIndSelPMPanelHeatingSWKI( &m_clIndSelPMParams, this );
	
	if( NULL == m_mapPanels[CDlgIndSelPMPanelBase::ePanelHeatingSWKI] )
	{
		ASSERT_RETURN;
	}

	m_mapPanels[CDlgIndSelPMPanelBase::ePanelHeatingSWKI]->Create( IDD_DLGINDSELPRESSMAINTHEATINGSWKI, this );

	if( NULL == m_mapPanels[CDlgIndSelPMPanelBase::ePanelHeatingSWKI]->GetSafeHwnd() )
	{
		ASSERT_RETURN;
	}

	// Create a new panel instance for cooling.
	m_mapPanels[CDlgIndSelPMPanelBase::ePanelCooling] = new CDlgIndSelPMPanelCooling( &m_clIndSelPMParams, this );
	
	if( NULL == m_mapPanels[CDlgIndSelPMPanelBase::ePanelCooling] )
	{
		ASSERT_RETURN;
	}

	m_mapPanels[CDlgIndSelPMPanelBase::ePanelCooling]->Create( IDD_DLGINDSELPRESSMAINTCOOLING, this );

	if( NULL == m_mapPanels[CDlgIndSelPMPanelBase::ePanelCooling]->GetSafeHwnd() )
	{
		ASSERT_RETURN;
	}

	// Create a new panel instance for cooling SWKI.
	m_mapPanels[CDlgIndSelPMPanelBase::ePanelCoolingSWKI] = new CDlgIndSelPMPanelCoolingSWKI( &m_clIndSelPMParams, this );
	
	if( NULL == m_mapPanels[CDlgIndSelPMPanelBase::ePanelCoolingSWKI] )
	{
		ASSERT_RETURN;
	}

	m_mapPanels[CDlgIndSelPMPanelBase::ePanelCoolingSWKI]->Create( IDD_DLGINDSELPRESSMAINTCOOLINGSWKI, this );

	if( NULL == m_mapPanels[CDlgIndSelPMPanelBase::ePanelCoolingSWKI]->GetSafeHwnd() )
	{
		ASSERT_RETURN;
	}

	// Create a new panel instance for solar.
	m_mapPanels[CDlgIndSelPMPanelBase::ePanelSolar] = new CDlgIndSelPMPanelSolar( &m_clIndSelPMParams, this );
	
	if( NULL == m_mapPanels[CDlgIndSelPMPanelBase::ePanelSolar] )
	{
		ASSERT_RETURN;
	}

	m_mapPanels[CDlgIndSelPMPanelBase::ePanelSolar]->Create( IDD_DLGINDSELPRESSMAINTSOLAR, this );

	if( NULL == m_mapPanels[CDlgIndSelPMPanelBase::ePanelSolar]->GetSafeHwnd() )
	{
		ASSERT_RETURN;
	}

	// Create a new panel instance for solar SWKI.
	m_mapPanels[CDlgIndSelPMPanelBase::ePanelSolarSWKI] = new CDlgIndSelPMPanelSolarSWKI( &m_clIndSelPMParams, this );
	
	if( NULL == m_mapPanels[CDlgIndSelPMPanelBase::ePanelSolarSWKI] )
	{
		ASSERT_RETURN;
	}

	m_mapPanels[CDlgIndSelPMPanelBase::ePanelSolarSWKI]->Create( IDD_DLGINDSELPRESSMAINTSOLARSWKI, this );

	if( NULL == m_mapPanels[CDlgIndSelPMPanelBase::ePanelSolarSWKI]->GetSafeHwnd() )
	{
		ASSERT_RETURN;
	}

	// Create a new panel instance for no pressurisation maintenance system.
	m_mapPanels[CDlgIndSelPMPanelBase::ePanelNoPressMaint] = new CDlgIndSelPMNo( &m_clIndSelPMParams, this );
	
	if( NULL == m_mapPanels[CDlgIndSelPMPanelBase::ePanelNoPressMaint] )
	{
		ASSERT_RETURN;
	}

	m_mapPanels[CDlgIndSelPMPanelBase::ePanelNoPressMaint]->Create( IDD_DLGINDSELPRESSMAINTNO, this );

	if( NULL == m_mapPanels[CDlgIndSelPMPanelBase::ePanelNoPressMaint]->GetSafeHwnd() )
	{
		ASSERT_RETURN;
	}

	// Move and size in regards to the 'IDC_STATIC_ANCHORBOX'.
	CWnd *pWnd = GetDlgItem( IDC_STATIC_ANCHORBOX );
	ASSERT( NULL != pWnd );
	ASSERT( IsWindow( pWnd->m_hWnd ) );

	CRect rect;
	pWnd->GetWindowRect( &rect );
	ScreenToClient( &rect );
	
	for( auto &iter : m_mapPanels )
	{
		iter.second->SetWindowPos( NULL, rect.left, rect.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE );
		iter.second->EnableWindow( TRUE );
	}
}

CDlgIndSelPMPanelBase::PanelType CDlgIndSelPressureMaintenance::_GetPanelToShow()
{
	if( NULL == m_clIndSelPMParams.m_pclSelectPMList || NULL == m_clIndSelPMParams.m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERTA_RETURN( CDlgIndSelPMPanelBase::PanelType::ePanelLast );
	}

	CDlgIndSelPMPanelBase::PanelType ePanelType = CDlgIndSelPMPanelBase::PanelType::ePanelLast;

	if( 0 == CString( m_ComboPMType.GetCBCurSelIDPtr().ID ).Compare( _T("PMT_NONE_TAB") ) )
	{
		ePanelType = CDlgIndSelPMPanelBase::PanelType::ePanelNoPressMaint;
	}
	else
	{
		PressurisationNorm eNorm = (PressurisationNorm)m_clIndSelPMParams.m_pclSelectPMList->GetpclPMInputUser()->GetNorm();

		switch( m_clIndSelPMParams.m_pclSelectPMList->GetpclPMInputUser()->GetApplicationType() )
		{
			case ProjectType::Heating:
				if( PressurisationNorm::PN_SWKIHE301_01 == eNorm )
				{
					ePanelType = CDlgIndSelPMPanelBase::PanelType::ePanelHeatingSWKI;
				}
				else
				{
					ePanelType = CDlgIndSelPMPanelBase::PanelType::ePanelHeating;
				}
				break;

			case ProjectType::Cooling:
				if( PressurisationNorm::PN_SWKIHE301_01 == eNorm )
				{
					ePanelType = CDlgIndSelPMPanelBase::PanelType::ePanelCoolingSWKI;
				}
				else
				{
					ePanelType = CDlgIndSelPMPanelBase::PanelType::ePanelCooling;
				}
				break;

			case ProjectType::Solar:
				if( PressurisationNorm::PN_SWKIHE301_01 == eNorm )
				{
					ePanelType = CDlgIndSelPMPanelBase::PanelType::ePanelSolarSWKI;
				}
				else
				{
					ePanelType = CDlgIndSelPMPanelBase::PanelType::ePanelSolar;
				}
				break;
		}
	}

	return ePanelType;
}

void CDlgIndSelPressureMaintenance::_FillComboPressMaintType( CString strPressureTypeID )
{
	if( NULL == m_clIndSelPMParams.m_pclSelectPMList || NULL == m_clIndSelPMParams.m_pclSelectPMList->GetpclPMInputUser() 
			|| NULL == m_clIndSelPMParams.m_pTADB )
	{
		ASSERT_RETURN;
	}

	m_ComboPMType.ResetContent();
	IDPTR SelectedIDPtr;

	if( false == strPressureTypeID.IsEmpty() )
	{
		SelectedIDPtr = m_clIndSelPMParams.m_pTADB->Get( ( LPCTSTR )strPressureTypeID );
	}

	// Fill combo pressure maintenance type.
	std::map<int, CData *> MapLst;
	CTableOrdered *pTabOrd = ( CTableOrdered * )( m_clIndSelPMParams.m_pTADB->Get( _T("PRESSMAINTTYPECB_TAB") ).MP );

	if( NULL != pTabOrd )
	{
		pTabOrd->FillMapListOrdered( &MapLst );
	}
	else
	{
		ASSERT_RETURN;
	}

	// Extract all PM product tab from VSSLTYPE_TAB.
	// Remark: 'VSSLTYPE_TAB' contains 'Expansion vessel', 'Expansion vessel with membrane', 'Primary vessel', 'Secondary vessel',
	//         'Intermediate vessel' and 'Aqua vessel'.
	CTable *pTabType = ( CTable * )( m_clIndSelPMParams.m_pTADB->Get( _T("VSSLTYPE_TAB") ).MP );

	if( NULL == pTabType )
	{
		ASSERT_RETURN;
	}

	// Use ordered set to remove duplicated entry.
	std::set<CTable *> setVesselType;

	for( IDPTR IDPtr = pTabType->GetFirst(); NULL != IDPtr.MP; IDPtr = pTabType->GetNext( IDPtr.MP ) )
	{
		CDB_StringID *pStrID = dynamic_cast<CDB_StringID *>( IDPtr.MP );
		CTable *pTab = dynamic_cast<CTable *>( m_clIndSelPMParams.m_pTADB->Get( pStrID->GetIDstr2() ).MP );
		setVesselType.insert( pTab );
	}

	// For table present in the setPMPTabID verify product availability	and store corresponding type.
	std::set<CData *> setPMPTypeID;

	for each( CTable * pTab in setVesselType )
	{
		if( NULL == pTab )
		{
			ASSERT_CONTINUE;
		}

		for( IDPTR IDPtr = pTab->GetFirst(); NULL != IDPtr.MP; IDPtr = pTab->GetNext( IDPtr.MP ) )
		{
			CDB_Product *pPMP = dynamic_cast<CDB_Product *>( IDPtr.MP );

			if( NULL == pPMP )
			{
				ASSERT_CONTINUE;
			}

			if( false == pPMP->IsAvailable() )
			{
				continue;
			}

			setPMPTypeID.insert( pPMP->GetTypeIDPtr().MP );
		}
	}

	// Identify un-available CB entry.
	int iSelPos = 0;

	ProjectType eApplicationType = m_clIndSelPMParams.m_pclSelectPMList->GetpclPMInputUser()->GetApplicationType();

	// Remark: 'MapList' contains list of all 'PRESSMAINTTYPECB_TAB' -> 'All', 'Expansion vessel', 'Expansion vessel (membrane)',
	//         'With compressor', 'With pump' and 'None'.
	for each( std::pair<int, CData *>PMDataType in MapLst )
	{
		CTable *pTab = dynamic_cast<CTable *>( PMDataType.second );

		if( NULL == pTab )
		{
			ASSERT_CONTINUE;
		}

		if( Solar == eApplicationType )
		{
			if( true == ( (IDPTR)pTab->GetIDPtr() ).IDMatch( L"PMT_WITHCOMPRESS_TAB" ) )
			{
				InitToolTips();

				continue;
			}

			if( true == ( (IDPTR)pTab->GetIDPtr() ).IDMatch( L"PMT_WITHPUMP_TAB" ) )
			{
				continue;
			}
		}

		bool bFound = false;
		int iCount = 0;

		// Each table in 'MapList' contains one or more vessel type ID.
		// For example 'PMT_EXPVSSL_TAB' contains 'VSSLTYPE_STAT' and 'VSSLTYPE_AQUA'.
		for( IDPTR IDPtr = pTab->GetFirst(); NULL != IDPtr.MP && false == bFound; IDPtr = pTab->GetNext( IDPtr.MP ) )
		{
			iCount++;
			CDB_StringID *pStrID = dynamic_cast<CDB_StringID *>( IDPtr.MP );

			// All objects in a 'CTable' are in fact an ID on a 'CData' inherited object.
			// For the 'PRESSMAINTTYPECB_TAB' table, it's in fact an ID on a 'CDB_StringID' object.
			// Except 'PMT_ALL_TAB' table that has an ID on a 'CTable' object instead of 'CDB_StringID'. This is why
			// in this case 'pStrID' is NULL.

			if( NULL == pStrID )
			{
				if( NULL != dynamic_cast<CTable *>( IDPtr.MP ) )
				{
					bFound = true;
				}
				else
				{
					ASSERT( 0 );
				}

				continue;
			}

			if( setPMPTypeID.end() != setPMPTypeID.find( IDPtr.MP ) )
			{
				bFound = true;
			}
		}

		if( 0 == iCount )
		{
			// Specific treatment for entries 'PMT_NONE_TAB', the table has no 'CDB_StringID'.
			bFound = true;
		}

		// Add this entry in the CB.
		if( true == bFound )
		{
			int i = m_ComboPMType.AddString( pTab->GetName() );
            m_ComboPMType.SetItemData( i, ( DWORD_PTR )pTab );

			if( SelectedIDPtr.MP == pTab )
			{
				iSelPos = i;
			}
		}
	}

	if( m_ComboPMType.GetCount() >= 0 )
	{
		m_ComboPMType.EnableWindow( TRUE );
		m_ComboPMType.SetCurSel( iSelPos );
	}
	else
	{
		m_ComboPMType.EnableWindow( FALSE );
	}

	m_clIndSelPMParams.m_pclSelectPMList->GetpclPMInputUser()->SetPressureMaintenanceTypeID( CString( m_ComboPMType.GetCBCurSelIDPtr().ID ) );
	m_clIndSelPMParams.m_bIsVesselMembraneTypeExistInPMTypeCombo = ( -1 != m_ComboPMType.FindCBIDPtr( _T("PMT_EXPVSSLMBR_TAB" ) ) ) ? true : false;
}

void CDlgIndSelPressureMaintenance::_ChangeComboPressMaintType()
{
	if( NULL == m_clIndSelPMParams.m_pclSelectPMList || NULL == m_clIndSelPMParams.m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERT_RETURN;
	}

	// Save the content of the combo.
	m_clIndSelPMParams.m_pclSelectPMList->GetpclPMInputUser()->SetPressureMaintenanceTypeID( CString( m_ComboPMType.GetCBCurSelIDPtr().ID ) );
	m_clIndSelPMParams.m_bIsVesselMembraneTypeExistInPMTypeCombo = ( -1 != m_ComboPMType.FindCBIDPtr( _T("PMT_EXPVSSLMBR_TAB" ) ) ) ? true : false;

	// Verify now fluid characteristic and temperatures.
	_VerifyFluidCharacteristics( false );

	// If we don't need to change the panel (For example goes from 'Expansion vessel' to 'With compressor'), we leave this method.
	CDlgIndSelPMPanelBase::PanelType ePanelToShow = _GetPanelToShow();

	if( ePanelToShow == m_eCurrentPanel )
	{
		return;
	}

	CPMInputUser *pclPMInputUser = m_clIndSelPMParams.m_pclSelectPMList->GetpclPMInputUser();

	// Check if user doesn't want pressure selection.
	if( CDlgIndSelPMPanelBase::PanelType::ePanelNoPressMaint == ePanelToShow )
	{
		if( CDlgIndSelPMPanelBase::ePanelLast != m_eCurrentPanel )
		{
			m_mapPanels[m_eCurrentPanel]->ShowWindow( SW_HIDE );
			m_mapPanels[m_eCurrentPanel]->EnableWindow( FALSE );
		}

		// Disable and hide combo norm and static text.
		GetDlgItem( IDC_STATIC_NORM )->ShowWindow( SW_HIDE );
		GetDlgItem( IDC_STATIC_NORM )->EnableWindow( FALSE );
		m_ComboNorm.ShowWindow( SW_HIDE );
		m_ComboNorm.EnableWindow( FALSE );

		CString strWaterMakeUpID = pclPMInputUser->GetWaterMakeUpTypeID();
		bool bIsWaterMakeUpAvailable = ( 0 == strWaterMakeUpID.Compare( _T("WMUP_TYPE_NONE") ) ) ? false : true;
		
		if( BST_UNCHECKED == pclPMInputUser->GetDegassingChecked() && false == bIsWaterMakeUpAvailable )
		{
			m_clButtonSuggest.EnableWindow( FALSE );
		}

		m_eCurrentPanel = ePanelToShow;
		m_mapPanels[m_eCurrentPanel]->ShowWindow( SW_SHOW );
		m_mapPanels[m_eCurrentPanel]->EnableWindow( TRUE );
	}
	else
	{
		if( CDlgIndSelPMPanelBase::ePanelLast != m_eCurrentPanel )
		{
			m_mapPanels[m_eCurrentPanel]->ShowWindow( SW_HIDE );
			m_mapPanels[m_eCurrentPanel]->EnableWindow( FALSE );
		}

		// Enable and show combo norm and static text.
		GetDlgItem( IDC_STATIC_NORM )->ShowWindow( SW_SHOW );
		GetDlgItem( IDC_STATIC_NORM )->EnableWindow( TRUE );
		m_ComboNorm.ShowWindow( SW_SHOW );
		m_ComboNorm.EnableWindow( TRUE );

		// For heating, cooling and solar panel, when we show/hide the 'Degassing' or/and 'Water make-up' group, we resize the client area
		// to match the new size. Here, we change the dialog container to this new size and we notify the 'DlgLeftTabSelManager' to change
		// the 'IDC_STATICGROUP' control size. This allows 'DlgLeftTabBase' to perfectly draw the corporate logo at the bottom of the left tab.
		CRect rectPMPanelWin;
		m_mapPanels[ePanelToShow]->GetWindowRect( &rectPMPanelWin );
		ScreenToClient( &rectPMPanelWin );

		if( NULL != pDlgLeftTabSelManager )
		{
			CRect rectIndSelPMClient;
			GetClientRect( &rectIndSelPMClient );
			rectIndSelPMClient.bottom = rectPMPanelWin.bottom;
			SetWindowPos( NULL, -1, -1, rectIndSelPMClient.Width(), rectIndSelPMClient.Height(), SWP_NOMOVE | SWP_NOACTIVATE );
			pDlgLeftTabSelManager->ChangeStaticGroupSize( rectIndSelPMClient );
		}

		m_eCurrentPanel = ePanelToShow;
		m_mapPanels[m_eCurrentPanel]->ShowWindow( SW_SHOW );
		m_mapPanels[m_eCurrentPanel]->EnableWindow( TRUE );
	}
}

void CDlgIndSelPressureMaintenance::_FillComboWaterMakeUpType( CString strWaterMakeUpID )
{
	if( NULL == m_clIndSelPMParams.m_pclSelectPMList || NULL == m_clIndSelPMParams.m_pclSelectPMList->GetpclPMInputUser() 
			|| NULL == m_clIndSelPMParams.m_pTADB )
	{
		ASSERT_RETURN;
	}

	m_ComboWaterMakeUpType.ResetContent();
	IDPTR SelectedIDPtr;

	if( false == strWaterMakeUpID.IsEmpty() )
	{
		SelectedIDPtr = m_clIndSelPMParams.m_pTADB->Get( ( LPCTSTR )strWaterMakeUpID );
	}

	// Fill combo pressure maintenance type.
	CRankEx rkList;
	_string str;
	DWORD_PTR dwSelItem = 0;
	CTable *pTab = (CTable *)( m_clIndSelPMParams.m_pTADB->Get( _T("WATERMAKEUPTYPECB_TAB") ).MP );

	for( IDPTR IDPtr = pTab->GetFirst(); NULL != IDPtr.MP; IDPtr = pTab->GetNext( IDPtr.MP ) )
	{
		CDB_StringID *pStrID = dynamic_cast<CDB_StringID *>( IDPtr.MP );

		if( NULL == pStrID )
		{
			ASSERT_CONTINUE;
		}

		str = pStrID->GetString();
		int i;

		if( false == pStrID->GetIDstrAs<int>( 0, i ) )
		{
			ASSERT_CONTINUE;
		}

		if( strWaterMakeUpID == IDPtr.ID )
		{
			dwSelItem = ( LPARAM )IDPtr.MP;
		}

		rkList.Add( str, i, ( LPARAM )IDPtr.MP );
	}

	rkList.Transfer( &m_ComboWaterMakeUpType );
	int iSelPos = m_ComboWaterMakeUpType.FindItemDataPtr( dwSelItem );

	if( -1 == iSelPos )
	{
		iSelPos = 0;
	}

	m_ComboWaterMakeUpType.SetCurSel( iSelPos );

	if( m_ComboWaterMakeUpType.GetCount() >= 0 )
	{
		m_ComboWaterMakeUpType.EnableWindow( TRUE );
		m_ComboWaterMakeUpType.SetCurSel( iSelPos );
	}
	else
	{
		m_ComboWaterMakeUpType.EnableWindow( FALSE );
	}

	m_clIndSelPMParams.m_pclSelectPMList->GetpclPMInputUser()->SetWaterMakeUpTypeID( CString( m_ComboWaterMakeUpType.GetCBCurSelIDPtr().ID ) );
}

void CDlgIndSelPressureMaintenance::_ChangeComboWaterMakeUpType()
{
	if( NULL == m_clIndSelPMParams.m_pclSelectPMList || NULL == m_clIndSelPMParams.m_pclSelectPMList->GetpclPMInputUser() 
		|| NULL == m_clIndSelPMParams.m_pclSelectPMList->GetpclPMInputUser()->GetPMWQSelectionPreferences() )
	{
		ASSERT_RETURN;
	}

	CString strWaterMakeUpID = CString( m_ComboWaterMakeUpType.GetCBCurSelIDPtr().ID );
	CPMInputUser *pclPMInputUser = m_clIndSelPMParams.m_pclSelectPMList->GetpclPMInputUser();
	pclPMInputUser->SetWaterMakeUpTypeID( strWaterMakeUpID );
	
	bool bIsWaterMakeUpAvailable = ( 0 == strWaterMakeUpID.Compare( _T("WMUP_TYPE_NONE") ) ) ? false : true;

	for( mapIntPanel iter = m_mapPanels.begin(); iter != m_mapPanels.end(); iter++ )
	{
		if( NULL != iter->second )
		{
			iter->second->EnableWaterMakeUp( bIsWaterMakeUpAvailable, ( BST_CHECKED == pclPMInputUser->GetDegassingChecked() ) ? true : false );
		}
	}

	CPMWQPrefs *pclPMWQPrefs = pclPMInputUser->GetPMWQSelectionPreferences();
	bool bChecked = pclPMWQPrefs->IsChecked( CPMWQPrefs::GeneralCombinedInOneDevice );
	bool bCheckedDisabled = pclPMWQPrefs->IsDisabled( CPMWQPrefs::GeneralCombinedInOneDevice );
    // HYS-1537 : Verify that the option is not disabled even if it is checked
	if( CDlgIndSelPMPanelBase::ePanelNoPressMaint == m_eCurrentPanel && true == bChecked && false == bCheckedDisabled )
	{
		// If user works without pressurisation and want combined devices, if one of 'Water make-up' or 'Degassing' option
		// is not chosen, then we uncheck the combined devices option.
		if( false == bIsWaterMakeUpAvailable || BST_UNCHECKED == pclPMInputUser->GetDegassingChecked() )
		{
			pclPMWQPrefs->SetChecked( CPMWQPrefs::GeneralCombinedInOneDevice, false );
			pMainFrame->UpdatePMWQSelectPrefsRibbon();
		}
	}
	else
	{
		// For heating, cooling and solar panel, when we show/hide the 'Degassing' or/and 'Water make-up' group, we resize the client area
		// to match the new size. Here, we change the dialog container to this new size and we notify the 'DlgLeftTabSelManager' to change
		// the 'IDC_STATICGROUP' control size. This allows 'DlgLeftTabBase' to perfectly draw the corporate logo at the bottom of the left tab.
		CRect rectPMPanelWin;
		m_mapPanels[m_eCurrentPanel]->GetWindowRect( &rectPMPanelWin );
		ScreenToClient( &rectPMPanelWin );

		if( NULL != pDlgLeftTabSelManager )
		{
			CRect rectIndSelPMClient;
			GetClientRect( &rectIndSelPMClient );
			rectIndSelPMClient.bottom = rectPMPanelWin.bottom;
			SetWindowPos( NULL, -1, -1, rectIndSelPMClient.Width(), rectIndSelPMClient.Height(), SWP_NOMOVE | SWP_NOACTIVATE );
			pDlgLeftTabSelManager->ChangeStaticGroupSize( rectIndSelPMClient );
		}
	}
}

void CDlgIndSelPressureMaintenance::_ChangeButtonDegassing()
{
	if( NULL == m_clIndSelPMParams.m_pclSelectPMList || NULL == m_clIndSelPMParams.m_pclSelectPMList->GetpclPMInputUser() 
			|| NULL == m_clIndSelPMParams.m_pclSelectPMList->GetpclPMInputUser()->GetPMWQSelectionPreferences() || NULL == pDlgLeftTabSelManager )
	{
		ASSERT_RETURN;
	}

	CPMInputUser *pclPMInputUser = m_clIndSelPMParams.m_pclSelectPMList->GetpclPMInputUser();
	pclPMInputUser->SetDegassingChecked( m_DegassingCheckbox.GetCheck() );
	
	CString strWaterMakeUpID = pclPMInputUser->GetWaterMakeUpTypeID();
	bool bIsWaterMakeUpAvailable = ( 0 == strWaterMakeUpID.Compare( _T("WMUP_TYPE_NONE") ) ) ? false : true;

	// For heating, cooling and solar panel, when we show/hide the 'Degassing' or/and 'Water make-up' group, we resize the client area
	// to match the new size. Here, we change the dialog container to this new size and we notify the 'DlgLeftTabSelManager' to change
	// the 'IDC_STATICGROUP' control size. This allows 'DlgLeftTabBase' to perfectly draw the corporate logo at the bottom of the left tab.
	for( auto &iter : m_mapPanels )
	{
		iter.second->EnableDegassing( ( BST_CHECKED == pclPMInputUser->GetDegassingChecked() ) ? true : false, bIsWaterMakeUpAvailable );

		if( CDlgIndSelPMPanelBase::PanelType::ePanelNoPressMaint == iter.first )
		{
			continue;
		}
		
		if( NULL != iter.second )
		{
			CRect rectPMPanelWin;
			iter.second->GetWindowRect( &rectPMPanelWin );
			ScreenToClient( &rectPMPanelWin );

			CRect rectIndSelPMClient;
			GetClientRect( &rectIndSelPMClient );
			rectIndSelPMClient.bottom = rectPMPanelWin.bottom;
			SetWindowPos( NULL, -1, -1, rectIndSelPMClient.Width(), rectIndSelPMClient.Height(), SWP_NOMOVE | SWP_NOACTIVATE );
			pDlgLeftTabSelManager->ChangeStaticGroupSize( rectIndSelPMClient );
		}
	}
}

void CDlgIndSelPressureMaintenance::_FillComboNorm( CString strNormID )
{
	if( NULL == m_clIndSelPMParams.m_pTADB )
	{
		ASSERT_RETURN;
	}

	CRankEx rkList;
	DWORD_PTR dwSelItem = 0;

	CString strComboNorm = _T("");
	CDlgIndSelPMPanelBase::PanelType ePanelType = _GetPanelToShow();

	if( CDlgIndSelPMPanelBase::PanelType::ePanelHeating == ePanelType 
			|| CDlgIndSelPMPanelBase::PanelType::ePanelHeatingSWKI == ePanelType )
	{
		 strComboNorm = _T("PMCOMBONORM_HEATING");
	}
	else if( CDlgIndSelPMPanelBase::PanelType::ePanelCooling == ePanelType 
			|| CDlgIndSelPMPanelBase::PanelType::ePanelCoolingSWKI == ePanelType )
	{
		 strComboNorm = _T("PMCOMBONORM_COOLING");
	}
	else if( CDlgIndSelPMPanelBase::PanelType::ePanelSolar == ePanelType
			|| CDlgIndSelPMPanelBase::PanelType::ePanelSolarSWKI == ePanelType )
	{
		 strComboNorm = _T("PMCOMBONORM_SOLAR");
	}
	else
	{
		return;
	}

	CDB_MultiString *pclMultiString = (CDB_MultiString *)( m_clIndSelPMParams.m_pTADB->Get( strComboNorm ).MP );
	ASSERT( NULL != pclMultiString );

	int i = 0;
	for( int i = 0; i < pclMultiString->GetNumofLines(); i++ )
	{
		CString strID = pclMultiString->GetString( i );
		
		CDB_StringID *pclNormID = (CDB_StringID *)( m_clIndSelPMParams.m_pTADB->Get( strID ).MP );
		ASSERT( NULL != pclNormID );

		if( strNormID == strID )
		{
			dwSelItem = ( LPARAM )pclNormID->GetIDPtr().MP;
		}

		wstring str = pclNormID->GetString();
		rkList.Add( str, i, ( LPARAM )pclNormID->GetIDPtr().MP );
	}

	rkList.Transfer( &m_ComboNorm );
	int iSel = m_ComboNorm.FindItemDataPtr( dwSelItem );

	if( -1 == iSel )
	{
		iSel = 0;
	}

	m_ComboNorm.SetCurSel( iSel );
}

CString CDlgIndSelPressureMaintenance::_GetNormID()
{
	CString strNormID = _T("PM_NONE");
	int iCur = m_ComboNorm.GetCurSel();

	if( iCur >= 0 )
	{
		CDB_StringID *pStrID = NULL;
        pStrID = dynamic_cast< CDB_StringID *>( (CData *)m_ComboNorm.GetItemData( iCur ) );

		if( NULL != pStrID )
		{
			strNormID = pStrID->GetIDPtr().ID;
		}
	}

	return strNormID;
}

void CDlgIndSelPressureMaintenance::_VerifySuggestButton()
{
	if( NULL == m_clIndSelPMParams.m_pclSelectPMList || NULL == m_clIndSelPMParams.m_pclSelectPMList->GetpclPMInputUser() 
			|| NULL == m_clIndSelPMParams.m_pclSelectPMList->GetpclPMInputUser()->GetPMWQSelectionPreferences() )
	{
		ASSERT_RETURN;
	}

	CPMInputUser *pclPMInputUser = m_clIndSelPMParams.m_pclSelectPMList->GetpclPMInputUser();
	CPMWQPrefs *pclPMWQPrefs = pclPMInputUser->GetPMWQSelectionPreferences();
	BOOL bEnable = TRUE;

	if( CDlgIndSelPMPanelBase::ePanelNoPressMaint == m_eCurrentPanel )
	{
		// HYS-1537 : Verify that the option is not disabled even if it is checked
		if( false == pclPMWQPrefs->IsChecked( CPMWQPrefs::GeneralCombinedInOneDevice ) || 
			true == pclPMWQPrefs->IsDisabled( CPMWQPrefs::GeneralCombinedInOneDevice ) )
		{
			// When user work without pressurisation and he wants no combined device, it must be at least one 'water make-up'
			// or 'degassing' that must be chosen.
			if( 0 == pclPMInputUser->GetWaterMakeUpTypeID().Compare( _T("WMUP_TYPE_NONE") )
					&& BST_UNCHECKED == pclPMInputUser->GetDegassingChecked() )
			{
				bEnable = FALSE;
			}
		}
		else
		{
			// When user work without pressurisation maintenance system and he wants combined device, both 'water make-up'
			// and 'degassing' must be chosen to enable the 'Suggest button'.
			if( 0 == pclPMInputUser->GetWaterMakeUpTypeID().Compare( _T("WMUP_TYPE_NONE") )
					|| BST_UNCHECKED == pclPMInputUser->GetDegassingChecked() )
			{
				bEnable = FALSE;
			}
		}
	}

	m_clButtonSuggest.EnableWindow( bEnable );
}

bool CDlgIndSelPressureMaintenance::_VerifyPAZAccessory( void )
{
	// If user is in EN 12953 norm with Transfero TI, he must select PAZ. This accessory is mandatory.
    // We show a message to the user to warn him. He can either cancel and go select the PAZ or continue
    // but he will need to add himself this accessory.

	if( NULL == pRViewSSelPM || NULL == m_clIndSelPMParams.m_pclSelectPMList || NULL == m_clIndSelPMParams.m_pclSelectPMList->GetpclPMInputUser() )
	{
		return true;
	}

	CPMInputUser* pclPMInputUser = m_clIndSelPMParams.m_pclSelectPMList->GetpclPMInputUser();

	if( CPMInputUser::MT_WithPump != pclPMInputUser->GetPressureMaintenanceType() || false == pclPMInputUser->IsNorm( ProjectType::Heating, PressurisationNorm::PN_EN12953 ) )
	{
		return true;
	}

	CDB_TecBox* pclTecBoxTransfero = NULL;
	long lRow = 0;

	if( false == pRViewSSelPM->GetTecBoxTransferoSelected( &pclTecBoxTransfero, &lRow ) )
	{
		return true;
	}

	if( 0 != CString( pclTecBoxTransfero->GetFamilyID() ).Compare( _T( "TBXFAM_TRANSFERO_TIC" ) ) )
	{
		return true;
	}

	CAccessoryList clTransferoAccessoryList;

	if( false == pRViewSSelPM->GetTransferoAccessoryList( &clTransferoAccessoryList ) )
	{
		return true;
	}

	bool bPAZAccessoryFound = false;
	CAccessoryList::AccessoryItem rAccessoryItem = clTransferoAccessoryList.GetFirst( CAccessoryList::AccessoryType::_AT_Accessory );

	while( _NULL_IDPTR != rAccessoryItem.IDPtr && false == bPAZAccessoryFound )
	{
		CDB_Product* pclAccessory = dynamic_cast<CDB_Product*>(rAccessoryItem.IDPtr.MP);

		if( NULL == pclAccessory )
		{
			ASSERT_CONTINUE;
		}

		if( 0 == CString( pclAccessory->GetFamilyID() ).Compare( _T( "ACCFAM_PMPAZ" ) ) )
		{
			bPAZAccessoryFound = true;
		}

		rAccessoryItem = clTransferoAccessoryList.GetNext( CAccessoryList::AccessoryType::_AT_Accessory );
	}

	if( false == bPAZAccessoryFound )
	{
		// Message bx.
		// When selecting Transfero TI in the EN 12953 norm, the PAZ Pmin TI accessory is mandatory.
		// Click 'Cancel' to go back and select the appropriate accessory.\r\n
		// Or click 'Continue' to allow selection without this accessory but don’t forget to manually add it later.
		CString strMsg = TASApp.LoadLocalizedString( IDS_DLGINDSELPM_PAZNEEDED );

		bPAZAccessoryFound = (IDOK == AfxMessageBox( strMsg, MB_OKCANCEL | MB_ICONEXCLAMATION )) ? true : false;
	}

	return bPAZAccessoryFound;
}

bool CDlgIndSelPressureMaintenance::_VerifyComCubeDMLConnectAccessory()
{
	// HYS-1643: If user is in EN 12953 norm with Transfero TI Connect, he must select ComCube DML Connect. This accessory is mandatory.
	// We show a message to the user to warn him. He can either cancel and go select the ComCube DML Connect or continue
	// but he will need to add himself this accessory.

	if( NULL == pRViewSSelPM || NULL == m_clIndSelPMParams.m_pclSelectPMList || NULL == m_clIndSelPMParams.m_pclSelectPMList->GetpclPMInputUser() )
	{
		return true;
	}

	CPMInputUser *pclPMInputUser = m_clIndSelPMParams.m_pclSelectPMList->GetpclPMInputUser();

	if( CPMInputUser::MT_WithPump != pclPMInputUser->GetPressureMaintenanceType() || false == pclPMInputUser->IsNorm( ProjectType::Heating, PressurisationNorm::PN_EN12953 ) )
	{
		return true;
	}

	CDB_TecBox *pclTecBoxTransfero = NULL;
	long lRow = 0;

	if( false == pRViewSSelPM->GetTecBoxTransferoSelected( &pclTecBoxTransfero, &lRow ) )
	{
		return true;
	}

	if( 0 != CString( pclTecBoxTransfero->GetFamilyID() ).Compare( _T("TBXFAM_TRANSFERO_TIC") ) )
	{
		return true;
	}
	
	CAccessoryList clTransferoVesselAccessoryList;
	
	if( false == pRViewSSelPM->GetTransferoVesselAccessoryList( &clTransferoVesselAccessoryList ) )
	{
		return true;
	}

	bool bDMLAccessoryFound = false;
	CAccessoryList::AccessoryItem rAccessoryItem = clTransferoVesselAccessoryList.GetFirst( CAccessoryList::AccessoryType::_AT_Accessory );

	while( _NULL_IDPTR != rAccessoryItem.IDPtr && false == bDMLAccessoryFound )
	{
		CDB_Product *pclAccessory = dynamic_cast<CDB_Product *>( rAccessoryItem.IDPtr.MP );

		if( NULL == pclAccessory )
		{
			ASSERT_CONTINUE;
		}

		if( 0 == CString( pclAccessory->GetIDPtr().ID ).Compare( _T("ACCCOMCUBE_DMLC") ) )
		{
			bDMLAccessoryFound = true;
		}

		rAccessoryItem = clTransferoVesselAccessoryList.GetNext( CAccessoryList::AccessoryType::_AT_Accessory );
	}

	if( false == bDMLAccessoryFound )
	{
		// Message box.
		// When selecting Transfero TI Connect in the EN 12953 norm, the ComCube DML Connect accessory is mandatory.
		// Click 'Cancel' to go back and select the appropriate accessory.\r\n
		// Or click 'Continue' to allow selection without this accessory but don’t forget to manually add it later.
		CString strMsg = TASApp.LoadLocalizedString( IDS_DLGINDSELPM_COMCUBEDMLCONNECTNEEDED  );
		
		bDMLAccessoryFound = ( IDOK == AfxMessageBox( strMsg, MB_OKCANCEL | MB_ICONEXCLAMATION ) ) ? true : false;
	}

	return bDMLAccessoryFound;
}

void CDlgIndSelPressureMaintenance::_UpdateWithWizardSelPMState()
{
	if( NULL == pDlgLeftTabSelManager )
	{
		ASSERT_RETURN;
	}

	CDlgSelectionBase *pclDlgSelectionBase = pDlgLeftTabSelManager->GetLeftTabDialog( ProductSubCategory::PSC_PM_ExpansionVessel, 
			ProductSelectionMode::ProductSelectionMode_Wizard );

	if( NULL == pclDlgSelectionBase || NULL == dynamic_cast<CDlgWizardSelPM*>( pclDlgSelectionBase ) )
	{
		return;
	}

	CDlgWizardSelPM *pDlgWizardSelPM = dynamic_cast<CDlgWizardSelPM*>( pclDlgSelectionBase );

	m_clIndSelPMParams.m_SelIDPtr = pDlgWizardSelPM->GetEditedObject();
	SetModificationMode( pDlgWizardSelPM->IsEditModeRunning() );

	m_clIndSelPMParams.m_pclSelectPMList = pDlgLeftTabSelManager->GetPMSelectionHelper()->GetSelectPMList( ProductSelectionMode_Individual );
	
	if( NULL == m_clIndSelPMParams.m_pclSelectPMList || NULL == m_clIndSelPMParams.m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERT_RETURN;
	}
		
	CSelectPMList *pclWizardSelectPMList = pDlgLeftTabSelManager->GetPMSelectionHelper()->GetSelectPMList( ProductSelectionMode_Wizard, m_clIndSelPMParams.m_pclSelectPMList->GetpclPMInputUser()->GetApplicationType() );
	
	if( NULL == pclWizardSelectPMList || NULL == pclWizardSelectPMList->GetpclPMInputUser() )
	{
		ASSERT_RETURN;
	}
	
	m_clIndSelPMParams.m_pclSelectPMList->GetpclPMInputUser()->CopyFrom( pclWizardSelectPMList->GetpclPMInputUser() );

	_VerifyPMWQPrefs();

	// First parameter: redraw, second parameter: do not display popup with error messages if exist.
	_UpdateAllFields( true, false );

	if( NULL != pRViewSSelPM )
	{
		if( true == pDlgWizardSelPM->IsRightViewResetNeeded() )
		{
			// User has not yet clicked on the 'Results' step. Thus here we don't show results.
			pRViewSSelPM->Reset();
		}
		else
		{
			EnableSelectButton( pRViewSSelPM->IsSelectionAvailable() );
		}
	}
}

void CDlgIndSelPressureMaintenance::UpdateWithWizardAllPrefInAllMode( void )
{
	// HYS-1537 : We copy all prefs in all mode
	CSelectPMList* pclHeatingIndSelPMList = pDlgLeftTabSelManager->GetPMSelectionHelper()->GetSelectPMList( ProductSelectionMode_Individual, Heating );
	CSelectPMList* pclHeatingWizSelPMList = pDlgLeftTabSelManager->GetPMSelectionHelper()->GetSelectPMList( ProductSelectionMode_Wizard, Heating );
	if( NULL != pclHeatingIndSelPMList && NULL != pclHeatingIndSelPMList->GetpclPMInputUser() )
	{
		CPMWQPrefs* pclPrefHeating = pclHeatingIndSelPMList->GetpclPMInputUser()->GetPMWQSelectionPreferences();
		if( NULL != pclHeatingWizSelPMList && NULL != pclHeatingWizSelPMList->GetpclPMInputUser() )
		{
			CPMWQPrefs* pclPrefHeatingWiz = pclHeatingWizSelPMList->GetpclPMInputUser()->GetPMWQSelectionPreferences();
			if( true != pclPrefHeating->CompareTo( pclPrefHeatingWiz ) )
			{
				pclPrefHeating->CopyFrom( pclPrefHeatingWiz );
			}
		}
	}

	CSelectPMList* pclCoolingIndSelPMList = pDlgLeftTabSelManager->GetPMSelectionHelper()->GetSelectPMList( ProductSelectionMode_Individual, Cooling );
	CSelectPMList* pclCoolingWizSelPMList = pDlgLeftTabSelManager->GetPMSelectionHelper()->GetSelectPMList( ProductSelectionMode_Wizard, Cooling );
	if( NULL != pclCoolingIndSelPMList && NULL != pclCoolingIndSelPMList->GetpclPMInputUser() )
	{
		CPMWQPrefs* pclPrefCooling = pclCoolingIndSelPMList->GetpclPMInputUser()->GetPMWQSelectionPreferences();
		if( NULL != pclCoolingWizSelPMList && NULL != pclCoolingWizSelPMList->GetpclPMInputUser() )
		{
			CPMWQPrefs* pclPrefCoolingWiz = pclCoolingWizSelPMList->GetpclPMInputUser()->GetPMWQSelectionPreferences();
			if( true != pclPrefCooling->CompareTo( pclPrefCoolingWiz ) )
			{
				pclPrefCooling->CopyFrom( pclPrefCoolingWiz );
			}
		}
	}

	CSelectPMList* pclSolarIndSelPMList = pDlgLeftTabSelManager->GetPMSelectionHelper()->GetSelectPMList( ProductSelectionMode_Individual, Solar );
	CSelectPMList* pclSolarWizSelPMList = pDlgLeftTabSelManager->GetPMSelectionHelper()->GetSelectPMList( ProductSelectionMode_Wizard, Solar );
	if( NULL != pclSolarIndSelPMList && NULL != pclSolarIndSelPMList->GetpclPMInputUser() )
	{
		CPMWQPrefs* pclPrefSolar = pclSolarIndSelPMList->GetpclPMInputUser()->GetPMWQSelectionPreferences();
		if( NULL != pclSolarWizSelPMList && NULL != pclSolarWizSelPMList->GetpclPMInputUser() )
		{
			CPMWQPrefs* pclPrefSolarWiz = pclSolarWizSelPMList->GetpclPMInputUser()->GetPMWQSelectionPreferences();
			if( true != pclPrefSolar->CompareTo( pclPrefSolarWiz ) )
			{
				pclPrefSolar->CopyFrom( pclPrefSolarWiz );
			}
		}
	}
}

void CDlgIndSelPressureMaintenance::_UpdateAllFields( bool bRedraw, bool bAllowShowErrorMessage )
{
	if( NULL == m_clIndSelPMParams.m_pclSelectPMList || NULL == m_clIndSelPMParams.m_pclSelectPMList->GetpclPMInputUser() 
			|| NULL == m_clIndSelPMParams.m_pclSelectPMList->GetpclPMInputUser()->GetPMWQSelectionPreferences() )
	{
		return;
	}

	if( true == bRedraw )
	{
		SetRedraw( FALSE );
	}

	CPMInputUser *pclPMInputUser = m_clIndSelPMParams.m_pclSelectPMList->GetpclPMInputUser();
	CPMWQPrefs *pclPMWQPrefs = pclPMInputUser->GetPMWQSelectionPreferences();

	// Update the PM & WQ selection preferences structure in the current selection to edit.
	pMainFrame->UpdatePMWQSelectPrefsRibbon();

	_FillComboPressMaintType( pclPMInputUser->GetPressureMaintenanceTypeID() );
	_FillComboWaterMakeUpType( pclPMInputUser->GetWaterMakeUpTypeID() );
	_FillComboNorm( pclPMInputUser->GetNormID() );

	m_DegassingCheckbox.SetCheck( pclPMInputUser->GetDegassingChecked() );
	_ChangeButtonDegassing();
	
	// Update panel in regards to the current item selected in the combo 'Pressure maintenance type'.
	// Remark: we do that just after the two check box. Because if pressure maintenance type is set to 'No pressure maintenance', we need
	//         to enable/disable 'Suggest' button in regards to the status of these two checkbox.
	_ChangeComboPressMaintType();
	_ChangeComboWaterMakeUpType();

	// Call the appropriate panel to fill fields and combos.
	if( false == pclPMInputUser->IsPressurisationSystemExist() )
	{
		m_mapPanels[CDlgIndSelPMPanelBase::ePanelNoPressMaint]->EnableOnEnChangeNotification( false );
		m_mapPanels[CDlgIndSelPMPanelBase::ePanelNoPressMaint]->ApplyPMInputUser( pclPMInputUser, bAllowShowErrorMessage );
		m_mapPanels[CDlgIndSelPMPanelBase::ePanelNoPressMaint]->EnableOnEnChangeNotification( true );
	}
	else
	{
		m_mapPanels[m_eCurrentPanel]->EnableOnEnChangeNotification( false );
		m_mapPanels[m_eCurrentPanel]->ApplyPMInputUser( pclPMInputUser, bAllowShowErrorMessage );
		m_mapPanels[m_eCurrentPanel]->EnableOnEnChangeNotification( true );
	}

	if( true == bRedraw )
	{
		SetRedraw( TRUE );
		Invalidate();
		UpdateWindow();
	}
}

void CDlgIndSelPressureMaintenance::_VerifyPMWQPrefs()
{
	if( NULL == pDlgLeftTabSelManager || NULL == pDlgLeftTabSelManager->GetPMSelectionHelper() 
			|| NULL == m_clIndSelPMParams.m_pclSelectPMList || NULL == m_clIndSelPMParams.m_pclSelectPMList->GetpclPMInputUser() )
	{
		return;
	}

	if( true == pDlgLeftTabSelManager->GetPMSelectionHelper()->VerifyPMWQPrefs( &m_clIndSelPMParams ) )
	{
		pMainFrame->UpdatePMWQSelectPrefsRibbon();
	}
}

void CDlgIndSelPressureMaintenance::_VerifyFluidCharacteristics( bool bShowErrorMsg )
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

void CDlgIndSelPressureMaintenance::_VerifyFluidCharacteristicsHelper( bool &bEnable, CString &strMsg, CDlgWaterChar *pclDlgWaterChar )
{
	if( NULL == m_clIndSelPMParams.m_pclSelectPMList || NULL == m_clIndSelPMParams.m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERT_RETURN;
	}

	CString strAdditiveFamilyID = _T("");
	
	if( NULL != pclDlgWaterChar )
	{
		strAdditiveFamilyID = pclDlgWaterChar->GetAdditiveFamilyID();
	}
	else
	{
		if( NULL == m_clIndSelPMParams.m_pclSelectPMList->GetpclPMInputUser()->GetpWC() )
		{
			ASSERT_RETURN;
		}

		strAdditiveFamilyID = m_clIndSelPMParams.m_pclSelectPMList->GetpclPMInputUser()->GetpWC()->GetAdditFamID();
	}

	bEnable = true;
	strMsg = _T("");

	// HYS-1041: Disable suggest button when the additive is not a Glycol.
	// HYS-1111: 2019-06-05: Christian Thesing and Norbert Ramser allow alcohol for PAG products.
	if( 0 != StringCompare( strAdditiveFamilyID, _T("GLYCOL_ADDIT") ) 
			&& 0 != StringCompare( strAdditiveFamilyID, _T("ALCOHOL_ADDIT") )
			&& 0 != StringCompare( strAdditiveFamilyID, _T("WATER_ADDIT") ) )
	{
		strMsg = TASApp.LoadLocalizedString( AFXMSG_BAD_ADDITIVE );
		bEnable = false;
	}

	// HYS-1104: Disable the 'Suggest' button if we are in EN12953 and not with water.
	if( m_clIndSelPMParams.m_pclSelectPMList->GetpclPMInputUser()->IsNorm( ProjectType::Heating, PressurisationNorm::PN_EN12953 ) 
			&& ( 0 != StringCompare( strAdditiveFamilyID, _T("WATER_ADDIT") ) ) )
	{
		strMsg = TASApp.LoadLocalizedString( IDS_DLGINDSELPM_EN12953ONLYWATER );
		bEnable = false;
	}
}