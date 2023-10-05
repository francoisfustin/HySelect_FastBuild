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
#include "DlgIndSelPMSysVolCooling.h"
#include "Ssdllmfc.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CDlgIndSelPMSysVolCooling::CDlgIndSelPMSysVolCooling( CIndSelPMParams *pclIndSelPMParams, double dSupplyTemp, double dReturnTemp, CWnd *pParent )
	: CDialogExt( IDD , pParent )
{
	m_pclIndSelPMParams = pclIndSelPMParams;
	m_bChangesDone = false;
	m_pclSDescGenerator = 0;
	m_pclSDescConsumer = 0;
	m_pclSSheetGenerator = 0;
	m_pclSSheetConsumer = 0;
	m_iColdGeneratorCount = 0;
	m_iConsumerCount = 0;
	m_dSupplyTemperature = dSupplyTemp;
	m_dReturnTemperature = dReturnTemp;
	m_bBufferValid = true;
	m_bGeneratorValid = true;
	m_bConsumerValid = true;
	m_ColdGeneratorRectClient = CRect( 0, 0, 0, 0 );
	m_ColdGeneratorRectClient = CRect( 0, 0, 0, 0 );
}

CDlgIndSelPMSysVolCooling::~CDlgIndSelPMSysVolCooling()
{
	while( m_ViewDescription.GetSheetNumber() > 0 )
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

BEGIN_MESSAGE_MAP( CDlgIndSelPMSysVolCooling, CDialogExt )
	ON_BN_CLICKED( IDC_ADDPIPES, &CDlgIndSelPMSysVolCooling::OnBnClickedAddPipes )
	ON_BN_CLICKED( IDC_RESETALL, &CDlgIndSelPMSysVolCooling::OnBnClickedResetAll )
	ON_BN_CLICKED( IDCANCEL, &CDlgIndSelPMSysVolCooling::OnBnClickedCancel )
	ON_BN_CLICKED( IDOK, &CDlgIndSelPMSysVolCooling::OnBnClickedOk )
	ON_EN_CHANGE( IDC_EDITMAXAIRINPUTTEMPTOAHU, &CDlgIndSelPMSysVolCooling::OnEnChangeMaxAirInputTempToAHU )
	ON_EN_CHANGE( IDC_EDITBUFFER, &CDlgIndSelPMSysVolCooling::OnChangeBuffer )
	ON_MESSAGE( SSM_COMBOCLOSEUP, &CDlgIndSelPMSysVolCooling::OnComboDropCloseUp )
	ON_MESSAGE( SSM_CLICK, &CDlgIndSelPMSysVolCooling::OnSSClick )
	ON_MESSAGE( SSM_DBLCLK, &CDlgIndSelPMSysVolCooling::OnSSDblClick )
	ON_MESSAGE( SSM_EDITCHANGE, &CDlgIndSelPMSysVolCooling::OnSSEditChange )
	ON_MESSAGE( SSM_TEXTTIPFETCH, &CDlgIndSelPMSysVolCooling::OnTextTipFetch )
END_MESSAGE_MAP()

void CDlgIndSelPMSysVolCooling::DoDataExchange( CDataExchange *pDX )
{
	CDialogExt::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_STATICMAXAIRINPUTTEMPTOAHU, m_clStaticMaxAirInputTempToAHU );
	DDX_Control( pDX, IDC_EDITMAXAIRINPUTTEMPTOAHU, m_clEditMaxAirInputTempForAHU );
	DDX_Control( pDX, IDC_EDITBUFFER, m_clEditOtherBuffer );
	DDX_Control( pDX, IDC_EDITPIPES, m_clEditAdditionalPiping );
	DDX_Control( pDX, IDC_STATICTOTALGENCOLDCAPACITY, m_clStaticTotalGenColdCapacity );
	DDX_Control( pDX, IDC_STATICTOTALGENCONTENT, m_clStaticTotalGenContent );
	DDX_Control( pDX, IDC_STATICTARGETLINEARDP, m_clStaticTargetLinearDp );
	DDX_Control( pDX, IDC_STATICTOTALCONSCOLDCAPACITY, m_clStaticTotalConsColdCapacity );
	DDX_Control( pDX, IDC_STATICTOTALCONSCONTENT, m_clStaticTotalConsContent );
	DDX_Control( pDX, IDC_STATICTOTALCONTENT, m_clStaticTotalContent );
	DDX_Control( pDX, IDC_STATICTOTALINSTALL, m_clStaticTotalInstallation );
}

BOOL CDlgIndSelPMSysVolCooling::OnInitDialog()
{
	if( NULL == m_pclIndSelPMParams || NULL == m_pclIndSelPMParams->m_pclSelectPMList || NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()
			|| NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetColdGeneratorList()
			|| NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetColdConsumersList()
			|| NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetCoolingPipeList() )
	{
		ASSERTA_RETURN( FALSE );
	}

	CDialogExt::OnInitDialog();

	CString str;
	str = TASApp.LoadLocalizedString( IDS_INDSELPMSYSVOLCOOLING_CAPTION );
	SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_INDSELPMSYSVOLCOOLING_GENERATORS );
	GetDlgItem( IDC_STATICHEATGENERATOR )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_INDSELPMSYSVOLCOOLING_MAXAIRINPUTTEMPTOAHU );
	GetDlgItem( IDC_STATICMAXAIRINPUTTEMPTOAHU )->SetWindowText( str );

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
	
	str = TASApp.LoadLocalizedString( IDS_INDSELPMSYSVOLCOOLING_INCLUDINGPIPENETWORK );
	GetDlgItem( IDC_STATICINCLUDINGPIPENETWORK )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_INDSELPMSYSVOLHEATING_RESET );
	GetDlgItem( IDC_RESETALL )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_CANCEL );
	GetDlgItem( IDCANCEL )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_OK );
	GetDlgItem( IDOK )->SetWindowText( str );

	// Add units.
	TCHAR tcName[_MAXCHARS];
	CUnitDatabase *pclUnitDB = CDimValue::AccessUDB();
	
	GetNameOf( pclUnitDB->GetDefaultUnit( _U_TEMPERATURE ), tcName );
	SetDlgItemText( IDC_STATICUNITMAXAIRINPUTTEMPTOAHU, tcName );

	GetNameOf( pclUnitDB->GetDefaultUnit( _U_VOLUME ), tcName );
	SetDlgItemText( IDC_STATICUNITBUFFER, tcName );
	SetDlgItemText( IDC_STATICUNITADDPIPES, tcName );

	// Set some static text in bold.
	m_clStaticTotalGenColdCapacity.SetFontBold( true );
	m_clStaticTotalGenContent.SetFontBold( true );
	m_clStaticTotalConsColdCapacity.SetFontBold( true );
	m_clStaticTotalConsContent.SetFontBold( true );
	m_clStaticTotalContent.SetFontBold( true );
	m_clStaticTotalInstallation.SetFontBold( true );

	CButton *pButtonAddPipe = (CButton *)GetDlgItem( IDC_ADDPIPES );

	if( NULL != pButtonAddPipe && NULL != pButtonAddPipe->GetSafeHwnd() )
	{
		pButtonAddPipe->SetIcon( (HICON)LoadImage( AfxGetApp()->m_hInstance, MAKEINTRESOURCE( IDI_PIPECALCULATOR ), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR ) );
	}

	CColdGeneratorList *pclColdGeneratorsList = m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetColdGeneratorList();
	CColdConsumersList *pclColdConsumersList = m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetColdConsumersList();
	CPipeList *pclColdPipeList = m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetCoolingPipeList();

	pclColdGeneratorsList->SetTempOutdoor( m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetMaxTemperature() );
	
	// HYS-958: If the max. air input temp. for air handlers is not yet defined, we take the max. temperature by default.
	if( -1.0 == pclColdConsumersList->GetMaxAirInputTempForAHU() )
	{
		pclColdConsumersList->SetMaxAirInputTempForAHU( m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetMaxTemperature() );
	}

	m_clEditMaxAirInputTempForAHU.SetCurrentValSI( pclColdConsumersList->GetMaxAirInputTempForAHU() );
	m_clEditMaxAirInputTempForAHU.Update();

	m_clEditOtherBuffer.SetWindowTextW( WriteCUDouble( _U_VOLUME, pclColdConsumersList->GetBufferContent() ) );
	
	// Target linear pressure drop.
	double dSPam = m_pclIndSelPMParams->m_pTADS->GetpTechParams()->GetPipeTargDp();
	
	FormatString( str, IDS_INDSELPMSYSVOLCOOLING_TARGETLINEARDP, WriteCUDouble( _U_LINPRESSDROP, dSPam, true ) );
	m_clStaticTargetLinearDp.SetWindowText( str );
	m_clStaticTargetLinearDp.SetToolTip( TASApp.LoadLocalizedString( IDS_INDSELPMSYSVOLCOOLING_TARGETLINEARDPTT ) );

	// Keep a copy in case of user abort.
	m_clColdGeneratorListCopy.CopyFrom( pclColdGeneratorsList );
	m_clConsumersListCopy.CopyFrom( pclColdConsumersList );
	m_clPipeListCopy.CopyFrom( pclColdPipeList );

	_InitializeSSheet();
	_LoadColdGenerator();
	_ButtonPlusGeneratorEnabled();
	_LoadColdConsumer();
	_ButtonPlusConsumerEnabled();
	_UpdateStaticValue();

	return TRUE;
}

