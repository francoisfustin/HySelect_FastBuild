#include "stdafx.h"
#include "TASelect.h"
#include "MainFrm.h"
#include "DlgTechParam.h"
#include "DlgLTtabctrl.h"
#include "DlgLeftTabSelManager.h"
#include "SelectPM.h"

IMPLEMENT_DYNAMIC( CDlgTechParam, CDlgCtrlPropTree )

CDlgTechParam::CDlgTechParam( eOpeningPage ePage, CWnd *pParent )
	: CDlgCtrlPropTree( pParent )
{
	m_iCurrentPage = ePage;
	m_bRefreshResults = true;
}

CDlgTechParam::~CDlgTechParam()
{
	for( int i = 0; i < m_ArpPage.GetSize(); i++ )
	{
		if( NULL != m_ArpPage[i] )
		{
			m_ArpPage[i]->DestroyWindow();
			m_ArpPage[i] = NULL;
		}
	}

	if( NULL != m_IDPtrTechParamSaved.MP )
	{
		TASApp.GetpTADB()->DeleteObject( m_IDPtrTechParamSaved );
	}
}

BEGIN_MESSAGE_MAP( CDlgTechParam, CDlgCtrlPropTree )
	ON_BN_CLICKED( IDOK, OnBnClickedOk )
	ON_BN_CLICKED( IDCANCEL, OnBnClickedCancel )
	ON_BN_CLICKED( IDC_BUTTONDEFAULT, OnBnClickedResetToDefault )
END_MESSAGE_MAP()

void CDlgTechParam::SaveAllPages()
{
	for( int i = 0; i < m_ArpPage.GetSize(); i++ )
	{
		if( NULL != m_ArpPage[i] )
		{
			m_ArpPage[i]->Save( REGISTRYSECTIONNAME_DLGTECHPARAM );
		}
	}
}

