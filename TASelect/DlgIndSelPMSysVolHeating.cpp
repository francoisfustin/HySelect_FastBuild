#include "stdafx.h"

#include "TASelect.h"
#include "TASelectDoc.h"
#include "MainFrm.h"
#include "Global.h"
#include "Utilities.h"
#include "Units.h"
#include "SSheet.h"
#include "Select.h"
#include "SelectPM.h"
#include "RViewDescription.h"
#include "ProductSelectionParameters.h"
#include "DlgIndSelAdditionalPiping.h"
#include "DlgIndSelPMSysVolHeating.h"
#include "Ssdllmfc.h"
#include "RViewDescription.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CDlgIndSelPMSysVolHeating::CDlgIndSelPMSysVolHeating( CIndSelPMParams *pclIndSelPMParams, double dSupplyTemp, double dReturnTemp, CWnd *pParent )
	: CDialogExt( IDD_DLGINDSELPMVOLSYSHEAT, pParent )
{
	m_pclIndSelPMParams = pclIndSelPMParams;
	m_bChangesDone = false;
	m_pclSDescGenerator = 0;
	m_pclSDescConsumer = 0;
	m_pclSSheetGenerator = 0;
	m_pclSSheetConsumer = 0;
	m_iHeatGeneratorCount = 0;
	m_iConsumerCount = 0;
	m_dSupplyTemperature = dSupplyTemp;
	m_dReturnTemperature = dReturnTemp;
	m_bBufferValid = true;
	m_bGeneratorValid = true;
	m_bConsumerValid = true;
	m_HeatGeneratorRectClient = CRect( 0, 0, 0, 0 );
	m_HeatConsumerRectClient = CRect( 0, 0, 0, 0 );
}

CDlgIndSelPMSysVolHeating::~CDlgIndSelPMSysVolHeating()
{
	while( m_ViewDescription.GetSheetNumber() )
	{
		CSheetDescription *pclSheetDescription = m_ViewDescription.GetTopSheetDescription();
		CSSheet *pclSSheet = pclSheetDescription->GetSSheetPointer();
		pclSSheet->Detach();
		m_ViewDescription.RemoveOneSheetDescription( pclSheetDescription->GetSheetDescriptionID() );
		delete pclSSheet;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PROTECTED METHODS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP( CDlgIndSelPMSysVolHeating, CDialogExt )
	ON_BN_CLICKED( IDC_CHECKGENPROTECT, &CDlgIndSelPMSysVolHeating::OnBnClickedCheckGenProtect )
	ON_BN_CLICKED( IDC_ADDPIPES, &CDlgIndSelPMSysVolHeating::OnBnClickedAddPipes )
	ON_BN_CLICKED( IDC_RESETALL, &CDlgIndSelPMSysVolHeating::OnBnClickedResetAll )
	ON_BN_CLICKED( IDCANCEL, &CDlgIndSelPMSysVolHeating::OnBnClickedCancel )
	ON_BN_CLICKED( IDOK, &CDlgIndSelPMSysVolHeating::OnBnClickedOk )
	ON_EN_CHANGE( IDC_EDITBUFFER, &CDlgIndSelPMSysVolHeating::OnChangeBuffer )
	ON_MESSAGE( SSM_COMBOCLOSEUP, &CDlgIndSelPMSysVolHeating::OnComboDropCloseUp )
	ON_MESSAGE( SSM_CLICK, &CDlgIndSelPMSysVolHeating::OnSSClick )
	ON_MESSAGE( SSM_DBLCLK, &CDlgIndSelPMSysVolHeating::OnSSDblClick )
	ON_MESSAGE( SSM_EDITCHANGE, &CDlgIndSelPMSysVolHeating::OnSSEditChange )
	ON_MESSAGE( SSM_LEAVECELL, &CDlgIndSelPMSysVolHeating::OnSSLeaveCell )
	// Spread DLL messages
	ON_MESSAGE( SSM_TEXTTIPFETCH, &CDlgIndSelPMSysVolHeating::OnTextTipFetch )
END_MESSAGE_MAP()

void CDlgIndSelPMSysVolHeating::DoDataExchange( CDataExchange *pDX )
{
	CDialogExt::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_CHECKGENPROTECT, m_clHeatGeneratorProtect );
	DDX_Control( pDX, IDC_EDITBUFFER, m_clEditOtherBuffer );
	DDX_Control( pDX, IDC_EDITPIPES, m_clEditAdditionalPiping );
	DDX_Control( pDX, IDC_STATICTOTALGENHEATCAPACITY, m_clStaticTotalGenHeatCapacity );
	DDX_Control( pDX, IDC_STATICTOTALGENCONTENT, m_clStaticTotalGenContent );
	DDX_Control( pDX, IDC_STATICTARGETLINEARDP, m_clStaticTargetLinearDp );
	DDX_Control( pDX, IDC_STATICTOTALCONSHEATCAPACITY, m_clStaticTotalConsHeatCapacity );
	DDX_Control( pDX, IDC_STATICTOTALCONSCONTENT, m_clStaticTotalConsContent );
	DDX_Control( pDX, IDC_STATICTOTALCONTENT, m_clStaticTotalContent );
	DDX_Control( pDX, IDC_STATICTOTALINSTALL, m_clStaticTotalInstallation );
}

BOOL CDlgIndSelPMSysVolHeating::OnInitDialog()
{
	if( NULL == m_pclIndSelPMParams || NULL == m_pclIndSelPMParams->m_pclSelectPMList || NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()
			|| NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetHeatGeneratorList()
			|| NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetHeatConsumersList()
			|| NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetHeatingPipeList() )
	{
		ASSERTA_RETURN( FALSE );
	}

	CDialogExt::OnInitDialog();

	CString str;
	str = TASApp.LoadLocalizedString( IDS_INDSELPMSYSVOLHEATING_CAPTION );
	SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_INDSELPMSYSVOLHEATING_HEATGENERATOR );
	GetDlgItem( IDC_STATICHEATGENERATOR )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_INDSELPMSYSVOLHEATING_CHECKGENPROTECT );
	GetDlgItem( IDC_CHECKGENPROTECT )->SetWindowText( str );
	
	if( false == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->IsNorm( ProjectType::All, PressurisationNorm::PN_SWKIHE301_01 ) )
	{
		str = TASApp.LoadLocalizedString( IDS_INDSELPMSYSVOLHEATING_OTHERBUFFER );
	}
	else
	{
		str = TASApp.LoadLocalizedString( IDS_INDSELPMSYSVOLHEATING_OTHER );
	}
	
	GetDlgItem( IDC_STATICBUFFER )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_INDSELPMSYSVOLHEATING_ADDPIPES );
	GetDlgItem( IDC_STATICADDITIONALPIPES )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_INDSELPMSYSVOLHEATING_CONSUMERS );
	GetDlgItem( IDC_STATICCONSUMERS )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_INDSELPMSYSVOLHEATING_TOTALINSTALL );
	GetDlgItem( IDC_STATICTOTALINSTALL )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_INDSELPMSYSVOLHEATING_RESET );
	GetDlgItem( IDC_RESETALL )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_CANCEL );
	GetDlgItem( IDCANCEL )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_OK );
	GetDlgItem( IDOK )->SetWindowText( str );

	// Add units.
	TCHAR tcName[_MAXCHARS];
	CUnitDatabase *pclUnitDB = CDimValue::AccessUDB();
	GetNameOf( pclUnitDB->GetDefaultUnit( _U_VOLUME ), tcName );
	SetDlgItemText( IDC_STATICUNITBUFFER, tcName );
	SetDlgItemText( IDC_STATICUNITADDPIPES, tcName );

	// Set some static text in bold.
	m_clStaticTotalGenHeatCapacity.SetFontBold( true );
	m_clStaticTotalGenContent.SetFontBold( true );
	m_clStaticTotalConsHeatCapacity.SetFontBold( true );
	m_clStaticTotalConsContent.SetFontBold( true );
	m_clStaticTotalContent.SetFontBold( true );
	m_clStaticTotalInstallation.SetFontBold( true );

	CButton *pButtonAddPipe = ( CButton * )GetDlgItem( IDC_ADDPIPES );

	if( NULL != pButtonAddPipe && NULL != pButtonAddPipe->GetSafeHwnd() )
	{
		pButtonAddPipe->SetIcon( ( HICON )LoadImage( AfxGetApp()->m_hInstance, MAKEINTRESOURCE( IDI_PIPECALCULATOR ),
				IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR ) );
	}

	CHeatGeneratorList *pclHeatGeneratorsList = m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetHeatGeneratorList();
	CHeatConsumersList *pclHeatConsumersList = m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetHeatConsumersList();

	m_clHeatGeneratorProtect.SetCheck( pclHeatGeneratorsList->GetHeatHeneratorProtect() );
	m_clEditOtherBuffer.SetWindowTextW( WriteCUDouble( _U_VOLUME, pclHeatConsumersList->GetBufferContent() ) );

	// Target linear pressure drop.
	double dSPam = m_pclIndSelPMParams->m_pTADS->GetpTechParams()->GetPipeTargDp();
	
	FormatString( str, IDS_INDSELPMSYSVOLHEATING_TARGETLINEARDP, WriteCUDouble( _U_LINPRESSDROP, dSPam, true ) );
	m_clStaticTargetLinearDp.SetWindowText( str );
	m_clStaticTargetLinearDp.SetToolTip( TASApp.LoadLocalizedString( IDS_INDSELPMSYSVOLHEATING_TARGETLINEARDPTT ) );

	_InitializeSSheet();
	_LoadHeatGenerator();
	_ButtonPlusGeneratorEnabled();
	_LoadHeatConsumer();
	_ButtonPlusConsumerEnabled();
	_UpdateStaticValue();

	// Vessel computing for heat protection are not well done.
	m_clHeatGeneratorProtect.EnableWindow( FALSE );
	m_clHeatGeneratorProtect.ShowWindow( SW_HIDE );

	return TRUE;
}