void CDlgIndSelPMSysVolCooling::_InitializeSSheet( void )
{
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();

	// Pay attention: because in this case I use TSpread directly as a custom control (see IDD_DLGPIPEDP in the resource), if you want to
	// use 'CSSheet' possibility, we have to attach TSpread CWnd to CSSheet newly created. And the destruction of the dialog, don't forget
	// to call 'Detach' before deleting 'CSSheet'.
	m_pclSSheetGenerator = new CSSheet();
	m_pclSSheetGenerator->Attach( GetDlgItem( IDC_SSCOLDGENLIST )->GetSafeHwnd() );
	m_pclSSheetGenerator->Reset();

	m_pclSDescGenerator = m_ViewDescription.AddSheetDescription( SD_ColdGenerator, -1, m_pclSSheetGenerator, CPoint( 0, 0 ) );

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
	m_pclSSheetGenerator->SetMaxRows( RD_ColdConsumer_FirstAvailRow );
	m_pclSSheetGenerator->SetFreeze( 0, RD_ColdConsumer_FirstAvailRow - 1 );

	// All cells are static by default and filled with _T("").
	m_pclSSheetGenerator->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	m_pclSSheetGenerator->FormatStaticText( -1, -1, -1, -1, _T( "" ) );

	// Add columns.
	m_pclSDescGenerator->AddColumn( CD_ColdGenerator_FirstColumn, 1 );
	m_pclSDescGenerator->AddColumnInPixels( CD_ColdGenerator_Generator, 1 );
	m_pclSDescGenerator->AddColumnInPixels( CD_ColdGenerator_Capacity, 1 );
	m_pclSDescGenerator->AddColumnInPixels( CD_ColdGenerator_Content, 1 );
	m_pclSDescGenerator->AddColumnInPixels( CD_ColdGenerator_ManualContent, 1 );
	m_pclSDescGenerator->AddColumnInPixels( CD_ColdGenerator_SupplyTemp, 1 );
	m_pclSDescGenerator->AddColumnInPixels( CD_ColdGenerator_ManualTemp, 1 );
	m_pclSDescGenerator->AddColumnInPixels( CD_ColdGenerator_Quantity, 1 );

	// Resize columns.
	_CheckColdGeneratorColumnWidth();

	// Row name.
	m_pclSSheetGenerator->SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );
	m_pclSSheetGenerator->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontSize, ( LPARAM )8 );
	m_pclSSheetGenerator->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, ( LPARAM )_TAH_TITLE_MAIN );
	m_pclSSheetGenerator->SetStaticText( CD_ColdGenerator_FirstColumn, RD_ColdConsumer_ColName, _T( "" ) );
	m_pclSSheetGenerator->SetStaticText( CD_ColdGenerator_FirstColumn, RD_ColdConsumer_UnitName, _T( "" ) );
	m_pclSSheetGenerator->SetStaticText( CD_ColdGenerator_Generator, RD_ColdConsumer_ColName, IDS_INDSELPMSYSVOLCOOLING_GENERATORS );
	m_pclSSheetGenerator->SetStaticText( CD_ColdGenerator_Generator, RD_ColdConsumer_UnitName, _T( "" ) );
	m_pclSSheetGenerator->SetStaticText( CD_ColdGenerator_Capacity, RD_ColdConsumer_ColName, IDS_INDSELPMSYSVOLCOOLING_CAPACITY );
	m_pclSSheetGenerator->SetStaticText( CD_ColdGenerator_Capacity, RD_ColdConsumer_UnitName, GetNameOf( pUnitDB->GetDefaultUnit( _U_TH_POWER ) ).c_str() );
	m_pclSSheetGenerator->SetStaticText( CD_ColdGenerator_SupplyTemp, RD_ColdConsumer_ColName, IDS_INDSELPMSYSVOLCOOLING_SUPPLYTEMP );
	m_pclSSheetGenerator->SetStaticText( CD_ColdGenerator_SupplyTemp, RD_ColdConsumer_UnitName, GetNameOf( pUnitDB->GetDefaultUnit( _U_TEMPERATURE ) ).c_str() );
	
	// Manual temp checkbox.
	m_pclSSheetGenerator->SetStaticText( CD_ColdGenerator_ManualTemp, RD_ColdConsumer_ColName, _T( "" ) );
	m_pclSSheetGenerator->SetPictureCellWithID( IDI_PENCILDIAGSHORT, CD_ColdGenerator_ManualTemp, RD_ColdConsumer_UnitName, CSSheet::PictureCellType::Icon );
	m_pclSSheetGenerator->SetBackColor( CD_ColdGenerator_ManualTemp, RD_ColdConsumer_UnitName, _TAH_ORANGE );
	
	// Content.
	m_pclSSheetGenerator->SetStaticText( CD_ColdGenerator_Content, RD_ColdConsumer_ColName, IDS_INDSELPMSYSVOLHEATING_CONTENT );
	m_pclSSheetGenerator->SetStaticText( CD_ColdGenerator_Content, RD_ColdConsumer_UnitName, GetNameOf( pUnitDB->GetDefaultUnit( _U_VOLUME ) ).c_str() );
	
	// Manual content checkbox.
	m_pclSSheetGenerator->SetStaticText( CD_ColdGenerator_ManualContent, RD_ColdConsumer_ColName, _T( "" ) );
	m_pclSSheetGenerator->SetPictureCellWithID( IDI_PENCILDIAGSHORT, CD_ColdGenerator_ManualContent, RD_ColdConsumer_UnitName, CSSheet::PictureCellType::Icon );
	m_pclSSheetGenerator->SetBackColor( CD_ColdGenerator_ManualContent, RD_ColdConsumer_UnitName, _TAH_ORANGE );
	
	// Quantity.
	m_pclSSheetGenerator->SetStaticText( CD_ColdGenerator_Quantity, RD_ColdConsumer_ColName, IDS_SELP_QUANTITY );
	m_pclSSheetGenerator->SetStaticText( CD_ColdGenerator_Quantity, RD_ColdConsumer_UnitName, _T( "" ) );

	// To fill a icon.
	m_pclSSheetGenerator->SetPictureCellWithID( IDI_PLUSGREEN, CD_ColdGenerator_FirstColumn, RD_ColdConsumer_FirstAvailRow, CSSheet::PictureCellType::Icon );

	m_pclSSheetGenerator->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );

	// Pay attention: because in this case I use TSpread directly as a custom control (see IDD_DLGPIPEDP in the resource), if you want to
	// use 'CSSheet' possibility, we have to attach TSpread CWnd to CSSheet newly created. And the destruction of the dialog, don't forget
	// to call 'Detach' before deleting 'CSSheet'.
	m_pclSSheetConsumer = new CSSheet();
	m_pclSSheetConsumer->Attach( GetDlgItem( IDC_SSCOLDCONSLIST )->GetSafeHwnd() );
	m_pclSSheetConsumer->Reset();

	m_pclSDescConsumer = m_ViewDescription.AddSheetDescription( SD_ColdConsumer, -1, m_pclSSheetConsumer, CPoint( 0, 0 ) );

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
	m_pclSSheetConsumer->SetMaxRows( RD_ColdConsumer_FirstAvailRow );
	m_pclSSheetConsumer->SetFreeze( 0, RD_ColdConsumer_FirstAvailRow - 1 );

	// All cells are static by default and filled with _T("").
	m_pclSSheetConsumer->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	m_pclSSheetConsumer->FormatStaticText( -1, -1, -1, -1, _T( "" ) );

	// Add columns.
	m_pclSDescConsumer->AddColumn( CD_ColdConsumer_FirstColumn, 1 );
	m_pclSDescConsumer->AddColumn( CD_ColdConsumer_Consumer, 1 );
	m_pclSDescConsumer->AddColumn( CD_ColdConsumer_Capacity, 1 );
	m_pclSDescConsumer->AddColumn( CD_ColdConsumer_Content, 1 );
	m_pclSDescConsumer->AddColumn( CD_ColdConsumer_ManualContent, 1 );
	m_pclSDescConsumer->AddColumn( CD_ColdConsumer_SupplyTemp, 1 );
	m_pclSDescConsumer->AddColumn( CD_ColdConsumer_ReturnTemp, 1 );
	m_pclSDescConsumer->AddColumn( CD_ColdConsumer_ManualTemp, 1 );
	m_pclSDescConsumer->AddColumn( CD_ColdConsumer_Troom, 1 );
	m_pclSDescConsumer->AddColumn( CD_ColdConsumer_Quantity, 1 );
	m_pclSDescConsumer->AddColumn( CD_ColdConsumer_Surface, 1 );

	// Resize columns.
	_CheckColdConsumerColumnWidth();

	// Row name.
	m_pclSSheetConsumer->SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );
	m_pclSSheetConsumer->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontSize, ( LPARAM )8 );
	m_pclSSheetConsumer->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, ( LPARAM )_TAH_TITLE_MAIN );
	m_pclSSheetConsumer->SetStaticText( CD_ColdConsumer_FirstColumn, RD_ColdConsumer_ColName, _T( "" ) );
	m_pclSSheetConsumer->SetStaticText( CD_ColdConsumer_FirstColumn, RD_ColdConsumer_UnitName, _T( "" ) );
	m_pclSSheetConsumer->SetStaticText( CD_ColdConsumer_Consumer, RD_ColdConsumer_ColName, IDS_INDSELPMSYSVOLHEATING_CONSUMERS );
	m_pclSSheetConsumer->SetStaticText( CD_ColdConsumer_Consumer, RD_ColdConsumer_UnitName, _T( "" ) );
	m_pclSSheetConsumer->SetStaticText( CD_ColdConsumer_Capacity, RD_ColdConsumer_ColName, IDS_INDSELPMSYSVOLCOOLING_CAPACITY );
	m_pclSSheetConsumer->SetStaticText( CD_ColdConsumer_Capacity, RD_ColdConsumer_UnitName, GetNameOf( pUnitDB->GetDefaultUnit( _U_TH_POWER ) ).c_str() );
	
	// Content.
	CString strContentStar;
	strContentStar = TASApp.LoadLocalizedString( IDS_INDSELPMSYSVOLHEATING_CONTENT );
	strContentStar += "*";
	m_pclSSheetConsumer->SetStaticText( CD_ColdConsumer_Content, RD_ColdConsumer_ColName, strContentStar );
	m_pclSSheetConsumer->SetStaticText( CD_ColdConsumer_Content, RD_ColdConsumer_UnitName, GetNameOf( pUnitDB->GetDefaultUnit( _U_VOLUME ) ).c_str() );
	m_pclSSheetConsumer->SetTextTip( SS_TT_STATUS_FLOATING, 250, NULL, _LIGHTYELLOW, _TAH_BLACK );
	
	// Manual content checkbox.
	m_pclSSheetConsumer->SetStaticText( CD_ColdConsumer_ManualContent, RD_ColdConsumer_ColName, _T( "" ) );
	m_pclSSheetConsumer->SetPictureCellWithID( IDI_PENCILDIAGSHORT, CD_ColdConsumer_ManualContent, RD_ColdConsumer_UnitName, CSSheet::PictureCellType::Icon );
	m_pclSSheetConsumer->SetBackColor( CD_ColdConsumer_ManualContent, RD_ColdConsumer_UnitName, _TAH_ORANGE );
	
	m_pclSSheetConsumer->SetStaticText( CD_ColdConsumer_SupplyTemp, RD_ColdConsumer_ColName, IDS_INDSELPMSYSVOLCOOLING_SUPPLYTEMP );
	m_pclSSheetConsumer->SetStaticText( CD_ColdConsumer_SupplyTemp, RD_ColdConsumer_UnitName, GetNameOf( pUnitDB->GetDefaultUnit( _U_TEMPERATURE ) ).c_str() );
	m_pclSSheetConsumer->SetStaticText( CD_ColdConsumer_ReturnTemp, RD_ColdConsumer_ColName, IDS_INDSELPMSYSVOLCOOLING_RETURNTEMP );
	m_pclSSheetConsumer->SetStaticText( CD_ColdConsumer_ReturnTemp, RD_ColdConsumer_UnitName, GetNameOf( pUnitDB->GetDefaultUnit( _U_TEMPERATURE ) ).c_str() );
	
	// Manual temp checkbox.
	m_pclSSheetConsumer->SetStaticText( CD_ColdConsumer_ManualTemp, RD_ColdConsumer_ColName, _T( "" ) );
	m_pclSSheetConsumer->SetPictureCellWithID( IDI_PENCILDIAGSHORT, CD_ColdConsumer_ManualTemp, RD_ColdConsumer_UnitName, CSSheet::PictureCellType::Icon );
	m_pclSSheetConsumer->SetBackColor( CD_ColdConsumer_ManualTemp, RD_ColdConsumer_UnitName, _TAH_ORANGE );
	
	m_pclSSheetConsumer->SetStaticText( CD_ColdConsumer_Troom, RD_ColdConsumer_ColName, IDS_INDSELPMSYSVOLCOOLING_TROOM );
	m_pclSSheetConsumer->SetStaticText( CD_ColdConsumer_Troom, RD_ColdConsumer_UnitName, GetNameOf( pUnitDB->GetDefaultUnit( _U_TEMPERATURE ) ).c_str() );
	
	// Quantity.
	m_pclSSheetConsumer->SetStaticText( CD_ColdConsumer_Quantity, RD_ColdConsumer_ColName, IDS_SELP_QUANTITY );
	m_pclSSheetConsumer->SetStaticText( CD_ColdConsumer_Quantity, RD_ColdConsumer_UnitName, _T( "" ) );
	
	// Surface.
	m_pclSSheetConsumer->SetStaticText( CD_ColdConsumer_Surface, RD_ColdConsumer_ColName, IDS_INDSELPMSYSVOLCOOLING_SURFACE );
	m_pclSSheetConsumer->SetStaticText( CD_ColdConsumer_Surface, RD_ColdConsumer_UnitName, GetNameOf( pUnitDB->GetDefaultUnit( _U_AREA ) ).c_str() );

	// To fill a icon.
	m_pclSSheetConsumer->SetPictureCellWithID( IDI_PLUSGREEN, CD_ColdGenerator_FirstColumn, RD_ColdConsumer_FirstAvailRow, CSSheet::PictureCellType::Icon );

	m_pclSSheetConsumer->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );

}

void CDlgIndSelPMSysVolCooling::OnBnClickedAddPipes()
{
	if( NULL == m_pclIndSelPMParams || NULL == m_pclIndSelPMParams->m_pclSelectPMList || NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()
			|| NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetCoolingPipeList() )
	{
		ASSERT_RETURN;
	}

	CPipeList *pclColdPipeList = m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetCoolingPipeList();

	CDlgIndSelAdditionalPiping dlg( m_pclIndSelPMParams, pclColdPipeList );
	dlg.DoModal();

	double dPipeContent = pclColdPipeList->GetTotalContent();
	m_clEditAdditionalPiping.SetWindowText( WriteCUDouble( _U_VOLUME, dPipeContent ) );
	_UpdateStaticValue();
}

void CDlgIndSelPMSysVolCooling::OnBnClickedResetAll()
{
	if( NULL == m_pclIndSelPMParams || NULL == m_pclIndSelPMParams->m_pclSelectPMList || NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()
			|| NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetColdGeneratorList()
			|| NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetColdConsumersList()
			|| NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetCoolingPipeList() )
	{
		ASSERT_RETURN;
	}

	CColdGeneratorList *pclColdGeneratorsList = m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetColdGeneratorList();
	CColdConsumersList *pclColdConsumersList = m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetColdConsumersList();
	CPipeList *pclColdPipeList = m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetCoolingPipeList();

	if( 0 != pclColdGeneratorsList->GetColdGeneratorCount() || 0 != pclColdConsumersList->GetConsumersCount() 
			|| 0 != pclColdPipeList->GetPipeList() )
	{
		CString str = TASApp.LoadLocalizedString( AFXMSG_INDSELPMSYSVOLHEATING_RESETALL );

		if( IDYES == ::AfxMessageBox( str, MB_YESNO | MB_ICONQUESTION ) )
		{
			pclColdGeneratorsList->Reset();
			pclColdConsumersList->Reset();
			pclColdPipeList->Reset();
			_UpdateStaticValue();

			// TODO: Clear all list and values.
			SS_CELLCOORD cellCoord;
			cellCoord.Col = 1;
			cellCoord.Row = RD_ColdConsumer_FirstAvailRow;

			while( m_iColdGeneratorCount )
			{
				_RemoveColdGenerator( &cellCoord );
			}

			while( m_iConsumerCount )
			{
				_RemoveColdConsumer( &cellCoord );
			}

			m_clEditOtherBuffer.SetWindowTextW( WriteCUDouble( _U_VOLUME, 0.0 ) );

			// If some pipe are removed warn user on Cancel.
			if( pclColdPipeList->GetPipeCount() )
			{
				m_bChangesDone = true;
			}

			pclColdPipeList->Reset();

			_ButtonPlusGeneratorEnabled();
			_ButtonPlusConsumerEnabled();
		}
	}
}

void CDlgIndSelPMSysVolCooling::OnBnClickedCancel()
{
	if( NULL == m_pclIndSelPMParams || NULL == m_pclIndSelPMParams->m_pclSelectPMList || NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()
			|| NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetColdGeneratorList()
			|| NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetColdConsumersList()
			|| NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetCoolingPipeList() )
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
		m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetColdGeneratorList()->CopyFrom( &m_clColdGeneratorListCopy );
		m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetColdConsumersList()->CopyFrom( &m_clConsumersListCopy );
		m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetCoolingPipeList()->CopyFrom( &m_clPipeListCopy );
	}

	CDialogExt::OnCancel();
}

void CDlgIndSelPMSysVolCooling::OnBnClickedOk()
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

void CDlgIndSelPMSysVolCooling::OnChangeBuffer()
{
	if( NULL == m_pclIndSelPMParams || NULL == m_pclIndSelPMParams->m_pclSelectPMList || NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()
			|| NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetColdConsumersList() )
	{
		ASSERT_RETURN;
	}

	CColdConsumersList *pclColdConsumersList = m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetColdConsumersList();

	double dPreviousBuffer = pclColdConsumersList->GetBufferContent();

	CString strBuffer;
	double dBuffer = 0.0;
	m_clEditOtherBuffer.GetWindowTextW( strBuffer );

	// If the buffer content is invalid put 0.0.
	if( RD_NOT_NUMBER == ReadDoubleFromStr( strBuffer.GetString(), &dBuffer ) || dBuffer < 0.0 )
	{
		m_bBufferValid = false;
		pclColdConsumersList->SetBufferContent( 0.0 );
	}
	else
	{
		m_bBufferValid = true;
		pclColdConsumersList->SetBufferContent( CDimValue::CUtoSI( _U_VOLUME, dBuffer ) );
	}

	// If the buffer is changed warn the user on cancel.
	if( dPreviousBuffer != pclColdConsumersList->GetBufferContent() )
	{
		m_bChangesDone = true;
	}

	_UpdateStaticValue();
}