BOOL CDlgTechParam::OnInitDialog()
{
	try
	{
		CDS_TechnicalParameter *pTechParamCurrent = TASApp.GetpTADS()->GetpTechParams();

		if( NULL == pTechParamCurrent )
		{
			return FALSE;
		}

		// Pay attention. If auto save file is activate, the 'CTASelectDoc::_FileSave' can be called and 'pTADS->VerifyDataSetIntegrity()' in it.
		// This last method will clean all orphan objects from 'CTADataStruct' database before to save it. To verify if an object is orphan or not
		// we simply check the 'CData::m_pOwners'.
		// This is why here to avoid problem we create the "PARAM_TECHSAVED" object in the 'CDatabase' database.
		TASApp.GetpTADB()->CreateObject( m_IDPtrTechParamSaved, CLASS( CDS_TechnicalParameter ), _T("PARAM_TECHSAVED") );

		m_pclTechParamSaved = dynamic_cast<CDS_TechnicalParameter *>( m_IDPtrTechParamSaved.MP );
		pTechParamCurrent->Copy( m_pclTechParamSaved );

		// Save also pipe safety factor that is in the 'CDS_ProjectParams' object. If user change it, we need to refresh all the network.
		CDS_ProjectParams *pPrjParam = TASApp.GetpTADS()->GetpProjectParams();
		
		if( NULL == pPrjParam )
		{
			HYSELECT_THROW( _T("Internal error: can't retrieve the 'GENERAL_PARAMS' object from the datastruct.") );
		}

		m_dPipeSafetyfactorSaved = pPrjParam->GetSafetyFactor();
	
		// HYS-1221 : Save the initial value
		m_bCheckDeletedAvailableSaved = pPrjParam->GetpHmCalcParams()->IsOldProductsAvailableForHC();

		CDlgCtrlPropTree::OnInitDialog();

		UpdateData( FALSE );

		m_bmodified = TASApp.GetpTADS()->IsModified();
		m_bRefreshResults = TASApp.GetpTADS()->IsRefreshResults();

		// Init the control property dialog.
		CString str;
		str = TASApp.LoadLocalizedString( IDS_DLGTECHPARAM_CAPTION );
		SetWindowText( str );

		str = TASApp.LoadLocalizedString( IDS_DLGTECHPARAM_BUTDEFAULT );
		GetDlgItem( IDC_BUTTONDEFAULT )->SetWindowText( str );

		str = TASApp.LoadLocalizedString( IDS_OK );
		GetDlgItem( IDOK )->SetWindowText( str );

		str = TASApp.LoadLocalizedString( IDS_CANCEL );
		GetDlgItem( IDCANCEL )->SetWindowText( str );

		_InitPages();

		m_PageGeneral.ShowWindow( SW_HIDE );

		if( true == TASApp.IsPressureMaintenanceDisplayed() )
		{
			m_PageGeneralPressureMaintenance.ShowWindow( SW_HIDE );
			m_PageGeneralWaterQuality.ShowWindow( SW_HIDE );
		}

		m_PageGeneralBalancingValves.ShowWindow( SW_HIDE );
		m_PageStdAndAdjustableControlValves.ShowWindow( SW_HIDE );

		if( true == TASApp.IsPICvDisplayed() )
		{
			m_PageGeneralPIBCV.ShowWindow( SW_HIDE );
		}

		if( true == TASApp.IsSmartControlValveDisplayed() || true == TASApp.IsSmartDpCDisplayed() )
		{
			m_PageGeneralSmartValves.ShowWindow( SW_HIDE );
		}

		m_PageGeneralDpControllers.ShowWindow( SW_HIDE );
		m_PageGeneralFixOrifice.ShowWindow( SW_HIDE );

		if( true == TASApp.IsTrvDisplayed() )
		{
			m_PageGeneralRadiatorValves.ShowWindow( SW_HIDE );
		}

		m_PageGeneralPipes.ShowWindow( SW_HIDE );
		m_PageGeneralDeviceSizes.ShowWindow( SW_HIDE );
		m_PageMainHC.ShowWindow( SW_HIDE );
		m_PageHCBalancingValves.ShowWindow( SW_HIDE );
		m_PageHCStandardControlValves.ShowWindow( SW_HIDE );

		if( true == TASApp.IsBCvDisplayed() )
		{
			m_PageHCAdjustableControlValves.ShowWindow( SW_HIDE );
		}

		if( true == TASApp.IsPICvDisplayed() )
		{
			m_PageHCPIBCV.ShowWindow( SW_HIDE );
		}

		m_PageHCActuators.ShowWindow( SW_HIDE );
		m_PageHCDpC.ShowWindow( SW_HIDE );

		if( true == TASApp.IsDpCBCVDisplayed() )
		{
			m_PageHCDpCBCV.ShowWindow( SW_HIDE );
		}
		
		if( true == TASApp.IsSmartControlValveDisplayed() )
		{
			m_PageHCSmartCV.ShowWindow( SW_HIDE );
		}

		m_PageHCPipes.ShowWindow( SW_HIDE );
		m_PageHCSmartCV.ShowWindow( SW_HIDE );

		// Fill the tree with all pages.
		HTREEITEM hGeneral;
		AddPageToTree( &m_PageGeneral, &hGeneral );											// General

		if( true == TASApp.IsPressureMaintenanceDisplayed() )
		{
			AddChildPageToTree( &m_PageGeneral, &m_PageGeneralPressureMaintenance );		// General\Pressure maintenance
			AddChildPageToTree( &m_PageGeneral, &m_PageGeneralWaterQuality );				// General\Water quality
		}

		AddChildPageToTree( &m_PageGeneral, &m_PageGeneralBalancingValves );				// General\Balancing valves
		AddChildPageToTree( &m_PageGeneral, &m_PageStdAndAdjustableControlValves );			// General\Std. & adjustable control valves

		if( true == TASApp.IsPICvDisplayed() )
		{
			AddChildPageToTree( &m_PageGeneral, &m_PageGeneralPIBCV );						// General\Press.-indep. bal. & control valves
		}

		if( true == TASApp.IsSmartControlValveDisplayed() && true == TASApp.IsSmartDpCDisplayed() )
		{
			AddChildPageToTree( &m_PageGeneral, &m_PageGeneralSmartValves );				// General\Smart ctrl. valves & smart diff. press. controllers
		}

		AddChildPageToTree( &m_PageGeneral, &m_PageGeneralDpControllers );					// General\Dp controllers
		AddChildPageToTree( &m_PageGeneral, &m_PageGeneralFixOrifice );						// General\Fixed orifices

		if( true == TASApp.IsTrvDisplayed() )
		{
			AddChildPageToTree( &m_PageGeneral, &m_PageGeneralRadiatorValves );				// General\Radiator valves
		}

		AddChildPageToTree( &m_PageGeneral, &m_PageGeneralPipes );							// General\Pipes
		AddChildPageToTree( &m_PageGeneral, &m_PageGeneralDeviceSizes );					// General\Device sizes

		HTREEITEM hMainHC;
		AddPageToTree( &m_PageMainHC, &hMainHC );											// Hydronic circuit calculation

		AddChildPageToTree( &m_PageMainHC, &m_PageHCBalancingValves );						// Hydronic circuit calculation\Balancing valves
		AddChildPageToTree( &m_PageMainHC, &m_PageHCStandardControlValves );				// Hydronic circuit calculation\Standard control valves

		if( true == TASApp.IsBCvDisplayed() )
		{
			AddChildPageToTree( &m_PageMainHC, &m_PageHCAdjustableControlValves );			// Hydronic circuit calculation\Adjustable control valves
		}

		if( true == TASApp.IsPICvDisplayed() )
		{
			AddChildPageToTree( &m_PageMainHC, &m_PageHCPIBCV );							// Hydronic circuit calculation\Press.-indep. bal. & control valves
		}

		AddChildPageToTree( &m_PageMainHC, &m_PageHCActuators );							// Hydronic circuit calculation\Actuators
		AddChildPageToTree( &m_PageMainHC, &m_PageHCDpC );									// Hydronic circuit calculation\Dp controllers

		if( true == TASApp.IsDpCBCVDisplayed() )
		{
			AddChildPageToTree( &m_PageMainHC, &m_PageHCDpCBCV );							// Hydronic circuit calculation\Combined Dp controller, control and balancing valves.
		}
		
		if( true == TASApp.IsSmartControlValveDisplayed() )
		{
			AddChildPageToTree( &m_PageMainHC, &m_PageHCSmartCV );							// Hydronic circuit calculation\Smart control valve.
		}

		AddChildPageToTree( &m_PageMainHC, &m_PageHCPipes );								// Hydronic circuit calculation\Pipes
		AddChildPageToTree( &m_PageMainHC, &m_PageHCPumps );								// Hydronic circuit calculation\Pumps

		m_Tree.Expand( hGeneral, TVE_EXPAND );
		m_Tree.Expand( hMainHC, TVE_EXPAND );

		// Show the desire page. Check in general page.
		if( true == _CheckPage( hGeneral ) )
		{
			return TRUE;
		}

		// Check now in the children of general page.
		for( HTREEITEM hItem = m_Tree.GetChildItem( hGeneral ); hItem != NULL ; hItem = m_Tree.GetNextSiblingItem( hItem ) )
		{
			if( true == _CheckPage( hItem ) )
			{
				return TRUE;
			}
		}

		// Check now in the HC main page.
		if( true == _CheckPage( hMainHC ) )
		{
			return TRUE;
		}

		// Check now in the children of HC main page.
		for( HTREEITEM hItem = m_Tree.GetChildItem( hMainHC ); hItem != NULL ; hItem = m_Tree.GetNextSiblingItem( hItem ) )
		{
			if( true == _CheckPage( hItem ) )
			{
				return TRUE;
			}
		}

		// In case nothing is found, display the default first page.
		CDlgCtrlPropPage *pPage = dynamic_cast<CDlgCtrlPropPage *>( (CDialog *)m_Tree.GetItemData( m_Tree.GetRootItem() ) );
		
		if( NULL == pPage )
		{
			HYSELECT_THROW( _T("Internal error: can't retrieve the 'CDlgCtrlPropPage' page.") );
		}
		
		m_Tree.SelectItem( m_Tree.GetRootItem() );
		SetCurPage( pPage );
		DisplayPage( pPage );

		return TRUE;
	}
	catch( CHySelectException &clHySelectException )
	{
		clHySelectException.AddMessage( _T("Error in 'CDlgTechParam::OnInitDialog'."), __LINE__, __FILE__ );
		throw;
	}
}