void CDlgIndSelPMSysVolHeating::_InitializeSSheet( void )
{
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();

	// Pay attention: because in this case I use TSpread directly as a custom control (see IDD_DLGPIPEDP in the resource), if you want to
	// use 'CSSheet' possibility, we have to attach TSpread CWnd to CSSheet newly created. And the destruction of the dialog, don't forget
	// to call 'Detach' before deleting 'CSSheet'.
	m_pclSSheetGenerator = new CSSheet();
	m_pclSSheetGenerator->Attach( GetDlgItem( IDC_SSHEATGENLIST )->GetSafeHwnd() );
	m_pclSSheetGenerator->Reset();

	m_pclSDescGenerator = m_ViewDescription.AddSheetDescription( SD_HeatGenerator, -1, m_pclSSheetGenerator, CPoint( 0, 0 ) );

	if( NULL == m_pclSDescGenerator )
	{
		return;
	}

	m_pclSSheetGenerator->SetBool( SSB_ALLOWDRAGDROP, FALSE );
	m_pclSSheetGenerator->SetBool( SSB_HORZSCROLLBAR, FALSE );
	m_pclSSheetGenerator->SetBool( SSB_VERTSCROLLBAR, TRUE );
	m_pclSSheetGenerator->SetBool( SSB_MOVEACTIVEONFOCUS, FALSE );
	m_pclSSheetGenerator->SetBool( SSB_EDITMODEPERMANENT, FALSE );
	m_pclSSheetGenerator->SetEditEnterAction( SS_ENTERACTION_SAME );
	m_pclSSheetGenerator->SetButtonDrawMode( SS_BDM_ALWAYSCOMBO );
	m_pclSSheetGenerator->SetTextTip( SS_TT_STATUS_FLOATING, 250, NULL, _LIGHTYELLOW, _TAH_BLACK );

	// Set max rows.
	m_pclSSheetGenerator->SetMaxRows( RD_HeatGenerator_FirstAvailRow );
	m_pclSSheetGenerator->SetFreeze( 0, RD_HeatGenerator_FirstAvailRow - 1 );

	// All cells are static by default and filled with _T("").
	m_pclSSheetGenerator->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	m_pclSSheetGenerator->FormatStaticText( -1, -1, -1, -1, _T( "" ) );

	// Add columns.
	m_pclSDescGenerator->AddColumn( CD_HeatGenerator_FirstColumn, 1 );
	m_pclSDescGenerator->AddColumn( CD_HeatGenerator_Generator, 1 );
	m_pclSDescGenerator->AddColumn( CD_HeatGenerator_Capacity, 1 );
	m_pclSDescGenerator->AddColumn( CD_HeatGenerator_Content, 1 );
	m_pclSDescGenerator->AddColumn( CD_HeatGenerator_ManualContent, 1 );

	// Resize columns.
	_CheckHeatGeneratorColumnWidth();

	// Row name.
	m_pclSSheetGenerator->SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );
	m_pclSSheetGenerator->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontSize, ( LPARAM )8 );
	m_pclSSheetGenerator->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, ( LPARAM )_TAH_TITLE_MAIN );
	m_pclSSheetGenerator->SetStaticText( CD_HeatGenerator_FirstColumn, RD_HeatGenerator_ColName, _T( "" ) );
	m_pclSSheetGenerator->SetStaticText( CD_HeatGenerator_FirstColumn, RD_HeatGenerator_UnitName, _T( "" ) );
	m_pclSSheetGenerator->SetStaticText( CD_HeatGenerator_Generator, RD_HeatGenerator_ColName, IDS_INDSELPMSYSVOLHEATING_HEATGENERATOR );
	m_pclSSheetGenerator->SetStaticText( CD_HeatGenerator_Generator, RD_HeatGenerator_UnitName, _T( "" ) );
	m_pclSSheetGenerator->SetStaticText( CD_HeatGenerator_Capacity, RD_HeatGenerator_ColName, IDS_INDSELPMSYSVOLHEATING_CAPACITY );
	m_pclSSheetGenerator->SetStaticText( CD_HeatGenerator_Capacity, RD_HeatGenerator_UnitName, GetNameOf( pUnitDB->GetDefaultUnit( _U_TH_POWER ) ).c_str() );
	m_pclSSheetGenerator->SetStaticText( CD_HeatGenerator_Content, RD_HeatGenerator_ColName, IDS_INDSELPMSYSVOLHEATING_CONTENT );
	m_pclSSheetGenerator->SetStaticText( CD_HeatGenerator_Content, RD_HeatGenerator_UnitName, GetNameOf( pUnitDB->GetDefaultUnit( _U_VOLUME ) ).c_str() );
	
	// Manual content checkbox.
	m_pclSSheetGenerator->SetStaticText( CD_HeatGenerator_ManualContent, RD_HeatGenerator_ColName, _T( "" ) );
	m_pclSSheetGenerator->SetPictureCellWithID( IDI_PENCILDIAGSHORT, CD_HeatGenerator_ManualContent, RD_HeatGenerator_UnitName, CSSheet::PictureCellType::Icon );
	m_pclSSheetGenerator->SetBackColor( CD_HeatGenerator_ManualContent, RD_HeatGenerator_UnitName, _TAH_ORANGE );
	
	// To fill an icon.
	m_pclSSheetGenerator->SetPictureCellWithID( IDI_PLUSGREEN, CD_HeatGenerator_FirstColumn, RD_HeatGenerator_FirstAvailRow, CSSheet::PictureCellType::Icon );

	m_pclSSheetGenerator->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );

	// Pay attention: because in this case I use TSpread directly as a custom control (see IDD_DLGPIPEDP in the resource), if you want to
	// use 'CSSheet' possibility, we have to attach TSpread CWnd to CSSheet newly created. And the destruction of the dialog, don't forget
	// to call 'Detach' before deleting 'CSSheet'.
	m_pclSSheetConsumer = new CSSheet();
	m_pclSSheetConsumer->Attach( GetDlgItem( IDC_SSHEATCONSLIST )->GetSafeHwnd() );
	m_pclSSheetConsumer->Reset();

	m_pclSDescConsumer = m_ViewDescription.AddSheetDescription( SD_HeatConsumer, -1, m_pclSSheetConsumer, CPoint( 0, 0 ) );

	if( NULL == m_pclSDescConsumer )
	{
		return;
	}

	m_pclSSheetConsumer->SetBool( SSB_ALLOWDRAGDROP, FALSE );
	m_pclSSheetConsumer->SetBool( SSB_HORZSCROLLBAR, FALSE );
	m_pclSSheetConsumer->SetBool( SSB_VERTSCROLLBAR, TRUE );
	m_pclSSheetConsumer->SetBool( SSB_MOVEACTIVEONFOCUS, FALSE );
	m_pclSSheetConsumer->SetBool( SSB_EDITMODEPERMANENT, FALSE );
	m_pclSSheetConsumer->SetEditEnterAction( SS_ENTERACTION_SAME );
	m_pclSSheetConsumer->SetButtonDrawMode( SS_BDM_ALWAYSCOMBO );
	m_pclSSheetConsumer->SetTextTip( SS_TT_STATUS_FLOATING, 250, NULL, _LIGHTYELLOW, _TAH_BLACK );

	// Set max rows.
	m_pclSSheetConsumer->SetMaxRows( RD_HeatGenerator_FirstAvailRow );
	m_pclSSheetConsumer->SetFreeze( 0, RD_HeatGenerator_FirstAvailRow - 1 );

	// All cells are static by default and filled with _T("").
	m_pclSSheetConsumer->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	m_pclSSheetConsumer->FormatStaticText( -1, -1, -1, -1, _T( "" ) );

	// Add columns.
	m_pclSDescConsumer->AddColumnInPixels( CD_HeatConsumer_FirstColumn, 1 );
	m_pclSDescConsumer->AddColumnInPixels( CD_HeatConsumer_Consumer, 1 );
	m_pclSDescConsumer->AddColumnInPixels( CD_HeatConsumer_Capacity, 1 );
	m_pclSDescConsumer->AddColumnInPixels( CD_HeatConsumer_Content, 1 );
	m_pclSDescConsumer->AddColumnInPixels( CD_HeatConsumer_ManualContent, 1 );
	m_pclSDescConsumer->AddColumnInPixels( CD_HeatConsumer_SupplyTemp, 1 );
	m_pclSDescConsumer->AddColumnInPixels( CD_HeatConsumer_ReturnTemp, 1 );
	m_pclSDescConsumer->AddColumnInPixels( CD_HeatConsumer_ManualTemp, 1 );

	// Resize columns.
	_CheckHeatConsumerColumnWidth();

	// Row name.
	m_pclSSheetConsumer->SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );
	m_pclSSheetConsumer->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontSize, ( LPARAM )8 );
	m_pclSSheetConsumer->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, ( LPARAM )_TAH_TITLE_MAIN );
	m_pclSSheetConsumer->SetStaticText( CD_HeatConsumer_FirstColumn, RD_HeatGenerator_ColName, _T( "" ) );
	m_pclSSheetConsumer->SetStaticText( CD_HeatConsumer_FirstColumn, RD_HeatGenerator_UnitName, _T( "" ) );
	m_pclSSheetConsumer->SetStaticText( CD_HeatConsumer_Consumer, RD_HeatGenerator_ColName, IDS_INDSELPMSYSVOLHEATING_CONSUMERS );
	m_pclSSheetConsumer->SetStaticText( CD_HeatConsumer_Consumer, RD_HeatGenerator_UnitName, _T( "" ) );
	m_pclSSheetConsumer->SetStaticText( CD_HeatConsumer_Capacity, RD_HeatGenerator_ColName, IDS_INDSELPMSYSVOLHEATING_CAPACITY );
	m_pclSSheetConsumer->SetStaticText( CD_HeatConsumer_Capacity, RD_HeatGenerator_UnitName, GetNameOf( pUnitDB->GetDefaultUnit( _U_TH_POWER ) ).c_str() );
	m_pclSSheetConsumer->SetStaticText( CD_HeatConsumer_Content, RD_HeatGenerator_ColName, IDS_INDSELPMSYSVOLHEATING_CONTENT );
	m_pclSSheetConsumer->SetStaticText( CD_HeatConsumer_Content, RD_HeatGenerator_UnitName, GetNameOf( pUnitDB->GetDefaultUnit( _U_VOLUME ) ).c_str() );
	
	// Manual content checkbox.
	m_pclSSheetConsumer->SetStaticText( CD_HeatConsumer_ManualContent, RD_HeatGenerator_ColName, _T( "" ) );
	m_pclSSheetConsumer->SetPictureCellWithID( IDI_PENCILDIAGSHORT, CD_HeatConsumer_ManualContent, RD_HeatGenerator_UnitName, CSSheet::PictureCellType::Icon );
	m_pclSSheetConsumer->SetBackColor( CD_HeatConsumer_ManualContent, RD_HeatGenerator_UnitName, _TAH_ORANGE );
	
	m_pclSSheetConsumer->SetStaticText( CD_HeatConsumer_SupplyTemp, RD_HeatGenerator_ColName, IDS_INDSELPMSYSVOLHEATING_SUPPLYTEMP );
	m_pclSSheetConsumer->SetStaticText( CD_HeatConsumer_SupplyTemp, RD_HeatGenerator_UnitName, GetNameOf( pUnitDB->GetDefaultUnit( _U_TEMPERATURE ) ).c_str() );
	m_pclSSheetConsumer->SetStaticText( CD_HeatConsumer_ReturnTemp, RD_HeatGenerator_ColName, IDS_INDSELPMSYSVOLHEATING_RETURNTEMP );
	m_pclSSheetConsumer->SetStaticText( CD_HeatConsumer_ReturnTemp, RD_HeatGenerator_UnitName, GetNameOf( pUnitDB->GetDefaultUnit( _U_TEMPERATURE ) ).c_str() );
	
	// Manual temp checkbox.
	m_pclSSheetConsumer->SetStaticText( CD_HeatConsumer_ManualTemp, RD_HeatGenerator_ColName, _T( "" ) );
	m_pclSSheetConsumer->SetPictureCellWithID( IDI_PENCILDIAGSHORT, CD_HeatConsumer_ManualTemp, RD_HeatGenerator_UnitName, CSSheet::PictureCellType::Icon );
	m_pclSSheetConsumer->SetBackColor( CD_HeatConsumer_ManualTemp, RD_HeatGenerator_UnitName, _TAH_ORANGE );

	// To fill a icon.
	m_pclSSheetConsumer->SetPictureCellWithID( IDI_PLUSGREEN, CD_HeatConsumer_FirstColumn, RD_HeatGenerator_FirstAvailRow, CSSheet::PictureCellType::Icon );

	m_pclSSheetConsumer->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );

}

void CDlgIndSelPMSysVolHeating::OnBnClickedCheckGenProtect()
{
	if( NULL == m_pclIndSelPMParams || NULL == m_pclIndSelPMParams->m_pclSelectPMList || NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()
			|| NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetHeatGeneratorList() )
	{
		ASSERT_RETURN;
	}

	m_bChangesDone = true;
	CButton *pCheckGenProtect = ( CButton * )GetDlgItem( IDC_CHECKGENPROTECT );
	UINT uiCheck = pCheckGenProtect->GetCheck();

	if( BST_CHECKED == uiCheck )
	{
		m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetHeatGeneratorList()->SetHeatGeneratorProtect( true );
	}
	else
	{
		m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetHeatGeneratorList()->SetHeatGeneratorProtect( false );
	}
}

void CDlgIndSelPMSysVolHeating::OnBnClickedAddPipes()
{
	if( NULL == m_pclIndSelPMParams || NULL == m_pclIndSelPMParams->m_pclSelectPMList || NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()
			|| NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetHeatingPipeList() )
	{
		ASSERT_RETURN;
	}

	CPipeList *pclHeatPipeList = m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetHeatingPipeList();

	CDlgIndSelAdditionalPiping dlg( m_pclIndSelPMParams, pclHeatPipeList );

	if( IDOK == dlg.DoModal() )
	{
	}

	double dPipeContent = pclHeatPipeList->GetTotalContent();
	m_clEditAdditionalPiping.SetWindowText( WriteCUDouble( _U_VOLUME, dPipeContent ) );
	_UpdateStaticValue();
}