void CDlgIndSelPMSysVolCooling::OnEnChangeMaxAirInputTempToAHU()
{
	if( NULL == m_pclIndSelPMParams || NULL == m_pclIndSelPMParams->m_pclSelectPMList || NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()
			|| NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetColdConsumersList() )
	{
		ASSERT_RETURN;
	}

	CColdConsumersList *pclColdConsumersList = m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetColdConsumersList();

	CString strBuffer;
	double dBuffer = 0.0;
	m_clEditMaxAirInputTempForAHU.GetWindowTextW( strBuffer );

	if( RD_OK == ReadDoubleFromStr( strBuffer.GetString(), &dBuffer ) || dBuffer < 0.0 )
	{
		pclColdConsumersList->SetMaxAirInputTempForAHU( m_clEditMaxAirInputTempForAHU.GetCurrentValSI() );
		_UpdateColdConsumer();
	}
}

LRESULT CDlgIndSelPMSysVolCooling::OnComboDropCloseUp( WPARAM wParam, LPARAM lParam )
{
	_UpdateColdGenerator();
	_ButtonPlusGeneratorEnabled();
	_UpdateColdConsumer();
	_ButtonPlusConsumerEnabled();
	return 0;
}

LRESULT CDlgIndSelPMSysVolCooling::OnSSDblClick( WPARAM wParam, LPARAM lParam )
{
	SS_CELLCOORD *pclCellCoord = ( SS_CELLCOORD * )lParam;

	if( NULL == pclCellCoord )
	{
		ASSERTA_RETURN( 0 );
	}

	// Avoid weird behavior on checkbox.
	if( IDC_SSCOLDGENLIST == wParam && ( CD_ColdGenerator_ManualContent == pclCellCoord->Col 
			|| CD_ColdGenerator_ManualTemp == pclCellCoord->Col ) )
	{
		// Returns 1 to ignore event.
		return 1;
	}

	if( IDC_SSCOLDCONSLIST == wParam && ( CD_ColdConsumer_ManualContent == pclCellCoord->Col 
			|| CD_ColdConsumer_ManualTemp == pclCellCoord->Col ) )
	{
		// Returns 1 to ignore event.
		return 1;
	}

	return 0;
}

