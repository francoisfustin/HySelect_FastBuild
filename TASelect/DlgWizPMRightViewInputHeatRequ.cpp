#include "stdafx.h"


#include "TASelect.h"
#include "TASelectDoc.h"
#include "MainFrm.h"
#include "DlgWizPMRightViewInputHeatRequ.h"
#include "RViewWizardSelBase.h"
#include "RViewWizardSelPM.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


CDlgWizardPM_RightViewInput_HeatingRequirement::CDlgWizardPM_RightViewInput_HeatingRequirement( CWizardSelPMParams *pclWizardSelPMParams, CWnd *pclParent )
	: CDlgWizardPM_RightViewInput_Base( pclWizardSelPMParams, HeatingRequirement, IDD, pclParent )
{
	m_bButtonGeneralCombiState = false;
	m_bButtonGeneralRedundancyPumpState = false;
	m_bButtonGeneralRedundancyTecBoxState = false;
	m_bButtonCompBasedCoatingState = false;
	m_bButtonCompBasedCompressedAirState = false;
	m_bButtonPumpDegassingBasedCoolingInsulationState = false;
}

void CDlgWizardPM_RightViewInput_HeatingRequirement::ApplyPMInputUserUpdated( bool bWaterCharUpdated, bool bShowErrorMsg )
{
	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pclSelectPMList 
			|| NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERT_RETURN;
	}

	CPMInputUser *pclPMInputUser = m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser();

	CImageList *pclImgListButton = TASApp.GetpRCImageManager()->GetImageList( CRCImageManager::ILN_PMWQPrefs );

	if( NULL == pclImgListButton )
	{
		return;
	}

	bool bAtLeastOneChange = false;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// General - Combined in on device.
	int iIsChecked = ( true == pclPMInputUser->GetPMWQSelectionPreferences()->IsChecked( CPMWQPrefs::GeneralCombinedInOneDevice ) ) ? BST_CHECKED : BST_UNCHECKED;
	BOOL bIsEnabled = ( true == pclPMInputUser->GetPMWQSelectionPreferences()->IsDisabled( CPMWQPrefs::GeneralCombinedInOneDevice ) ) ? FALSE : TRUE;

	if( m_ButtonGeneralCombi.GetCheck() != iIsChecked )
	{
		m_ButtonGeneralCombi.SetCheck( iIsChecked );
		m_bButtonGeneralCombiState = ( BST_CHECKED == iIsChecked ) ? true : false;
		int iNewImage = ( true == m_bButtonGeneralCombiState ) ? CRCImageManager::ILPMWQP_CombinedSelected: CRCImageManager::ILPMWQP_Combined;
		m_ButtonGeneralCombi.SetIcon( ( HICON )pclImgListButton->ExtractIcon( iNewImage ) );
		bAtLeastOneChange = true;
	}

	// HYS-1537 : When hte option is checked and disabled. Deactivated it.
	if( FALSE == bIsEnabled && true == m_bButtonGeneralCombiState )
	{
		m_bButtonGeneralCombiState = false;
		m_ButtonGeneralCombi.SetIcon( (HICON)pclImgListButton->ExtractIcon( CRCImageManager::ILPMWQP_Combined ) );
	}

	m_ButtonGeneralCombi.EnableWindow( bIsEnabled );
	m_StaticGeneralCombi.EnableWindow( bIsEnabled );
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// General - Redundancy pump/compressors.
	iIsChecked = ( true == pclPMInputUser->GetPMWQSelectionPreferences()->IsChecked( CPMWQPrefs::GeneralRedundancyPumpComp ) ) ? BST_CHECKED : BST_UNCHECKED;
	bIsEnabled = ( true == pclPMInputUser->GetPMWQSelectionPreferences()->IsDisabled( CPMWQPrefs::GeneralRedundancyPumpComp ) ) ? FALSE : TRUE;

	if( m_ButtonGeneralRedundancyPump.GetCheck() != iIsChecked )
	{
		m_ButtonGeneralRedundancyPump.SetCheck( iIsChecked );
		m_bButtonGeneralRedundancyPumpState = ( BST_CHECKED == iIsChecked ) ? true : false;
		int iNewImage = ( true == m_bButtonGeneralRedundancyPumpState ) ? CRCImageManager::ILPMWQP_RedundPumpComprSelected: CRCImageManager::ILPMWQP_RedundPumpCompr;
		m_ButtonGeneralRedundancyPump.SetIcon( ( HICON )pclImgListButton->ExtractIcon( iNewImage ) );
		bAtLeastOneChange = true;
	}
	// HYS-1537 : When hte option is checked and disabled. Deactivated it.
	if( FALSE == bIsEnabled && true == m_bButtonGeneralRedundancyPumpState )
	{
		m_bButtonGeneralRedundancyPumpState = false;
		m_ButtonGeneralRedundancyPump.SetIcon( (HICON)pclImgListButton->ExtractIcon( CRCImageManager::ILPMWQP_RedundPumpCompr ) );
	}
	m_ButtonGeneralRedundancyPump.EnableWindow( bIsEnabled );
	m_StaticGeneralRedundancyPump.EnableWindow( bIsEnabled );
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// General - Redundancy complete TecBox.
	iIsChecked = ( true == pclPMInputUser->GetPMWQSelectionPreferences()->IsChecked( CPMWQPrefs::GeneralRedundancyTecBox ) ) ? BST_CHECKED : BST_UNCHECKED;
	bIsEnabled = ( true == pclPMInputUser->GetPMWQSelectionPreferences()->IsDisabled( CPMWQPrefs::GeneralRedundancyTecBox ) ) ? FALSE : TRUE;

	if( m_ButtonGeneralRedundancyTecBox.GetCheck() != iIsChecked )
	{
		m_ButtonGeneralRedundancyTecBox.SetCheck( iIsChecked );
		m_bButtonGeneralRedundancyTecBoxState = ( BST_CHECKED == iIsChecked ) ? true : false;
		int iNewImage = ( true == m_bButtonGeneralRedundancyTecBoxState ) ? CRCImageManager::ILPMWQP_RedundTecBoxSelected: CRCImageManager::ILPMWQP_RedundTecBox;
		m_ButtonGeneralRedundancyTecBox.SetIcon( ( HICON )pclImgListButton->ExtractIcon( iNewImage ) );
		bAtLeastOneChange = true;
	}
	// HYS-1537 : When hte option is checked and disabled. Deactivated it.
	if( FALSE == bIsEnabled && true == m_bButtonGeneralRedundancyTecBoxState )
	{
		m_bButtonGeneralRedundancyTecBoxState = false;
		m_ButtonGeneralRedundancyTecBox.SetIcon( (HICON)pclImgListButton->ExtractIcon( CRCImageManager::ILPMWQP_RedundTecBox ) );
	}
	m_ButtonGeneralRedundancyTecBox.EnableWindow( bIsEnabled );
	m_StaticGeneralRedundancyTecBox.EnableWindow( bIsEnabled );
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Compresso - Internal coating.
	iIsChecked = ( true == pclPMInputUser->GetPMWQSelectionPreferences()->IsChecked( CPMWQPrefs::CompressoInternalCoating ) ) ? BST_CHECKED : BST_UNCHECKED;
	bIsEnabled = ( true == pclPMInputUser->GetPMWQSelectionPreferences()->IsDisabled( CPMWQPrefs::CompressoInternalCoating ) ) ? FALSE : TRUE;

	if( m_ButtonCompBasedCoating.GetCheck() != iIsChecked )
	{
		m_ButtonCompBasedCoating.SetCheck( iIsChecked );
		m_bButtonCompBasedCoatingState = ( BST_CHECKED == iIsChecked ) ? true : false;
		int iNewImage = ( true == m_bButtonCompBasedCoatingState ) ? CRCImageManager::ILPMWQP_InternalCoatingSelected: CRCImageManager::ILPMWQP_InternalCoating;
		m_ButtonCompBasedCoating.SetIcon( ( HICON )pclImgListButton->ExtractIcon( iNewImage ) );
		bAtLeastOneChange = true;
	}
	// HYS-1537 : When hte option is checked and disabled. Deactivated it.
	if( FALSE == bIsEnabled && true == m_bButtonCompBasedCoatingState )
	{
		m_bButtonCompBasedCoatingState = false;
		m_ButtonCompBasedCoating.SetIcon( (HICON)pclImgListButton->ExtractIcon( CRCImageManager::ILPMWQP_InternalCoating ) );
	}
	m_ButtonCompBasedCoating.EnableWindow( bIsEnabled );
	m_StaticCompBasedCoating.EnableWindow( bIsEnabled );
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Compresso - External air.
	iIsChecked = ( true == pclPMInputUser->GetPMWQSelectionPreferences()->IsChecked( CPMWQPrefs::CompressoExternalAir ) ) ? BST_CHECKED : BST_UNCHECKED;
	bIsEnabled = ( true == pclPMInputUser->GetPMWQSelectionPreferences()->IsDisabled( CPMWQPrefs::CompressoExternalAir ) ) ? FALSE : TRUE;

	if( m_ButtonCompBasedCompressedAir.GetCheck() != iIsChecked )
	{
		m_ButtonCompBasedCompressedAir.SetCheck( iIsChecked );
		m_bButtonCompBasedCompressedAirState = ( BST_CHECKED == iIsChecked ) ? true : false;
		int iNewImage = ( true == m_bButtonCompBasedCompressedAirState ) ? CRCImageManager::ILPMWQP_ExternalAirSelected: CRCImageManager::ILPMWQP_ExternalAir;
		m_ButtonCompBasedCompressedAir.SetIcon( ( HICON )pclImgListButton->ExtractIcon( iNewImage ) );
		bAtLeastOneChange = true;
	}
	// HYS-1537 : When hte option is checked and disabled. Deactivated it.
	if( FALSE == bIsEnabled && true == m_bButtonCompBasedCompressedAirState )
	{
		m_bButtonCompBasedCompressedAirState = false;
		m_ButtonCompBasedCompressedAir.SetIcon( (HICON)pclImgListButton->ExtractIcon( CRCImageManager::ILPMWQP_ExternalAir ) );
	}
	m_ButtonCompBasedCompressedAir.EnableWindow( bIsEnabled );
	m_StaticCompBasedCompressedAir.EnableWindow( bIsEnabled );
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Pump-based and degassing - Cooling version.
	iIsChecked = ( true == pclPMInputUser->GetPMWQSelectionPreferences()->IsChecked( CPMWQPrefs::PumpDegassingCoolingVersion ) ) ? BST_CHECKED : BST_UNCHECKED;
	bIsEnabled = ( true == pclPMInputUser->GetPMWQSelectionPreferences()->IsDisabled( CPMWQPrefs::PumpDegassingCoolingVersion ) ) ? FALSE : TRUE;

	if( m_ButtonPumpDegassingBasedCoolingInsulation.GetCheck() != iIsChecked )
	{
		m_ButtonPumpDegassingBasedCoolingInsulation.SetCheck( iIsChecked );
		m_bButtonPumpDegassingBasedCoolingInsulationState = ( BST_CHECKED == iIsChecked ) ? true : false;
		int iNewImage = ( true == m_bButtonPumpDegassingBasedCoolingInsulationState ) ? CRCImageManager::ILPMWQP_InsulatedVaporSealingSelected: CRCImageManager::ILPMWQP_InsulatedVaporSealing;
		m_ButtonPumpDegassingBasedCoolingInsulation.SetIcon( ( HICON )pclImgListButton->ExtractIcon( iNewImage ) );
		bAtLeastOneChange = true;
	}
	// HYS-1537 : When hte option is checked and disabled. Deactivated it.
	if( FALSE == bIsEnabled && true == m_bButtonPumpDegassingBasedCoolingInsulationState )
	{
		m_bButtonPumpDegassingBasedCoolingInsulationState = false;
		m_ButtonPumpDegassingBasedCoolingInsulation.SetIcon( (HICON)pclImgListButton->ExtractIcon( CRCImageManager::ILPMWQP_InsulatedVaporSealing ) );
	}
	m_ButtonPumpDegassingBasedCoolingInsulation.EnableWindow( bIsEnabled );
	m_StaticPumpDegassingBasedCoolingInsulation.EnableWindow( bIsEnabled );
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	if( true == bAtLeastOneChange )
	{
		_VerifyInputValues();
	}
}