void CDlgIndSelPMSysVolHeating::OnBnClickedResetAll()
{
	if( NULL == m_pclIndSelPMParams || NULL == m_pclIndSelPMParams->m_pclSelectPMList || NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()
			|| NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetHeatGeneratorList()
			|| NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetHeatConsumersList()
			|| NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetHeatingPipeList() )
	{
		ASSERT_RETURN;
	}

	CHeatGeneratorList *pclHeatGeneratorsList = m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetHeatGeneratorList();
	CHeatConsumersList *pclHeatConsumersList = m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetHeatConsumersList();
	CPipeList *pclHeatPipeList = m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetHeatingPipeList();

	if( 0 != pclHeatGeneratorsList->GetHeatGeneratorCount() || 0 != pclHeatConsumersList->GetConsumersCount() || 0 != pclHeatPipeList->GetPipeList() )
	{
		CString str = TASApp.LoadLocalizedString( AFXMSG_INDSELPMSYSVOLHEATING_RESETALL );

		if( IDYES == ::AfxMessageBox( str, MB_YESNO | MB_ICONQUESTION ) )
		{
			pclHeatGeneratorsList->Reset();
			pclHeatConsumersList->Reset();
			pclHeatPipeList->Reset();
			_UpdateStaticValue();

			// TODO: Clear all list and values.
			SS_CELLCOORD cellCoord;
			cellCoord.Col = 1;
			cellCoord.Row = RD_HeatGenerator_FirstAvailRow;

			while( m_iHeatGeneratorCount )
			{
				_RemoveHeatGenerator( &cellCoord );
			}

			m_clHeatGeneratorProtect.SetCheck( BST_UNCHECKED );

			while( m_iConsumerCount )
			{
				_RemoveHeatConsumer( &cellCoord );
			}

			m_clEditOtherBuffer.SetWindowTextW( WriteCUDouble( _U_VOLUME, 0.0 ) );

			// if some pipe are removed warn user on Cancel
			if( pclHeatPipeList->GetPipeCount() )
			{
				m_bChangesDone = true;
			}

			pclHeatPipeList->Reset();

			_ButtonPlusGeneratorEnabled();
			_ButtonPlusConsumerEnabled();
		}
	}
}

void CDlgIndSelPMSysVolHeating::OnBnClickedCancel()
{
	if( NULL == m_pclIndSelPMParams || NULL == m_pclIndSelPMParams->m_pclSelectPMList || NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()
			|| NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetHeatGeneratorList()
			|| NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetHeatConsumersList()
			|| NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetHeatingPipeList() )
	{
		ASSERT_RETURN;
	}

	if( true == m_bChangesDone )
	{
		CString str = TASApp.LoadLocalizedString( AFXMSG_INDSELPMSYSVOLHEATING_ABORT );

		if( IDNO == ::AfxMessageBox( str, MB_YESNO | MB_ICONQUESTION ) )
		{
			return;
		}

		// Restore the copy.
		m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetHeatGeneratorList()->CopyFrom( &m_clHeatGeneratorListCopy );
		m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetHeatConsumersList()->CopyFrom( &m_clConsumersListCopy );
		m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetHeatingPipeList()->CopyFrom( &m_clPipeListCopy );
	}

	CDialogExt::OnCancel();
}

void CDlgIndSelPMSysVolHeating::OnBnClickedOk()
{
	PREVENT_ENTER_KEY

	// PWU : Comment from JCC - No test needed (4/9/2014)
	//if (m_pclHeatGeneratorList->GetTotalHeatCapacity() < m_pclConsumersList->GetTotalHeatCapacity())
	//{
	//	CString str = TASApp.LoadLocalizedString(AFXMSG_INDSELPMSYSVOLHEATING_CONSBIGGERGEN);
	//	::AfxMessageBox(str, MB_OK | MB_ICONERROR);
	//	return;
	//}
	CDialog::OnOK();
}

void CDlgIndSelPMSysVolHeating::OnChangeBuffer()
{
	if( NULL == m_pclIndSelPMParams || NULL == m_pclIndSelPMParams->m_pclSelectPMList || NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()
			|| NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetHeatConsumersList() )
	{
		ASSERT_RETURN;
	}

	CHeatConsumersList *pclHeatConsumersList = m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetHeatConsumersList();

	double previousBuffer = pclHeatConsumersList->GetBufferContent();

	CString strBuffer;
	double dBuffer = 0.0;
	m_clEditOtherBuffer.GetWindowTextW( strBuffer );

	// if the buffer content is invalid put 0.0
	if( RD_NOT_NUMBER == ReadDoubleFromStr( strBuffer.GetString(), &dBuffer ) || dBuffer < 0.0 )
	{
		m_bBufferValid = false;
		pclHeatConsumersList->SetBufferContent( 0.0 );
	}
	else
	{
		m_bBufferValid = true;
		pclHeatConsumersList->SetBufferContent( CDimValue::CUtoSI( _U_VOLUME, dBuffer ) );
	}

	// if the buffer is changed warn the user on cancel
	if( previousBuffer != pclHeatConsumersList->GetBufferContent() )
	{
		m_bChangesDone = true;
	}

	_UpdateStaticValue();
}

LRESULT CDlgIndSelPMSysVolHeating::OnComboDropCloseUp( WPARAM wParam, LPARAM lParam )
{
	_UpdateHeatGenerator();
	_UpdateHeatConsumer();
	return 0;
}

LRESULT CDlgIndSelPMSysVolHeating::OnSSDblClick( WPARAM wParam, LPARAM lParam )
{
	SS_CELLCOORD *pclCellCoord = ( SS_CELLCOORD * )lParam;

	if( NULL == pclCellCoord )
	{
		ASSERTA_RETURN( 0 );
	}

	// Avoid weird behavior on checkbox.
	if( IDC_SSHEATGENLIST == wParam && CD_HeatGenerator_ManualContent == pclCellCoord->Col )
	{
		// Returns 1 to ignore event.
		return 1;
	}

	if( IDC_SSHEATCONSLIST == wParam && ( CD_HeatConsumer_ManualContent == pclCellCoord->Col 
			|| CD_HeatConsumer_ManualTemp == pclCellCoord->Col ) )
	{
		// Returns 1 to ignore event.
		return 1;
	}

	return 0;
}

LRESULT CDlgIndSelPMSysVolHeating::OnSSClick( WPARAM wParam, LPARAM lParam )
{
	SS_CELLCOORD *pclCellCoord = ( SS_CELLCOORD * )lParam;

	if( NULL == pclCellCoord )
	{
		ASSERTA_RETURN( 0 );
	}

	int iRecordIndex = pclCellCoord->Row - RD_HeatGenerator_FirstAvailRow;

	CSheetDescription *pclSDesc = m_ViewDescription.GetFromSSheetID( UINT( wParam ) );

	if( NULL == pclSDesc )
	{
		ASSERTA_RETURN( 0 );
	}

	CSSheet *pclSSheet = pclSDesc->GetSSheetPointer();

	if( NULL == pclSSheet )
	{
		ASSERTA_RETURN( 0 );
	}

	// Intercept below first row only.
	if( pclCellCoord->Row < RD_HeatGenerator_FirstAvailRow )
	{
		return 0;
	}

	LRESULT lEventProcessed = 0;

	if( IDC_SSHEATGENLIST == wParam )
	{
		if( CD_HeatGenerator_FirstColumn == pclCellCoord->Col )
		{
			if( true == _ButtonPlusGeneratorEnabled() && iRecordIndex == m_iHeatGeneratorCount ) // plus clicked
			{
				_AddHeatGenerator( pclSSheet );
				pclSSheet->ShowCell( CD_HeatGenerator_FirstColumn, pclCellCoord->Row + 1, SS_SHOW_BOTTOMLEFT );
			}
			else if( m_iHeatGeneratorCount > 0 && iRecordIndex < m_iHeatGeneratorCount ) // trash clicked
			{
				_RemoveHeatGenerator( pclCellCoord );
			}

			_UpdateHeatGenerator();
			_ButtonPlusGeneratorEnabled();

			lEventProcessed = 1;
		}
		else if( CD_HeatGenerator_ManualContent == pclCellCoord->Col && iRecordIndex < m_iHeatGeneratorCount ) // manual clicked
		{
			m_bChangesDone = true;

			if( true == _IsCheckBoxChecked( m_pclSSheetGenerator, pclCellCoord->Col, pclCellCoord->Row ) )
			{
				pclSSheet->SetCellText( CD_HeatGenerator_ManualContent, pclCellCoord->Row, _T("0") );
				pclSSheet->SetActiveCell( CD_HeatGenerator_Capacity, pclCellCoord->Row );
				pclSSheet->FormatStaticText( CD_HeatGenerator_Content, pclCellCoord->Row, CD_HeatGenerator_Content, pclCellCoord->Row, _T( "" ), SSS_ALIGN_RIGHT );
			}
			else
			{
				pclSSheet->SetCellText( CD_HeatGenerator_ManualContent, pclCellCoord->Row, _T("1") );
				pclSSheet->SetActiveCell( CD_HeatGenerator_Content, pclCellCoord->Row );
				pclSSheet->FormatEditDouble( CD_HeatGenerator_Content, pclCellCoord->Row, _T( "" ), SSS_ALIGN_RIGHT, true );
				SS_CELLTYPE rCellType;
				pclSSheet->SetTypeEdit( &rCellType, SSS_ALIGN_RIGHT, 15, SS_CHRSET_ALPHANUM, SS_CASE_NOCASE );
				pclSSheet->SetCellType( CD_HeatGenerator_Content, pclCellCoord->Row, &rCellType );
			}

			_UpdateHeatGenerator();
			_ButtonPlusGeneratorEnabled();

			lEventProcessed = 1;
		}
	}
	else if( IDC_SSHEATCONSLIST == wParam )
	{
		if( CD_HeatConsumer_FirstColumn == pclCellCoord->Col )
		{
			if( true == _ButtonPlusConsumerEnabled() && iRecordIndex == m_iConsumerCount ) // plus clicked
			{
				_AddHeatConsumer( pclSSheet );
				pclSSheet->ShowCell( CD_HeatConsumer_FirstColumn, pclCellCoord->Row + 1, SS_SHOW_BOTTOMLEFT );
			}
			else if( m_iConsumerCount > 0 && iRecordIndex < m_iConsumerCount ) // trash clicked
			{
				_RemoveHeatConsumer( pclCellCoord );
			}

			_UpdateHeatConsumer();
			_ButtonPlusConsumerEnabled();
			
			lEventProcessed = 1;
		}
		else if( CD_HeatConsumer_ManualContent == pclCellCoord->Col && iRecordIndex < m_iConsumerCount )
		{
			m_bChangesDone = true;

			if( true == _IsCheckBoxChecked( m_pclSSheetConsumer, pclCellCoord->Col, pclCellCoord->Row ) )
			{
				pclSSheet->SetCellText( CD_HeatConsumer_ManualContent, pclCellCoord->Row, _T("0") );
				pclSSheet->SetActiveCell( CD_HeatConsumer_Capacity, pclCellCoord->Row );
				pclSSheet->FormatStaticText( CD_HeatConsumer_Content, pclCellCoord->Row, CD_HeatConsumer_Content, pclCellCoord->Row, _T( "" ), SSS_ALIGN_RIGHT );
			}
			else
			{
				pclSSheet->SetCellText( CD_HeatConsumer_ManualContent, pclCellCoord->Row, _T("1") );
				pclSSheet->SetActiveCell( CD_HeatConsumer_Content, pclCellCoord->Row );
				pclSSheet->FormatEditDouble( CD_HeatConsumer_Content, pclCellCoord->Row, _T( "" ), SSS_ALIGN_RIGHT, true );
				SS_CELLTYPE rCellType;
				pclSSheet->SetTypeEdit( &rCellType, SSS_ALIGN_RIGHT, 15, SS_CHRSET_ALPHANUM, SS_CASE_NOCASE );
				pclSSheet->SetCellType( CD_HeatConsumer_Content, pclCellCoord->Row, &rCellType );
			}

			_UpdateHeatConsumer();
			_ButtonPlusConsumerEnabled();
			
			lEventProcessed = 1;
		}
		else if( CD_HeatConsumer_ManualTemp == pclCellCoord->Col && iRecordIndex < m_iConsumerCount )
		{
			m_bChangesDone = true;

			if( true == _IsCheckBoxChecked( m_pclSSheetConsumer, pclCellCoord->Col, pclCellCoord->Row ) )
			{
				pclSSheet->SetCellText( CD_HeatConsumer_ManualTemp, pclCellCoord->Row, _T("0") );
				pclSSheet->SetActiveCell( CD_HeatConsumer_Capacity, pclCellCoord->Row );
				pclSSheet->FormatStaticText( CD_HeatConsumer_SupplyTemp, pclCellCoord->Row, CD_HeatConsumer_SupplyTemp, pclCellCoord->Row, _T( "" ), SSS_ALIGN_RIGHT );
				pclSSheet->FormatStaticText( CD_HeatConsumer_ReturnTemp, pclCellCoord->Row, CD_HeatConsumer_ReturnTemp, pclCellCoord->Row, _T( "" ), SSS_ALIGN_RIGHT );
			}
			else
			{
				pclSSheet->SetCellText( CD_HeatConsumer_ManualTemp, pclCellCoord->Row, _T("1") );
				pclSSheet->SetActiveCell( CD_HeatConsumer_SupplyTemp, pclCellCoord->Row );
				pclSSheet->FormatEditDouble( CD_HeatConsumer_SupplyTemp, pclCellCoord->Row, _T( "" ), SSS_ALIGN_RIGHT, true );
				pclSSheet->FormatEditDouble( CD_HeatConsumer_ReturnTemp, pclCellCoord->Row, _T( "" ), SSS_ALIGN_RIGHT, true );
				SS_CELLTYPE rCellType;
				pclSSheet->SetTypeEdit( &rCellType, SSS_ALIGN_RIGHT, 15, SS_CHRSET_ALPHANUM, SS_CASE_NOCASE );
				pclSSheet->SetCellType( CD_HeatConsumer_SupplyTemp, pclCellCoord->Row, &rCellType );
				pclSSheet->SetCellType( CD_HeatConsumer_ReturnTemp, pclCellCoord->Row, &rCellType );
			}

			_UpdateHeatConsumer();
			_ButtonPlusConsumerEnabled();
			
			lEventProcessed = 1;
		}
	}

	if( 1 != lEventProcessed )
	{
		if( true == pclSSheet->IsCellProperty( pclCellCoord->Col, pclCellCoord->Row, CSSheet::CellDouble ) )
		{
			pclSSheet->SetEditMode( TRUE );
		}
	}

	return lEventProcessed;
}