void CDlgTechParam::OnBnClickedOk()
{
	PREVENT_ENTER_KEY

	// Verify info before saving it.
	if( false == m_pCurPage->VerifyInfo() )
	{
		return;
	}

	// Transfer technical parameters to m_TADS.
	// Save the technical parameters in the registry.
	// (useful when starting the application from scratch).
	for( int i = 0; i < m_ArpPage.GetSize(); i++ )
	{
		if( NULL != m_ArpPage[i] )
		{
			m_ArpPage[i]->Save( REGISTRYSECTIONNAME_DLGTECHPARAM );
		}
	}

	CDlgCtrlPropTree::OnOK();

	// Verify if there is a change. To not send 'WM_USER_TECHPARCHANGE' for nothing!
	bool bSendWmUserTechParChangeMsg = false;
	CDS_ProjectParams *pPrjParam = TASApp.GetpTADS()->GetpProjectParams();
	ASSERT( NULL != pPrjParam );

	if( NULL == pPrjParam )
	{
		return;
	}

	if( ( m_dPipeSafetyfactorSaved != pPrjParam->GetSafetyFactor() ) 
		|| m_bCheckDeletedAvailableSaved != pPrjParam->GetpHmCalcParams()->IsOldProductsAvailableForHC() )
	{
		bSendWmUserTechParChangeMsg = true;
	}

	if( false == bSendWmUserTechParChangeMsg )
	{
		CDS_TechnicalParameter *pclTechParamCurrent = TASApp.GetpTADS()->GetpTechParams();

		if( 0 != pclTechParamCurrent->Compare( m_pclTechParamSaved ) )
		{
			bSendWmUserTechParChangeMsg = true;
		}
	}

	if( true == bSendWmUserTechParChangeMsg )
	{
		// Send message to inform about modification of technical parameters.
		::AfxGetApp()->m_pMainWnd->SendMessage( WM_USER_TECHPARCHANGE );
		::AfxGetApp()->m_pMainWnd->SendMessageToDescendants( WM_USER_TECHPARCHANGE );
	}
}