LRESULT CDlgIndSelPMSysVolCooling::OnSSClick( WPARAM wParam, LPARAM lParam )
{
	SS_CELLCOORD *pclCellCoord = ( SS_CELLCOORD * )lParam;

	if( NULL == pclCellCoord )
	{
		ASSERTA_RETURN( 0 );
	}

	int iRecordIndex = pclCellCoord->Row - RD_ColdConsumer_FirstAvailRow;

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
	if( pclCellCoord->Row < RD_ColdGenerator_FirstAvailRow )
	{
		return 0;
	}

	LRESULT lEventProcessed = 0;

	if( IDC_SSCOLDGENLIST == wParam )
	{
		if( CD_ColdGenerator_FirstColumn == pclCellCoord->Col )
		{
			if( true == _ButtonPlusGeneratorEnabled() && iRecordIndex == m_iColdGeneratorCount ) // plus clicked
			{
				_AddColdGenerator( pclSSheet );
				pclSSheet->ShowCell( CD_ColdGenerator_FirstColumn, pclCellCoord->Row + 1, SS_SHOW_BOTTOMLEFT );
			}
			else if( m_iColdGeneratorCount > 0 && iRecordIndex < m_iColdGeneratorCount ) // trash clicked
			{
				_RemoveColdGenerator( pclCellCoord );
			}

			_UpdateColdGenerator();
			_ButtonPlusGeneratorEnabled();

			lEventProcessed = 1;
		}
		else if( CD_ColdGenerator_ManualContent == pclCellCoord->Col && iRecordIndex < m_iColdGeneratorCount ) // manual clicked
		{
			m_bChangesDone = true;

			if( true == _IsCheckBoxChecked( m_pclSSheetGenerator, pclCellCoord->Col, pclCellCoord->Row ) )
			{
				pclSSheet->SetCellText( CD_ColdGenerator_ManualContent, pclCellCoord->Row, _T("0") );
				pclSSheet->SetActiveCell( CD_ColdGenerator_Capacity, pclCellCoord->Row );
				pclSSheet->FormatStaticText( CD_ColdGenerator_Content, pclCellCoord->Row, CD_ColdGenerator_Content, pclCellCoord->Row, _T( "" ), SSS_ALIGN_RIGHT );
			}
			else
			{
				pclSSheet->SetCellText( CD_ColdGenerator_ManualContent, pclCellCoord->Row, _T("1") );
				pclSSheet->SetActiveCell( CD_ColdGenerator_Content, pclCellCoord->Row );
				pclSSheet->FormatEditDouble( CD_ColdGenerator_Content, pclCellCoord->Row, _T( "" ), SSS_ALIGN_RIGHT, true );
				SS_CELLTYPE rCellType;
				pclSSheet->SetTypeEdit( &rCellType, SSS_ALIGN_RIGHT, 15, SS_CHRSET_ALPHANUM, SS_CASE_NOCASE );
				pclSSheet->SetCellType( CD_ColdGenerator_Content, pclCellCoord->Row, &rCellType );
			}

			_UpdateColdGenerator();
			_ButtonPlusGeneratorEnabled();
			
			lEventProcessed = 1;
		}
		else if( CD_ColdGenerator_ManualTemp == pclCellCoord->Col && iRecordIndex < m_iColdGeneratorCount ) // manual temp clicked
		{
			m_bChangesDone = true;

			if( true == _IsCheckBoxChecked( m_pclSSheetGenerator, pclCellCoord->Col, pclCellCoord->Row ) )
			{
				pclSSheet->SetCellText( CD_ColdGenerator_ManualTemp, pclCellCoord->Row, _T("0") );
				pclSSheet->SetActiveCell( CD_ColdGenerator_Capacity, pclCellCoord->Row );
				pclSSheet->FormatStaticText( CD_ColdGenerator_SupplyTemp, pclCellCoord->Row, CD_ColdGenerator_SupplyTemp, pclCellCoord->Row, _T( "" ), SSS_ALIGN_RIGHT );
			}
			else
			{
				pclSSheet->SetCellText( CD_ColdGenerator_ManualTemp, pclCellCoord->Row, _T("1") );
				pclSSheet->SetActiveCell( CD_ColdGenerator_SupplyTemp, pclCellCoord->Row );
				pclSSheet->FormatEditDouble( CD_ColdGenerator_SupplyTemp, pclCellCoord->Row, _T( "" ), SSS_ALIGN_RIGHT, true );
				SS_CELLTYPE rCellType;
				pclSSheet->SetTypeEdit( &rCellType, SSS_ALIGN_RIGHT, 15, SS_CHRSET_ALPHANUM, SS_CASE_NOCASE );
				pclSSheet->SetCellType( CD_ColdGenerator_SupplyTemp, pclCellCoord->Row, &rCellType );
			}

			_UpdateColdGenerator();
			_ButtonPlusGeneratorEnabled();
			
			lEventProcessed = 1;
		}
	}
	else if( IDC_SSCOLDCONSLIST == wParam )
	{
		if( CD_ColdConsumer_FirstColumn == pclCellCoord->Col )
		{
			if( true == _ButtonPlusConsumerEnabled() && iRecordIndex == m_iConsumerCount ) // plus clicked
			{
				_AddColdConsumer( pclSSheet );
				pclSSheet->ShowCell( CD_ColdConsumer_FirstColumn, pclCellCoord->Row + 1, SS_SHOW_BOTTOMLEFT );
			}
			else if( m_iConsumerCount > 0 && iRecordIndex < m_iConsumerCount ) // trash clicked
			{
				_RemoveColdConsumer( pclCellCoord );
			}

			_UpdateColdConsumer();
			_ButtonPlusConsumerEnabled();
			
			lEventProcessed = 1;
		}
		else if( CD_ColdConsumer_ManualContent == pclCellCoord->Col && iRecordIndex < m_iConsumerCount )
		{
			m_bChangesDone = true;

			if( true == _IsCheckBoxChecked( m_pclSSheetConsumer, pclCellCoord->Col, pclCellCoord->Row ) )
			{
				pclSSheet->SetCellText( CD_ColdConsumer_ManualContent, pclCellCoord->Row, _T("0") );
				pclSSheet->SetActiveCell( CD_ColdConsumer_Capacity, pclCellCoord->Row );
				pclSSheet->FormatStaticText( CD_ColdConsumer_Content, pclCellCoord->Row, CD_ColdConsumer_Content, pclCellCoord->Row, _T( "" ), SSS_ALIGN_RIGHT );
			}
			else
			{
				pclSSheet->SetCellText( CD_ColdConsumer_ManualContent, pclCellCoord->Row, _T("1") );
				pclSSheet->SetActiveCell( CD_ColdConsumer_Content, pclCellCoord->Row );
				pclSSheet->FormatEditDouble( CD_ColdConsumer_Content, pclCellCoord->Row, _T( "" ), SSS_ALIGN_RIGHT, true );
				SS_CELLTYPE rCellType;
				pclSSheet->SetTypeEdit( &rCellType, SSS_ALIGN_RIGHT, 15, SS_CHRSET_ALPHANUM, SS_CASE_NOCASE );
				pclSSheet->SetCellType( CD_ColdConsumer_Content, pclCellCoord->Row, &rCellType );
			}

			_UpdateColdConsumer();
			_ButtonPlusConsumerEnabled();
			
			lEventProcessed = 1;
		}
		else if( CD_ColdConsumer_ManualTemp == pclCellCoord->Col && iRecordIndex < m_iConsumerCount )
		{
			m_bChangesDone = true;

			if( true == _IsCheckBoxChecked( m_pclSSheetConsumer, pclCellCoord->Col, pclCellCoord->Row ) )
			{
				pclSSheet->SetCellText( CD_ColdConsumer_ManualTemp, pclCellCoord->Row, _T("0") );
				pclSSheet->SetActiveCell( CD_ColdConsumer_Capacity, pclCellCoord->Row );
				pclSSheet->FormatStaticText( CD_ColdConsumer_SupplyTemp, pclCellCoord->Row, CD_ColdConsumer_SupplyTemp, pclCellCoord->Row, _T( "" ), SSS_ALIGN_RIGHT );
				pclSSheet->FormatStaticText( CD_ColdConsumer_ReturnTemp, pclCellCoord->Row, CD_ColdConsumer_ReturnTemp, pclCellCoord->Row, _T( "" ), SSS_ALIGN_RIGHT );
			}
			else
			{
				pclSSheet->SetCellText( CD_ColdConsumer_ManualTemp, pclCellCoord->Row, _T("1") );
				pclSSheet->SetActiveCell( CD_ColdConsumer_SupplyTemp, pclCellCoord->Row );
				pclSSheet->FormatEditDouble( CD_ColdConsumer_SupplyTemp, pclCellCoord->Row, _T( "" ), SSS_ALIGN_RIGHT, true );
				pclSSheet->FormatEditDouble( CD_ColdConsumer_ReturnTemp, pclCellCoord->Row, _T( "" ), SSS_ALIGN_RIGHT, true );
				SS_CELLTYPE rCellType;
				pclSSheet->SetTypeEdit( &rCellType, SSS_ALIGN_RIGHT, 15, SS_CHRSET_ALPHANUM, SS_CASE_NOCASE );
				pclSSheet->SetCellType( CD_ColdConsumer_SupplyTemp, pclCellCoord->Row, &rCellType );
				pclSSheet->SetCellType( CD_ColdConsumer_ReturnTemp, pclCellCoord->Row, &rCellType );
			}

			_UpdateColdConsumer();
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

LRESULT CDlgIndSelPMSysVolCooling::OnSSEditChange( WPARAM wParam, LPARAM lParam )
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

	bool bULONGNeeded = false;

	// intercept below first row only
	if( pclCellCoord->Row < RD_ColdConsumer_FirstAvailRow )
	{
		return 0;
	}

	if( IDC_SSCOLDGENLIST == wParam 
		&& ( CD_ColdGenerator_Capacity == pclCellCoord->Col
			 || CD_ColdGenerator_Content == pclCellCoord->Col
			 || CD_ColdGenerator_SupplyTemp == pclCellCoord->Col
			 || CD_ColdGenerator_Quantity == pclCellCoord->Col ) )
	{
		// Inside generator list.
		if( CD_ColdGenerator_Quantity == pclCellCoord->Col )
		{
			bULONGNeeded = true;
		}
	}
	else if( IDC_SSCOLDCONSLIST == wParam 
			 && ( CD_ColdConsumer_Capacity == pclCellCoord->Col
				  || CD_ColdConsumer_Content == pclCellCoord->Col
				  || CD_ColdConsumer_SupplyTemp == pclCellCoord->Col
				  || CD_ColdConsumer_ReturnTemp == pclCellCoord->Col
				  || CD_ColdConsumer_Troom == pclCellCoord->Col
				  || CD_ColdConsumer_Quantity == pclCellCoord->Col
				  || CD_ColdConsumer_Surface == pclCellCoord->Col ) )
	{
		// Inside consumer list.
		if( CD_ColdConsumer_Quantity == pclCellCoord->Col )
		{
			bULONGNeeded = true;
		}
	}
	else
	{
		return 0;
	}

	CString strValue = pclSSheet->GetCellText( pclCellCoord->Col, pclCellCoord->Row );
	double dValue;

	// HYS-1307: Enable to use "-" sign for temperature.
	bool bIsForTemperature = false;

	if( ( pclCellCoord->Col == CD_ColdConsumer_SupplyTemp ) || ( pclCellCoord->Col == CD_ColdConsumer_ReturnTemp ) 
			|| ( pclCellCoord->Col == CD_ColdGenerator_SupplyTemp ) )
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
				
				// HYS-1307: If "-" sign is first allow to continue ( Not set the cell with the string without "-" sign ).
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

	if( true == bULONGNeeded )
	{
		dValue = static_cast<unsigned long>( dValue );
		CString strLong;
		strLong.Format( _T("%d"), static_cast<unsigned long>( dValue ) );

		if( 0 != strLong.CompareNoCase( strValue ) )
		{
			pclSSheet->SetCellText( pclCellCoord->Col, pclCellCoord->Row, strLong );
		}
	}

	if( IDC_SSCOLDGENLIST == wParam )
	{
		_UpdateColdGenerator();
		_ButtonPlusGeneratorEnabled();
	}
	else if( IDC_SSCOLDCONSLIST == wParam )
	{
		_UpdateColdConsumer();
		_ButtonPlusConsumerEnabled();
	}

	return 0;
}

LRESULT CDlgIndSelPMSysVolCooling::OnTextTipFetch( WPARAM wParam, LPARAM lParam )
{
	SS_TEXTTIPFETCH *pTextTipFetch = (SS_TEXTTIPFETCH *)lParam;

	// Identify sheet description by the CSSheet window ID.
	CSheetDescription *pclSheetDescription = m_ViewDescription.GetFromSSheetID( (UINT)wParam );

	CString str = _T("");
	LRESULT ReturnValue = 0;
	CSSheet *pclSheet = m_pclSSheetConsumer;

	WORD borderType = SS_BORDERTYPE_LEFT | SS_BORDERTYPE_TOP | SS_BORDERTYPE_RIGHT | SS_BORDERTYPE_BOTTOM;
	WORD borderStyle = SS_BORDERSTYLE_SOLID;
	COLORREF borderColor = _TAH_RED_LIGHT;
	WORD border;
	WORD style;
	COLORREF color;

	std::wstring strGenSupplyTemp = m_pclSSheetGenerator->GetCellText( CD_ColdGenerator_SupplyTemp, pTextTipFetch->Row );
	double dGenSupplyTemp = 0.0;
	std::wstring strSupplyTemp = m_pclSSheetConsumer->GetCellText( CD_ColdConsumer_SupplyTemp, pTextTipFetch->Row );
	double dSupplyTemp = 0.0;
	std::wstring strReturnTemp = m_pclSSheetConsumer->GetCellText( CD_ColdConsumer_ReturnTemp, pTextTipFetch->Row );
	double dReturnTemp = 0.0;
	std::wstring strRoomTemp = m_pclSSheetConsumer->GetCellText( CD_ColdConsumer_Troom, pTextTipFetch->Row );
	double dRoomTemp = 0.0;
	double dMaxInputAirTemp = m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetColdConsumersList()->GetMaxAirInputTempForAHU();
	CString strConsumerID;

	if( SD_ColdGenerator == pclSheetDescription->GetSheetDescriptionID() )
	{
		if( CD_ColdGenerator_Capacity == pTextTipFetch->Col )
		{
			if( ( TRUE == m_pclSSheetGenerator->GetBorder( pTextTipFetch->Col, pTextTipFetch->Row, &border, &style, &color ) )
					&& ( border == borderType && style == borderStyle && color == _TAH_RED_LIGHT ) )
			{
				str = TASApp.LoadLocalizedString( IDS_INDSELPMSYSVOL_CAPACITYCANTBEZERO_TT );
			}
		}
		else if( CD_ColdGenerator_Content == pTextTipFetch->Col )
		{
			if( ( TRUE == m_pclSSheetGenerator->GetBorder( pTextTipFetch->Col, pTextTipFetch->Row, &border, &style, &color ) )
					&& ( border == borderType && style == borderStyle && color == _TAH_RED_LIGHT ) )
			{
				str = TASApp.LoadLocalizedString( IDS_INDSELPMSYSVOL_CONTENTCANTBEZERO_TT );
			}
		}
		else if( CD_ColdGenerator_SupplyTemp == pTextTipFetch->Col )
		{
			pclSheet = m_pclSSheetGenerator;

			if( ( TRUE == m_pclSSheetGenerator->GetBorder( pTextTipFetch->Col, pTextTipFetch->Row, &border, &style, &color ) )
					&& ( border == borderType && style == borderStyle && color == _TAH_RED_LIGHT ) )
			{
				// HYS-1612: pay attention, the 'm_dSupplyTemperature' variable is in SI and the 'dGenSupplyTemp' variable can be with other unit !
				if( RD_OK == ReadCUDoubleFromStr( _U_TEMPERATURE, strGenSupplyTemp, &dGenSupplyTemp ) )
				{
					if( dGenSupplyTemp < m_dSupplyTemperature )
					{
						str = TASApp.LoadLocalizedString( IDS_INDSELPMSYSVOL_TSGTSSYSL_TT );
					}
				}
			}
		}
	}
	else if( SD_ColdConsumer == pclSheetDescription->GetSheetDescriptionID() )
	{
		if( CD_ColdConsumer_Capacity == pTextTipFetch->Col )
		{
			if( ( TRUE == m_pclSSheetGenerator->GetBorder( pTextTipFetch->Col, pTextTipFetch->Row, &border, &style, &color ) )
					&& ( border == borderType && style == borderStyle && color == _TAH_RED_LIGHT ) )
			{
				str = TASApp.LoadLocalizedString( IDS_INDSELPMSYSVOL_CAPACITYCANTBEZERO_TT );
			}
		}
		else if( CD_ColdConsumer_SupplyTemp == pTextTipFetch->Col )
		{
			if( ( TRUE == m_pclSSheetConsumer->GetBorder( pTextTipFetch->Col, pTextTipFetch->Row, &border, &style, &color ) )
					&& ( border == borderType && style == borderStyle && color == _TAH_RED_LIGHT ) )
			{
				_FindConsumerID( m_pclSSheetConsumer, pTextTipFetch->Row, strConsumerID );
			
				// HYS-1612: pay attention that all temperatures read from string are in custom unit!
				ReadDoubleReturn_enum eSupplyReadStatus = ReadCUDoubleFromStr( _U_TEMPERATURE, strSupplyTemp, &dSupplyTemp );
				ReadDoubleReturn_enum eReturnReadStatus = ReadCUDoubleFromStr( _U_TEMPERATURE, strReturnTemp, &dReturnTemp );
				ReadDoubleReturn_enum eRoomReadStatus = ReadCUDoubleFromStr( _U_TEMPERATURE, strRoomTemp, &dRoomTemp );

				if( RD_OK == eSupplyReadStatus && dSupplyTemp < m_dSupplyTemperature )
				{
					str = TASApp.LoadLocalizedString( IDS_INDSELPMSYSVOL_TSTUTSSYSL_TT );
				}
				else if( RD_OK == eSupplyReadStatus && RD_OK == eReturnReadStatus && dSupplyTemp >= dReturnTemp )
				{
					str = TASApp.LoadLocalizedString( IDS_INDSELPMSYSVOL_TSTRL_TT );
				}
				else if( RD_OK == eRoomReadStatus && 0 != strConsumerID.Compare( _T( "PM_C_SURFACECOOLING" ) ) 
						&& dSupplyTemp >= dRoomTemp )
				{
					str = TASApp.LoadLocalizedString( IDS_INDSELPMSYSVOL_TSTROOML_TT );
				}
				else if( RD_OK == eSupplyReadStatus && RD_OK == eReturnReadStatus && RD_OK == eRoomReadStatus
						&& 0 == strConsumerID.Compare( _T( "PM_C_FANCOIL" ) ) )
				{
					if( dSupplyTemp >= dRoomTemp - 1 )
					{
						str = TASApp.LoadLocalizedString( IDS_INDSELPMSYSVOL_TSFCTROOMFCL_TT );
					}
					else if( dSupplyTemp >= dReturnTemp - 1 )
					{
						str = TASApp.LoadLocalizedString( IDS_INDSELPMSYSVOL_TSFCTRFCL_TT );
					}
				}
				else if( RD_OK == eSupplyReadStatus && 0 == strConsumerID.Compare( _T( "PM_C_AIRHANDLERUNIT" ) ) 
						&& dSupplyTemp >= dMaxInputAirTemp )
				{
					str = TASApp.LoadLocalizedString( IDS_INDSELPMSYSVOL_TSAHUTAL_TT );
				}
			}
		}
		else if( CD_ColdConsumer_ReturnTemp == pTextTipFetch->Col )
		{
			if( ( TRUE == m_pclSSheetConsumer->GetBorder( pTextTipFetch->Col, pTextTipFetch->Row, &border, &style, &color ) )
					&& ( border == borderType && style == borderStyle && color == _TAH_RED_LIGHT ) )
			{
				_FindConsumerID( m_pclSSheetConsumer, pTextTipFetch->Row, strConsumerID );

				// HYS-1612: pay attention that all temperatures read from string are in custom unit!
				ReadDoubleReturn_enum eSupplyReadStatus = ReadCUDoubleFromStr( _U_TEMPERATURE, strSupplyTemp, &dSupplyTemp );
				ReadDoubleReturn_enum eReturnReadStatus = ReadCUDoubleFromStr( _U_TEMPERATURE, strReturnTemp, &dReturnTemp );
				ReadDoubleReturn_enum eRoomReadStatus = ReadCUDoubleFromStr( _U_TEMPERATURE, strRoomTemp, &dRoomTemp );
			
				if( RD_OK == eSupplyReadStatus && RD_OK == eReturnReadStatus && dSupplyTemp >= dReturnTemp )
				{
					str = TASApp.LoadLocalizedString( IDS_INDSELPMSYSVOL_TSTRL_TT );
				}
				else if( RD_OK == eReturnReadStatus && RD_OK == eRoomReadStatus && 0 != strConsumerID.Compare( _T( "PM_C_SURFACECOOLING" ) )
						&& dReturnTemp >= dRoomTemp )
				{
					str = TASApp.LoadLocalizedString( IDS_INDSELPMSYSVOL_TRTROOML_TT );
				}
				else if( RD_OK == eSupplyReadStatus && RD_OK == eReturnReadStatus && 0 == strConsumerID.Compare( _T( "PM_C_FANCOIL" ) )
						&& dSupplyTemp >= dReturnTemp - 1 )
				{
					str = TASApp.LoadLocalizedString( IDS_INDSELPMSYSVOL_TSFCTRFCL_TT );
				}
				else if( RD_OK == eReturnReadStatus && 0 == strConsumerID.Compare( _T( "PM_C_AIRHANDLERUNIT" ) )
						&& dReturnTemp >= dMaxInputAirTemp )
				{
					str = TASApp.LoadLocalizedString( IDS_INDSELPMSYSVOL_TRAHUTAL_TT );
				}
			}
		}
		else if( CD_ColdConsumer_Troom == pTextTipFetch->Col )
		{
			if( ( TRUE == m_pclSSheetConsumer->GetBorder( pTextTipFetch->Col, pTextTipFetch->Row, &border, &style, &color ) )
					&& ( border == borderType && style == borderStyle && color == _TAH_RED_LIGHT ) )
			{
				_FindConsumerID( m_pclSSheetConsumer, pTextTipFetch->Row, strConsumerID );

				// HYS-1612: pay attention that all temperatures read from string are in custom unit!
				ReadDoubleReturn_enum eSupplyReadStatus = ReadCUDoubleFromStr( _U_TEMPERATURE, strSupplyTemp, &dSupplyTemp );
				ReadDoubleReturn_enum eReturnReadStatus = ReadCUDoubleFromStr( _U_TEMPERATURE, strReturnTemp, &dReturnTemp );
				ReadDoubleReturn_enum eRoomReadStatus = ReadCUDoubleFromStr( _U_TEMPERATURE, strRoomTemp, &dRoomTemp );
			
				if( RD_OK == eSupplyReadStatus && RD_OK == eRoomReadStatus && 0 != strConsumerID.Compare( _T( "PM_C_SURFACECOOLING" ) )
						&& dSupplyTemp >= dRoomTemp )
				{
					str = TASApp.LoadLocalizedString( IDS_INDSELPMSYSVOL_TSTROOML_TT );
				}
				else if( RD_OK == eReturnReadStatus && RD_OK == eRoomReadStatus && 0 != strConsumerID.Compare( _T( "PM_C_SURFACECOOLING" ) )
						&& dReturnTemp >= dRoomTemp )
				{
					str = TASApp.LoadLocalizedString( IDS_INDSELPMSYSVOL_TRTROOML_TT );
				}
				else if( RD_OK == eSupplyReadStatus && RD_OK == eRoomReadStatus && 0 == strConsumerID.Compare( _T( "PM_C_FANCOIL" ) )
						&& dSupplyTemp >= dRoomTemp - 1 )
				{
					str = TASApp.LoadLocalizedString( IDS_INDSELPMSYSVOL_TSFCTROOMFCL_TT );
				}
			}
		}
	}

	if( false == str.IsEmpty() )
	{
		pTextTipFetch->hText = NULL;
		pTextTipFetch->wMultiLine = SS_TT_MULTILINE_AUTO;
		pTextTipFetch->nWidth = (SHORT)pclSheet->GetTipTextWidth( str );
		wcsncpy_s( pTextTipFetch->szText, SS_TT_TEXTMAX, (LPCTSTR)str, SS_TT_TEXTMAX );
		pTextTipFetch->fShow = true;
		ReturnValue = 0;
	}

	return ReturnValue;
}

BOOL CDlgIndSelPMSysVolCooling::PreTranslateMessage( MSG *pMsg )
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
		GetDlgItem( IDC_SSCOLDGENLIST )->GetWindowRect( &rectHeatGen );

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
			CSheetDescription *pclSDesc = m_ViewDescription.GetFromSSheetID( IDC_SSCOLDGENLIST );
			CSSheet *plcSSheet = pclSDesc->GetSSheetPointer();
			plcSSheet->SendMessage( pMsg->message, pMsg->wParam, pMsg->lParam );
			GetDlgItem( IDC_SSCOLDGENLIST )->SetFocus();
		}

		CRect rectHeatCons;
		GetDlgItem( IDC_SSCOLDCONSLIST )->GetWindowRect( &rectHeatCons );

		CRect rectIntersectHeatCons;
		rectIntersectHeatCons.IntersectRect( rectCurrentFocus, rectHeatCons );

		if( TRUE == rectHeatCons.PtInRect( pMsg->pt ) && TRUE == rectIntersectHeatCons.IsRectNull() )
		{
			CSheetDescription *pclSDesc = m_ViewDescription.GetFromSSheetID( IDC_SSCOLDCONSLIST );
			CSSheet *plcSSheet = pclSDesc->GetSSheetPointer();
			plcSSheet->SendMessage( pMsg->message, pMsg->wParam, pMsg->lParam );
			GetDlgItem( IDC_SSCOLDCONSLIST )->SetFocus();
		}
	}

	return __super::PreTranslateMessage( pMsg ); // allow default behavior (return TRUE if you want to discard message)
}