LRESULT CDlgIndSelPMSysVolHeating::OnSSEditChange( WPARAM wParam, LPARAM lParam )
{
	CSheetDescription *pclSDesc = m_ViewDescription.GetFromSSheetID( UINT( wParam ) );

	if( NULL == pclSDesc )
	{
		ASSERTA_RETURN( 0 );
	}

	CSSheet *pclSSheet = pclSDesc->GetSSheetPointer();

	if( NULL == pclSSheet )
	{
		ASSERTA_RETURN( 0 );
	}

	SS_CELLCOORD *pclCellCoord = ( SS_CELLCOORD * )lParam;

	if( NULL == pclCellCoord )
	{
		ASSERTA_RETURN( 0 );
	}

	// Intercept below first row only.
	if( pclCellCoord->Row < RD_HeatGenerator_FirstAvailRow )
	{
		return 0;
	}

	CString strValue = pclSSheet->GetCellText( pclCellCoord->Col, pclCellCoord->Row );
	double dValue;
	
	// HYS-1307: Enable to use "-" sign for temperature
	bool bIsForTemperature = false;

	if( ( pclCellCoord->Col == CD_HeatConsumer_SupplyTemp ) || ( pclCellCoord->Col == CD_HeatConsumer_ReturnTemp ) )
	{
		bIsForTemperature = true;
	}

	switch( ReadDouble( strValue, &dValue ) )
	{
		case RD_NOT_NUMBER:
		{
			// Number is incomplete ?
			// Try to remove partial engineering notation if exist
			int i = strValue.FindOneOf( _T("eE") );

			if( i != -1 )
			{
				strValue.Delete( i );

				if( RD_OK == ReadDouble( strValue, &dValue ) && i != 0 )
				{
					break;
				}
			}

			i = strValue.FindOneOf( _T("abcdfghijklmnopqrstuvwxyzABCDFGHIJKLMNOPQRSTUVWXYZ") );

			if( i != -1 )
			{
				strValue.Delete( i );

				if( RD_OK == ReadDouble( strValue, &dValue ) )
				{
					break;
				}
			}

			i = strValue.FindOneOf( _T("-+") );

			if( i != -1 )
			{
				strValue.Delete( i );
				
				// HYS-1307: Enable to use "-" sign for temperature
				if( true == bIsForTemperature && true == strValue.IsEmpty() )
				{
					strValue = _T( "0.0" );
				}

				if( RD_OK == ReadDouble( strValue, &dValue ) )
				{
					break;
				}
			}

			pclSSheet->SetCellText( pclCellCoord->Col, pclCellCoord->Row, strValue );
		}
		break;

		case RD_EMPTY:
			break;

		case RD_OK:
			break;
	}

	if( IDC_SSHEATGENLIST == wParam )
	{
		_UpdateHeatGenerator();
		_ButtonPlusGeneratorEnabled();
	}
	else if( IDC_SSHEATCONSLIST == wParam )
	{
		_UpdateHeatConsumer();
		_ButtonPlusConsumerEnabled();
	}

	return 0;
}

LRESULT CDlgIndSelPMSysVolHeating::OnSSLeaveCell( WPARAM wParam, LPARAM lParam )
{
	return 0;
}

LRESULT CDlgIndSelPMSysVolHeating::OnTextTipFetch( WPARAM wParam, LPARAM lParam )
{
	SS_TEXTTIPFETCH *pTextTipFetch = (SS_TEXTTIPFETCH *)lParam;

	// Identify sheet description by the CSSheet window ID.
	CSheetDescription *pclSheetDescription = m_ViewDescription.GetFromSSheetID( (UINT)wParam );

	CString str;
	LRESULT ReturnValue = 0;

	WORD borderType = SS_BORDERTYPE_LEFT | SS_BORDERTYPE_TOP | SS_BORDERTYPE_RIGHT | SS_BORDERTYPE_BOTTOM;
	WORD borderStyle = SS_BORDERSTYLE_SOLID;
	COLORREF borderColor = _TAH_RED_LIGHT;
	WORD border;
	WORD style;
	COLORREF color;

	std::wstring strSupplyTemp = m_pclSSheetConsumer->GetCellText( CD_HeatConsumer_SupplyTemp, pTextTipFetch->Row );
	double dSupplyTemp = 0.0;
	std::wstring strReturnTemp = m_pclSSheetConsumer->GetCellText( CD_HeatConsumer_ReturnTemp, pTextTipFetch->Row );
	double dReturnTemp = 0.0;

	if( SD_HeatGenerator == pclSheetDescription->GetSheetDescriptionID() )
	{
		if( CD_HeatGenerator_Capacity == pTextTipFetch->Col )
		{
			if( ( TRUE == m_pclSSheetGenerator->GetBorder( pTextTipFetch->Col, pTextTipFetch->Row, &border, &style, &color ) )
					&& ( border == borderType && style == borderStyle && color == _TAH_RED_LIGHT ) )
			{
				str = TASApp.LoadLocalizedString( IDS_INDSELPMSYSVOL_CAPACITYCANTBEZERO_TT );
			}
		}
		else if( CD_HeatGenerator_Content == pTextTipFetch->Col )
		{
			if( ( TRUE == m_pclSSheetGenerator->GetBorder( pTextTipFetch->Col, pTextTipFetch->Row, &border, &style, &color ) )
					&& ( border == borderType && style == borderStyle && color == _TAH_RED_LIGHT ) )
			{
				str = TASApp.LoadLocalizedString( IDS_INDSELPMSYSVOL_CONTENTCANTBEZERO_TT );
			}
		}
	}
	else if( SD_HeatConsumer == pclSheetDescription->GetSheetDescriptionID() )
	{
		if( CD_HeatConsumer_Capacity == pTextTipFetch->Col )
		{
			if( ( TRUE == m_pclSSheetGenerator->GetBorder( pTextTipFetch->Col, pTextTipFetch->Row, &border, &style, &color ) )
					&& ( border == borderType && style == borderStyle && color == _TAH_RED_LIGHT ) )
			{
				str = TASApp.LoadLocalizedString( IDS_INDSELPMSYSVOL_CAPACITYCANTBEZERO_TT );
			}
		}
		else if( CD_HeatConsumer_Content == pTextTipFetch->Col )
		{
			if( ( TRUE == m_pclSSheetGenerator->GetBorder( pTextTipFetch->Col, pTextTipFetch->Row, &border, &style, &color ) )
					&& ( border == borderType && style == borderStyle && color == _TAH_RED_LIGHT ) )
			{
				str = TASApp.LoadLocalizedString( IDS_INDSELPMSYSVOL_CONTENTCANTBEZERO_TT );
			}
		}
		else if( CD_HeatConsumer_SupplyTemp == pTextTipFetch->Col )
		{
			if( ( TRUE == m_pclSSheetConsumer->GetBorder( pTextTipFetch->Col, pTextTipFetch->Row, &border, &style, &color ) )
					&& ( border == borderType && style == borderStyle && color == _TAH_RED_LIGHT ) )
			{
				// HYS-1612: pay attention, the 'm_dSupplyTemperature' variable is in SI and the 'dSupplyTemp' variable can be with other unit !
				if( RD_OK == ReadCUDoubleFromStr( _U_TEMPERATURE, strSupplyTemp, &dSupplyTemp ) )
				{
					if( dSupplyTemp > m_dSupplyTemperature )
					{
						str = TASApp.LoadLocalizedString( IDS_INDSELPMSYSVOL_TSTUTSSYSH_TT );
					}
					else if( RD_OK == ReadCUDoubleFromStr( _U_TEMPERATURE, strReturnTemp, &dReturnTemp ) )
					{
						if( dSupplyTemp <= dReturnTemp )
						{
							str = TASApp.LoadLocalizedString( IDS_INDSELPMSYSVOL_TSTRH_TT );
						}
						else if( ( dSupplyTemp + dReturnTemp ) / 2.0 <= 20.0 )
						{
							// This condition is only because the interpolation equation (See in the 'CHeatConsumersList::_ComputeWaterContent' method
							// in the 'SelectPM.cpp' file).
							FormatString( str, IDS_INDSELPMSYSVOL_TSTRSUM_TT, WriteCUDouble( _U_TEMPERATURE, 20.0, true ) );
						}
					}
				}
			}
		}
		else if( CD_HeatConsumer_ReturnTemp == pTextTipFetch->Col )
		{
			if( ( TRUE == m_pclSSheetConsumer->GetBorder( pTextTipFetch->Col, pTextTipFetch->Row, &border, &style, &color ) )
					&& ( border == borderType && style == borderStyle && color == _TAH_RED_LIGHT ) )
			{
				// HYS-1612: pay attention, the 'strReturnTemp' and 'strSupplyTemp' variables can be in a custom unit.
				// For the first condition it's not important, it's more for the second one.
				if( RD_OK == ReadCUDoubleFromStr( _U_TEMPERATURE, strReturnTemp, &dReturnTemp )
						&& RD_OK == ReadCUDoubleFromStr( _U_TEMPERATURE, strSupplyTemp, &dSupplyTemp ) )
				{
					if( dSupplyTemp <= dReturnTemp )
					{
						str = TASApp.LoadLocalizedString( IDS_INDSELPMSYSVOL_TSTRH_TT );
					}
					else if( ( dSupplyTemp + dReturnTemp ) / 2.0 <= 20.0 )
					{
						// This condition is only because the interpolation equation (See in the 'CHeatConsumersList::_ComputeWaterContent' method
						// in the 'SelectPM.cpp' file).
						FormatString( str, IDS_INDSELPMSYSVOL_TSTRSUM_TT, WriteCUDouble( _U_TEMPERATURE, 20.0, true ) );
					}
				}
			}
		}
	}

	if( false == str.IsEmpty() )
	{
		pTextTipFetch->hText = NULL;
		pTextTipFetch->wMultiLine = SS_TT_MULTILINE_AUTO;
		pTextTipFetch->nWidth = (SHORT)m_pclSSheetConsumer->GetTipTextWidth( str );
		wcsncpy_s( pTextTipFetch->szText, SS_TT_TEXTMAX, (LPCTSTR)str, SS_TT_TEXTMAX );
		pTextTipFetch->fShow = true;
		ReturnValue = 0;
	}

	return ReturnValue;
}