void CDlgTechParam::OnBnClickedCancel()
{
	CDlgCtrlPropTree::OnCancel();

	// HYS-1221 : Retrieve the initial value
	CDS_ProjectParams *pPrjParam = TASApp.GetpTADS()->GetpProjectParams();
	ASSERT( NULL != pPrjParam );

	if( NULL == pPrjParam )
	{
		return;
	}
	CDS_TechnicalParameter *pclTechParamCurrent = TASApp.GetpTADS()->GetpTechParams();

	if( m_dPipeSafetyfactorSaved != pPrjParam->GetSafetyFactor() )
	{
		pPrjParam->SetSafetyFactor( m_dPipeSafetyfactorSaved );
	}

	if( m_bCheckDeletedAvailableSaved != pPrjParam->GetpHmCalcParams()->IsOldProductsAvailableForHC() )
	{
		pPrjParam->GetpHmCalcParams()->SetCheckOldProductsAvailableForHC( m_bCheckDeletedAvailableSaved );
	}

	if( 0 != pclTechParamCurrent->Compare( m_pclTechParamSaved ) )
	{
		m_pclTechParamSaved->Copy( pclTechParamCurrent );
	}
	TASApp.GetpTADS()->Modified( m_bmodified );
	TASApp.GetpTADS()->RefreshResults( m_bRefreshResults );
}