void CDlgIndSelPMSysVolCooling::_UpdateStaticValue( void )
{
	if( NULL == m_pclIndSelPMParams || NULL == m_pclIndSelPMParams->m_pclSelectPMList || NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()
			|| NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetColdGeneratorList()
			|| NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetColdConsumersList()
			|| NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetCoolingPipeList() )
	{
		ASSERT_RETURN;
	}

	CColdGeneratorList *pclColdGeneratorsList = m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetColdGeneratorList();
	CColdConsumersList *pclColdConsumersList = m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetColdConsumersList();
	CPipeList *pclColdPipeList = m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetCoolingPipeList();

	double dTotalGenColdCapacity = pclColdGeneratorsList->GetTotalColdCapacity();
	m_clStaticTotalGenColdCapacity.SetWindowText( WriteCUDouble( _U_TH_POWER, dTotalGenColdCapacity, true ) );
	double dTotalGenContent = pclColdGeneratorsList->GetTotalContent();
	m_clStaticTotalGenContent.SetWindowText( WriteCUDouble( _U_VOLUME, dTotalGenContent, true ) );

	double dTotalConsColdCapacity = pclColdConsumersList->GetTotalColdCapacity();
	m_clStaticTotalConsColdCapacity.SetWindowText( WriteCUDouble( _U_TH_POWER, dTotalConsColdCapacity, true ) );
	
	double dTotalConsContent = pclColdConsumersList->GetTotalContent( m_dSupplyTemperature, m_dReturnTemperature );
	dTotalConsContent -= pclColdConsumersList->GetBufferContent();

	m_clStaticTotalConsContent.SetWindowText( WriteCUDouble( _U_VOLUME, dTotalConsContent, true ) );
	
	// HYS-869
	if( dTotalGenColdCapacity < dTotalConsColdCapacity )
	{
		m_clStaticTotalGenColdCapacity.SetTextColor( _ORANGE );
		m_clStaticTotalConsColdCapacity.SetTextColor( _ORANGE );
		m_clStaticTotalGenColdCapacity.SetToolTip( TASApp.LoadLocalizedString( IDS_INDSELPMSYSVOL_TOTALCAPACITY_TT ) );
		m_clStaticTotalConsColdCapacity.SetToolTip( TASApp.LoadLocalizedString( IDS_INDSELPMSYSVOL_TOTALCAPACITY_TT ) );
	}
	else
	{
		m_clStaticTotalGenColdCapacity.ResetToolTip();
		m_clStaticTotalConsColdCapacity.ResetToolTip();
		m_clStaticTotalGenColdCapacity.SetTextColor( _BLACK );
		m_clStaticTotalConsColdCapacity.SetTextColor( _BLACK );
	}
	double dTotalContent = pclColdGeneratorsList->GetTotalContent() + pclColdConsumersList->GetTotalContent( m_dSupplyTemperature,
			m_dReturnTemperature ) + pclColdPipeList->GetTotalContent();

	m_clStaticTotalContent.SetWindowText( WriteCUDouble( _U_VOLUME, dTotalContent, true ) );

	double dPipeContent = pclColdPipeList->GetTotalContent();
	m_clEditAdditionalPiping.SetWindowText( WriteCUDouble( _U_VOLUME, dPipeContent ) );
}

void CDlgIndSelPMSysVolCooling::_RemoveColdGenerator( SS_CELLCOORD *plcCellCoord )
{
	if( NULL == m_pclIndSelPMParams || NULL == m_pclIndSelPMParams->m_pclSelectPMList || NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()
			|| NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetColdGeneratorList() || NULL == plcCellCoord )
	{
		ASSERT_RETURN;
	}

	CColdGeneratorList *pclColdGeneratorsList = m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetColdGeneratorList();

	m_iColdGeneratorCount--;
	m_pclSSheetGenerator->DelRow( plcCellCoord->Row ); // drop a row
	m_pclSSheetGenerator->SetMaxRows( RD_ColdConsumer_FirstAvailRow + m_iColdGeneratorCount ); // Set max rows.
	pclColdGeneratorsList->RemoveColdGenerator( plcCellCoord->Row - RD_ColdConsumer_FirstAvailRow );
	m_bChangesDone = true;

	_CheckColdGeneratorColumnWidth();
}

void CDlgIndSelPMSysVolCooling::_RemoveColdConsumer( SS_CELLCOORD *plcCellCoord )
{
	if( NULL == m_pclIndSelPMParams || NULL == m_pclIndSelPMParams->m_pclSelectPMList || NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()
			|| NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetColdConsumersList() || NULL == plcCellCoord )
	{
		ASSERT_RETURN;
	}

	CColdConsumersList *pclColdConsumersList = m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetColdConsumersList();

	m_iConsumerCount--;
	m_pclSSheetConsumer->DelRow( plcCellCoord->Row ); // drop a row
	m_pclSSheetConsumer->SetMaxRows( RD_ColdConsumer_FirstAvailRow + m_iConsumerCount ); // Set max rows.
	pclColdConsumersList->RemoveConsumers( plcCellCoord->Row - RD_ColdConsumer_FirstAvailRow );
	m_bChangesDone = true;

	_CheckColdConsumerColumnWidth();
}

void CDlgIndSelPMSysVolCooling::_AddColdGenerator( CSSheet *plcSSheet, CString strType, double dCapacity, double dContent, bool bManual,
		double dTempSupply, bool bManualTemp, long lQuantity, bool bOnlyGUI )
{
	if( NULL == m_pclIndSelPMParams || NULL == m_pclIndSelPMParams->m_pclSelectPMList || NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()
			|| NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetColdGeneratorList() || NULL == plcSSheet )
	{
		ASSERT_RETURN;
	}

	CColdGeneratorList *pclColdGeneratorsList = m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetColdGeneratorList();

	m_iColdGeneratorCount++;
	int iCurrentRow = RD_ColdConsumer_FirstAvailRow + m_iColdGeneratorCount - 1;

	// Set max rows.
	plcSSheet->SetMaxRows( RD_ColdConsumer_FirstAvailRow + m_iColdGeneratorCount );

	// Add plus icon.
	plcSSheet->SetPictureCellWithID( IDI_PLUSGREEN, CD_ColdGenerator_FirstColumn, RD_ColdConsumer_FirstAvailRow + m_iColdGeneratorCount,
			CSSheet::PictureCellType::Icon );

	// Add checkbox.
	plcSSheet->SetCheckBox( CD_ColdGenerator_ManualContent, iCurrentRow, _T( "" ), false, true );
	plcSSheet->SetCheckBox( CD_ColdGenerator_ManualTemp, iCurrentRow, _T( "" ), false, true );

	// Add trash icon.
	plcSSheet->SetPictureCellWithID( IDI_TRASH, CD_ColdGenerator_FirstColumn, iCurrentRow, CSSheet::PictureCellType::Icon );

	// Add generator combo.
	plcSSheet->FormatComboList( CD_ColdGenerator_Generator, iCurrentRow );
	_FillColdGeneratorCombo( plcSSheet, CD_ColdGenerator_Generator, iCurrentRow, strType );

	// Add capacity edit.
	plcSSheet->FormatEditDouble( CD_ColdGenerator_Capacity, iCurrentRow, _T( "" ), SSS_ALIGN_RIGHT );
	
	CString strCapacity;
	strCapacity.Format( _T("%g"), dCapacity );
	plcSSheet->SetCellText( CD_ColdGenerator_Capacity, iCurrentRow, strCapacity );
	
	SS_CELLTYPE rCellType;
	plcSSheet->SetTypeEdit( &rCellType, SSS_ALIGN_RIGHT, 15, SS_CHRSET_ALPHANUM, SS_CASE_NOCASE );
	plcSSheet->SetCellType( CD_ColdGenerator_Capacity, iCurrentRow, &rCellType );

	CString strGeneratorID;
	_FindGeneratorID( plcSSheet, iCurrentRow, strGeneratorID );

	if( false == bOnlyGUI ) // Used to load data at dialog opening
	{
		pclColdGeneratorsList->AddColdGenerator( strGeneratorID, dCapacity, dContent, m_dSupplyTemperature );
		m_bChangesDone = true;
	}

	CString strContent;
	strContent.Format( _T("%g"), dContent );

	if( true == bManual ) // Manual Content input
	{
		plcSSheet->SetCellText( CD_ColdGenerator_ManualContent, iCurrentRow, _T("1") );
		plcSSheet->FormatEditDouble( CD_ColdGenerator_Content, iCurrentRow, strContent, SSS_ALIGN_RIGHT );
		plcSSheet->SetCellType( CD_ColdGenerator_Content, iCurrentRow, &rCellType );
	}
	else
	{
		plcSSheet->FormatStaticText( CD_ColdGenerator_Content, iCurrentRow, CD_ColdGenerator_Content, iCurrentRow, strContent, SSS_ALIGN_RIGHT );
	}

	CString strSupplyTemp;
	strSupplyTemp.Format( _T("%g"), bManualTemp ? dTempSupply : m_dSupplyTemperature );

	if( true == bManualTemp ) // Manual Supply Temp input
	{
		plcSSheet->SetCellText( CD_ColdGenerator_ManualTemp, iCurrentRow, _T("1") );
		plcSSheet->FormatEditDouble( CD_ColdGenerator_SupplyTemp, iCurrentRow, strSupplyTemp, SSS_ALIGN_RIGHT );
		plcSSheet->SetCellType( CD_ColdGenerator_SupplyTemp, iCurrentRow, &rCellType );
	}
	else
	{
		plcSSheet->FormatStaticText( CD_ColdGenerator_SupplyTemp, iCurrentRow, CD_ColdGenerator_SupplyTemp, iCurrentRow, strSupplyTemp, SSS_ALIGN_RIGHT );
	}

	// Add quantity edit.
	plcSSheet->FormatEditDouble( CD_ColdGenerator_Quantity, iCurrentRow, _T( "" ), SSS_ALIGN_RIGHT );
	CString strQuantity;
	strQuantity.Format( _T("%d"), lQuantity );
	plcSSheet->SetCellText( CD_ColdGenerator_Quantity, iCurrentRow, strQuantity );
	plcSSheet->SetCellType( CD_ColdGenerator_Quantity, iCurrentRow, &rCellType );

	_CheckColdGeneratorColumnWidth();
}

void CDlgIndSelPMSysVolCooling::_AddColdConsumer( CSSheet *plcSSheet, CString strType, double dCapacity, double dContent, bool bManualContent,
		double dSupplyTemp, double dReturnTemp, bool bManualTemp, bool bOnlyGUI, double dRoomTemp, long lQuantity, double dSurface )
{
	if( NULL == m_pclIndSelPMParams || NULL == m_pclIndSelPMParams->m_pclSelectPMList || NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()
			|| NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetColdConsumersList() || NULL == plcSSheet )
	{
		ASSERT_RETURN;
	}

	CColdConsumersList *pclColdConsumersList = m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetColdConsumersList();
	CColdGeneratorList *pclColdGeneratorList = m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetColdGeneratorList();

	m_iConsumerCount++;
	int iCurrentRow = RD_ColdConsumer_FirstAvailRow + m_iConsumerCount - 1;

	// Set max rows.
	plcSSheet->SetMaxRows( RD_ColdConsumer_FirstAvailRow + m_iConsumerCount );

	// Add plus icon.
	plcSSheet->SetPictureCellWithID( IDI_PLUSGREEN, CD_ColdConsumer_FirstColumn, iCurrentRow + 1, CSSheet::PictureCellType::Icon );

	// Add checkbox.
	plcSSheet->SetCheckBox( CD_ColdConsumer_ManualContent, iCurrentRow, _T( "" ), false, true );
	plcSSheet->SetCheckBox( CD_ColdConsumer_ManualTemp,    iCurrentRow, _T( "" ), false, true );

	// Add trash icon.
	plcSSheet->SetPictureCellWithID( IDI_TRASH, CD_ColdConsumer_FirstColumn, iCurrentRow, CSSheet::PictureCellType::Icon );

	// Add consumer combo.
	plcSSheet->FormatComboList( CD_ColdConsumer_Consumer, iCurrentRow );
	_FillConsumersCombo( plcSSheet, CD_ColdConsumer_Consumer, iCurrentRow, strType );

	// Add capacity edit.
	plcSSheet->FormatEditDouble( CD_ColdConsumer_Capacity, iCurrentRow, _T( "" ), SSS_ALIGN_RIGHT );
	
	// HYS-869
	if( ( false == bOnlyGUI ) && ( pclColdGeneratorList->GetTotalColdCapacity() > pclColdConsumersList->GetTotalColdCapacity() ) )
	{
		dCapacity = CDimValue::SItoCU( _U_TH_POWER, ( pclColdGeneratorList->GetTotalColdCapacity() - pclColdConsumersList->GetTotalColdCapacity() ) );
	}

	CString strCapacity;
	strCapacity.Format( _T("%g"), dCapacity );
	plcSSheet->SetCellText( CD_ColdConsumer_Capacity, iCurrentRow, strCapacity );
	
	SS_CELLTYPE rCellType;
	plcSSheet->SetTypeEdit( &rCellType, SSS_ALIGN_RIGHT, 15, SS_CHRSET_ALPHANUM, SS_CASE_NOCASE );
	plcSSheet->SetCellType( CD_ColdConsumer_Capacity, iCurrentRow, &rCellType );

	// Add Troom edit.
	plcSSheet->SetCellType( CD_ColdConsumer_Troom, iCurrentRow, &rCellType );
	CString strTRoom;
	strTRoom.Format( _T("%g"), dRoomTemp ? dRoomTemp : 20 );

	// Add Quantity.
	plcSSheet->SetCellType( CD_ColdConsumer_Quantity, iCurrentRow, &rCellType );
	CString strQuantity;
	strQuantity.Format( _T("%d"), lQuantity );
	plcSSheet->FormatEditDouble( CD_ColdConsumer_Quantity, iCurrentRow, strQuantity, SSS_ALIGN_RIGHT );

	// Add surface.
	plcSSheet->SetCellType( CD_ColdConsumer_Surface, iCurrentRow, &rCellType );
	CString strSurface;
	strSurface.Format( _T("%g"), dSurface );

	CString strConsumerID;
	_FindConsumerID( plcSSheet, iCurrentRow, strConsumerID );

	if( 0 == strConsumerID.Compare( _T("PM_C_SURFACECOOLING") ) )
	{
		plcSSheet->SetBackColor( CD_ColdConsumer_Troom, iCurrentRow, _LIGHTGRAY );
		plcSSheet->FormatEditDouble( CD_ColdConsumer_Surface, iCurrentRow, strSurface, SSS_ALIGN_RIGHT );
	}
	else if( 0 == strConsumerID.Compare( _T("PM_C_FANCOIL") ) )
	{
		plcSSheet->FormatEditDouble( CD_ColdConsumer_Troom, iCurrentRow, strTRoom, SSS_ALIGN_RIGHT );
		plcSSheet->SetBackColor( CD_ColdConsumer_Surface, iCurrentRow, _LIGHTGRAY );
	}
	else
	{
		plcSSheet->FormatEditDouble( CD_ColdConsumer_Troom, iCurrentRow, strTRoom, SSS_ALIGN_RIGHT );
		plcSSheet->SetBackColor( CD_ColdConsumer_Surface, iCurrentRow, _LIGHTGRAY );
	}

	if( false == bOnlyGUI ) // Used to load data at dialog opening
	{
		pclColdConsumersList->AddConsumers( strConsumerID, dCapacity, dContent, m_dSupplyTemperature, m_dReturnTemperature,
				bManualTemp, dRoomTemp, lQuantity, dSurface );
		
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
		plcSSheet->SetCellText( CD_ColdConsumer_ManualContent, iCurrentRow, _T("1") );
		plcSSheet->FormatEditDouble( CD_ColdConsumer_Content, iCurrentRow, strContent, SSS_ALIGN_RIGHT );
		plcSSheet->SetCellType( CD_ColdConsumer_Content, iCurrentRow, &rCellType );
	}
	else
	{
		plcSSheet->FormatStaticText( CD_ColdConsumer_Content, iCurrentRow, CD_ColdConsumer_Content, iCurrentRow, strContent, SSS_ALIGN_RIGHT );
	}

	if( true == bManualTemp ) // Manual input
	{
		plcSSheet->SetCellText( CD_ColdConsumer_ManualTemp, iCurrentRow, _T("1") );
		plcSSheet->FormatEditDouble( CD_ColdConsumer_SupplyTemp, iCurrentRow, strSupplyTemp, SSS_ALIGN_RIGHT );
		plcSSheet->FormatEditDouble( CD_ColdConsumer_ReturnTemp, iCurrentRow, strReturnTemp, SSS_ALIGN_RIGHT );
		plcSSheet->SetCellType( CD_ColdConsumer_SupplyTemp, iCurrentRow, &rCellType );
		plcSSheet->SetCellType( CD_ColdConsumer_ReturnTemp, iCurrentRow, &rCellType );
	}
	else
	{
		plcSSheet->FormatStaticText( CD_ColdConsumer_SupplyTemp, iCurrentRow, CD_ColdConsumer_SupplyTemp, iCurrentRow, strSupplyTemp, SSS_ALIGN_RIGHT );
		plcSSheet->FormatStaticText( CD_ColdConsumer_ReturnTemp, iCurrentRow, CD_ColdConsumer_ReturnTemp, iCurrentRow, strReturnTemp, SSS_ALIGN_RIGHT );
	}

	_CheckColdConsumerColumnWidth();
}