BOOL CDlgIndSelPMSysVolHeating::PreTranslateMessage( MSG *pMsg )
{
	if( true == m_bBufferValid && true == m_bGeneratorValid && true == m_bConsumerValid )
	{
		GetDlgItem( IDOK )->EnableWindow( TRUE );
	}
	else
	{
		GetDlgItem( IDOK )->EnableWindow( FALSE );
	}

	if( WM_LBUTTONDOWN == pMsg->message )
	{
		// PWU : The first click on the spread is ignored if the focus is elsewhere.
		// This code catch the first click and send it directly to the spread.
		CRect rectHeatGen;
		GetDlgItem( IDC_SSHEATGENLIST )->GetWindowRect( &rectHeatGen );

		CRect rectCurrentFocus;
		CWnd *pFocusedCtrl = GetFocus();

		if( NULL != pFocusedCtrl )
		{
			pFocusedCtrl->GetWindowRect( rectCurrentFocus );
		}

		CRect rectIntersectHeatGen;
		rectIntersectHeatGen.IntersectRect( rectCurrentFocus, rectHeatGen );

		if( TRUE == rectHeatGen.PtInRect( pMsg->pt ) && TRUE == rectIntersectHeatGen.IsRectNull() )
		{
			CSheetDescription *pclSDesc = m_ViewDescription.GetFromSSheetID( IDC_SSHEATGENLIST );
			CSSheet *plcSSheet = pclSDesc->GetSSheetPointer();
			plcSSheet->SendMessage( pMsg->message, pMsg->wParam, pMsg->lParam );
			GetDlgItem( IDC_SSHEATGENLIST )->SetFocus();
		}

		CRect rectHeatCons;
		GetDlgItem( IDC_SSHEATCONSLIST )->GetWindowRect( &rectHeatCons );

		CRect rectIntersectHeatCons;
		rectIntersectHeatCons.IntersectRect( rectCurrentFocus, rectHeatCons );

		if( TRUE == rectHeatCons.PtInRect( pMsg->pt ) && TRUE == rectIntersectHeatCons.IsRectNull() )
		{
			CSheetDescription *pclSDesc = m_ViewDescription.GetFromSSheetID( IDC_SSHEATCONSLIST );
			CSSheet *plcSSheet = pclSDesc->GetSSheetPointer();
			plcSSheet->SendMessage( pMsg->message, pMsg->wParam, pMsg->lParam );
			GetDlgItem( IDC_SSHEATCONSLIST )->SetFocus();
		}
	}

	return __super::PreTranslateMessage( pMsg ); // allow default behavior (return TRUE if you want to discard message)
}

void CDlgIndSelPMSysVolHeating::_UpdateStaticValue( void )
{
	if( NULL == m_pclIndSelPMParams || NULL == m_pclIndSelPMParams->m_pclSelectPMList || NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()
			|| NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetHeatGeneratorList()
			|| NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetHeatConsumersList()
			|| NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetHeatingPipeList() )
	{
		ASSERT_RETURN;
	}

	CHeatGeneratorList *pclHeatGeneratorsList = m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetHeatGeneratorList();
	CHeatConsumersList *pclHeatConsumersList = m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetHeatConsumersList();
	CPipeList *pclHeatPipeList = m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetHeatingPipeList();

	double dTotalGenHeatCapacity = pclHeatGeneratorsList->GetTotalHeatCapacity();
	m_clStaticTotalGenHeatCapacity.SetWindowText( WriteCUDouble( _U_TH_POWER, dTotalGenHeatCapacity, true ) );
	double dTotalGenContent = pclHeatGeneratorsList->GetTotalContent();
	m_clStaticTotalGenContent.SetWindowText( WriteCUDouble( _U_VOLUME, dTotalGenContent, true ) );

	double dTotalConsHeatCapacity = pclHeatConsumersList->GetTotalHeatCapacity();
	m_clStaticTotalConsHeatCapacity.SetWindowText( WriteCUDouble( _U_TH_POWER, dTotalConsHeatCapacity, true ) );
	double dTotalConsContent = pclHeatConsumersList->GetTotalContent( m_dSupplyTemperature, m_dReturnTemperature ) - pclHeatConsumersList->GetBufferContent();
	m_clStaticTotalConsContent.SetWindowText( WriteCUDouble( _U_VOLUME, dTotalConsContent, true ) );

	// HYS-869
	if( dTotalGenHeatCapacity < dTotalConsHeatCapacity )
	{
		m_clStaticTotalGenHeatCapacity.SetTextColor( _ORANGE );
		m_clStaticTotalConsHeatCapacity.SetTextColor( _ORANGE );
		m_clStaticTotalGenHeatCapacity.SetToolTip( TASApp.LoadLocalizedString( IDS_INDSELPMSYSVOL_TOTALCAPACITY_TT ) );
		m_clStaticTotalConsHeatCapacity.SetToolTip( TASApp.LoadLocalizedString( IDS_INDSELPMSYSVOL_TOTALCAPACITY_TT ) );
	}
	else
	{
		m_clStaticTotalGenHeatCapacity.ResetToolTip();
		m_clStaticTotalConsHeatCapacity.ResetToolTip();
		m_clStaticTotalGenHeatCapacity.SetTextColor( _BLACK );
		m_clStaticTotalConsHeatCapacity.SetTextColor( _BLACK );
	}

	double dTotalContent = pclHeatGeneratorsList->GetTotalContent() + pclHeatConsumersList->GetTotalContent( m_dSupplyTemperature,
			m_dReturnTemperature ) + pclHeatPipeList->GetTotalContent();

	m_clStaticTotalContent.SetWindowText( WriteCUDouble( _U_VOLUME, dTotalContent, true ) );

	double dPipeContent = pclHeatPipeList->GetTotalContent();
	m_clEditAdditionalPiping.SetWindowText( WriteCUDouble( _U_VOLUME, dPipeContent ) );
}

void CDlgIndSelPMSysVolHeating::_RemoveHeatGenerator( SS_CELLCOORD *plcCellCoord )
{
	if( NULL == m_pclIndSelPMParams || NULL == m_pclIndSelPMParams->m_pclSelectPMList || NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()
			|| NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetHeatGeneratorList() )
	{
		ASSERT_RETURN;
	}

	CHeatGeneratorList *pclHeatGeneratorsList = m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetHeatGeneratorList();

	m_iHeatGeneratorCount--;
	m_pclSSheetGenerator->DelRow( plcCellCoord->Row ); // drop a row
	m_pclSSheetGenerator->SetMaxRows( RD_HeatGenerator_FirstAvailRow + m_iHeatGeneratorCount ); // Set max rows.
	pclHeatGeneratorsList->RemoveHeatGenerator( plcCellCoord->Row - RD_HeatGenerator_FirstAvailRow );
	m_bChangesDone = true;

	_CheckHeatGeneratorColumnWidth();
}

void CDlgIndSelPMSysVolHeating::_RemoveHeatConsumer( SS_CELLCOORD *plcCellCoord )
{
	if( NULL == m_pclIndSelPMParams || NULL == m_pclIndSelPMParams->m_pclSelectPMList || NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()
			|| NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetHeatGeneratorList()
			|| NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetHeatConsumersList()
			|| NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetHeatingPipeList() )
	{
		ASSERT_RETURN;
	}

	CHeatConsumersList *pclHeatConsumersList = m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetHeatConsumersList();

	m_iConsumerCount--;
	m_pclSSheetConsumer->DelRow( plcCellCoord->Row ); // drop a row
	m_pclSSheetConsumer->SetMaxRows( RD_HeatGenerator_FirstAvailRow + m_iConsumerCount ); // Set max rows.
	pclHeatConsumersList->RemoveConsumers( plcCellCoord->Row - RD_HeatGenerator_FirstAvailRow );
	m_bChangesDone = true;
	
	_CheckHeatConsumerColumnWidth();
}

void CDlgIndSelPMSysVolHeating::_AddHeatGenerator( CSSheet *pclSSheet, CString strType, double dCapacity, double dContent, bool bManual, bool bOnlyGUI )
{
	if( NULL == m_pclIndSelPMParams || NULL == m_pclIndSelPMParams->m_pclSelectPMList || NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()
			|| NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetHeatGeneratorList() || NULL == pclSSheet )
	{
		ASSERT_RETURN;
	}

	CHeatGeneratorList *pclHeatGeneratorsList = m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetHeatGeneratorList();

	m_iHeatGeneratorCount++;
	int iCurrentRow = RD_HeatGenerator_FirstAvailRow + m_iHeatGeneratorCount - 1;

	// Set max rows.
	pclSSheet->SetMaxRows( RD_HeatGenerator_FirstAvailRow + m_iHeatGeneratorCount );

	// Add plus icon.
	pclSSheet->SetPictureCellWithID( IDI_PLUSGREEN, CD_HeatGenerator_FirstColumn, RD_HeatGenerator_FirstAvailRow + m_iHeatGeneratorCount,
			CSSheet::PictureCellType::Icon );

	// Add checkbox.
	pclSSheet->SetCheckBox( CD_HeatGenerator_ManualContent, iCurrentRow, _T( "" ), false, true );

	// Add trash icon.
	pclSSheet->SetPictureCellWithID( IDI_TRASH, CD_HeatGenerator_FirstColumn, iCurrentRow, CSSheet::PictureCellType::Icon );

	// Add generator combo.
	pclSSheet->FormatComboList( CD_HeatGenerator_Generator, iCurrentRow );
	_FillHeatGeneratorCombo( pclSSheet, CD_HeatGenerator_Generator, iCurrentRow, strType );

	// Add capacity edit.
	pclSSheet->FormatEditDouble( CD_HeatGenerator_Capacity, iCurrentRow, _T( "" ), SSS_ALIGN_RIGHT );
	
	CString strCapacity;
	strCapacity.Format( _T("%g"), dCapacity );
	pclSSheet->SetCellText( CD_HeatGenerator_Capacity, iCurrentRow, strCapacity );
	
	SS_CELLTYPE rCellType;
	pclSSheet->SetTypeEdit( &rCellType, SSS_ALIGN_RIGHT, 15, SS_CHRSET_ALPHANUM, SS_CASE_NOCASE );
	pclSSheet->SetCellType( CD_HeatGenerator_Capacity, iCurrentRow, &rCellType );

	CString strGeneratorID;
	_FindGeneratorID( pclSSheet, iCurrentRow, strGeneratorID );

	if( false == bOnlyGUI ) // Used to load data at dialog opening
	{
		pclHeatGeneratorsList->AddHeatGenerator( strGeneratorID, dCapacity, dContent );
		m_bChangesDone = true;
	}

	CString strContent;
	strContent.Format( _T("%g"), dContent );

	if( true == bManual ) // Manual input
	{
		pclSSheet->SetCellText( CD_HeatGenerator_ManualContent, iCurrentRow, _T("1") );
		pclSSheet->FormatEditDouble( CD_HeatGenerator_Content, iCurrentRow, strContent, SSS_ALIGN_RIGHT );
		pclSSheet->SetCellType( CD_HeatGenerator_Content, iCurrentRow, &rCellType );
	}
	else
	{
		pclSSheet->FormatStaticText( CD_HeatGenerator_Content, iCurrentRow, CD_HeatGenerator_Content, iCurrentRow, strContent, SSS_ALIGN_RIGHT );
	}

	_CheckHeatGeneratorColumnWidth();
}