BEGIN_MESSAGE_MAP( CDlgWizardPM_RightViewInput_HeatingRequirement, CDlgWizardPM_RightViewInput_Base )
	ON_BN_CLICKED( IDC_BUTTON_COMBI, OnBnClickedGeneralCombi )
	ON_BN_CLICKED( IDC_BUTTON_REDUNDPUMP, OnBnClickedGeneralRedundancyPump )
	ON_BN_CLICKED( IDC_BUTTON_REDUNDTECBOX, OnBnClickedGeneralRedundancyTecBox )
	ON_BN_CLICKED( IDC_BUTTON_COATING, OnBnClickedCompBasedCoating )
	ON_BN_CLICKED( IDC_BUTTON_COMPRESSAIR, OnBnClickedCompBasedCompressedAir )
	ON_BN_CLICKED( IDC_BUTTON_COOLINGINSULATION, OnBnClickedPumpDegasingBasedCoolingInsulation )
END_MESSAGE_MAP()

void CDlgWizardPM_RightViewInput_HeatingRequirement::DoDataExchange( CDataExchange *pDX )
{
	CDlgWizardPM_RightViewInput_Base::DoDataExchange( pDX );

	DDX_Control( pDX, IDC_GROUPGENERAL, m_GroupGeneral );
	DDX_Control( pDX, IDC_GROUPCOMP, m_GroupCompressor );
	DDX_Control( pDX, IDC_GROUPPUMPDEGASSING, m_GroupPumpDegassing );

	DDX_Control( pDX, IDC_STATIC_COMBI, m_StaticGeneralCombi );
	DDX_Control( pDX, IDC_STATIC_REDUNDPUMP, m_StaticGeneralRedundancyPump );
	DDX_Control( pDX, IDC_STATIC__REDUNDTECBOX, m_StaticGeneralRedundancyTecBox );
	DDX_Control( pDX, IDC_STATIC_COATING, m_StaticCompBasedCoating );
	DDX_Control( pDX, IDC_STATIC_COMPRESSAIR, m_StaticCompBasedCompressedAir );
	DDX_Control( pDX, IDC_STATIC_COOLINGINSULATION, m_StaticPumpDegassingBasedCoolingInsulation );

	DDX_Control( pDX, IDC_BUTTON_COMBI, m_ButtonGeneralCombi );
	DDX_Control( pDX, IDC_BUTTON_REDUNDPUMP, m_ButtonGeneralRedundancyPump );
	DDX_Control( pDX, IDC_BUTTON_REDUNDTECBOX, m_ButtonGeneralRedundancyTecBox );
	DDX_Control( pDX, IDC_BUTTON_COATING, m_ButtonCompBasedCoating );
	DDX_Control( pDX, IDC_BUTTON_COMPRESSAIR, m_ButtonCompBasedCompressedAir );
	DDX_Control( pDX, IDC_BUTTON_COOLINGINSULATION, m_ButtonPumpDegassingBasedCoolingInsulation );
}