void CDlgIndSelPMSysVolCooling::_FillColdGeneratorCombo( CSSheet *pclSheet, long lCol, long lRow, const TCHAR *ptcSelect )
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
		CTable *pclTable = dynamic_cast<CTable *>( m_pclIndSelPMParams->m_pTADB->Get( _T("PMCOLDGENERATOR_TAB") ).MP );

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

void CDlgIndSelPMSysVolCooling::_FillConsumersCombo( CSSheet *pclSheet, long lCol, long lRow, const TCHAR *ptcSelect )
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
		CTable *pclTable = dynamic_cast<CTable *>( m_pclIndSelPMParams->m_pTADB->Get( _T("PMCOLDCONSUMERS_TAB") ).MP );

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

void CDlgIndSelPMSysVolCooling::_FindGeneratorID( CSSheet *pclSheet, int iRow, CString &strID )
{
	if( NULL == pclSheet )
	{
		ASSERT_RETURN;
	}

	strID = _T( "" );
	TCHAR tcComboValue[256];
	int iCurrentSelection = pclSheet->ComboBoxSendMessage( CD_ColdGenerator_Generator, iRow, SS_CBM_GETCURSEL, 0, 0 );
	pclSheet->ComboBoxSendMessage( CD_ColdGenerator_Generator, iRow, SS_CBM_GETLBTEXT, iCurrentSelection, ( LPARAM )tcComboValue );

	for( std::map<int, CDB_StringID *>::iterator mapIter = m_mapGeneratorType.begin(); mapIter != m_mapGeneratorType.end(); ++mapIter )
	{
		if( CString( mapIter->second->GetString() ).CompareNoCase( tcComboValue ) == 0 )
		{
			strID = mapIter->second->GetIDPtr().ID;
			return;
		}
	}
}

void CDlgIndSelPMSysVolCooling::_FindConsumerID( CSSheet *pclSheet, int iRow, CString &strID )
{
	if( NULL == pclSheet )
	{
		ASSERT_RETURN;
	}

	strID = _T( "" );
	TCHAR tcComboValue[256];
	int iCurrentSelection = pclSheet->ComboBoxSendMessage( CD_ColdConsumer_Consumer, iRow, SS_CBM_GETCURSEL, 0, 0 );
	pclSheet->ComboBoxSendMessage( CD_ColdConsumer_Consumer, iRow, SS_CBM_GETLBTEXT, iCurrentSelection, ( LPARAM )tcComboValue );

	for( std::map<int, CDB_StringID *>::iterator mapIter = m_mapConsumerType.begin(); mapIter != m_mapConsumerType.end(); ++mapIter )
	{
		if( CString( mapIter->second->GetString() ).CompareNoCase( tcComboValue ) == 0 )
		{
			strID = mapIter->second->GetIDPtr().ID;
			return;
		}
	}
}

bool CDlgIndSelPMSysVolCooling::_IsCheckBoxChecked( CSSheet *pclSheet, SS_COORD col, SS_COORD row )
{
	if( NULL == pclSheet )
	{
		ASSERTA_RETURN( false );
	}

	TCHAR tcCheckBox[16];
	pclSheet->GetValue( col, row, tcCheckBox );

	if( _T( '0' ) == tcCheckBox[0] )
	{
		return false;
	}
	else
	{
		return true;
	}
}

void CDlgIndSelPMSysVolCooling::_UpdateColdGenerator()
{
	if( NULL == m_pclIndSelPMParams || NULL == m_pclIndSelPMParams->m_pclSelectPMList || NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()
			|| NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetColdGeneratorList() )
	{
		ASSERT_RETURN;
	}

	CColdGeneratorList *pclColdGeneratorsList = m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetColdGeneratorList();

	for( int i = 0; i < m_iColdGeneratorCount; i++ )
	{
		int iRecordIndex = i;

		// Get generator ID.
		CString strGeneratorID;
		_FindGeneratorID( m_pclSSheetGenerator, i + RD_ColdConsumer_FirstAvailRow, strGeneratorID );

		// Get Capacity in SI.
		TCHAR tcCapacity[16];
		double dCapacity;
		m_pclSSheetGenerator->GetValue( CD_ColdGenerator_Capacity, i + RD_ColdConsumer_FirstAvailRow, tcCapacity );
		ReadCUDoubleFromStr( _U_TH_POWER, tcCapacity, &dCapacity );

		double dContent = -1.0;
		double dTemp = -1.0;
		bool bManual = _IsCheckBoxChecked( m_pclSSheetGenerator, CD_ColdGenerator_ManualContent, i + RD_ColdConsumer_FirstAvailRow );
		bool bManualTemp = _IsCheckBoxChecked( m_pclSSheetGenerator, CD_ColdGenerator_ManualTemp, i + RD_ColdConsumer_FirstAvailRow );

		if( true == bManual )
		{
			// If in manual input then get the Content in SI
			TCHAR tcContent[16];
			m_pclSSheetGenerator->GetValue( CD_ColdGenerator_Content, i + RD_ColdConsumer_FirstAvailRow, tcContent );
			ReadCUDoubleFromStr( _U_VOLUME, tcContent, &dContent );
			m_pclSSheetGenerator->SetBackColor( CD_ColdGenerator_Content, i + RD_ColdConsumer_FirstAvailRow, _WHITE );
		}

		if( true == bManualTemp )
		{
			// If in manual input then get the Content in SI
			TCHAR tcContent[16];
			m_pclSSheetGenerator->GetValue( CD_ColdGenerator_SupplyTemp, i + RD_ColdConsumer_FirstAvailRow, tcContent );
			ReadCUDoubleFromStr( _U_TEMPERATURE, tcContent, &dTemp );
			m_pclSSheetGenerator->SetBackColor( CD_ColdGenerator_SupplyTemp, i + RD_ColdConsumer_FirstAvailRow, _WHITE );
		}
		else
		{
			dTemp = m_dSupplyTemperature;
		}

		// Get Quantity.
		TCHAR tcQuantity[16];
		long lQuantity;
		m_pclSSheetGenerator->GetValue( CD_ColdGenerator_Quantity, i + RD_ColdConsumer_FirstAvailRow, tcQuantity );
		ReadLongFromStr( tcQuantity, &lQuantity );

		// Update database.
		pclColdGeneratorsList->UpdateColdGenerator( iRecordIndex, strGeneratorID, dCapacity, dContent, dTemp, bManualTemp, lQuantity );

		if( false == bManual )
		{
			// If not in manual input then refresh Content in spreadsheet
			dContent = pclColdGeneratorsList->GetColdGenerator( iRecordIndex )->m_dContent;

			m_pclSSheetGenerator->FormatStaticText( CD_ColdGenerator_Content, i + RD_ColdConsumer_FirstAvailRow, CD_ColdGenerator_Content,
					i + RD_ColdConsumer_FirstAvailRow, WriteCUDouble( _U_VOLUME, dContent ), SSS_ALIGN_RIGHT );

			m_pclSSheetGenerator->SetBackColor( CD_ColdGenerator_Content, i + RD_ColdConsumer_FirstAvailRow, _LIGHTGRAY );
		}

		if( false == bManualTemp )
		{
			// If not in manual input then refresh Content in spreadsheet
			dTemp = pclColdGeneratorsList->GetColdGenerator( iRecordIndex )->m_dSupplyTemp;

			m_pclSSheetGenerator->FormatStaticText( CD_ColdGenerator_SupplyTemp, i + RD_ColdConsumer_FirstAvailRow, CD_ColdGenerator_SupplyTemp,
					i + RD_ColdConsumer_FirstAvailRow, WriteCUDouble( _U_TEMPERATURE, dTemp ), SSS_ALIGN_RIGHT );

			m_pclSSheetGenerator->SetBackColor( CD_ColdGenerator_SupplyTemp, i + RD_ColdConsumer_FirstAvailRow, _LIGHTGRAY );
		}
	}

	_UpdateStaticValue();
}