void CDlgIndSelPMSysVolHeating::_AddHeatConsumer( CSSheet *plcSSheet, CString pstrType, double dCapacity, double dContent, bool bManualContent,
		double dSupplyTemp, double dReturnTemp, bool bManualTemp, bool bOnlyGUI )
{
	if( NULL == m_pclIndSelPMParams || NULL == m_pclIndSelPMParams->m_pclSelectPMList || NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()
			|| NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetHeatConsumersList() || NULL == plcSSheet )
	{
		ASSERT_RETURN;
	}

	CHeatConsumersList *pclHeatConsumersList = m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetHeatConsumersList();
	CHeatGeneratorList *pclHeatGeneratorsList = m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetHeatGeneratorList();

	m_iConsumerCount++;
	int iCurrentRow = RD_HeatGenerator_FirstAvailRow + m_iConsumerCount - 1;

	// Set max rows.
	plcSSheet->SetMaxRows( RD_HeatGenerator_FirstAvailRow + m_iConsumerCount );

	// Add plus icon.
	plcSSheet->SetPictureCellWithID( IDI_PLUSGREEN, CD_HeatConsumer_FirstColumn, iCurrentRow + 1, CSSheet::PictureCellType::Icon );

	// Add checkbox.
	plcSSheet->SetCheckBox( CD_HeatConsumer_ManualContent, iCurrentRow, _T( "" ), false, true );
	plcSSheet->SetCheckBox( CD_HeatConsumer_ManualTemp,    iCurrentRow, _T( "" ), false, true );

	// Add trash icon.
	plcSSheet->SetPictureCellWithID( IDI_TRASH, CD_HeatConsumer_FirstColumn, iCurrentRow, CSSheet::PictureCellType::Icon );

	// Add generator combo.
	plcSSheet->FormatComboList( CD_HeatConsumer_Consumer, iCurrentRow );
	_FillConsumersCombo( plcSSheet, CD_HeatConsumer_Consumer, iCurrentRow, pstrType );

	// Add capacity edit.
	plcSSheet->FormatEditDouble( CD_HeatConsumer_Capacity, iCurrentRow, _T( "" ), SSS_ALIGN_RIGHT );
	
	// HYS-869
	if( ( false == bOnlyGUI ) && ( pclHeatGeneratorsList->GetTotalHeatCapacity() > pclHeatConsumersList->GetTotalHeatCapacity() ) )
	{
		dCapacity = CDimValue::SItoCU( _U_TH_POWER, ( pclHeatGeneratorsList->GetTotalHeatCapacity() - pclHeatConsumersList->GetTotalHeatCapacity() ) );
	}

	CString strCapacity;
	strCapacity.Format( _T("%g"), dCapacity );
	plcSSheet->SetCellText( CD_HeatConsumer_Capacity, iCurrentRow, strCapacity );
	
	SS_CELLTYPE rCellType;
	plcSSheet->SetTypeEdit( &rCellType, SSS_ALIGN_RIGHT, 15, SS_CHRSET_ALPHANUM, SS_CASE_NOCASE );
	plcSSheet->SetCellType( CD_HeatConsumer_Capacity, iCurrentRow, &rCellType );

	CString strConsumerID;
	_FindConsumerID( plcSSheet, iCurrentRow, strConsumerID );

	if( false == bOnlyGUI ) // Used to load data at dialog opening
	{
		pclHeatConsumersList->AddConsumers( strConsumerID, dCapacity, dContent, m_dSupplyTemperature, m_dReturnTemperature, bManualTemp );
		m_bChangesDone = true;
	}

	CString strContent;
	strContent.Format( _T("%g"), dContent );
	CString strSupplyTemp;
	strSupplyTemp.Format( _T("%g"), ( bManualTemp ? dSupplyTemp : m_dSupplyTemperature ) );
	CString strReturnTemp;
	strReturnTemp.Format( _T("%g"), ( bManualTemp ? dReturnTemp : m_dReturnTemperature ) );

	if( true == bManualContent ) // Manual input
	{
		plcSSheet->SetCellText( CD_HeatConsumer_ManualContent, iCurrentRow, _T("1") );
		plcSSheet->FormatEditDouble( CD_HeatConsumer_Content, iCurrentRow, strContent, SSS_ALIGN_RIGHT );
		plcSSheet->SetCellType( CD_HeatConsumer_Content, iCurrentRow, &rCellType );
	}
	else
	{
		plcSSheet->FormatStaticText( CD_HeatConsumer_ManualContent, iCurrentRow, CD_HeatConsumer_Content, iCurrentRow, strContent, SSS_ALIGN_RIGHT );
	}

	if( true == bManualTemp ) // Manual input
	{
		plcSSheet->SetCellText( CD_HeatConsumer_ManualTemp, iCurrentRow, _T("1") );
		plcSSheet->FormatEditDouble( CD_HeatConsumer_SupplyTemp, iCurrentRow, strSupplyTemp, SSS_ALIGN_RIGHT );
		plcSSheet->FormatEditDouble( CD_HeatConsumer_ReturnTemp, iCurrentRow, strReturnTemp, SSS_ALIGN_RIGHT );
		plcSSheet->SetCellType( CD_HeatConsumer_SupplyTemp, iCurrentRow, &rCellType );
		plcSSheet->SetCellType( CD_HeatConsumer_ReturnTemp, iCurrentRow, &rCellType );
	}
	else
	{
		plcSSheet->FormatStaticText( CD_HeatConsumer_SupplyTemp, iCurrentRow, CD_HeatConsumer_SupplyTemp, iCurrentRow, strSupplyTemp, SSS_ALIGN_RIGHT );
		plcSSheet->FormatStaticText( CD_HeatConsumer_ReturnTemp, iCurrentRow, CD_HeatConsumer_ReturnTemp, iCurrentRow, strReturnTemp, SSS_ALIGN_RIGHT );
	}

	_CheckHeatConsumerColumnWidth();
}

void CDlgIndSelPMSysVolHeating::_FillHeatGeneratorCombo( CSSheet *pclSheet, long lCol, long lRow, const TCHAR *ptcSelect )
{
	if( NULL == m_pclIndSelPMParams || NULL == m_pclIndSelPMParams->m_pTADB || NULL == pclSheet || NULL == ptcSelect )
	{
		ASSERT_RETURN;
	}

	if( lCol < 1 || lCol > pclSheet->GetMaxCols() || lRow < 1 || lRow > pclSheet->GetMaxRows() )
	{
		return;
	}

	if( true == m_mapGeneratorType.empty() )
	{
		CTable *pclTable = dynamic_cast<CTable *>( m_pclIndSelPMParams->m_pTADB->Get( _T("PMHEATGENERATOR_TAB") ).MP );

		if( NULL == pclTable )
		{
			ASSERT_RETURN;
		}

		for( IDPTR IDPtr = pclTable->GetFirst(); NULL != IDPtr.MP; IDPtr = pclTable->GetNext( IDPtr.MP ) )
		{
			CDB_StringID *pStrID = dynamic_cast<CDB_StringID *>( IDPtr.MP );

			if( NULL == pStrID )
			{
				continue;
			}

			m_mapGeneratorType[_wtoi( pStrID->GetIDstr() )] = pStrID;
		}
	}

	if( true == m_mapGeneratorType.empty() )
	{
		return;
	}

	pclSheet->ComboBoxSendMessage( lCol, lRow, SS_CBM_RESETCONTENT, 0, 0 );
	int iCount = 0;
	int iSelectIndex = 0;

	for( std::map<int, CDB_StringID *>::iterator mapIter = m_mapGeneratorType.begin(); mapIter != m_mapGeneratorType.end(); ++mapIter )
	{
		pclSheet->ComboBoxSendMessage( lCol, lRow, SS_CBM_ADDSTRING, 0, ( LPARAM )( mapIter->second->GetString() ) );

		if( _tcscmp( ptcSelect, mapIter->second->GetIDPtr().ID ) == 0 )
		{
			iSelectIndex = iCount;
		}

		iCount++;
	}

	pclSheet->ComboBoxSendMessage( lCol, lRow, SS_CBM_SETCURSEL, iSelectIndex, 0 );
}

void CDlgIndSelPMSysVolHeating::_FillConsumersCombo( CSSheet *pclSheet, long lCol, long lRow, const TCHAR *ptcSelect )
{
	if( NULL == m_pclIndSelPMParams || NULL == m_pclIndSelPMParams->m_pTADB || NULL == pclSheet || NULL == ptcSelect )
	{
		ASSERT_RETURN;
	}

	if( lCol < 1 || lCol > pclSheet->GetMaxCols() || lRow < 1 || lRow > pclSheet->GetMaxRows() )
	{
		return;
	}

	if( true == m_mapConsumerType.empty() )
	{
		CTable *pclTable = dynamic_cast<CTable *>( m_pclIndSelPMParams->m_pTADB->Get( _T("PMCONSUMERS_TAB") ).MP );

		if( NULL == pclTable )
		{
			ASSERT_RETURN;
		}

		for( IDPTR IDPtr = pclTable->GetFirst(); NULL != IDPtr.MP; IDPtr = pclTable->GetNext( IDPtr.MP ) )
		{
			CDB_StringID *pStrID = dynamic_cast<CDB_StringID *>( IDPtr.MP );

			if( NULL == pStrID )
			{
				continue;
			}

			m_mapConsumerType[_wtoi( pStrID->GetIDstr() )] = pStrID;
		}
	}

	if( true == m_mapConsumerType.empty() )
	{
		return;
	}

	pclSheet->ComboBoxSendMessage( lCol, lRow, SS_CBM_RESETCONTENT, 0, 0 );
	int iCount = 0;
	int iSelectIndex = 0;

	for( std::map<int, CDB_StringID *>::iterator mapIter = m_mapConsumerType.begin(); mapIter != m_mapConsumerType.end(); ++mapIter )
	{
		pclSheet->ComboBoxSendMessage( lCol, lRow, SS_CBM_ADDSTRING, 0, ( LPARAM )( mapIter->second->GetString() ) );

		if( _tcscmp( ptcSelect, mapIter->second->GetIDPtr().ID ) == 0 )
		{
			iSelectIndex = iCount;
		}

		iCount++;
	}

	pclSheet->ComboBoxSendMessage( lCol, lRow, SS_CBM_SETCURSEL, iSelectIndex, 0 );
}

void CDlgIndSelPMSysVolHeating::_FindGeneratorID( CSSheet *pclSSheet, int iRow, CString &strID )
{
	if( NULL == pclSSheet )
	{
		ASSERT_RETURN;
	}

	strID = _T( "" );
	TCHAR tcComboValue[256];
	int iCurrentSelection = pclSSheet->ComboBoxSendMessage( CD_HeatGenerator_Generator, iRow, SS_CBM_GETCURSEL, 0, 0 );
	pclSSheet->ComboBoxSendMessage( CD_HeatGenerator_Generator, iRow, SS_CBM_GETLBTEXT, iCurrentSelection, ( LPARAM )tcComboValue );

	for( std::map<int, CDB_StringID *>::iterator mapIter = m_mapGeneratorType.begin(); mapIter != m_mapGeneratorType.end(); ++mapIter )
	{
		if( CString( mapIter->second->GetString() ).CompareNoCase( tcComboValue ) == 0 )
		{
			strID = mapIter->second->GetIDPtr().ID;
			return;
		}
	}
}

void CDlgIndSelPMSysVolHeating::_FindConsumerID( CSSheet *pclSSheet, int iRow, CString &strID )
{
	if( NULL == pclSSheet )
	{
		ASSERT_RETURN;
	}

	strID = _T( "" );
	TCHAR tcComboValue[256];
	int iCurrentSelection = pclSSheet->ComboBoxSendMessage( CD_HeatConsumer_Consumer, iRow, SS_CBM_GETCURSEL, 0, 0 );
	pclSSheet->ComboBoxSendMessage( CD_HeatConsumer_Consumer, iRow, SS_CBM_GETLBTEXT, iCurrentSelection, ( LPARAM )tcComboValue );

	for( std::map<int, CDB_StringID *>::iterator mapIter = m_mapConsumerType.begin(); mapIter != m_mapConsumerType.end(); ++mapIter )
	{
		if( CString( mapIter->second->GetString() ).CompareNoCase( tcComboValue ) == 0 )
		{
			strID = mapIter->second->GetIDPtr().ID;
			return;
		}
	}
}

bool CDlgIndSelPMSysVolHeating::_IsCheckBoxChecked( CSSheet *pclSSheet, SS_COORD col, SS_COORD row )
{
	if( NULL == pclSSheet )
	{
		ASSERTA_RETURN ( false );
	}

	TCHAR tcCheckBox[16];
	pclSSheet->GetValue( col, row, tcCheckBox );

	if( _T( '0' ) == tcCheckBox[0] )
	{
		return false;
	}
	else
	{
		return true;
	}
}