BOOL CDlgWizardPM_RightViewInput_HeatingRequirement::OnInitDialog()
{
	CDlgWizardPM_RightViewInput_Base::OnInitDialog();

	SetMainTitle( TASApp.LoadLocalizedString( IDS_DLGWIZPM_RVIEWHEATREQU_MAINTITLE ) );

	CString str;
	CImageList *pclImgListButton = TASApp.GetpRCImageManager()->GetImageList( CRCImageManager::ILN_PMWQPrefs );

	//////////////////////////////////////////////////////////////////////////
	// GROUP
	m_GroupGeneral.SetWindowTextW( TASApp.LoadLocalizedString( IDS_DLGWIZPM_RVIEWINPUTALLREQU_GROUPGENERAL ) );
	m_GroupGeneral.SetBckgndColor( RGB( 255, 255, 255 ) );

	m_GroupCompressor.SetWindowTextW( TASApp.LoadLocalizedString( IDS_DLGWIZPM_RVIEWINPUTALLREQU_GROUPCOMPRESSOR ) );
	m_GroupCompressor.SetBckgndColor( RGB( 255, 255, 255 ) );

	m_GroupPumpDegassing.SetWindowTextW( TASApp.LoadLocalizedString( IDS_DLGWIZPM_RVIEWINPUTALLREQU_GROUPPUMPDEGASSING ) );
	m_GroupPumpDegassing.SetBckgndColor( RGB( 255, 255, 255 ) );

	//////////////////////////////////////////////////////////////////////////
	// BUTTON

	// General: Combination of functions preferably in one device.
	m_ButtonGeneralCombi.SetIcon( ( HICON )pclImgListButton->ExtractIcon( CRCImageManager::ILPMWQP_Combined ) );
	m_bButtonGeneralCombiState = false;

	// General: Redundancy of pumps / compressors.
	m_ButtonGeneralRedundancyPump.SetIcon( ( HICON )pclImgListButton->ExtractIcon( CRCImageManager::ILPMWQP_RedundPumpCompr ) );
	m_bButtonGeneralRedundancyPumpState = false;

	// General: Redundancy of complete TecBox
	m_ButtonGeneralRedundancyTecBox.SetIcon( ( HICON )pclImgListButton->ExtractIcon( CRCImageManager::ILPMWQP_RedundTecBox ) );
	m_bButtonGeneralRedundancyTecBoxState = false;

	// Compressor-based: Expansion vessel with internal coating.
	m_ButtonCompBasedCoating.SetIcon( ( HICON )pclImgListButton->ExtractIcon( CRCImageManager::ILPMWQP_InternalCoating ) );
	m_bButtonCompBasedCoatingState = false;

	// Compressor-based: Device working with external compressed air.
	m_ButtonCompBasedCompressedAir.SetIcon( ( HICON )pclImgListButton->ExtractIcon( CRCImageManager::ILPMWQP_ExternalAir ) );
	m_bButtonCompBasedCompressedAirState = false;

	// Pump-based and vacuum degassing: Cooling insulation with condensation water protection.
	m_ButtonPumpDegassingBasedCoolingInsulation.SetIcon( ( HICON )pclImgListButton->ExtractIcon( CRCImageManager::ILPMWQP_InsulatedVaporSealing ) );
	m_bButtonPumpDegassingBasedCoolingInsulationState = false;


	//////////////////////////////////////////////////////////////////////////
	// STATIC
	
	// Combination of functions preferably in one device.
	str = TASApp.LoadLocalizedString( IDS_DLGWIZPM_RVIEWINPUTALLREQU_COMBI );
	m_StaticGeneralCombi.SetWindowText( str );

	// Redundancy of pumps / compressors.
	str = TASApp.LoadLocalizedString( IDS_DLGWIZPM_RVIEWINPUTALLREQU_REDUNDPUMP );
	m_StaticGeneralRedundancyPump.SetWindowText( str );

	// Redundancy of complete TecBox.
	str = TASApp.LoadLocalizedString( IDS_DLGWIZPM_RVIEWINPUTALLREQU_REDUNDTECBOX );
	m_StaticGeneralRedundancyTecBox.SetWindowText( str );

	// Expansion vessel with internal coating.
	str = TASApp.LoadLocalizedString( IDS_DLGWIZPM_RVIEWINPUTALLREQU_COATING );
	m_StaticCompBasedCoating.SetWindowText( str );

	// Device working with external compressed air.
	str = TASApp.LoadLocalizedString( IDS_DLGWIZPM_RVIEWINPUTALLREQU_COMPRESSAIR );
	m_StaticCompBasedCompressedAir.SetWindowText( str );

	// Cooling insulation with condensation water protection.
	str = TASApp.LoadLocalizedString( IDS_DLGWIZPM_RVIEWINPUTALLREQU_COOLINGINSULATION );
	m_StaticPumpDegassingBasedCoolingInsulation.SetWindowText( str );

	m_StaticGeneralCombi.SetBackColor( RGB( 255, 255, 255 ) );
	m_StaticGeneralRedundancyPump.SetBackColor( RGB( 255, 255, 255 ) );
	m_StaticGeneralRedundancyTecBox.SetBackColor( RGB( 255, 255, 255 ) );
	m_StaticCompBasedCoating.SetBackColor( RGB( 255, 255, 255 ) );
	m_StaticCompBasedCompressedAir.SetBackColor( RGB( 255, 255, 255 ) );
	m_StaticPumpDegassingBasedCoolingInsulation.SetBackColor( RGB( 255, 255, 255 ) );

	ApplyPMInputUserUpdated();

	return TRUE;
}