void CDlgIndSelPMSysVolCooling::_UpdateColdConsumer()
{
	if( NULL == m_pclIndSelPMParams || NULL == m_pclIndSelPMParams->m_pclSelectPMList || NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()
			|| NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetColdConsumersList() )
	{
		ASSERT_RETURN;
	}

	CColdConsumersList *pclColdConsumersList = m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetColdConsumersList();

	for( int i = 0; i < m_iConsumerCount; i++ )
	{
		int iRecordIndex = i;

		// Get consumer ID.
		CString strConsumerID;
		_FindConsumerID( m_pclSSheetConsumer, i + RD_ColdConsumer_FirstAvailRow, strConsumerID );

		// Get Capacity in SI.
		TCHAR tcCapacity[16];
		double dCapacity;
		m_pclSSheetConsumer->GetValue( CD_ColdConsumer_Capacity, i + RD_ColdConsumer_FirstAvailRow, tcCapacity );
		ReadCUDoubleFromStr( _U_TH_POWER, tcCapacity, &dCapacity );

		// Get RoomTemp in SI.
		TCHAR tcRoomTemp[16];
		double dRoomTemp;
		m_pclSSheetConsumer->GetValue( CD_ColdConsumer_Troom, i + RD_ColdConsumer_FirstAvailRow, tcRoomTemp );
		ReadCUDoubleFromStr( _U_TEMPERATURE, tcRoomTemp, &dRoomTemp );

		// Get Quantity.
		TCHAR tcQuantity[16];
		long lQuantity;
		m_pclSSheetConsumer->GetValue( CD_ColdConsumer_Quantity, i + RD_ColdConsumer_FirstAvailRow, tcQuantity );
		ReadLongFromStr( tcQuantity, &lQuantity );

		// Get Surface in SI.
		TCHAR tcSurface[16];
		double dSurface;
		m_pclSSheetConsumer->GetValue( CD_ColdConsumer_Surface, i + RD_ColdConsumer_FirstAvailRow, tcSurface );
		ReadCUDoubleFromStr( _U_AREA, tcSurface, &dSurface );

		double dContent = -1.0;
		double dSupplyTemp = m_dSupplyTemperature;
		double dReturnTemp = m_dReturnTemperature;
		bool bManualContent = _IsCheckBoxChecked( m_pclSSheetConsumer, CD_ColdConsumer_ManualContent, i + RD_ColdConsumer_FirstAvailRow );
		bool bManualTemp = _IsCheckBoxChecked( m_pclSSheetConsumer, CD_ColdConsumer_ManualTemp, i + RD_ColdConsumer_FirstAvailRow );

		if( true == bManualContent )
		{
			// If in manual input then get the Content in SI.
			TCHAR tcContent[16];
			m_pclSSheetConsumer->GetValue( CD_ColdConsumer_Content, i + RD_ColdConsumer_FirstAvailRow, tcContent );
			ReadCUDoubleFromStr( _U_VOLUME, tcContent, &dContent );
			m_pclSSheetConsumer->SetBackColor( CD_ColdConsumer_Content, i + RD_ColdConsumer_FirstAvailRow, _WHITE );
		}

		if( true == bManualTemp )
		{
			// If in manual input then get the Content in SI
			TCHAR tcSupplyTemp[16];
			TCHAR tcReturnTemp[16];
			m_pclSSheetConsumer->GetValue( CD_ColdConsumer_SupplyTemp, i + RD_ColdConsumer_FirstAvailRow, tcSupplyTemp );
			m_pclSSheetConsumer->GetValue( CD_ColdConsumer_ReturnTemp, i + RD_ColdConsumer_FirstAvailRow, tcReturnTemp );
			ReadCUDoubleFromStr( _U_TEMPERATURE, tcSupplyTemp, &dSupplyTemp );
			ReadCUDoubleFromStr( _U_TEMPERATURE, tcReturnTemp, &dReturnTemp );
			m_pclSSheetConsumer->SetBackColor( CD_ColdConsumer_SupplyTemp, i + RD_ColdConsumer_FirstAvailRow, _WHITE );
			m_pclSSheetConsumer->SetBackColor( CD_ColdConsumer_ReturnTemp, i + RD_ColdConsumer_FirstAvailRow, _WHITE );
		}

		int iCurrentRow = i + RD_ColdConsumer_FirstAvailRow;

		if( 0 == strConsumerID.Compare( _T("PM_C_SURFACECOOLING") ) )
		{
			// Reformat only if changed.
			if( 0 != pclColdConsumersList->GetConsumer( iRecordIndex )->m_strConsumersTypeID.Compare( _T("PM_C_SURFACECOOLING") ) )
			{
				m_pclSSheetConsumer->FormatStaticText( CD_ColdConsumer_Troom, iCurrentRow, CD_ColdConsumer_Troom, iCurrentRow, _T( "" ), SSS_ALIGN_RIGHT );
				m_pclSSheetConsumer->SetBackColor( CD_ColdConsumer_Troom, iCurrentRow, _LIGHTGRAY );

				dSurface = pclColdConsumersList->GetConsumer( iRecordIndex )->m_dSurface;
				_swprintf_c( tcSurface, 15, _T("%g"), dSurface );
				m_pclSSheetConsumer->FormatEditDouble( CD_ColdConsumer_Surface, iCurrentRow, tcSurface, SSS_ALIGN_RIGHT );
			}
		}
		else if( 0 == strConsumerID.Compare( _T("PM_C_FANCOIL") ) )
		{
			// Reformat only if changed
			if( 0 != pclColdConsumersList->GetConsumer( iRecordIndex )->m_strConsumersTypeID.Compare( _T("PM_C_FANCOIL") ) )
			{
				dRoomTemp = pclColdConsumersList->GetConsumer(iRecordIndex)->m_dRoomTemp;
				_swprintf_c(tcRoomTemp, 15, _T("%g"), dRoomTemp);
				m_pclSSheetConsumer->FormatEditDouble(CD_ColdConsumer_Troom, iCurrentRow, tcRoomTemp, SSS_ALIGN_RIGHT);

				m_pclSSheetConsumer->FormatStaticText(CD_ColdConsumer_Surface, iCurrentRow, CD_ColdConsumer_Surface, iCurrentRow, _T(""), SSS_ALIGN_RIGHT);
				m_pclSSheetConsumer->SetBackColor(CD_ColdConsumer_Surface, iCurrentRow, _LIGHTGRAY);
			}
		}
		else if( 0 == strConsumerID.Compare( _T("PM_C_AIRHANDLERUNIT") ) )
		{
			// Reformat only if changed
			if( 0 != pclColdConsumersList->GetConsumer( iRecordIndex )->m_strConsumersTypeID.Compare( _T("PM_C_AIRHANDLERUNIT") ) )
			{
				dRoomTemp = pclColdConsumersList->GetConsumer( iRecordIndex )->m_dRoomTemp;
				_swprintf_c( tcRoomTemp, 15, _T("%g"), dRoomTemp );
				m_pclSSheetConsumer->FormatEditDouble( CD_ColdConsumer_Troom, iCurrentRow, tcRoomTemp, SSS_ALIGN_RIGHT );

				m_pclSSheetConsumer->FormatStaticText( CD_ColdConsumer_Surface, iCurrentRow, CD_ColdConsumer_Surface, iCurrentRow, _T( "" ), SSS_ALIGN_RIGHT );
				m_pclSSheetConsumer->SetBackColor( CD_ColdConsumer_Surface, iCurrentRow, _LIGHTGRAY );
			}
		}

		// Update database
		pclColdConsumersList->UpdateConsumers( iRecordIndex, strConsumerID, dCapacity, dContent, dSupplyTemp, dReturnTemp, bManualTemp,
				dRoomTemp, lQuantity, dSurface );

		if( false == bManualContent )
		{
			// If not in manual input then refresh Content in spreadsheet
			dContent = pclColdConsumersList->GetConsumer( iRecordIndex )->m_dContent;

			m_pclSSheetConsumer->FormatStaticText( CD_ColdConsumer_Content, i + RD_ColdConsumer_FirstAvailRow, CD_ColdConsumer_Content,
					i + RD_ColdConsumer_FirstAvailRow, WriteCUDouble( _U_VOLUME, dContent ), SSS_ALIGN_RIGHT );

			m_pclSSheetConsumer->SetBackColor( CD_ColdConsumer_Content, i + RD_ColdConsumer_FirstAvailRow, _LIGHTGRAY );
		}

		if( false == bManualTemp )
		{
			// If not in manual input then refresh Supply Temperature in spreadsheet
			m_pclSSheetConsumer->FormatStaticText( CD_ColdConsumer_SupplyTemp, i + RD_ColdConsumer_FirstAvailRow, CD_ColdConsumer_SupplyTemp,
					i + RD_ColdConsumer_FirstAvailRow, WriteCUDouble( _U_TEMPERATURE, m_dSupplyTemperature ), SSS_ALIGN_RIGHT );

			m_pclSSheetConsumer->SetBackColor( CD_ColdConsumer_SupplyTemp, i + RD_ColdConsumer_FirstAvailRow, _LIGHTGRAY );
			
			// If not in manual input then refresh Return Temperature in spreadsheet
			m_pclSSheetConsumer->FormatStaticText( CD_ColdConsumer_ReturnTemp, i + RD_ColdConsumer_FirstAvailRow, CD_ColdConsumer_ReturnTemp,
					i + RD_ColdConsumer_FirstAvailRow, WriteCUDouble( _U_TEMPERATURE, m_dReturnTemperature ), SSS_ALIGN_RIGHT );

			m_pclSSheetConsumer->SetBackColor( CD_ColdConsumer_ReturnTemp, i + RD_ColdConsumer_FirstAvailRow, _LIGHTGRAY );
		}
	}

	_UpdateStaticValue();
}

void CDlgIndSelPMSysVolCooling::_LoadColdGenerator()
{
	if( NULL == m_pclIndSelPMParams || NULL == m_pclIndSelPMParams->m_pclSelectPMList || NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()
			|| NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetColdGeneratorList() )
	{
		ASSERT_RETURN;
	}

	CColdGeneratorList *pclColdGeneratorsList = m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetColdGeneratorList();

	for( int i = 0; i < pclColdGeneratorsList->GetColdGeneratorCount(); i++ )
	{
		CColdGeneratorList::ColdGeneratorSave *coldGen = pclColdGeneratorsList->GetColdGenerator( i );
		
		_AddColdGenerator( 
				m_pclSSheetGenerator, 
				coldGen->m_strColdGeneratorTypeID, 
				CDimValue::SItoCU( _U_TH_POWER, coldGen->m_dColdCapacity ),
				CDimValue::SItoCU( _U_VOLUME, coldGen->m_dContent ), 
				coldGen->m_bManual, 
				CDimValue::SItoCU( _U_TEMPERATURE, coldGen->m_dSupplyTemp ),
				coldGen->m_bManualTemp, 
				coldGen->m_lQuantity, 
				true );
	}

	_UpdateColdGenerator();
}

void CDlgIndSelPMSysVolCooling::_LoadColdConsumer()
{
	if( NULL == m_pclIndSelPMParams || NULL == m_pclIndSelPMParams->m_pclSelectPMList || NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()
			|| NULL == m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetColdConsumersList() )
	{
		ASSERT_RETURN;
	}

	CColdConsumersList *pclColdConsumersList = m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetColdConsumersList();

	for( int i = 0; i < pclColdConsumersList->GetConsumersCount(); i++ )
	{
		CColdConsumersList::ConsumersSave *coldCons = pclColdConsumersList->GetConsumer( i );
		
		_AddColdConsumer( m_pclSSheetConsumer,
						  coldCons->m_strConsumersTypeID,
						  CDimValue::SItoCU( _U_TH_POWER, coldCons->m_dColdCapacity ),
						  CDimValue::SItoCU( _U_VOLUME, coldCons->m_dContent ),
						  coldCons->m_bManualContent,
						  CDimValue::SItoCU( _U_TEMPERATURE, coldCons->m_dSupplyTemp ),
						  CDimValue::SItoCU( _U_TEMPERATURE, coldCons->m_dReturnTemp ),
						  coldCons->m_bManualTemp,
						  true,
						  CDimValue::SItoCU( _U_TEMPERATURE, coldCons->m_dRoomTemp ),
						  coldCons->m_lQuantity,
						  CDimValue::SItoCU( _U_AREA, coldCons->m_dSurface ) );
	}

	_UpdateColdConsumer();
}

bool CDlgIndSelPMSysVolCooling::_ButtonPlusGeneratorEnabled()
{
	// Avoid to add a new generator if some input are invalid
	// not valid :
	// * content or capacity <= 0
	// * NAN
	// * Empty Cell
	bool bEnable = true;
	int iLastDataRow = m_iColdGeneratorCount + RD_ColdConsumer_FirstAvailRow - 1;
	WORD border = SS_BORDERTYPE_LEFT | SS_BORDERTYPE_TOP | SS_BORDERTYPE_RIGHT | SS_BORDERTYPE_BOTTOM;

	for( int i = iLastDataRow; i >= RD_ColdConsumer_FirstAvailRow; i-- )
	{
		std::wstring strContent = m_pclSSheetGenerator->GetCellText( CD_ColdGenerator_Content, i );
		double dContent = 0.0;

		// Border need to be removed if (manual && good) or (auto).
		m_pclSSheetGenerator->SetCellBorder( CD_ColdGenerator_Content, i, false );

		if( RD_OK != ReadDoubleFromStr( strContent, &dContent ) || dContent <= 0.0 )
		{
			if( _IsCheckBoxChecked( m_pclSSheetGenerator, CD_ColdGenerator_ManualContent, i ) )
			{
				// Red border to help the user to find errors.
				m_pclSSheetGenerator->SetCellBorder( CD_ColdGenerator_Content, i, CD_ColdGenerator_Content, i, true, border, SS_BORDERSTYLE_SOLID, 
						_TAH_RED_LIGHT );
			}

			bEnable = false;
		}

		std::wstring strCapacity = m_pclSSheetGenerator->GetCellText( CD_ColdGenerator_Capacity, i );
		double dCapacity = 0.0;

		if( RD_OK != ReadDoubleFromStr( strCapacity, &dCapacity ) || dCapacity <= 0.0 )
		{
			// Red border to help the user to find errors.
			m_pclSSheetGenerator->SetCellBorder( CD_ColdGenerator_Capacity, i, CD_ColdGenerator_Capacity, i, true, border, SS_BORDERSTYLE_SOLID, 
					_TAH_RED_LIGHT );

			bEnable = false;
		}
		else
		{
			// Remove border if input is valid.
			m_pclSSheetGenerator->SetCellBorder( CD_ColdGenerator_Capacity, i, false );
		}
		
		// HYS-1164 : Add Genrators supply temp
		std::wstring strGenSupplyTemp = m_pclSSheetGenerator->GetCellText( CD_ColdGenerator_SupplyTemp, i );
		double dGenSupplyTemp = 0.0;

		// HYS-1612: pay attention that all temperatures read from string are in custom unit!
		if( RD_OK != ReadCUDoubleFromStr( _U_TEMPERATURE, strGenSupplyTemp, &dGenSupplyTemp )
				|| dGenSupplyTemp < m_dSupplyTemperature )
		{
			// Red border to help the user to find errors.
			m_pclSSheetGenerator->SetCellBorder( CD_ColdGenerator_SupplyTemp, i, CD_ColdGenerator_SupplyTemp, i, true, border, SS_BORDERSTYLE_SOLID,
				_TAH_RED_LIGHT );

			bEnable = false;
		}
		else
		{
			// Remove border if input is valid.
			m_pclSSheetGenerator->SetCellBorder( CD_ColdGenerator_SupplyTemp, i, false );
		}
	}

	if( true == bEnable )
	{
		m_pclSSheetGenerator->SetBackColor( 
				CD_ColdGenerator_FirstColumn,
				RD_ColdConsumer_FirstAvailRow + m_iColdGeneratorCount,
				CD_ColdGenerator_LastColumn,
				RD_ColdConsumer_FirstAvailRow + m_iColdGeneratorCount,
				_WHITE );
	}
	else
	{
		m_pclSSheetGenerator->SetBackColor( 
				CD_ColdGenerator_FirstColumn,
				RD_ColdConsumer_FirstAvailRow + m_iColdGeneratorCount,
				CD_ColdGenerator_LastColumn,
				RD_ColdConsumer_FirstAvailRow + m_iColdGeneratorCount,
				_TAH_GREY_XLIGHT );
	}

	return m_bGeneratorValid = bEnable;
}