void CDlgTechParam::OnBnClickedResetToDefault()
{
	BeginWaitCursor();

	CDS_TechnicalParameter *pTech = TASApp.GetpTADS()->GetpTechParams();
	CDS_TechnicalParameter *pTADBTech = (CDS_TechnicalParameter *)( TASApp.GetpTADB()->Get( _T("PARAM_TECH") ).MP );

	// Preserve project and application values.
	ProjectType eProjectApplicationType = pTech->GetProjectApplicationType();
	ProjectType eProductSelectionApplicationType = pTech->GetProductSelectionApplicationType();

	// Initialize TADS with TADB Tech param content.
	pTADBTech->Copy( pTech );

	pTech->SetProjectApplicationType( eProjectApplicationType );
	pTech->SetProductSelectionApplicationType( eProductSelectionApplicationType );

	// HYS-1058: We reset 'Pump-based and vacuum degassing - Cooling insulation' to true.
	pDlgLeftTabSelManager->GetPMSelectionHelper()->GetPMInputUser( ProductSelectionMode_Individual, ProjectType::Heating )->GetPMWQSelectionPreferences()->Reset();
	pDlgLeftTabSelManager->GetPMSelectionHelper()->GetPMInputUser( ProductSelectionMode_Individual, ProjectType::Solar )->GetPMWQSelectionPreferences()->Reset();
	pDlgLeftTabSelManager->GetPMSelectionHelper()->GetPMInputUser( ProductSelectionMode_Individual, ProjectType::Cooling )->GetPMWQSelectionPreferences()->Reset();
	pDlgLeftTabSelManager->GetPMSelectionHelper()->GetPMInputUser( ProductSelectionMode_Individual, ProjectType::Cooling )->GetPMWQSelectionPreferences()->SetChecked( CPMWQPrefs::PumpDegassingCoolingVersion, true );
	pDlgLeftTabSelManager->GetPMSelectionHelper()->GetPMInputUser( ProductSelectionMode_Individual, ProjectType::Cooling )->GetPMWQSelectionPreferences()->SetDisabled( CPMWQPrefs::PumpDegassingCoolingVersion, false );

	pDlgLeftTabSelManager->GetPMSelectionHelper()->GetPMInputUser( ProductSelectionMode_Wizard, ProjectType::Heating )->GetPMWQSelectionPreferences()->Reset();
	pDlgLeftTabSelManager->GetPMSelectionHelper()->GetPMInputUser( ProductSelectionMode_Wizard, ProjectType::Solar )->GetPMWQSelectionPreferences()->Reset();
	pDlgLeftTabSelManager->GetPMSelectionHelper()->GetPMInputUser( ProductSelectionMode_Wizard, ProjectType::Cooling )->GetPMWQSelectionPreferences()->Reset();
	pDlgLeftTabSelManager->GetPMSelectionHelper()->GetPMInputUser( ProductSelectionMode_Wizard, ProjectType::Cooling )->GetPMWQSelectionPreferences()->SetChecked( CPMWQPrefs::PumpDegassingCoolingVersion, true );
	pDlgLeftTabSelManager->GetPMSelectionHelper()->GetPMInputUser( ProductSelectionMode_Wizard, ProjectType::Cooling )->GetPMWQSelectionPreferences()->SetDisabled( CPMWQPrefs::PumpDegassingCoolingVersion, false );

	for( int i = 0; i < m_ArpPage.GetSize(); i++ )
	{
		if( NULL != m_ArpPage[i] )
		{
			m_ArpPage[i]->Init( true );
		}
	}

	EndWaitCursor();
}