void CDlgWizardPM_RightViewInput_HeatingRequirement::OnSize( UINT nType, int cx, int cy )
{
	CDlgWizardPM_RightViewInput_Base::OnSize( nType, cx, cy );

	if( NULL == GetDlgItem( IDC_GROUPGENERAL ) || NULL == GetDlgItem( IDC_GROUPGENERAL )->GetSafeHwnd() )
	{
		// Not yet ready.
		return;
	}

	CRect rectClient;
	GetClientRect( &rectClient );

	// Resize 'General' group.
	CRect rectGroup;
	GetDlgItem( IDC_GROUPGENERAL )->GetClientRect( &rectGroup );
	GetDlgItem( IDC_GROUPGENERAL )->SetWindowPos( NULL, -1, -1, rectClient.Width() - 8, rectGroup.Height(), SWP_NOMOVE | SWP_NOZORDER );

	// Resize 'Compressor-based' group.
	GetDlgItem( IDC_GROUPCOMP )->GetClientRect( &rectGroup );
	GetDlgItem( IDC_GROUPCOMP )->SetWindowPos( NULL, -1, -1, rectClient.Width() - 8, rectGroup.Height(), SWP_NOMOVE | SWP_NOZORDER );

	// Resize 'Pump-based and vacuum degassing' group.
	GetDlgItem( IDC_GROUPPUMPDEGASSING )->GetClientRect( &rectGroup );
	GetDlgItem( IDC_GROUPPUMPDEGASSING )->SetWindowPos( NULL, -1, -1, rectClient.Width() - 8, rectGroup.Height(), SWP_NOMOVE | SWP_NOZORDER );
}