void CDlgIndSelPMSysVolHeating::_UpdateHeatGenerator()
{
	if( NULL == m_pclIndSelPMParams || NULL == m_pclIndSelPMParams->m_pclSelectPMList || NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()
			|| NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetHeatGeneratorList() )
	{
		ASSERT_RETURN;
	}

	CHeatGeneratorList *pclHeatGeneratorsList = m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetHeatGeneratorList();

	for( int i = 0; i < m_iHeatGeneratorCount; i++ )
	{
		int iRecordIndex = i;

		// Get generator ID.
		CString strGeneratorID;
		_FindGeneratorID( m_pclSSheetGenerator, i + RD_HeatGenerator_FirstAvailRow, strGeneratorID );

		// Get Capacity in SI.
		TCHAR tcCapacity[16];
		double dCapacity;
		m_pclSSheetGenerator->GetValue( CD_HeatGenerator_Capacity, i + RD_HeatGenerator_FirstAvailRow, tcCapacity );
		ReadCUDoubleFromStr( _U_TH_POWER, tcCapacity, &dCapacity );

		double dContent = -1.0;
		bool bManual = _IsCheckBoxChecked( m_pclSSheetGenerator, CD_HeatGenerator_ManualContent, i + RD_HeatGenerator_FirstAvailRow );

		if( true == bManual )
		{
			// If in manual input then get the Content in SI.
			TCHAR tcContent[16];
			m_pclSSheetGenerator->GetValue( CD_HeatGenerator_Content, i + RD_HeatGenerator_FirstAvailRow, tcContent );
			ReadCUDoubleFromStr( _U_VOLUME, tcContent, &dContent );
			m_pclSSheetGenerator->SetBackColor( CD_HeatGenerator_Content, i + RD_HeatGenerator_FirstAvailRow, _WHITE );
		}

		// Update database
		pclHeatGeneratorsList->UpdateHeatGenerator( iRecordIndex, strGeneratorID, dCapacity, dContent );

		if( false == bManual )
		{
			// If not in manual input then refresh Content in spreadsheet
			dContent = pclHeatGeneratorsList->GetHeatGenerator( iRecordIndex )->m_dContent;
			
			m_pclSSheetGenerator->FormatStaticText( CD_HeatGenerator_Content,
													i + RD_HeatGenerator_FirstAvailRow,
													CD_HeatGenerator_Content,
													i + RD_HeatGenerator_FirstAvailRow,
													WriteCUDouble( _U_VOLUME, dContent ),
													SSS_ALIGN_RIGHT );

			m_pclSSheetGenerator->SetBackColor( CD_HeatGenerator_Content, i + RD_HeatGenerator_FirstAvailRow, _LIGHTGRAY );
		}
	}

	_UpdateStaticValue();
}

void CDlgIndSelPMSysVolHeating::_UpdateHeatConsumer()
{
	if( NULL == m_pclIndSelPMParams || NULL == m_pclIndSelPMParams->m_pclSelectPMList || NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()
			|| NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetHeatConsumersList() )
	{
		ASSERT_RETURN;
	}

	CHeatConsumersList *pclHeatConsumersList = m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetHeatConsumersList();

	for( int i = 0; i < m_iConsumerCount; i++ )
	{
		int iRecordIndex = i;

		// Get generator ID.
		CString strConsumerID;
		_FindConsumerID( m_pclSSheetConsumer, i + RD_HeatGenerator_FirstAvailRow, strConsumerID );

		// Get Capacity in SI.
		TCHAR tcCapacity[16];
		double dCapacity;
		m_pclSSheetConsumer->GetValue( CD_HeatConsumer_Capacity, i + RD_HeatGenerator_FirstAvailRow, tcCapacity );
		ReadCUDoubleFromStr( _U_TH_POWER, tcCapacity, &dCapacity );

		double dContent = -1.0;
		double dSupplyTemp = m_dSupplyTemperature;
		double dReturnTemp = m_dReturnTemperature;
		bool bManualContent = _IsCheckBoxChecked( m_pclSSheetConsumer, CD_HeatConsumer_ManualContent, i + RD_HeatGenerator_FirstAvailRow );
		bool bManualTemp = _IsCheckBoxChecked( m_pclSSheetConsumer, CD_HeatConsumer_ManualTemp, i + RD_HeatGenerator_FirstAvailRow );

		if( true == bManualContent )
		{
			// If in manual input then get the Content in SI.
			TCHAR tcContent[16];
			m_pclSSheetConsumer->GetValue( CD_HeatConsumer_Content, i + RD_HeatGenerator_FirstAvailRow, tcContent );
			ReadCUDoubleFromStr( _U_VOLUME, tcContent, &dContent );
			m_pclSSheetConsumer->SetBackColor( CD_HeatConsumer_Content, i + RD_HeatGenerator_FirstAvailRow, _WHITE );
		}

		if( true == bManualTemp )
		{
			// If in manual input then get the Content in SI.
			TCHAR tcSupplyTemp[16];
			TCHAR tcReturnTemp[16];
			m_pclSSheetConsumer->GetValue( CD_HeatConsumer_SupplyTemp, i + RD_HeatGenerator_FirstAvailRow, tcSupplyTemp );
			m_pclSSheetConsumer->GetValue( CD_HeatConsumer_ReturnTemp, i + RD_HeatGenerator_FirstAvailRow, tcReturnTemp );
			ReadCUDoubleFromStr( _U_TEMPERATURE, tcSupplyTemp, &dSupplyTemp );
			ReadCUDoubleFromStr( _U_TEMPERATURE, tcReturnTemp, &dReturnTemp );
			m_pclSSheetConsumer->SetBackColor( CD_HeatConsumer_SupplyTemp, i + RD_HeatGenerator_FirstAvailRow, _WHITE );
			m_pclSSheetConsumer->SetBackColor( CD_HeatConsumer_ReturnTemp, i + RD_HeatGenerator_FirstAvailRow, _WHITE );
		}

		// Update database.
		pclHeatConsumersList->UpdateConsumers( iRecordIndex, strConsumerID, dCapacity, dContent, dSupplyTemp, dReturnTemp, bManualTemp );

		if( false == bManualContent )
		{
			// If not in manual input then refresh Content in spreadsheet
			dContent = pclHeatConsumersList->GetConsumers( iRecordIndex )->m_dContent;

			m_pclSSheetConsumer->FormatStaticText( CD_HeatConsumer_Content,
												   i + RD_HeatGenerator_FirstAvailRow,
												   CD_HeatConsumer_Content,
												   i + RD_HeatGenerator_FirstAvailRow,
												   WriteCUDouble( _U_VOLUME, dContent ),
												   SSS_ALIGN_RIGHT );

			m_pclSSheetConsumer->SetBackColor( CD_HeatGenerator_Content, i + RD_HeatGenerator_FirstAvailRow, _LIGHTGRAY );
		}

		if( false == bManualTemp )
		{
			// If not in manual input then refresh the supply temperature in spreadsheet.
			m_pclSSheetConsumer->FormatStaticText( CD_HeatConsumer_SupplyTemp,
												   i + RD_HeatGenerator_FirstAvailRow,
												   CD_HeatConsumer_SupplyTemp,
												   i + RD_HeatGenerator_FirstAvailRow,
												   WriteCUDouble( _U_TEMPERATURE, m_dSupplyTemperature ),
												   SSS_ALIGN_RIGHT );

			m_pclSSheetConsumer->SetBackColor( CD_HeatConsumer_SupplyTemp, i + RD_HeatGenerator_FirstAvailRow, _LIGHTGRAY );
			
			// If not in manual input then refresh the return temperature in spreadsheet.
			m_pclSSheetConsumer->FormatStaticText( CD_HeatConsumer_ReturnTemp,
												   i + RD_HeatGenerator_FirstAvailRow,
												   CD_HeatConsumer_ReturnTemp,
												   i + RD_HeatGenerator_FirstAvailRow,
												   WriteCUDouble( _U_TEMPERATURE, m_dReturnTemperature ),
												   SSS_ALIGN_RIGHT );
			
			m_pclSSheetConsumer->SetBackColor( CD_HeatConsumer_ReturnTemp, i + RD_HeatGenerator_FirstAvailRow, _LIGHTGRAY );
		}
	}

	_UpdateStaticValue();
}

void CDlgIndSelPMSysVolHeating::_LoadHeatGenerator()
{
	if( NULL == m_pclIndSelPMParams || NULL == m_pclIndSelPMParams->m_pclSelectPMList || NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()
			|| NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetHeatGeneratorList() )
	{
		ASSERT_RETURN;
	}

	CHeatGeneratorList *pclHeatGeneratorsList = m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetHeatGeneratorList();

	for( int i = 0; i < pclHeatGeneratorsList->GetHeatGeneratorCount(); i++ )
	{
		_AddHeatGenerator( 
				m_pclSSheetGenerator,
				pclHeatGeneratorsList->GetHeatGenerator( i )->m_strHeatGeneratorTypeID,
				CDimValue::SItoCU( _U_TH_POWER, pclHeatGeneratorsList->GetHeatGenerator( i )->m_dHeatCapacity ),
				CDimValue::SItoCU( _U_VOLUME, pclHeatGeneratorsList->GetHeatGenerator( i )->m_dContent ),
				pclHeatGeneratorsList->GetHeatGenerator( i )->m_bManual,
				true );
	}

	_UpdateHeatGenerator();
}

void CDlgIndSelPMSysVolHeating::_LoadHeatConsumer()
{
	if( NULL == m_pclIndSelPMParams || NULL == m_pclIndSelPMParams->m_pclSelectPMList || NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()
			|| NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetHeatConsumersList() )
	{
		ASSERT_RETURN;
	}

	CHeatConsumersList *pclHeatConsumersList = m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetHeatConsumersList();

	for( int i = 0; i < pclHeatConsumersList->GetConsumersCount(); i++ )
	{
		_AddHeatConsumer( 
				m_pclSSheetConsumer,
				pclHeatConsumersList->GetConsumers( i )->m_strConsumersTypeID,
				CDimValue::SItoCU( _U_TH_POWER, pclHeatConsumersList->GetConsumers( i )->m_dHeatCapacity ),
				CDimValue::SItoCU( _U_VOLUME, pclHeatConsumersList->GetConsumers( i )->m_dContent ),
				pclHeatConsumersList->GetConsumers( i )->m_bManualContent,
				CDimValue::SItoCU( _U_TEMPERATURE, pclHeatConsumersList->GetConsumers( i )->m_dSupplyTemp ),
				CDimValue::SItoCU( _U_TEMPERATURE, pclHeatConsumersList->GetConsumers( i )->m_dReturnTemp ),
				pclHeatConsumersList->GetConsumers( i )->m_bManualTemp,
				true );
	}

	_UpdateHeatConsumer();
}

bool CDlgIndSelPMSysVolHeating::_ButtonPlusGeneratorEnabled()
{
	// avoid to add a new generator if some input are invalid
	// not valid :
	// * content or capacity <= 0
	// * NAN
	// * Empty Cell
	bool bEnable = true;
	int iLastDataRow = m_iHeatGeneratorCount + RD_HeatGenerator_FirstAvailRow - 1;
	WORD border = SS_BORDERTYPE_LEFT | SS_BORDERTYPE_TOP | SS_BORDERTYPE_RIGHT | SS_BORDERTYPE_BOTTOM;

	for( int i = iLastDataRow; i >= RD_HeatGenerator_FirstAvailRow; i-- )
	{
		std::wstring strContent = m_pclSSheetGenerator->GetCellText( CD_HeatGenerator_Content, i );
		double dContent = 0.0;

		// Border need to be removed if (manual && good) or (auto).
		m_pclSSheetGenerator->SetCellBorder( CD_HeatGenerator_Content, i, false );

		if( RD_OK != ReadDoubleFromStr( strContent, &dContent ) || dContent <= 0.0 )
		{
			if( true == _IsCheckBoxChecked( m_pclSSheetGenerator, CD_HeatGenerator_ManualContent, i ) )
			{
				// Red border to help the user to find errors.
				m_pclSSheetGenerator->SetCellBorder( CD_HeatGenerator_Content, i, CD_HeatGenerator_Content, i, true, border, SS_BORDERSTYLE_SOLID, _TAH_RED_LIGHT );
			}

			bEnable = false;
		}

		std::wstring strCapacity = m_pclSSheetGenerator->GetCellText( CD_HeatGenerator_Capacity, i );
		double dCapacity = 0.0;

		if( RD_OK != ReadDoubleFromStr( strCapacity, &dCapacity ) || dCapacity <= 0.0 )
		{
			// Red border to help the user to find errors.
			m_pclSSheetGenerator->SetCellBorder( CD_HeatGenerator_Capacity, i, CD_HeatGenerator_Capacity, i, true, border, SS_BORDERSTYLE_SOLID, _TAH_RED_LIGHT );
			bEnable = false;
		}
		else
		{
			// Remove border if input is valid.
			m_pclSSheetGenerator->SetCellBorder( CD_HeatGenerator_Capacity, i, false );
		}
	}

	if( true == bEnable )
	{
		m_pclSSheetGenerator->SetBackColor( 
				CD_HeatGenerator_FirstColumn,
				RD_HeatGenerator_FirstAvailRow + m_iHeatGeneratorCount,
				CD_HeatGenerator_LastColumn,
				RD_HeatGenerator_FirstAvailRow + m_iHeatGeneratorCount,
				_WHITE );
	}
	else
	{
		m_pclSSheetGenerator->SetBackColor( 
				CD_HeatGenerator_FirstColumn,
				RD_HeatGenerator_FirstAvailRow + m_iHeatGeneratorCount,
				CD_HeatGenerator_LastColumn,
				RD_HeatGenerator_FirstAvailRow + m_iHeatGeneratorCount,
				_TAH_GREY_XLIGHT );
	}

	return m_bGeneratorValid = bEnable;
}