void CDlgTechParam::_InitPages()
{
	// Specify the hydronic calculation pages for Bv, Cv, BCv, PICv and Dpc.
	m_PageHCBalancingValves.SetProductSubCategory( ProductSubCategory::PSC_BC_RegulatingValve );
	m_PageHCStandardControlValves.SetProductSubCategory( ProductSubCategory::PSC_BC_ControlValve );
	m_PageHCAdjustableControlValves.SetProductSubCategory( ProductSubCategory::PSC_BC_BalAndCtrlValve );
	m_PageHCPIBCV.SetProductSubCategory( ProductSubCategory::PSC_BC_PressureIndepCtrlValve );
	m_PageHCDpC.SetProductSubCategory( ProductSubCategory::PSC_BC_DpController );
	m_PageHCDpCBCV.SetProductSubCategory( ProductSubCategory::PSC_BC_CombinedDpCBalCtrlValve );

	m_ArpPage.RemoveAll();

	// Create 'General' page.
	m_PageGeneral.Create( IDD_TABDIALOGTECHGEN, this );
	m_PageGeneral.SetPageNumber( eOpeningPage::PageGeneral );
	m_ArpPage.Add( &m_PageGeneral );

	// Create 'General\Pressure maintenance' page.
	if( true == TASApp.IsPressureMaintenanceDisplayed() )
	{
		m_PageGeneralPressureMaintenance.Create( IDD_TABDIALOGTECHPM, this );
		m_PageGeneralPressureMaintenance.SetPageNumber( eOpeningPage::PageGeneralPressureMaintenance );
		m_ArpPage.Add( &m_PageGeneralPressureMaintenance );

		m_PageGeneralWaterQuality.Create( IDD_TABDIALOGTECHWQ, this );
		m_PageGeneralWaterQuality.SetPageNumber( eOpeningPage::PageGeneralWaterQuality );
		m_ArpPage.Add( &m_PageGeneralWaterQuality );
	}

	// Create 'General\Balancing' page.
	m_PageGeneralBalancingValves.Create( IDD_TABDIALOGTECHVALVES, this );
	m_PageGeneralBalancingValves.SetPageNumber( eOpeningPage::PageGeneralBalancingValves );
	m_ArpPage.Add( &m_PageGeneralBalancingValves );

	// Create 'General\Control' page.
	m_PageStdAndAdjustableControlValves.Create( IDD_TABDIALOGTECHCV, this );
	m_PageStdAndAdjustableControlValves.SetPageNumber( eOpeningPage::PageGeneralStdAndAdjustableControlValves );
	m_ArpPage.Add( &m_PageStdAndAdjustableControlValves );

	// Create 'General\Press.-indep. ctrl valves' page.
	if( true == TASApp.IsPICvDisplayed() )
	{
		m_PageGeneralPIBCV.Create( IDD_TABDIALOGTECHPICV, this );
		m_PageGeneralPIBCV.SetPageNumber( eOpeningPage::PageGeneralPIBCV );
		m_ArpPage.Add( &m_PageGeneralPIBCV );
	}

	// Create 'General\Smart ctrl. valves & smart diff. press. controllers' page.
	if( true == TASApp.IsSmartControlValveDisplayed() || true == TASApp.IsSmartDpCDisplayed() )
	{
		m_PageGeneralSmartValves.Create( IDD_TABDIALOGTECHSMARTVALVES, this );
		m_PageGeneralSmartValves.SetPageNumber( eOpeningPage::PageGeneralSmartValves );
		m_ArpPage.Add( &m_PageGeneralSmartValves );
	}

	// Create 'General\Dp controllers' page.
	m_PageGeneralDpControllers.Create( IDD_TABDIALOGTECHDPC, this );
	m_PageGeneralDpControllers.SetPageNumber( eOpeningPage::PageGeneralDpControllers );
	m_ArpPage.Add( &m_PageGeneralDpControllers );

	// Create 'General\Fixed orifices' page.
	m_PageGeneralFixOrifice.Create( IDD_TABDIALOGTECHFIXORIFICE, this );
	m_PageGeneralFixOrifice.SetPageNumber( eOpeningPage::PageGeneralFixOrifice );
	m_ArpPage.Add( &m_PageGeneralFixOrifice );

	// Create 'General\Radiator valves' page.
	if( true == TASApp.IsTrvDisplayed() )
	{
		m_PageGeneralRadiatorValves.Create( IDD_TABDIALOGTECHRAD, this );
		m_PageGeneralRadiatorValves.SetPageNumber( eOpeningPage::PageGeneralRadiatorValves );
		m_ArpPage.Add( &m_PageGeneralRadiatorValves );
	}

	// Create 'General\Pipes' page.
	m_PageGeneralPipes.Create( IDD_TABDIALOGTECHPIPES, this );
	m_PageGeneralPipes.SetPageNumber( eOpeningPage::PageGeneralPipes );
	m_ArpPage.Add( &m_PageGeneralPipes );

	// Create 'General\Device sized' page.
	m_PageGeneralDeviceSizes.Create( IDD_TABDIALOGTECHDEVSIZE, this );
	m_PageGeneralDeviceSizes.SetPageNumber( eOpeningPage::PageGeneralDeviceSizes );
	m_ArpPage.Add( &m_PageGeneralDeviceSizes );

	// Create 'Hydronic circuit calculation' page.
	m_PageMainHC.Create( IDD_TABDIALOGTECHMAINHC, this );
	m_PageMainHC.SetPageNumber( eOpeningPage::PageMainHC );
	m_ArpPage.Add( &m_PageMainHC );

	// Create 'Hydronic circuit calculation\Balancing valves' page.
	m_PageHCBalancingValves.Create( IDD_TABDIALOGTECHHYDROCALC, this );
	m_PageHCBalancingValves.SetPageNumber( eOpeningPage::PageHCBv );
	m_ArpPage.Add( &m_PageHCBalancingValves );

	// Create 'Hydronic circuit calculation\Control valves' page.
	m_PageHCStandardControlValves.Create( IDD_TABDIALOGTECHHYDROCALC, this );
	m_PageHCStandardControlValves.SetPageNumber( eOpeningPage::PageHCStandardControlValves );
	m_ArpPage.Add( &m_PageHCStandardControlValves );

	// Create 'Hydronic circuit calculation\Balancing and control valves' page.
	if( true == TASApp.IsBCvDisplayed() )
	{
		m_PageHCAdjustableControlValves.Create( IDD_TABDIALOGTECHHYDROCALC, this );
		m_PageHCAdjustableControlValves.SetPageNumber( eOpeningPage::PageHCAdjustableControlValves );
		m_ArpPage.Add( &m_PageHCAdjustableControlValves );
	}

	// Create 'Hydronic circuit calculation\Press.-indep. ctrl valves' page.
	if( true == TASApp.IsPICvDisplayed() )
	{
		m_PageHCPIBCV.Create( IDD_TABDIALOGTECHHYDROCALC, this );
		m_PageHCPIBCV.SetPageNumber( eOpeningPage::PageHCPICv );
		m_ArpPage.Add( &m_PageHCPIBCV );
	}

	// Create 'Hydronic circuit calculation\Actuators' page.
	m_PageHCActuators.Create( IDD_TABDIALOGTECHHCACTUATORS, this );
	m_PageHCActuators.SetPageNumber( eOpeningPage::PageHCActuators );
	m_ArpPage.Add( &m_PageHCActuators );

	// Create 'Hydronic circuit calculation\Dp controllers' page.
	m_PageHCDpC.Create( IDD_TABDIALOGTECHHYDROCALC, this );
	m_PageHCDpC.SetPageNumber( eOpeningPage::PageHCDpC );
	m_ArpPage.Add( &m_PageHCDpC );

	// Create 'Hydronic circuit calculation\Combined Dp controller, control and balancing valves' page.
	m_PageHCDpCBCV.Create( IDD_TABDIALOGTECHHYDROCALC, this );
	m_PageHCDpCBCV.SetPageNumber( eOpeningPage::PageHCDpCBCV );
	m_ArpPage.Add( &m_PageHCDpCBCV );

	// HYS-1763: Create 'Hydronic circuit calculation\Smart control valve ' page.
	m_PageHCSmartCV.Create( IDD_TABDIALOGTECHHCSMART, this );
	m_PageHCSmartCV.SetPageNumber( eOpeningPage::PageHCSmartCV );
	m_ArpPage.Add( &m_PageHCSmartCV );

	// Create 'Hydronic circuit calculation\Pipes' page.
	m_PageHCPipes.Create( IDD_TABDIALOGTECHHCPIPES, this );
	m_PageHCPipes.SetPageNumber( eOpeningPage::PageHCPipes );
	m_ArpPage.Add( &m_PageHCPipes );

	// Create 'Hydronic circuit calculation\Pumps' page.
	m_PageHCPumps.Create( IDD_TABDIALOGTECHHCPUMPS, this );
	m_PageHCPumps.SetPageNumber( eOpeningPage::PageHCPumps );
	m_ArpPage.Add( &m_PageHCPumps );
}

bool CDlgTechParam::_CheckPage( HTREEITEM hItem )
{
	CDlgCtrlPropPage *pPage = dynamic_cast<CDlgCtrlPropPage *>( ( CDialogEx * )m_Tree.GetItemData( hItem ) );
	ASSERT( NULL != pPage );

	if( m_iCurrentPage == pPage->GetPageNumber() )
	{
		m_Tree.SelectItem( hItem );
		SetCurPage( pPage );
		DisplayPage( pPage );
		return true;
	}

	return false;
}