void CDlgWizardPM_RightViewInput_HeatingRequirement::OnBnClickedGeneralCombi()
{
	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pclSelectPMList || NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser() 
			|| NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetPMWQSelectionPreferences()  )
	{
		ASSERT_RETURN;
	}

	CImageList *pclImgListButton = TASApp.GetpRCImageManager()->GetImageList( CRCImageManager::ILN_PMWQPrefs );
	m_bButtonGeneralCombiState = !m_bButtonGeneralCombiState;
	int iNewImage = ( true == m_bButtonGeneralCombiState ) ? CRCImageManager::ILPMWQP_CombinedSelected: CRCImageManager::ILPMWQP_Combined;
	m_ButtonGeneralCombi.SetIcon( ( HICON )pclImgListButton->ExtractIcon( iNewImage ) );
	m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetPMWQSelectionPreferences()->SetChecked( CPMWQPrefs::GeneralCombinedInOneDevice, m_bButtonGeneralCombiState );
}

void CDlgWizardPM_RightViewInput_HeatingRequirement::OnBnClickedGeneralRedundancyPump()
{
	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pclSelectPMList || NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser() 
			|| NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetPMWQSelectionPreferences()  )
	{
		ASSERT_RETURN;
	}

	CImageList *pclImgListButton = TASApp.GetpRCImageManager()->GetImageList( CRCImageManager::ILN_PMWQPrefs );
	m_bButtonGeneralRedundancyPumpState = !m_bButtonGeneralRedundancyPumpState;
	int iNewImage = ( true == m_bButtonGeneralRedundancyPumpState ) ? CRCImageManager::ILPMWQP_RedundPumpComprSelected: CRCImageManager::ILPMWQP_RedundPumpCompr;
	m_ButtonGeneralRedundancyPump.SetIcon( ( HICON )pclImgListButton->ExtractIcon( iNewImage ) );
	m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetPMWQSelectionPreferences()->SetChecked( CPMWQPrefs::GeneralRedundancyPumpComp, m_bButtonGeneralRedundancyPumpState );
}