bool CDlgIndSelPMSysVolHeating::_ButtonPlusConsumerEnabled()
{
	// Avoid to add a new consumer if some input are invalid.
	// Cases where not valid :
	// * content or capacity <= 0
	// * NAN
	// * Empty Cell
	bool bEnable = true;
	int iLastDataRow = m_iConsumerCount + RD_HeatGenerator_FirstAvailRow - 1;
	WORD border = SS_BORDERTYPE_LEFT | SS_BORDERTYPE_TOP | SS_BORDERTYPE_RIGHT | SS_BORDERTYPE_BOTTOM;

	for( int i = iLastDataRow; i >= RD_HeatGenerator_FirstAvailRow; i-- )
	{
		std::wstring strContent = m_pclSSheetConsumer->GetCellText( CD_HeatConsumer_Content, i );
		double dContent = 0.0;

		// Border need to be removed if (manual && good) or (auto).
		m_pclSSheetConsumer->SetCellBorder( CD_HeatConsumer_Content, i, false );

		if( RD_OK != ReadDoubleFromStr( strContent, &dContent ) || dContent <= 0.0 )
		{
			if( _IsCheckBoxChecked( m_pclSSheetConsumer, CD_HeatConsumer_ManualContent, i ) )
			{
				// Red border to help the user to find errors.
				m_pclSSheetConsumer->SetCellBorder( CD_HeatConsumer_Content, i, CD_HeatConsumer_Content, i, true, border, SS_BORDERSTYLE_SOLID, _TAH_RED_LIGHT );
			}

			bEnable = false;
		}

		std::wstring strCapacity = m_pclSSheetConsumer->GetCellText( CD_HeatConsumer_Capacity, i );
		double dCapacity = 0.0;

		if( RD_OK != ReadDoubleFromStr( strCapacity, &dCapacity ) || dCapacity <= 0.0 )
		{
			// Red border to help the user to find errors.
			m_pclSSheetConsumer->SetCellBorder( CD_HeatConsumer_Capacity, i, CD_HeatConsumer_Capacity, i, true, border, SS_BORDERSTYLE_SOLID, _TAH_RED_LIGHT );
			bEnable = false;
		}
		else
		{
			// Remove border if input is valid.
			m_pclSSheetConsumer->SetCellBorder( CD_HeatConsumer_Capacity, i, false );
		}

		std::wstring strSupplyTemp = m_pclSSheetConsumer->GetCellText( CD_HeatConsumer_SupplyTemp, i );
		double dSupplyTemp = 0.0;

		// HYS-1612: pay attention, the 'm_dSupplyTemperature' variable is in SI and the 'dSupplyTemp' variable can be with other unit!
		if( RD_OK != ReadCUDoubleFromStr( _U_TEMPERATURE, strSupplyTemp, &dSupplyTemp ) || dSupplyTemp > m_dSupplyTemperature )
		{
			// Red border to help the user to find errors.
			m_pclSSheetConsumer->SetCellBorder( CD_HeatConsumer_SupplyTemp, i, CD_HeatConsumer_SupplyTemp, i, true, border, SS_BORDERSTYLE_SOLID, _TAH_RED_LIGHT );
			bEnable = false;
		}
		else
		{
			// Remove border if input is valid.
			m_pclSSheetConsumer->SetCellBorder( CD_HeatConsumer_SupplyTemp, i, false );
		}

		std::wstring strReturnTemp = m_pclSSheetConsumer->GetCellText( CD_HeatConsumer_ReturnTemp, i );
		double dReturnTemp = 0.0;

		// HYS-1612: pay attention, the 'dReturnTemp' variable can be in a custom unit!
		if( RD_OK != ReadCUDoubleFromStr( _U_TEMPERATURE, strReturnTemp, &dReturnTemp ) )
		{
			// Red border to help the user to find errors.
			m_pclSSheetConsumer->SetCellBorder( CD_HeatConsumer_ReturnTemp, i, CD_HeatConsumer_ReturnTemp, i, true, border, SS_BORDERSTYLE_SOLID, _TAH_RED_LIGHT );
			bEnable = false;
		}
		else
		{
			// Remove border if input is valid.
			m_pclSSheetConsumer->SetCellBorder( CD_HeatConsumer_ReturnTemp, i, false );
		}

		// HYS-940
		bool pbSupplyTempOK = true;
		bool pbReturnTempOK = true;

		m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetHeatConsumersList()->VerifyConsHeatingTemperatureValues( dSupplyTemp, dReturnTemp, &pbSupplyTempOK, &pbReturnTempOK );
		
		if( pbSupplyTempOK == false )
		{
			// Red border to help the user to find errors.
			m_pclSSheetConsumer->SetCellBorder( CD_HeatConsumer_SupplyTemp, i, CD_HeatConsumer_SupplyTemp, i, true, border, SS_BORDERSTYLE_SOLID, _TAH_RED_LIGHT );
			bEnable = false;
		}

		if( pbReturnTempOK == false )
		{
			// Red border to help the user to find errors.
			m_pclSSheetConsumer->SetCellBorder( CD_HeatConsumer_ReturnTemp, i, CD_HeatConsumer_ReturnTemp, i, true, border, SS_BORDERSTYLE_SOLID, _TAH_RED_LIGHT );
			bEnable = false;
		}
	}

	if( true == bEnable )
	{
		m_pclSSheetConsumer->SetBackColor(
				CD_HeatConsumer_FirstColumn,
				RD_HeatGenerator_FirstAvailRow + m_iConsumerCount,
				CD_HeatConsumer_LastColumn,
				RD_HeatGenerator_FirstAvailRow + m_iConsumerCount,
				_WHITE );
	}
	else
	{
		m_pclSSheetConsumer->SetBackColor( 
				CD_HeatConsumer_FirstColumn,
				RD_HeatGenerator_FirstAvailRow + m_iConsumerCount,
				CD_HeatConsumer_LastColumn,
				RD_HeatGenerator_FirstAvailRow + m_iConsumerCount,
				_TAH_GREY_XLIGHT );
	}

	return m_bConsumerValid = bEnable;
}

void CDlgIndSelPMSysVolHeating::_CheckHeatGeneratorColumnWidth()
{
	if( NULL == m_pclSSheetGenerator )
	{
		return;
	}
	
	CRect rectClient;
	m_pclSSheetGenerator->GetClientRect( &rectClient );

	if( rectClient.Width() == m_HeatGeneratorRectClient.Width() )
	{
		return;
	}

	m_HeatGeneratorRectClient = rectClient;

	double dAvailableWidth = m_pclSSheetGenerator->LogUnitsToColWidthW( m_HeatGeneratorRectClient.Width() );

	// '-1' for the border size (Set by default to 1).
	double dStandardWidthInPixel = m_pclSSheetGenerator->ColWidthToLogUnits( 1 ) - 1;
	double dTotalColWidth = HGCW_FirstColumn + HGCW_Generator + HGCW_Capacity + HGCW_Content + HGCW_Manual;
	double dRatio = dAvailableWidth / dTotalColWidth * dStandardWidthInPixel;

	// Change column size.
	long lWidthInPixel = (long)( dRatio * HGCW_FirstColumn );
	long lTotalWidthInPixel = lWidthInPixel;
	m_pclSSheetGenerator->SetColWidthInPixels( CD_HeatGenerator_FirstColumn, lWidthInPixel );

	lWidthInPixel = (long)( dRatio * HGCW_Generator );
	lTotalWidthInPixel += lWidthInPixel;
	m_pclSSheetGenerator->SetColWidthInPixels( CD_HeatGenerator_Generator, lWidthInPixel );

	lWidthInPixel = (long)( dRatio * HGCW_Capacity );
	lTotalWidthInPixel += lWidthInPixel;
	m_pclSSheetGenerator->SetColWidthInPixels( CD_HeatGenerator_Capacity, lWidthInPixel );

	lWidthInPixel = (long)( dRatio * HGCW_Content );
	lTotalWidthInPixel += lWidthInPixel;
	m_pclSSheetGenerator->SetColWidthInPixels( CD_HeatGenerator_Content, lWidthInPixel );
	
	m_pclSSheetGenerator->SetColWidthInPixels( CD_HeatGenerator_ManualContent, m_HeatGeneratorRectClient.Width() - lTotalWidthInPixel );
}

void CDlgIndSelPMSysVolHeating::_CheckHeatConsumerColumnWidth()
{
	if( NULL == m_pclSSheetConsumer )
	{
		return;
	}
	
	CRect rectClient;
	m_pclSSheetConsumer->GetClientRect( &rectClient );

	if( rectClient.Width() == m_HeatConsumerRectClient.Width() )
	{
		return;
	}

	m_HeatConsumerRectClient = rectClient;

	double dAvailableWidth = m_pclSSheetConsumer->LogUnitsToColWidthW( m_HeatConsumerRectClient.Width() );
	double dStandardWidthInPixel = m_pclSSheetConsumer->ColWidthToLogUnits( 1 ) - 1;
	double dTotalColWidth = HCCW_FirstColumn + HCCW_Consumer + HCCW_Capacity + HCCW_Content + HCCW_ManualContent + HCCW_SupplyTemp;
	dTotalColWidth += HCCW_ReturnTemp + HCCW_ManualTemp;

	double dRatio = dAvailableWidth / dTotalColWidth * dStandardWidthInPixel;

	long lWidthInPixel = (long)( dRatio * HCCW_FirstColumn );
	long lTotalWidthInPixel = lWidthInPixel;
	m_pclSSheetConsumer->SetColWidthInPixels( CD_HeatConsumer_FirstColumn, lWidthInPixel );

	lWidthInPixel = (long)( dRatio * HCCW_Consumer );
	lTotalWidthInPixel += lWidthInPixel;
	m_pclSSheetConsumer->SetColWidthInPixels( CD_HeatConsumer_Consumer, lWidthInPixel );

	lWidthInPixel = (long)( dRatio * HCCW_Capacity );
	lTotalWidthInPixel += lWidthInPixel;
	m_pclSSheetConsumer->SetColWidthInPixels( CD_HeatConsumer_Capacity, lWidthInPixel );

	lWidthInPixel = (long)( dRatio * HCCW_Content );
	lTotalWidthInPixel += lWidthInPixel;
	m_pclSSheetConsumer->SetColWidthInPixels( CD_HeatConsumer_Content, lWidthInPixel );

	lWidthInPixel = (long)( dRatio * HCCW_ManualContent );
	lTotalWidthInPixel += lWidthInPixel;
	m_pclSSheetConsumer->SetColWidthInPixels( CD_HeatConsumer_ManualContent, lWidthInPixel );

	lWidthInPixel = (long)( dRatio * HCCW_SupplyTemp );
	lTotalWidthInPixel += lWidthInPixel;
	m_pclSSheetConsumer->SetColWidthInPixels( CD_HeatConsumer_SupplyTemp, lWidthInPixel );

	lWidthInPixel = (long)( dRatio * HCCW_ReturnTemp );
	lTotalWidthInPixel += lWidthInPixel;
	m_pclSSheetConsumer->SetColWidthInPixels( CD_HeatConsumer_ReturnTemp, lWidthInPixel );

	m_pclSSheetConsumer->SetColWidthInPixels( CD_HeatConsumer_ManualTemp, m_HeatConsumerRectClient.Width() - lTotalWidthInPixel );
}