bool CDlgIndSelPMSysVolCooling::_ButtonPlusConsumerEnabled()
{
	// Avoid to add a new consumer if some input are invalid.
	// Cases where not valid :
	// * content or capacity <= 0
	// * NAN
	// * Empty Cell
	bool bEnable = true;
	int iLastDataRow = m_iConsumerCount + RD_ColdConsumer_FirstAvailRow - 1;
	WORD border = SS_BORDERTYPE_LEFT | SS_BORDERTYPE_TOP | SS_BORDERTYPE_RIGHT | SS_BORDERTYPE_BOTTOM;

	for( int i = iLastDataRow; i >= RD_ColdConsumer_FirstAvailRow; i-- )
	{
		CString strConsumerID;
		_FindConsumerID( m_pclSSheetConsumer, i, strConsumerID );

		std::wstring strContent = m_pclSSheetConsumer->GetCellText( CD_ColdConsumer_Content, i );
		double dContent = 0.0;

		// Border need to be removed if (manual && good) or (auto).
		m_pclSSheetConsumer->SetCellBorder( CD_ColdGenerator_Content, i, false );

		if( RD_OK != ReadDoubleFromStr( strContent, &dContent ) || dContent <= 0 )
		{
			if( true == _IsCheckBoxChecked( m_pclSSheetConsumer, CD_ColdGenerator_ManualContent, i ) )
			{
				// Red border to help the user to find errors.
				m_pclSSheetConsumer->SetCellBorder( CD_ColdConsumer_Content, i, CD_ColdConsumer_Content, i, true, border, SS_BORDERSTYLE_SOLID, 
						_TAH_RED_LIGHT );
			}

			bEnable = false;
		}

		std::wstring strCapacity = m_pclSSheetConsumer->GetCellText( CD_ColdConsumer_Capacity, i );
		double dCapacity = 0.0;

		if( RD_OK != ReadDoubleFromStr( strCapacity, &dCapacity ) || dCapacity <= 0.0 )
		{
			// Red border to help the user to find errors.
			m_pclSSheetConsumer->SetCellBorder( CD_ColdConsumer_Capacity, i, CD_ColdConsumer_Capacity, i, true, border, SS_BORDERSTYLE_SOLID, 
					_TAH_RED_LIGHT );

			bEnable = false;
		}
		else
		{
			// Remove border if input is valid.
			m_pclSSheetConsumer->SetCellBorder( CD_ColdConsumer_Capacity, i, false );
		}

		std::wstring strSupplyTemp = m_pclSSheetConsumer->GetCellText( CD_ColdConsumer_SupplyTemp, i );
		double dSupplyTemp = 0.0;

		// HYS-1612: pay attention that all temperatures read from string are in custom unit!
		if( RD_OK != ReadCUDoubleFromStr( _U_TEMPERATURE, strSupplyTemp, &dSupplyTemp )
				|| dSupplyTemp < m_dSupplyTemperature )
		{
			// Red border to help the user to find errors.
			m_pclSSheetConsumer->SetCellBorder( CD_ColdConsumer_SupplyTemp, i, CD_ColdConsumer_SupplyTemp, i, true, border, SS_BORDERSTYLE_SOLID, 
					_TAH_RED_LIGHT );

			bEnable = false;
		}
		else
		{
			// Remove border if input is valid.
			m_pclSSheetConsumer->SetCellBorder( CD_ColdConsumer_SupplyTemp, i, false );
		}

		std::wstring strReturnTemp = m_pclSSheetConsumer->GetCellText( CD_ColdConsumer_ReturnTemp, i );
		double dReturnTemp = 0.0;

		// HYS-1612: pay attention that all temperatures read from string are in custom unit!
		if( RD_OK != ReadCUDoubleFromStr( _U_TEMPERATURE, strReturnTemp, &dReturnTemp ) )
		{
			// Red border to help the user to find errors.
			m_pclSSheetConsumer->SetCellBorder( CD_ColdConsumer_ReturnTemp, i, CD_ColdConsumer_ReturnTemp, i, true, border, 
					SS_BORDERSTYLE_SOLID, _TAH_RED_LIGHT );

			bEnable = false;
		}
		else
		{
			// Remove border if input is valid.
			m_pclSSheetConsumer->SetCellBorder( CD_ColdConsumer_ReturnTemp, i, false );
		}

		/*//////////////////////////////*/
		std::wstring strRoomTemp = m_pclSSheetConsumer->GetCellText( CD_ColdConsumer_Troom, i );
		double dRoomTemp = 0.0;

		// HYS-1612: pay attention that all temperatures read from string are in custom unit!
		if( RD_OK != ReadCUDoubleFromStr( _U_TEMPERATURE, strRoomTemp, &dRoomTemp )
                && 0 != strConsumerID.Compare( _T("PM_C_SURFACECOOLING") ) )
		{
			// Red border to help the user to find errors.
			m_pclSSheetConsumer->SetCellBorder( CD_ColdConsumer_Troom, i, CD_ColdConsumer_Troom, i, true, border, SS_BORDERSTYLE_SOLID, _TAH_RED_LIGHT );
			bEnable = false;
		}
		else
		{
			// Remove border if input is valid.
			m_pclSSheetConsumer->SetCellBorder( CD_ColdConsumer_Troom, i, false );
		}
		
		// HYS-940
		bool bSupplyTempOK = true;
		bool bReturnTempOK = true;
		bool bRoomTempOK = true;

		m_pclIndSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetColdConsumersList()->VerifyConsCoolingTemperatureValues( strConsumerID, 
				dSupplyTemp, dReturnTemp, dRoomTemp, &bSupplyTempOK, &bReturnTempOK, &bRoomTempOK );
		
		if( false == bSupplyTempOK )
		{
			// Red border to help the user to find errors.
			m_pclSSheetConsumer->SetCellBorder( CD_ColdConsumer_SupplyTemp, i, CD_ColdConsumer_SupplyTemp, i, true, border, SS_BORDERSTYLE_SOLID, _TAH_RED_LIGHT );

			bEnable = false;
		}
		
		if( false == bReturnTempOK )
		{
			// Red border to help the user to find errors.
			m_pclSSheetConsumer->SetCellBorder( CD_ColdConsumer_ReturnTemp, i, CD_ColdConsumer_ReturnTemp, i, true, border,	SS_BORDERSTYLE_SOLID, _TAH_RED_LIGHT );

			bEnable = false;
		}
		
		if( false == bRoomTempOK )
		{
			// Red border to help the user to find errors.
			m_pclSSheetConsumer->SetCellBorder( CD_ColdConsumer_Troom, i, CD_ColdConsumer_Troom, i, true, border, SS_BORDERSTYLE_SOLID, _TAH_RED_LIGHT );
			bEnable = false;
		}

		// Surface validation.
		std::wstring strSurface = m_pclSSheetConsumer->GetCellText( CD_ColdConsumer_Surface, i );
		double dSurface = 0.0;

		if( RD_OK != ReadDoubleFromStr( strSurface, &dSurface )
				&& 0 == strConsumerID.Compare( _T("PM_C_SURFACECOOLING") ) )
		{
			// Red border to help the user to find errors.
			m_pclSSheetConsumer->SetCellBorder( CD_ColdConsumer_Surface, i, CD_ColdConsumer_Surface, i, true, border, SS_BORDERSTYLE_SOLID, _TAH_RED_LIGHT );
			bEnable = false;
		}
		else
		{
			// Remove border if input is valid.
			m_pclSSheetConsumer->SetCellBorder( CD_ColdConsumer_Surface, i, false );
		}
	}

	if( true == bEnable )
	{
		m_pclSSheetConsumer->SetBackColor( 
				CD_ColdConsumer_FirstColumn,
				RD_ColdConsumer_FirstAvailRow + m_iConsumerCount,
				CD_ColdConsumer_LastColumn,
				RD_ColdConsumer_FirstAvailRow + m_iConsumerCount,
				_WHITE );
	}
	else
	{
		m_pclSSheetConsumer->SetBackColor(
				CD_ColdConsumer_FirstColumn,
				RD_ColdConsumer_FirstAvailRow + m_iConsumerCount,
				CD_ColdConsumer_LastColumn,
				RD_ColdConsumer_FirstAvailRow + m_iConsumerCount,
				_TAH_GREY_XLIGHT );
	}

	return m_bConsumerValid = bEnable;
}

void CDlgIndSelPMSysVolCooling::_CheckColdGeneratorColumnWidth()
{
	if( NULL == m_pclSSheetGenerator )
	{
		return;
	}
	
	CRect rectClient;
	m_pclSSheetGenerator->GetClientRect( &rectClient );

	if( rectClient.Width() == m_ColdGeneratorRectClient.Width() )
	{
		return;
	}

	m_ColdGeneratorRectClient = rectClient;

	double dAvailableWidth = m_pclSSheetGenerator->LogUnitsToColWidthW( m_ColdGeneratorRectClient.Width() );

	// '-1' for the border size (Set by default to 1).
	double dStandardWidthInPixel = m_pclSSheetGenerator->ColWidthToLogUnits( 1 ) - 1;
	
	double dTotalColWidth = CGCW_First + CGCW_Generator + CGCW_Capacity + CGCW_Content + CGCW_Manual;
	dTotalColWidth += CGCW_SupplyTemp + CGCW_ManualTemp + CGCW_Quantity;

	double dRatio = dAvailableWidth / dTotalColWidth * dStandardWidthInPixel;

	// Add columns.
	long lWidthInPixel = (long)( dRatio * CGCW_First );
	long lTotalWidthInPixel = lWidthInPixel;
	m_pclSSheetGenerator->SetColWidthInPixels( CD_ColdGenerator_FirstColumn, lWidthInPixel );

	lWidthInPixel = (long)( dRatio * CGCW_Generator );
	lTotalWidthInPixel += lWidthInPixel;
	m_pclSSheetGenerator->SetColWidthInPixels( CD_ColdGenerator_Generator, lWidthInPixel );

	lWidthInPixel = (long)( dRatio * CGCW_Capacity );
	lTotalWidthInPixel += lWidthInPixel;
	m_pclSSheetGenerator->SetColWidthInPixels( CD_ColdGenerator_Capacity, lWidthInPixel );

	lWidthInPixel = (long)( dRatio * CGCW_Content );
	lTotalWidthInPixel += lWidthInPixel;
	m_pclSSheetGenerator->SetColWidthInPixels( CD_ColdGenerator_Content, lWidthInPixel );

	lWidthInPixel = (long)( dRatio * CGCW_Manual );
	lTotalWidthInPixel += lWidthInPixel;
	m_pclSSheetGenerator->SetColWidthInPixels( CD_ColdGenerator_ManualContent, lWidthInPixel );

	lWidthInPixel = (long)( dRatio * CGCW_SupplyTemp );
	lTotalWidthInPixel += lWidthInPixel;
	m_pclSSheetGenerator->SetColWidthInPixels( CD_ColdGenerator_SupplyTemp, lWidthInPixel );

	lWidthInPixel = (long)( dRatio * CGCW_ManualTemp );
	lTotalWidthInPixel += lWidthInPixel;
	m_pclSSheetGenerator->SetColWidthInPixels( CD_ColdGenerator_ManualTemp, lWidthInPixel );

	m_pclSSheetGenerator->SetColWidthInPixels( CD_ColdGenerator_Quantity, m_ColdGeneratorRectClient.Width() - lTotalWidthInPixel );
}

void CDlgIndSelPMSysVolCooling::_CheckColdConsumerColumnWidth()
{
	if( NULL == m_pclSSheetConsumer )
	{
		return;
	}
	
	CRect rectClient;
	m_pclSSheetConsumer->GetClientRect( &rectClient );

	if( rectClient.Width() == m_ColdConsumerRectClient.Width() )
	{
		return;
	}

	m_ColdConsumerRectClient = rectClient;

	double dAvailableWidth = m_pclSSheetConsumer->LogUnitsToColWidthW( m_ColdConsumerRectClient.Width() );
	double dStandardWidthInPixel = m_pclSSheetConsumer->ColWidthToLogUnits( 1 ) - 1;
	double dTotalColWidth = CCCW_First + CCCW_Consumer + CCCW_Capacity + CCCW_Content + CCCW_ManualContent + CCCW_SupplyTemp;
	dTotalColWidth += CCCW_ReturnTemp + CCCW_ManualTemp + CCCW_Troom + CCCW_Quantity + CCCW_Surface;

	double dRatio = dAvailableWidth / dTotalColWidth * dStandardWidthInPixel;

	long lWidthInPixel = (long)( dRatio * CCCW_First );
	long lTotalWidthInPixel = lWidthInPixel;
	m_pclSSheetConsumer->SetColWidthInPixels( CD_ColdConsumer_FirstColumn, lWidthInPixel );

	lWidthInPixel = (long)( dRatio * CCCW_Consumer );
	lTotalWidthInPixel += lWidthInPixel;
	m_pclSSheetConsumer->SetColWidthInPixels( CD_ColdConsumer_Consumer, lWidthInPixel );

	lWidthInPixel = (long)( dRatio * CCCW_Capacity );
	lTotalWidthInPixel += lWidthInPixel;
	m_pclSSheetConsumer->SetColWidthInPixels( CD_ColdConsumer_Capacity, lWidthInPixel );

	lWidthInPixel = (long)( dRatio * CCCW_Content );
	lTotalWidthInPixel += lWidthInPixel;
	m_pclSSheetConsumer->SetColWidthInPixels( CD_ColdConsumer_Content, lWidthInPixel );

	lWidthInPixel = (long)( dRatio * CCCW_ManualContent );
	lTotalWidthInPixel += lWidthInPixel;
	m_pclSSheetConsumer->SetColWidthInPixels( CD_ColdConsumer_ManualContent, lWidthInPixel );

	lWidthInPixel = (long)( dRatio * CCCW_SupplyTemp );
	lTotalWidthInPixel += lWidthInPixel;
	m_pclSSheetConsumer->SetColWidthInPixels( CD_ColdConsumer_SupplyTemp, lWidthInPixel );

	lWidthInPixel = (long)( dRatio * CCCW_ReturnTemp );
	lTotalWidthInPixel += lWidthInPixel;
	m_pclSSheetConsumer->SetColWidthInPixels( CD_ColdConsumer_ReturnTemp, lWidthInPixel );

	lWidthInPixel = (long)( dRatio * CCCW_ManualTemp );
	lTotalWidthInPixel += lWidthInPixel;
	m_pclSSheetConsumer->SetColWidthInPixels( CD_ColdConsumer_ManualTemp, lWidthInPixel );

	lWidthInPixel = (long)( dRatio * CCCW_Troom );
	lTotalWidthInPixel += lWidthInPixel;
	m_pclSSheetConsumer->SetColWidthInPixels( CD_ColdConsumer_Troom, lWidthInPixel );

	lWidthInPixel = (long)( dRatio * CCCW_Quantity );
	lTotalWidthInPixel += lWidthInPixel;
	m_pclSSheetConsumer->SetColWidthInPixels( CD_ColdConsumer_Quantity, lWidthInPixel );

	m_pclSSheetConsumer->SetColWidthInPixels( CD_ColdConsumer_Surface, m_ColdConsumerRectClient.Width() - lTotalWidthInPixel );
}