void CDlgWizardPM_RightViewInput_HeatingRequirement::OnBnClickedGeneralRedundancyTecBox()
{
	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pclSelectPMList || NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser() 
			|| NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetPMWQSelectionPreferences()  )
	{
		ASSERT_RETURN;
	}

	CImageList *pclImgListButton = TASApp.GetpRCImageManager()->GetImageList( CRCImageManager::ILN_PMWQPrefs );
	m_bButtonGeneralRedundancyTecBoxState = !m_bButtonGeneralRedundancyTecBoxState;
	int iNewImage = ( true == m_bButtonGeneralRedundancyTecBoxState ) ? CRCImageManager::ILPMWQP_RedundTecBoxSelected: CRCImageManager::ILPMWQP_RedundTecBox;
	m_ButtonGeneralRedundancyTecBox.SetIcon( ( HICON )pclImgListButton->ExtractIcon( iNewImage ) );
	m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetPMWQSelectionPreferences()->SetChecked( CPMWQPrefs::GeneralRedundancyTecBox, m_bButtonGeneralRedundancyTecBoxState );
}

void CDlgWizardPM_RightViewInput_HeatingRequirement::OnBnClickedCompBasedCoating()
{
	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pclSelectPMList || NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser() 
			|| NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetPMWQSelectionPreferences()  )
	{
		ASSERT_RETURN;
	}

	CImageList *pclImgListButton = TASApp.GetpRCImageManager()->GetImageList( CRCImageManager::ILN_PMWQPrefs );
	m_bButtonCompBasedCoatingState = !m_bButtonCompBasedCoatingState;
	int iNewImage = ( true == m_bButtonCompBasedCoatingState ) ? CRCImageManager::ILPMWQP_InternalCoatingSelected: CRCImageManager::ILPMWQP_InternalCoating;
	m_ButtonCompBasedCoating.SetIcon( ( HICON )pclImgListButton->ExtractIcon( iNewImage ) );
	m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetPMWQSelectionPreferences()->SetChecked( CPMWQPrefs::CompressoInternalCoating, m_bButtonCompBasedCoatingState );
}

void CDlgWizardPM_RightViewInput_HeatingRequirement::OnBnClickedCompBasedCompressedAir()
{
	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pclSelectPMList || NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser() 
			|| NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetPMWQSelectionPreferences()  )
	{
		ASSERT_RETURN;
	}

	CImageList *pclImgListButton = TASApp.GetpRCImageManager()->GetImageList( CRCImageManager::ILN_PMWQPrefs );
	m_bButtonCompBasedCompressedAirState = !m_bButtonCompBasedCompressedAirState;
	int iNewImage = ( true == m_bButtonCompBasedCompressedAirState ) ? CRCImageManager::ILPMWQP_ExternalAirSelected: CRCImageManager::ILPMWQP_ExternalAir;
	m_ButtonCompBasedCompressedAir.SetIcon( ( HICON )pclImgListButton->ExtractIcon( iNewImage ) );
	m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetPMWQSelectionPreferences()->SetChecked( CPMWQPrefs::CompressoExternalAir, m_bButtonCompBasedCompressedAirState );
}

void CDlgWizardPM_RightViewInput_HeatingRequirement::OnBnClickedPumpDegasingBasedCoolingInsulation()
{
	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pclSelectPMList || NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser() 
			|| NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetPMWQSelectionPreferences()  )
	{
		ASSERT_RETURN;
	}

	CImageList *pclImgListButton = TASApp.GetpRCImageManager()->GetImageList( CRCImageManager::ILN_PMWQPrefs );
	m_bButtonPumpDegassingBasedCoolingInsulationState = !m_bButtonPumpDegassingBasedCoolingInsulationState;
	int iNewImage = ( true == m_bButtonPumpDegassingBasedCoolingInsulationState ) ? CRCImageManager::ILPMWQP_InsulatedVaporSealingSelected: CRCImageManager::ILPMWQP_InsulatedVaporSealing;
	m_ButtonPumpDegassingBasedCoolingInsulation.SetIcon( ( HICON )pclImgListButton->ExtractIcon( iNewImage ) );
	m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetPMWQSelectionPreferences()->SetChecked( CPMWQPrefs::PumpDegassingCoolingVersion, m_bButtonPumpDegassingBasedCoolingInsulationState );
}

void CDlgWizardPM_RightViewInput_HeatingRequirement::_VerifyInputValues()
{
	// Nothing to verify for the moment.
}
