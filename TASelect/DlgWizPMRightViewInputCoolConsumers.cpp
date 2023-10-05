#include "stdafx.h"
#include "TASelect.h"
#include "TASelectDoc.h"
#include "MainFrm.h"
#include "Global.h"
#include "SelectPM.h"
#include "DlgIndSelAdditionalPiping.h"
#include "DlgWizPMRightViewInputCoolConsumers.h"
#include "DlgIndSelPMSysVolCooling.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


CDlgWizardPM_RightViewInput_CoolingConsumers::CDlgWizardPM_RightViewInput_CoolingConsumers( CWizardSelPMParams *pclWizardSelPMParams, CWnd *pclParent )
	: CDlgWizardPM_RightViewInput_Base( pclWizardSelPMParams, CoolingConsumers, IDD, pclParent )
{
	m_bChangesDone = false;
	m_pclSDescConsumer = NULL;
	m_pclSSheetConsumer = NULL;
	m_rectSheetInitialSize.SetRectEmpty();
	m_iConsumerCount = 0;
	m_bConsumerValid = true;
	m_bBufferValid = true;

	m_ColdConsumerRectClient = CRect( 0, 0, 0, 0 );
	m_bGoToNormalModeMsgSent = false;
	m_bInitialized = false;
}

CDlgWizardPM_RightViewInput_CoolingConsumers::~CDlgWizardPM_RightViewInput_CoolingConsumers()
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

void CDlgWizardPM_RightViewInput_CoolingConsumers::FillPMInputUser()
{
	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pclSelectPMList 
			|| NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERT_RETURN;
	}

	CPMInputUser *pclPMInputUser = m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser();

	if( false == pclPMInputUser->IfSysVolExtDefExist() && false == pclPMInputUser->IfInstPowExtDefExist() )
	{
		// If user has finally put nothing, we reset to the backup value.
		// Remark: these values have been saved by 'CDlgWizardPM_RightViewInput_CoolingSystemVolumePower' before switching
		//         to this advance mode.
		pclPMInputUser->SetSystemVolume( pclPMInputUser->GetSystemVolumeBackup() );
		pclPMInputUser->SetInstalledPower( pclPMInputUser->GetInstalledPowerBackup() );
	}
	else
	{
		double dTotalContent = pclPMInputUser->GetColdGeneratorList()->GetTotalContent();
		
		dTotalContent += pclPMInputUser->GetColdConsumersList()->GetTotalContent( pclPMInputUser->GetSupplyTemperature(), 
				pclPMInputUser->GetReturnTemperature() );

		dTotalContent += pclPMInputUser->GetPipeList()->GetTotalContent();
		pclPMInputUser->SetSystemVolume( dTotalContent );

		pclPMInputUser->SetInstalledPower( pclPMInputUser->GetColdGeneratorList()->GetTotalColdCapacity() );
	}
}

BOOL CDlgWizardPM_RightViewInput_CoolingConsumers::PreTranslateMessage( MSG *pMsg )
{
	EnableNextButton( m_bConsumerValid && m_bBufferValid );

	if( WM_LBUTTONDOWN == pMsg->message )
	{
		// PWU : The first click on the spread is ignored if the focus is elsewhere.
		// This code catch the first click and send it directly to the spread.
		CRect rectColdCons;
		GetDlgItem( IDC_SSCOLDCONSLIST )->GetWindowRect( &rectColdCons );

		CRect rectCurrentFocus;
		CWnd *pFocusedCtrl = GetFocus();

		if( NULL != pFocusedCtrl )
		{
			pFocusedCtrl->GetWindowRect( rectCurrentFocus );
		}

		CRect rectIntersectColdCons;
		rectIntersectColdCons.IntersectRect( rectCurrentFocus, rectColdCons );

		if( rectColdCons.PtInRect( pMsg->pt ) && rectIntersectColdCons.IsRectNull() )
		{
			CSheetDescription *pclSDesc = m_ViewDescription.GetFromSSheetID( IDC_SSCOLDCONSLIST );
			CSSheet *plcSSheet = pclSDesc->GetSSheetPointer();
			plcSSheet->SendMessage( pMsg->message, pMsg->wParam, pMsg->lParam );
			GetDlgItem( IDC_SSCOLDCONSLIST )->SetFocus();
		}
	}

	return __super::PreTranslateMessage( pMsg ); // allow default behavior (return TRUE if you want to discard message)
}

BEGIN_MESSAGE_MAP( CDlgWizardPM_RightViewInput_CoolingConsumers, CDlgWizardPM_RightViewInput_Base )
	ON_WM_LBUTTONDOWN()
	ON_BN_CLICKED( IDC_ADDPIPES, OnBnClickedAddPipes )
	ON_BN_CLICKED( IDC_RESET, OnBnClickedReset )
	ON_BN_CLICKED( IDC_RESETALL, OnBnClickedResetAll )
	ON_BN_CLICKED( IDC_BUTTON_CANCEL, OnBnClickedCancel )
	ON_EN_KILLFOCUS( IDC_EDITBUFFER, OnKillFocusOtherVolume )
	ON_EN_CHANGE( IDC_EDITMAXAIRINPUTTEMPTOAHU, OnEnChangeMaxAirInputTempToAHU )
	ON_MESSAGE( SSM_COMBOCLOSEUP, OnComboDropCloseUp )
	ON_MESSAGE( SSM_CLICK, OnSSClick )
	ON_MESSAGE( SSM_DBLCLK, OnSSDblClick )
	ON_MESSAGE( SSM_EDITCHANGE, OnSSEditChange )
	ON_MESSAGE( SSM_TEXTTIPFETCH, OnTextTipFetch )
END_MESSAGE_MAP()

void CDlgWizardPM_RightViewInput_CoolingConsumers::DoDataExchange( CDataExchange *pDX )
{
	CDlgWizardPM_RightViewInput_Base::DoDataExchange( pDX );

	DDX_Control( pDX, IDC_STATICTOTALCONSCOLDCAPACITY, m_StaticTotalConsumerColdCapacity );
	DDX_Control( pDX, IDC_STATICTOTALCONSCONTENT, m_StaticTotalConsumerContent );
	DDX_Control( pDX, IDC_STATICBUFFER, m_StaticOtherVolume );
	DDX_Control( pDX, IDC_STATICADDITIONALPIPES, m_StaticAdditionalPipe );
	DDX_Control( pDX, IDC_STATICTARGETLINEARDP, m_StaticTargetLinearDp );
	DDX_Control( pDX, IDC_STATICTOTALINSTALL, m_StaticTotalInstallation );
	DDX_Control( pDX, IDC_STATICTOTALCONTENT, m_StaticTotalContent );

	DDX_Control( pDX, IDC_EDITBUFFER, m_ExtEditOtherVolume );
	DDX_Control( pDX, IDC_EDITPIPES, m_ExtEditAdditionalPipe );

	DDX_Control( pDX, IDC_STATICUNITBUFFER, m_StaticOtherVolumeUnit );
	DDX_Control( pDX, IDC_STATICUNITADDPIPES, m_StaticAdditionalPipeUnit );
	DDX_Control( pDX, IDC_STATIC_GONORMALMODE2, m_ButtonGoNormalMode );
	DDX_Control( pDX, IDC_STATICMAXAIRINPUTTEMPTOAHU, m_clStaticMaxAirInputTempToAHU );
	DDX_Control( pDX, IDC_EDITMAXAIRINPUTTEMPTOAHU, m_clEditMaxAirInputTempForAHU );
	DDX_Control( pDX, IDC_STATICUNITMAXAIRINPUTTEMPTOAHU, m_StaticMaxAirInputTempToAHUUnit );
}

BOOL CDlgWizardPM_RightViewInput_CoolingConsumers::OnInitDialog()
{
	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pclSelectPMList 
			|| NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERTA_RETURN( FALSE );
	}

	CPMInputUser *pclPMInputUser = m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser();

	CDlgWizardPM_RightViewInput_Base::OnInitDialog();

	SetMainTitle( TASApp.LoadLocalizedString( IDS_DLGWIZPM_RVIEWCOLDCONSUMERS_MAINTITLE ) );

	CString str;
	TCHAR name[_MAXCHARS];
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();
	
	//////////////////////////////////////////////////////////////////////////
	// EDITTEXT

	m_ExtEditOtherVolume.SetCurrentValSI( pclPMInputUser->GetColdConsumersList()->GetBufferContent() );
	InitNumericalEdit( &m_ExtEditOtherVolume, _U_VOLUME );

	m_ExtEditAdditionalPipe.SetCurrentValSI( pclPMInputUser->GetPipeList()->GetTotalContent() );
	InitNumericalEdit( &m_ExtEditAdditionalPipe, _U_VOLUME );

	//////////////////////////////////////////////////////////////////////////
	// STATIC

	if( false == pclPMInputUser->IsNorm( ProjectType::All, PressurisationNorm::PN_SWKIHE301_01 ) )
	{
		// Other, e.g. buffer:
		str = TASApp.LoadLocalizedString( IDS_DLGWIZPM_RVIEWCOLDCONSUMERS_OTHERBUFFER );
	}
	else
	{
		// Other:
		str = TASApp.LoadLocalizedString( IDS_DLGWIZPM_RVIEWCOLDCONSUMERS_OTHER );
	}

	m_StaticOtherVolume.SetTextAndToolTip( str );

	// Other, e.g. buffer: (Unit)
	GetNameOf( pUnitDB->GetDefaultUnit( _U_VOLUME ), name );
	SetDlgItemText( IDC_STATICUNITBUFFER, name );

	// Additional piping:
	str = TASApp.LoadLocalizedString( IDS_DLGWIZPM_RVIEWCOLDCONSUMERS_ADDPIPES );
	m_StaticAdditionalPipe.SetTextAndToolTip( str );

	// Additional piping: (Unit)
	GetNameOf( pUnitDB->GetDefaultUnit( _U_VOLUME ), name );
	SetDlgItemText( IDC_STATICUNITADDPIPES, name );
	
	// HYS-1164 : Synchronize with dlg system volume : Add max. air input temp. for air handlers
	str = TASApp.LoadLocalizedString( IDS_INDSELPMSYSVOLCOOLING_MAXAIRINPUTTEMPTOAHU );
	GetDlgItem( IDC_STATICMAXAIRINPUTTEMPTOAHU )->SetWindowText( str );

	GetNameOf( pUnitDB->GetDefaultUnit( _U_TEMPERATURE ), name );
	SetDlgItemText( IDC_STATICUNITMAXAIRINPUTTEMPTOAHU, name );
	m_StaticMaxAirInputTempToAHUUnit.SetBackColor( RGB( 255, 255, 255 ) );

	// HYS-958: If the max. air input temp. for air handlers is not yet defined, we take the max. temperature by default.
	if( -1.0 == pclPMInputUser->GetColdConsumersList()->GetMaxAirInputTempForAHU() )
	{
		pclPMInputUser->GetColdConsumersList()->SetMaxAirInputTempForAHU( m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetMaxTemperature() );
	}

	m_clEditMaxAirInputTempForAHU.SetCurrentValSI( pclPMInputUser->GetColdConsumersList()->GetMaxAirInputTempForAHU() );
	m_clEditMaxAirInputTempForAHU.Update();

	// Target linear Dp: xx Pa/m
	double dPipeTargetDp = m_pclTechParam->GetPipeTargDp();
	CString strTargetLinearDp = WriteCUDouble( _U_LINPRESSDROP, dPipeTargetDp );
	GetNameOf( pUnitDB->GetDefaultUnit( _U_LINPRESSDROP ), name );
	FormatString( str, IDS_DLGWIZPM_RVIEWCOOLCONSUMERS_TARGETLINEARDP, strTargetLinearDp, name );
	m_StaticTargetLinearDp.SetWindowTextW( str );
	m_StaticTargetLinearDp.SetToolTip( TASApp.LoadLocalizedString( IDS_INDSELPMSYSVOLCOOLING_TARGETLINEARDPTT ) );

	// Total installation content (Generators and consumers):
	str = TASApp.LoadLocalizedString( IDS_DLGWIZPM_RVIEWCOLDGENERATORS_TOTALCONTENT );
	m_StaticTotalInstallation.SetTextAndToolTip( str );

	m_StaticTotalConsumerColdCapacity.SetBackColor( RGB( 255, 255, 255 ) );
	m_StaticTotalConsumerContent.SetBackColor( RGB( 255, 255, 255 ) );
	m_StaticOtherVolume.SetBackColor( RGB( 255, 255, 255 ) );
	m_StaticAdditionalPipe.SetBackColor( RGB( 255, 255, 255 ) );
	m_StaticTargetLinearDp.SetBackColor( RGB( 255, 255, 255 ) );
	m_StaticTotalInstallation.SetBackColor( RGB( 255, 255, 255 ) );
	m_StaticTotalContent.SetBackColor( RGB( 255, 255, 255 ) );

	// Set some static text in bold.
	m_StaticTotalConsumerColdCapacity.SetFontBold( true );
	m_StaticTotalConsumerContent.SetFontBold( true );
	
	m_ExtEditOtherVolume.ActiveSpecialValidation( true, this );
	m_ExtEditAdditionalPipe.ActiveSpecialValidation( true, this );
	
	m_StaticOtherVolumeUnit.SetBackColor( RGB( 255, 255, 255 ) );
	m_StaticAdditionalPipeUnit.SetBackColor( RGB( 255, 255, 255 ) );
	
	CButton *pbtAddPipe = ( CButton * )GetDlgItem( IDC_ADDPIPES );

	if( NULL != pbtAddPipe && NULL != pbtAddPipe->GetSafeHwnd() )
	{
		pbtAddPipe->SetIcon( ( HICON )LoadImage( AfxGetApp()->m_hInstance,
												MAKEINTRESOURCE( IDI_PIPECALCULATOR ),
												IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR ) );
	}

	m_ButtonGoNormalMode.ShowWindow( SW_HIDE );
	m_ButtonGoNormalMode.GetWindowRect( &m_ButtonGoNormalModePos );
	ScreenToClient( &m_ButtonGoNormalModePos );

	str = TASApp.LoadLocalizedString( IDS_CANCEL );
	GetDlgItem( IDC_BUTTON_CANCEL )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_BUTTON_RESET );
	GetDlgItem( IDC_RESET )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_BUTTON_RESETALL );
	GetDlgItem( IDC_RESETALL )->SetWindowText( str );

	_InitializeSSheet();
	_LoadColdConsumer();
	_ButtonPlusConsumerEnabled();
	_UpdateStaticValue();

	m_bInitialized = true;

	return TRUE;
}

void CDlgWizardPM_RightViewInput_CoolingConsumers::OnPaint()
{
	CDlgWizardPM_RightViewInput_Base::OnPaint();

	DrawGoAdvancedNormalModeButtton( Draw_NormalMode, m_ButtonGoNormalModePos );
}

void CDlgWizardPM_RightViewInput_CoolingConsumers::OnSize( UINT nType, int cx, int cy )
{
	CDlgWizardPM_RightViewInput_Base::OnSize( nType, cx, cy );

	if( NULL == GetDlgItem( IDC_SSCOLDCONSLIST ) || NULL == GetDlgItem( IDC_SSCOLDCONSLIST )->GetSafeHwnd() )
	{
		// Not yet ready.
		return;
	}

	CRect rectClient;
	GetClientRect( &rectClient );

	// Move reset button.
	CRect rectButtonReset;
	GetDlgItem( IDC_RESET )->GetWindowRect( &rectButtonReset);
	ScreenToClient( &rectButtonReset );
	rectButtonReset.MoveToXY( 4, rectClient.bottom - 4 - rectButtonReset.Height() );
	GetDlgItem( IDC_RESET )->SetWindowPos( NULL, rectButtonReset.left, rectButtonReset.top, -1, -1, SWP_NOSIZE | SWP_NOZORDER );

	// Move reset all button.
	GetDlgItem( IDC_RESETALL )->SetWindowPos( NULL, rectButtonReset.right + 4, rectButtonReset.top, -1, -1, SWP_NOSIZE | SWP_NOZORDER );

	// Move abort button.
	CRect rectButtonResetAll;
	GetDlgItem( IDC_RESETALL )->GetWindowRect( &rectButtonResetAll);
	ScreenToClient( &rectButtonResetAll );
	GetDlgItem( IDC_BUTTON_CANCEL )->SetWindowPos( NULL, rectButtonResetAll.right + 4, rectButtonResetAll.top, -1, -1, SWP_NOSIZE | SWP_NOZORDER );
	
	// Resize sheet.
	_CheckColdConsumerSheetHeight();
	_CheckColdConsumerColumnWidth();
	
	CRect rectSheet;
	GetDlgItem( IDC_SSCOLDCONSLIST )->GetWindowRect( &rectSheet );
	ScreenToClient( &rectSheet );

	// Resize separator below sheet.
	CRect rectSeparatorBelowSheet;
	GetDlgItem( IDC_STATICBELOWSHEET )->GetClientRect( &rectSeparatorBelowSheet );
	int iNewY = rectSheet.bottom + 1;
	GetDlgItem( IDC_STATICBELOWSHEET )->SetWindowPos( NULL, rectSheet.left, iNewY, rectSheet.Width(), 2, SWP_NOZORDER );

	// Move total consumers cold capacity.
	CRect rectTotalConsumerColdCapacity;
	m_StaticTotalConsumerColdCapacity.GetWindowRect( &rectTotalConsumerColdCapacity );
	ScreenToClient( &rectTotalConsumerColdCapacity );
	CRect rectCellCapacity = m_pclSSheetConsumer->GetCellCoordInPixel( CD_ColdConsumer_Capacity, RD_ColdConsumer_UnitName );
	int iNewX = 20 + rectCellCapacity.right - rectTotalConsumerColdCapacity.Width();
	iNewY += 4;
	m_StaticTotalConsumerColdCapacity.SetWindowPos( NULL, iNewX, iNewY, -1, -1, SWP_NOSIZE | SWP_NOZORDER );

	// Move total consumers content.
	CRect rectTotalConsumerContent;
	m_StaticTotalConsumerContent.GetWindowRect( &rectTotalConsumerContent );
	ScreenToClient( &rectTotalConsumerContent );
	CRect rectCellContent = m_pclSSheetConsumer->GetCellCoordInPixel( CD_ColdConsumer_Content, RD_ColdConsumer_UnitName );
	iNewX = 20 + rectCellContent.right - rectTotalConsumerContent.Width();
	m_StaticTotalConsumerContent.SetWindowPos( NULL, iNewX, iNewY, -1, -1, SWP_NOSIZE | SWP_NOZORDER );

	// Move static other volume unit.
	CRect rectStaticOtherVolumeUnit;
	m_StaticOtherVolumeUnit.GetWindowRect( &rectStaticOtherVolumeUnit );
	ScreenToClient( &rectStaticOtherVolumeUnit );
	iNewX = 20 + rectCellContent.right;
	iNewY += rectTotalConsumerContent.Height() + 14;
	m_StaticOtherVolumeUnit.SetWindowPos( NULL, iNewX, iNewY, -1, -1, SWP_NOSIZE | SWP_NOZORDER );

	// Move edit field for other volume.
	CRect rectEditOtherVolume;
	m_ExtEditOtherVolume.GetWindowRect( &rectEditOtherVolume );
	ScreenToClient( &rectEditOtherVolume );
	iNewX -= ( 2 + rectEditOtherVolume.Width() );
	iNewY -= 4;
	m_ExtEditOtherVolume.SetWindowPos( NULL, iNewX, iNewY, -1, -1, SWP_NOSIZE | SWP_NOZORDER );

	// Move static other volume.
	CRect rectButtonAdditionalPipe;
	GetDlgItem( IDC_ADDPIPES )->GetWindowRect( &rectButtonAdditionalPipe );
	ScreenToClient( &rectButtonAdditionalPipe );

	CRect rectStaticOtherVolume;
	m_StaticOtherVolume.GetWindowRect( &rectStaticOtherVolume );
	ScreenToClient( &rectStaticOtherVolume );
	iNewX -= ( 2 + rectButtonAdditionalPipe.Width() + 2 + rectStaticOtherVolume.Width() );
	iNewY += 3;
	m_StaticOtherVolume.SetWindowPos( NULL, iNewX, iNewY, -1, -1, SWP_NOSIZE | SWP_NOZORDER );

	// Move static additional pipe unit.
	CRect rectStaticAdditionalPipeUnit;
	m_StaticAdditionalPipeUnit.GetWindowRect( &rectStaticAdditionalPipeUnit );
	ScreenToClient( &rectStaticAdditionalPipeUnit );
	iNewX = 20 + rectCellContent.right;
	iNewY += rectStaticOtherVolume.Height() + 13;
	m_StaticAdditionalPipeUnit.SetWindowPos( NULL, iNewX, iNewY, -1, -1, SWP_NOSIZE | SWP_NOZORDER );

	// Move edit field for additional pipe.
	CRect rectEditAdditionalPipe;
	m_ExtEditAdditionalPipe.GetWindowRect( &rectEditAdditionalPipe );
	ScreenToClient( &rectEditAdditionalPipe );
	iNewX -= ( 2 + rectEditAdditionalPipe.Width() );
	iNewY -= 4;
	m_ExtEditAdditionalPipe.SetWindowPos( NULL, iNewX, iNewY, -1, -1, SWP_NOSIZE | SWP_NOZORDER );

	// Move additional pipe button.
	iNewX -= ( 2 + rectButtonAdditionalPipe.Width() );
	GetDlgItem( IDC_ADDPIPES )->SetWindowPos( NULL, iNewX, iNewY, -1, -1, SWP_NOSIZE | SWP_NOZORDER );

	// Move static additional pipe.
	CRect rectStaticAdditionalPipe;
	m_StaticAdditionalPipe.GetWindowRect( &rectStaticAdditionalPipe );
	ScreenToClient( &rectStaticAdditionalPipe );
	iNewX -= ( 2 + rectStaticAdditionalPipe.Width() );
	iNewY += 3;
	m_StaticAdditionalPipe.SetWindowPos( NULL, iNewX, iNewY, -1, -1, SWP_NOSIZE | SWP_NOZORDER );

	// Move target linear Dp.
	CRect rectStaticTargetLinearDp;
	m_StaticTargetLinearDp.GetWindowRect( &rectStaticTargetLinearDp );
	ScreenToClient( &rectStaticTargetLinearDp );
	iNewX = rectClient.Width() - 20 - rectStaticTargetLinearDp.Width();
	m_StaticTargetLinearDp.SetWindowPos( NULL, iNewX, iNewY, -1, -1, SWP_NOSIZE | SWP_NOZORDER );

	// Move separator.
	CRect rectSeparator;
	GetDlgItem( IDC_STATIC )->GetWindowRect( &rectSeparator );
	ScreenToClient( &rectSeparator );
	iNewX = rectClient.Width() - 20 - rectSeparator.Width();
	iNewY += rectEditAdditionalPipe.Height() + 15;
	GetDlgItem( IDC_STATIC )->SetWindowPos( NULL, iNewX, iNewY, -1, -1, SWP_NOSIZE | SWP_NOZORDER );

	// Move total content value.
	CRect rectTotalContentValue;
	m_StaticTotalContent.GetWindowRect( &rectTotalContentValue );
	ScreenToClient( &rectTotalContentValue );
	iNewX = rectClient.Width() - 20 - rectTotalContentValue.Width();
	iNewY += 5;
	m_StaticTotalContent.SetWindowPos( NULL, iNewX, iNewY, -1, -1, SWP_NOSIZE | SWP_NOZORDER );

	// Move total content text.
	CRect rectTotalContentText;
	m_StaticTotalInstallation.GetWindowRect( &rectTotalContentText );
	ScreenToClient( &rectTotalContentText );
	iNewX -= ( rectTotalContentText.Width() + 4 );
	m_StaticTotalInstallation.SetWindowPos( NULL, iNewX, iNewY, -1, -1, SWP_NOSIZE | SWP_NOZORDER );
}

void CDlgWizardPM_RightViewInput_CoolingConsumers::OnLButtonDown( UINT nFlags, CPoint point )
{
	if( false == m_bGoToNormalModeMsgSent )
	{
		CRect rectClient;
		GetClientRect( &rectClient );
		CRect rectGoButton( 20, m_ButtonGoNormalModePos.top, rectClient.Width() - 20, m_ButtonGoNormalModePos.top + 20 );

		if( TRUE == rectGoButton.PtInRect( (POINT)point ) )
		{
			m_bGoToNormalModeMsgSent = true;
			m_pclParent->PostMessage( WM_USER_RIGHTVIEWWIZPM_SWITCHMODE, WPARAM( CoolingSystemVolumePower ), 0 );
		}
	}

	CDlgWizardPM_RightViewInput_Base::OnLButtonDown( nFlags, point );
}

void CDlgWizardPM_RightViewInput_CoolingConsumers::OnBnClickedAddPipes()
{
	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pclSelectPMList 
			|| NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERT_RETURN;
	}

	CPMInputUser *pclPMInputUser = m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser();

	CDlgIndSelAdditionalPiping dlg( m_pclWizardSelPMParams, pclPMInputUser->GetPipeList() );

	if( IDOK == dlg.DoModal() )
	{
	}

	double dPipeContent = pclPMInputUser->GetPipeList()->GetTotalContent();
	m_ExtEditAdditionalPipe.SetWindowText( WriteCUDouble( _U_VOLUME, dPipeContent ) );
	_UpdateStaticValue();
}

void CDlgWizardPM_RightViewInput_CoolingConsumers::OnKillFocusOtherVolume()
{
	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pclSelectPMList 
			|| NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERT_RETURN;
	}

	CPMInputUser *pclPMInputUser = m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser();
	
	double previousBuffer = pclPMInputUser->GetColdConsumersList()->GetBufferContent();

	CString strBuffer;
	double dBuffer = 0.0;
	m_ExtEditOtherVolume.GetWindowTextW( strBuffer );

	// If the buffer content is invalid put 0.0.
	if( RD_NOT_NUMBER == ReadDoubleFromStr( strBuffer.GetString(), &dBuffer ) || dBuffer < 0.0 )
	{
		m_bBufferValid = false;
		pclPMInputUser->GetColdConsumersList()->SetBufferContent( 0.0 );
	}
	else
	{
		m_bBufferValid = true;
		pclPMInputUser->GetColdConsumersList()->SetBufferContent( CDimValue::CUtoSI( _U_VOLUME, dBuffer ) );
	}

	// If the buffer is changed warn the user on cancel.
	if( previousBuffer != pclPMInputUser->GetColdConsumersList()->GetBufferContent() )
	{
		m_bChangesDone = true;
	}

	_UpdateStaticValue();
}

void CDlgWizardPM_RightViewInput_CoolingConsumers::OnBnClickedReset()
{
	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pclSelectPMList 
			|| NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERT_RETURN;
	}

	CPMInputUser *pclPMInputUser = m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser();

	if( 0 != pclPMInputUser->GetColdConsumersList()->GetConsumersCount() 
			|| 0 != pclPMInputUser->GetPipeList()->GetPipeCount() )
	{
		CString str = TASApp.LoadLocalizedString( IDS_DLGWIZPM_RVIEWCOOLCONSUMERS_RESET );

		if( IDYES == ::AfxMessageBox( str, MB_YESNO | MB_ICONQUESTION ) )
		{
			pclPMInputUser->GetColdConsumersList()->Reset();
			pclPMInputUser->GetPipeList()->Reset();
			_UpdateStaticValue();

			SS_CELLCOORD cellCoord;
			cellCoord.Col = 1;
			cellCoord.Row = RD_ColdConsumer_FirstAvailRow;

			while( m_iConsumerCount )
			{
				_RemoveColdConsumer( &cellCoord );
			}

			m_ExtEditOtherVolume.SetWindowTextW( WriteCUDouble( _U_VOLUME, 0.0 ) );

			m_bChangesDone = false;
			_ButtonPlusConsumerEnabled();
		}
	}
}

void CDlgWizardPM_RightViewInput_CoolingConsumers::OnBnClickedResetAll()
{
	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pclSelectPMList 
			|| NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERT_RETURN;
	}

	CPMInputUser *pclPMInputUser = m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser();

	if( 0 != pclPMInputUser->GetColdGeneratorList()->GetColdGeneratorCount() 
			|| 0 != pclPMInputUser->GetColdConsumersList()->GetConsumersCount() 
			|| 0 != pclPMInputUser->GetPipeList()->GetPipeCount() )
	{
		CString str = TASApp.LoadLocalizedString( IDS_DLGWIZPM_RVIEWCOLDGENERATORCONSUMERS_RESETALL );

		if( IDYES == ::AfxMessageBox( str, MB_YESNO | MB_ICONQUESTION ) )
		{
			pclPMInputUser->GetColdGeneratorList()->Reset();
			pclPMInputUser->GetColdConsumersList()->Reset();
			pclPMInputUser->GetPipeList()->Reset();
			_UpdateStaticValue();

			SS_CELLCOORD cellCoord;
			cellCoord.Col = 1;
			cellCoord.Row = RD_ColdConsumer_FirstAvailRow;

			while( m_iConsumerCount )
			{
				_RemoveColdConsumer( &cellCoord );
			}

			m_ExtEditOtherVolume.SetWindowTextW( WriteCUDouble( _U_VOLUME, 0.0 ) );

			m_bChangesDone = false;
			_ButtonPlusConsumerEnabled();
		}
	}
}

void CDlgWizardPM_RightViewInput_CoolingConsumers::OnBnClickedCancel()
{
	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pclSelectPMList 
			|| NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERT_RETURN;
	}

	CPMInputUser *pclPMInputUser = m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser();

	if( false == m_bGoToNormalModeMsgSent )
	{
		if( true == m_bChangesDone )
		{
			CString str = TASApp.LoadLocalizedString( AFXMSG_INDSELPMSYSVOLHEATING_ABORT );

			if( IDNO == ::AfxMessageBox( str, MB_YESNO | MB_ICONQUESTION ) )
			{
				return;
			}

			// Restore the copy.
			// Remark: these values have been saved by 'CDlgWizardPM_RightViewInput_CoolingSystemVolumePower' before switching
			//         to this advance mode.
			pclPMInputUser->GetColdGeneratorList()->CopyFrom( pclPMInputUser->GetpColdGeneratorListBackup() );
			pclPMInputUser->GetColdConsumersList()->CopyFrom( pclPMInputUser->GetpColdConsumersListBackup() );
			pclPMInputUser->GetPipeList()->CopyFrom( pclPMInputUser->GetpPipeListBackup() );

			m_bChangesDone = false;
		}

		m_bGoToNormalModeMsgSent = true;
		m_pclParent->PostMessage( WM_USER_RIGHTVIEWWIZPM_SWITCHMODE, WPARAM( CoolingSystemVolumePower ), 0 );
	}
}

LRESULT CDlgWizardPM_RightViewInput_CoolingConsumers::OnComboDropCloseUp( WPARAM wParam, LPARAM lParam )
{
	_UpdateColdConsumer();
	return 0;
}

LRESULT CDlgWizardPM_RightViewInput_CoolingConsumers::OnSSClick( WPARAM wParam, LPARAM lParam )
{
	SS_CELLCOORD *pclCellCoord = ( SS_CELLCOORD * )lParam;
	int iRecordIndex = pclCellCoord->Row - RD_ColdConsumer_FirstAvailRow;

	CSheetDescription *pclSDesc = m_ViewDescription.GetFromSSheetID( UINT( wParam ) );
	CSSheet *pclSSheet = pclSDesc->GetSSheetPointer();

	// Intercept click on the first column and below first row only.
	if( ( pclCellCoord->Col != CD_ColdConsumer_FirstColumn && pclCellCoord->Col != CD_ColdConsumer_ManualContent
			&& pclCellCoord->Col != CD_ColdConsumer_ManualTemp )
		|| pclCellCoord->Row < RD_ColdConsumer_FirstAvailRow )
	{
		return 0;
	}

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
		return 1; // event processed
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
		return 1;
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
		return 1;
	}

	return 0;
}

LRESULT CDlgWizardPM_RightViewInput_CoolingConsumers::OnSSDblClick( WPARAM wParam, LPARAM lParam )
{
	SS_CELLCOORD *pclCellCoord = ( SS_CELLCOORD * )lParam;

	// Avoid weird behavior on checkbox.
	if( pclCellCoord->Col == CD_ColdConsumer_ManualContent || pclCellCoord->Col == CD_ColdConsumer_ManualTemp )
	{
		return 1;
	}

	return 0;
}

LRESULT CDlgWizardPM_RightViewInput_CoolingConsumers::OnSSEditChange( WPARAM wParam, LPARAM lParam )
{
	CSheetDescription *pclSDesc = m_ViewDescription.GetFromSSheetID( UINT( wParam ) );
	CSSheet *pclSSheet = pclSDesc->GetSSheetPointer();

	SS_CELLCOORD *pclCellCoord = ( SS_CELLCOORD * )lParam;

	// Intercept below first row only.
	if( pclCellCoord->Row < RD_ColdConsumer_FirstAvailRow )
	{
		return 0;
	}

	CString strValue = pclSSheet->GetCellText( pclCellCoord->Col, pclCellCoord->Row );
	double dValue;

	// HYS-1307: Enable to use "-" sign for temperature
	bool bIsForTemperature = false;
	if( CD_ColdConsumer_SupplyTemp == pclCellCoord->Col ||
		CD_ColdConsumer_ReturnTemp == pclCellCoord->Col )
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

	if( CD_ColdConsumer_Quantity == pclCellCoord->Col )
	{
		dValue = static_cast<unsigned long>( dValue );
		CString strLong;
		strLong.Format( _T("%d"), static_cast<unsigned long>( dValue ) );

		if( 0 != strLong.CompareNoCase( strValue ) )
		{
			pclSSheet->SetCellText( pclCellCoord->Col, pclCellCoord->Row, strLong );
		}
	}

	_UpdateColdConsumer();
	_ButtonPlusConsumerEnabled();

	return 0;
}

LRESULT CDlgWizardPM_RightViewInput_CoolingConsumers::OnTextTipFetch( WPARAM wParam, LPARAM lParam )
{
	SS_TEXTTIPFETCH *pTextTipFetch = (SS_TEXTTIPFETCH *)lParam;

	CString str;
	LRESULT ReturnValue = 0;

	WORD borderType = SS_BORDERTYPE_LEFT | SS_BORDERTYPE_TOP | SS_BORDERTYPE_RIGHT | SS_BORDERTYPE_BOTTOM;
	WORD borderStyle = SS_BORDERSTYLE_SOLID;
	COLORREF borderColor = _TAH_RED_LIGHT;
	WORD border;
	WORD style;
	COLORREF color;

	CPMInputUser *pclPMInputUser = m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser();
	std::wstring strSupplyTemp = m_pclSSheetConsumer->GetCellText( CD_ColdConsumer_SupplyTemp, pTextTipFetch->Row );
	double dSupplyTemp = 0.0;
	std::wstring strReturnTemp = m_pclSSheetConsumer->GetCellText( CD_ColdConsumer_ReturnTemp, pTextTipFetch->Row );
	double dReturnTemp = 0.0;
	std::wstring strRoomTemp = m_pclSSheetConsumer->GetCellText( CD_ColdConsumer_Troom, pTextTipFetch->Row );
	double dRoomTemp = 0.0;
	double dMaxInputAirTemp = pclPMInputUser->GetColdConsumersList()->GetMaxAirInputTempForAHU();
	CString strConsumerID;

	switch( pTextTipFetch->Col )
	{
		// 
		case CD_ColdConsumer_SupplyTemp:

			if( TRUE == m_pclSSheetConsumer->GetBorder( pTextTipFetch->Col, pTextTipFetch->Row, &border, &style, &color )
					&& ( border == borderType && style == borderStyle && color == _TAH_RED_LIGHT ) )
			{
				_FindConsumerID( m_pclSSheetConsumer, pTextTipFetch->Row, strConsumerID );

				// HYS-1612: pay attention that all temperatures read from string are in custom unit!
				ReadDoubleReturn_enum eSupplyReadStatus = ReadCUDoubleFromStr( _U_TEMPERATURE, strSupplyTemp, &dSupplyTemp );
				ReadDoubleReturn_enum eReturnReadStatus = ReadCUDoubleFromStr( _U_TEMPERATURE, strReturnTemp, &dReturnTemp );
				ReadDoubleReturn_enum eRoomReadStatus = ReadCUDoubleFromStr( _U_TEMPERATURE, strRoomTemp, &dRoomTemp );

				if( RD_OK == eSupplyReadStatus )
				{
					if( dSupplyTemp < pclPMInputUser->GetSupplyTemperature() )
					{
						str = TASApp.LoadLocalizedString( IDS_INDSELPMSYSVOL_TSTUTSSYSL_TT );
					}
					else if( RD_OK == eReturnReadStatus && dSupplyTemp >= dReturnTemp )
					{
						str = TASApp.LoadLocalizedString( IDS_INDSELPMSYSVOL_TSTRL_TT );
					}
					else if( RD_OK == eRoomReadStatus && 0 != strConsumerID.Compare( _T( "PM_C_SURFACECOOLING" ) ) 
							&& dSupplyTemp >= dRoomTemp )
					{
						str = TASApp.LoadLocalizedString( IDS_INDSELPMSYSVOL_TSTROOML_TT );
					}
					else if( 0 == strConsumerID.Compare( _T( "PM_C_FANCOIL" ) ) )
					{
						if( RD_OK == eRoomReadStatus && dSupplyTemp >= dRoomTemp - 1 )
						{
							str = TASApp.LoadLocalizedString( IDS_INDSELPMSYSVOL_TSFCTROOMFCL_TT );
						}
						else if( RD_OK == eReturnReadStatus && dSupplyTemp >= dReturnTemp - 1 )
						{
							str = TASApp.LoadLocalizedString( IDS_INDSELPMSYSVOL_TSFCTRFCL_TT );
						}
					}
					else if( 0 == strConsumerID.Compare( _T( "PM_C_AIRHANDLERUNIT" ) )
							&& dSupplyTemp >= dMaxInputAirTemp )
					{
						str = TASApp.LoadLocalizedString( IDS_INDSELPMSYSVOL_TSAHUTAL_TT );
					}
				}
			}
			break;

		case CD_ColdConsumer_ReturnTemp:
			if( TRUE == m_pclSSheetConsumer->GetBorder( pTextTipFetch->Col, pTextTipFetch->Row, &border, &style, &color )
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
			break;

		case CD_ColdConsumer_Troom:
			if( TRUE == m_pclSSheetConsumer->GetBorder( pTextTipFetch->Col, pTextTipFetch->Row, &border, &style, &color )
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

void CDlgWizardPM_RightViewInput_CoolingConsumers::OnEnChangeMaxAirInputTempToAHU()
{
	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pclSelectPMList || NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()
		|| NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetColdConsumersList() )
	{
		ASSERT_RETURN;
	}

	CColdConsumersList *pclColdConsumersList = m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetColdConsumersList();

	CString strBuffer;
	double dBuffer = 0.0;
	m_clEditMaxAirInputTempForAHU.GetWindowTextW( strBuffer );

	if( RD_OK == ReadDoubleFromStr( strBuffer.GetString(), &dBuffer ) || dBuffer < 0.0 )
	{
		pclColdConsumersList->SetMaxAirInputTempForAHU( m_clEditMaxAirInputTempForAHU.GetCurrentValSI() );
		_UpdateColdConsumer();
	}
}

LRESULT CDlgWizardPM_RightViewInput_CoolingConsumers::OnUnitChange( WPARAM wParam, LPARAM lParam )
{
	CDlgWizardPM_RightViewInput_Base::OnUnitChange( wParam, lParam );

	TCHAR name[_MAXCHARS];
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();

	// Update units for the columns in the sheet.
	m_pclSSheetConsumer->SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );
	m_pclSSheetConsumer->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontSize, ( LPARAM )8 );
	m_pclSSheetConsumer->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, ( LPARAM )_TAH_TITLE_MAIN );
	m_pclSSheetConsumer->SetStaticText( CD_ColdConsumer_Capacity, RD_ColdConsumer_UnitName, GetNameOf( pUnitDB->GetDefaultUnit( _U_TH_POWER ) ).c_str() );
	m_pclSSheetConsumer->SetStaticText( CD_ColdConsumer_Surface, RD_ColdConsumer_UnitName, GetNameOf( pUnitDB->GetDefaultUnit( _U_AREA ) ).c_str() );
	m_pclSSheetConsumer->SetStaticText( CD_ColdConsumer_Content, RD_ColdConsumer_UnitName, GetNameOf( pUnitDB->GetDefaultUnit( _U_VOLUME ) ).c_str() );
	m_pclSSheetConsumer->SetStaticText( CD_ColdConsumer_SupplyTemp, RD_ColdConsumer_UnitName, GetNameOf( pUnitDB->GetDefaultUnit( _U_TEMPERATURE ) ).c_str() );
	m_pclSSheetConsumer->SetStaticText( CD_ColdConsumer_ReturnTemp, RD_ColdConsumer_UnitName, GetNameOf( pUnitDB->GetDefaultUnit( _U_TEMPERATURE ) ).c_str() );
	m_pclSSheetConsumer->SetStaticText( CD_ColdConsumer_Troom, RD_ColdConsumer_UnitName, GetNameOf( pUnitDB->GetDefaultUnit( _U_TEMPERATURE ) ).c_str() );

	m_pclSSheetConsumer->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );

	// Update all value in the sheet.
	_UpdateColdConsumerValues();

	// Update units in the statics.
	_UpdateStaticValue();

	// Target linear Dp: xx Pa/m
	double dPipeTargetDp = m_pclTechParam->GetPipeTargDp();
	CString strTargetLinearDp = WriteCUDouble( _U_LINPRESSDROP, dPipeTargetDp );
	GetNameOf( pUnitDB->GetDefaultUnit( _U_LINPRESSDROP ), name );
	CString str;
	FormatString( str, IDS_DLGWIZPM_RVIEWCOOLCONSUMERS_TARGETLINEARDP, strTargetLinearDp, name );
	m_StaticTargetLinearDp.SetWindowTextW( str );
	m_StaticTargetLinearDp.SetToolTip( TASApp.LoadLocalizedString( IDS_INDSELPMSYSVOLCOOLING_TARGETLINEARDPTT ) );

	return 0;
}

LRESULT CDlgWizardPM_RightViewInput_CoolingConsumers::OnTechParChange( WPARAM wParam, LPARAM lParam )
{
	TCHAR name[_MAXCHARS];
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();

	// Target linear Dp: xx Pa/m
	double dPipeTargetDp = m_pclTechParam->GetPipeTargDp();
	CString strTargetLinearDp = WriteCUDouble( _U_LINPRESSDROP, dPipeTargetDp );
	GetNameOf( pUnitDB->GetDefaultUnit( _U_LINPRESSDROP ), name );
	CString str;
	FormatString( str, IDS_DLGWIZPM_RVIEWCOOLCONSUMERS_TARGETLINEARDP, strTargetLinearDp, name );
	m_StaticTargetLinearDp.SetWindowTextW( str );
	m_StaticTargetLinearDp.SetToolTip( TASApp.LoadLocalizedString( IDS_INDSELPMSYSVOLCOOLING_TARGETLINEARDPTT ) );

	return 0;
}

void CDlgWizardPM_RightViewInput_CoolingConsumers::_InitializeSSheet()
{
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();

	// Pay attention: because in this case I use TSpread directly as a custom control (see IDD_DLGPIPEDP in the resource), if you want to
	// use 'CSSheet' possibility, we have to attach TSpread CWnd to CSSheet newly created. And the destruction of the dialog, don't forget
	// to call 'Detach' before deleting 'CSSheet'.
	m_pclSSheetConsumer = new CSSheet();
	m_pclSSheetConsumer->Attach( GetDlgItem( IDC_SSCOLDCONSLIST )->GetSafeHwnd() );
	m_pclSSheetConsumer->Reset();

	// Save initial size.
	m_pclSSheetConsumer->GetClientRect( &m_rectSheetInitialSize );
	m_pclSDescConsumer = m_ViewDescription.AddSheetDescription( SD_ColdConsumer, -1, m_pclSSheetConsumer, CPoint( 0, 0 ) );

	if( NULL == m_pclSDescConsumer )
	{
		return;
	}

	m_pclSSheetConsumer->SetBool( SSB_ALLOWDRAGDROP, FALSE );
	m_pclSSheetConsumer->SetBool( SSB_HORZSCROLLBAR, FALSE );
	m_pclSSheetConsumer->SetBool( SSB_VERTSCROLLBAR, TRUE );
	m_pclSSheetConsumer->SetBool( SSB_MOVEACTIVEONFOCUS, FALSE );
	m_pclSSheetConsumer->SetBool( SSB_EDITMODEPERMANENT, TRUE );
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
	m_pclSSheetConsumer->SetPictureCellWithID( IDI_PLUSGREEN, CD_ColdConsumer_FirstColumn, RD_ColdConsumer_FirstAvailRow, CSSheet::PictureCellType::Icon );

	m_pclSSheetConsumer->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
}

void CDlgWizardPM_RightViewInput_CoolingConsumers::_UpdateStaticValue()
{
	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pclSelectPMList 
			|| NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERT_RETURN;
	}

	CPMInputUser *pclPMInputUser = m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser();

	double dTotalConsColdCapacity = pclPMInputUser->GetColdConsumersList()->GetTotalColdCapacity();
	m_StaticTotalConsumerColdCapacity.SetWindowText( WriteCUDouble( _U_TH_POWER, dTotalConsColdCapacity, true ) );
	
	double dTotalConsContent = pclPMInputUser->GetColdConsumersList()->GetTotalContent( pclPMInputUser->GetSupplyTemperature(), 
			pclPMInputUser->GetReturnTemperature() ) - pclPMInputUser->GetColdConsumersList()->GetBufferContent();
	
	m_StaticTotalConsumerContent.SetWindowText( WriteCUDouble( _U_VOLUME, dTotalConsContent, true ) );

	// HYS-869
	if( dTotalConsColdCapacity > pclPMInputUser->GetColdGeneratorList()->GetTotalColdCapacity() )
	{
		m_StaticTotalConsumerColdCapacity.SetTextColor( _ORANGE );
		m_StaticTotalConsumerColdCapacity.SetToolTip( TASApp.LoadLocalizedString( IDS_INDSELPMSYSVOL_TOTALCAPACITY_TT ) );
	}
	else
	{
		m_StaticTotalConsumerColdCapacity.ResetToolTip();
		m_StaticTotalConsumerColdCapacity.SetTextColor( _BLACK );
	}

	double dOtherVolume = pclPMInputUser->GetColdConsumersList()->GetBufferContent();
	m_ExtEditOtherVolume.SetCurrentValSI( dOtherVolume );
	m_ExtEditOtherVolume.Update();

	double dPipeContent = pclPMInputUser->GetPipeList()->GetTotalContent();
	m_ExtEditAdditionalPipe.SetCurrentValSI( dPipeContent );
	m_ExtEditAdditionalPipe.Update();

	double dTotalContent = pclPMInputUser->GetColdGeneratorList()->GetTotalContent();
	
	dTotalContent += pclPMInputUser->GetColdConsumersList()->GetTotalContent( pclPMInputUser->GetSupplyTemperature(), 
			pclPMInputUser->GetReturnTemperature() );
	
	dTotalContent += dPipeContent;
	m_StaticTotalContent.SetWindowText( WriteCUDouble( _U_VOLUME, dTotalContent, true ) );

	_VerifyResetButtons();
}

void CDlgWizardPM_RightViewInput_CoolingConsumers::_RemoveColdConsumer( SS_CELLCOORD * plcCellCoord )
{
	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pclSelectPMList 
			|| NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERT_RETURN;
	}

	CPMInputUser *pclPMInputUser = m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser();

	m_pclSSheetConsumer->SetRedraw( FALSE );
	m_iConsumerCount--;
	m_pclSSheetConsumer->DelRow( plcCellCoord->Row ); // drop a row
	m_pclSSheetConsumer->SetMaxRows( RD_ColdConsumer_FirstAvailRow + m_iConsumerCount ); // Set max rows.
	pclPMInputUser->GetColdConsumersList()->RemoveConsumers( plcCellCoord->Row - RD_ColdConsumer_FirstAvailRow );
	m_bChangesDone = true;
	
	_CheckColdConsumerColumnWidth();
	m_pclSSheetConsumer->SetRedraw( TRUE );

	// Manually call 'OnSize' method to well move all the components.
	CRect rectClient;
	GetClientRect( &rectClient );
	OnSize( SIZE_RESTORED, rectClient.Width(), rectClient.Height() );

	// To force a paint refresh.
	Invalidate();
	UpdateWindow();
}

void CDlgWizardPM_RightViewInput_CoolingConsumers::_FillConsumersCombo( CSSheet* pclSheet, long lCol, long lRow, const TCHAR* strSelect )
{
	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pTADB )
	{
		ASSERT_RETURN;
	}

	if( NULL == pclSheet || lCol < 1 || lCol > pclSheet->GetMaxCols() || lRow < 1 || lRow > pclSheet->GetMaxRows() )
	{
		return;
	}

	if( m_mapConsumerType.empty() )
	{
		CTable *pclTable = dynamic_cast<CTable *>( m_pclWizardSelPMParams->m_pTADB->Get( _T("PMCOLDCONSUMERS_TAB") ).MP );

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

	if( m_mapConsumerType.empty() )
	{
		return;
	}

	pclSheet->ComboBoxSendMessage( lCol, lRow, SS_CBM_RESETCONTENT, 0, 0 );
	int iCount = 0;
	int iSelectIndex = 0;

	for( std::map<int, CDB_StringID *>::iterator mapIter = m_mapConsumerType.begin(); mapIter != m_mapConsumerType.end(); ++mapIter )
	{
		pclSheet->ComboBoxSendMessage( lCol, lRow, SS_CBM_ADDSTRING, 0, ( LPARAM )( mapIter->second->GetString() ) );

		if( _tcscmp( strSelect, mapIter->second->GetIDPtr().ID ) == 0 )
		{
			iSelectIndex = iCount;
		}

		iCount++;
	}

	pclSheet->ComboBoxSendMessage( lCol, lRow, SS_CBM_SETCURSEL, iSelectIndex, 0 );
}

void CDlgWizardPM_RightViewInput_CoolingConsumers::_AddColdConsumer( CSSheet *pclSSheet, CString pstrType, double dCapacity, double dContent,
		bool bManualContent, double dSupplyTemp, double dReturnTemp, bool bManualTemp, bool bOnlyGUI, double dRoomTemp, long lQuantity, 
		double dSurface )
{
	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pclSelectPMList 
			|| NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERT_RETURN;
	}

	CPMInputUser *pclPMInputUser = m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser();

	m_iConsumerCount++;
	int iCurrentRow = RD_ColdConsumer_FirstAvailRow + m_iConsumerCount - 1;

	// Set max rows.
	pclSSheet->SetRedraw( FALSE );
	pclSSheet->SetMaxRows( RD_ColdConsumer_FirstAvailRow + m_iConsumerCount );

	// Add plus icon.
	pclSSheet->SetPictureCellWithID( IDI_PLUSGREEN, CD_ColdConsumer_FirstColumn, iCurrentRow + 1, CSSheet::PictureCellType::Icon );

	// Add checkbox.
	pclSSheet->SetCheckBox( CD_ColdConsumer_ManualContent, iCurrentRow, _T( "" ), false, true );
	pclSSheet->SetCheckBox( CD_ColdConsumer_ManualTemp, iCurrentRow, _T( "" ), false, true );

	// Add trash icon.
	pclSSheet->SetPictureCellWithID( IDI_TRASH, CD_ColdConsumer_FirstColumn, iCurrentRow, CSSheet::PictureCellType::Icon );

	// Add consumer combo.
	pclSSheet->FormatComboList( CD_ColdConsumer_Consumer, iCurrentRow );
	_FillConsumersCombo( pclSSheet, CD_ColdConsumer_Consumer, iCurrentRow, pstrType );

	//HYS-869
	if( ( false == bOnlyGUI ) && ( pclPMInputUser->GetColdGeneratorList()->GetTotalColdCapacity() > pclPMInputUser->GetColdConsumersList()->GetTotalColdCapacity() ) )
	{
		dCapacity = CDimValue::SItoCU( _U_TH_POWER, ( pclPMInputUser->GetColdGeneratorList()->GetTotalColdCapacity() - pclPMInputUser->GetColdConsumersList()->GetTotalColdCapacity() ) );
	}

	// Add capacity edit.
	pclSSheet->FormatEditDouble( CD_ColdConsumer_Capacity, iCurrentRow, _T( "" ), SSS_ALIGN_RIGHT );
	
	CString strCapacity;
	strCapacity.Format( _T("%g"), dCapacity );
	pclSSheet->SetCellText( CD_ColdConsumer_Capacity, iCurrentRow, strCapacity );
	
	SS_CELLTYPE rCellType;
	pclSSheet->SetTypeEdit( &rCellType, SSS_ALIGN_RIGHT, 15, SS_CHRSET_ALPHANUM, SS_CASE_NOCASE );
	pclSSheet->SetCellType( CD_ColdConsumer_Capacity, iCurrentRow, &rCellType );

	// Add Troom edit.
	pclSSheet->SetCellType( CD_ColdConsumer_Troom, iCurrentRow, &rCellType );
	CString strTRoom;
	strTRoom.Format( _T("%g"), dRoomTemp ? dRoomTemp : 20 );

	// Add Quantity.
	pclSSheet->SetCellType( CD_ColdConsumer_Quantity, iCurrentRow, &rCellType );
	CString strQuantity;
	strQuantity.Format( _T("%d"), lQuantity );
	pclSSheet->FormatEditDouble( CD_ColdConsumer_Quantity, iCurrentRow, strQuantity, SSS_ALIGN_RIGHT );

	// Add surface.
	pclSSheet->SetCellType( CD_ColdConsumer_Surface, iCurrentRow, &rCellType );
	CString strSurface;
	strSurface.Format( _T("%g"), dSurface );

	CString strConsumerID;
	_FindConsumerID( pclSSheet, iCurrentRow, strConsumerID );

	if( 0 == strConsumerID.Compare( _T("PM_C_SURFACECOOLING") ) )
	{
		pclSSheet->SetBackColor( CD_ColdConsumer_Troom, iCurrentRow, _LIGHTGRAY );
		pclSSheet->FormatEditDouble( CD_ColdConsumer_Surface, iCurrentRow, strSurface, SSS_ALIGN_RIGHT );
	}
	else if( 0 == strConsumerID.Compare( _T("PM_C_FANCOIL") ) )
	{
		pclSSheet->FormatEditDouble( CD_ColdConsumer_Troom, iCurrentRow, strTRoom, SSS_ALIGN_RIGHT );
		pclSSheet->SetBackColor( CD_ColdConsumer_Surface, iCurrentRow, _LIGHTGRAY );
	}
	else
	{
		pclSSheet->FormatEditDouble( CD_ColdConsumer_Troom, iCurrentRow, strTRoom, SSS_ALIGN_RIGHT );
		pclSSheet->SetBackColor( CD_ColdConsumer_Surface, iCurrentRow, _LIGHTGRAY );
	}

	if( false == bOnlyGUI ) // Used to load data at dialog opening
	{
		pclPMInputUser->GetColdConsumersList()->AddConsumers( 
				strConsumerID, 
				dCapacity, 
				dContent, 
				pclPMInputUser->GetSupplyTemperature(), 
				pclPMInputUser->GetReturnTemperature(), 
				bManualTemp,
				dRoomTemp,
				lQuantity,
				dSurface );

		m_bChangesDone = true;
	}

	CString strContent;
	strContent.Format( _T("%g"), dContent );
	CString strSupplyTemp;
	strSupplyTemp.Format( _T("%g"), ( bManualTemp ? dSupplyTemp : pclPMInputUser->GetSupplyTemperature() ) );
	CString strReturnTemp;
	strReturnTemp.Format( _T("%g"), ( bManualTemp ? dReturnTemp : pclPMInputUser->GetReturnTemperature() ) );

	if( true == bManualContent ) // Manual input
	{
		pclSSheet->SetCellText( CD_ColdConsumer_ManualContent, iCurrentRow, _T("1") );
		pclSSheet->FormatEditDouble( CD_ColdConsumer_Content, iCurrentRow, strContent, SSS_ALIGN_RIGHT );
		pclSSheet->SetCellType( CD_ColdConsumer_Content, iCurrentRow, &rCellType );
	}
	else
	{
		pclSSheet->FormatStaticText( CD_ColdConsumer_ManualContent, iCurrentRow, CD_ColdConsumer_Content, iCurrentRow, strContent, SSS_ALIGN_RIGHT );
	}

	if( bManualTemp ) // Manual input
	{
		pclSSheet->SetCellText( CD_ColdConsumer_ManualTemp, iCurrentRow, _T("1") );
		pclSSheet->FormatEditDouble( CD_ColdConsumer_SupplyTemp, iCurrentRow, strSupplyTemp, SSS_ALIGN_RIGHT );
		pclSSheet->FormatEditDouble( CD_ColdConsumer_ReturnTemp, iCurrentRow, strReturnTemp, SSS_ALIGN_RIGHT );
		pclSSheet->SetCellType( CD_ColdConsumer_SupplyTemp, iCurrentRow, &rCellType );
		pclSSheet->SetCellType( CD_ColdConsumer_ReturnTemp, iCurrentRow, &rCellType );
	}
	else
	{
		pclSSheet->FormatStaticText( CD_ColdConsumer_SupplyTemp, iCurrentRow, CD_ColdConsumer_SupplyTemp, iCurrentRow, strSupplyTemp, SSS_ALIGN_RIGHT );
		pclSSheet->FormatStaticText( CD_ColdConsumer_ReturnTemp, iCurrentRow, CD_ColdConsumer_ReturnTemp, iCurrentRow, strReturnTemp, SSS_ALIGN_RIGHT );
	}

	_CheckColdConsumerColumnWidth();
	pclSSheet->SetRedraw( TRUE );

	if( true == m_bInitialized )
	{
		// Manually call 'OnSize' method to well move all the components.
		CRect rectClient;
		GetClientRect( &rectClient );
		OnSize( SIZE_RESTORED, rectClient.Width(), rectClient.Height() );

		// To force a paint refresh.
		Invalidate();
		UpdateWindow();
	}
}

void CDlgWizardPM_RightViewInput_CoolingConsumers::_FindConsumerID( CSSheet * plcSSheet, int row, CString &strID )
{
	strID = _T( "" );
	TCHAR strComboValue[256];
	int iCurrentSelection = plcSSheet->ComboBoxSendMessage( CD_ColdConsumer_Consumer, row, SS_CBM_GETCURSEL, 0, 0 );
	plcSSheet->ComboBoxSendMessage( CD_ColdConsumer_Consumer, row, SS_CBM_GETLBTEXT, iCurrentSelection, ( LPARAM )strComboValue );

	for( std::map<int, CDB_StringID *>::iterator mapIter = m_mapConsumerType.begin(); mapIter != m_mapConsumerType.end(); ++mapIter )
	{
		if( CString( mapIter->second->GetString() ).CompareNoCase( strComboValue ) == 0 )
		{
			strID = mapIter->second->GetIDPtr().ID;
			return;
		}
	}
}

void CDlgWizardPM_RightViewInput_CoolingConsumers::_LoadColdConsumer()
{
	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pclSelectPMList 
			|| NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERT_RETURN;
	}

	CPMInputUser *pclPMInputUser = m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser();

	for( int i = 0; i < pclPMInputUser->GetColdConsumersList()->GetConsumersCount(); i++ )
	{
		CColdConsumersList::ConsumersSave *pColdConsumer = pclPMInputUser->GetColdConsumersList()->GetConsumer( i );

		_AddColdConsumer( 
				m_pclSSheetConsumer,
				pColdConsumer->m_strConsumersTypeID,
				CDimValue::SItoCU( _U_TH_POWER, pColdConsumer->m_dColdCapacity ),
				CDimValue::SItoCU( _U_VOLUME, pColdConsumer->m_dContent ),
				pColdConsumer->m_bManualContent,
				CDimValue::SItoCU( _U_TEMPERATURE, pColdConsumer->m_dSupplyTemp ),
				CDimValue::SItoCU( _U_TEMPERATURE, pColdConsumer->m_dReturnTemp ),
				pColdConsumer->m_bManualTemp,
				true,
				CDimValue::SItoCU( _U_TEMPERATURE, pColdConsumer->m_dRoomTemp ),
				pColdConsumer->m_lQuantity,
				CDimValue::SItoCU( _U_AREA, pColdConsumer->m_dSurface ) );
	}

	_UpdateColdConsumer();
}

void CDlgWizardPM_RightViewInput_CoolingConsumers::_UpdateColdConsumer()
{
	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pclSelectPMList 
			|| NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERT_RETURN;
	}

	CPMInputUser *pclPMInputUser = m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser();

	for( int i = 0; i < m_iConsumerCount; i++ )
	{
		int iRecordIndex = i;

		// Get consumer ID.
		CString strConsumerID;
		_FindConsumerID( m_pclSSheetConsumer, i + RD_ColdConsumer_FirstAvailRow, strConsumerID );

		// Get capacity in SI.
		TCHAR strCapacity[16];
		double dCapacity;
		m_pclSSheetConsumer->GetValue( CD_ColdConsumer_Capacity, i + RD_ColdConsumer_FirstAvailRow, strCapacity );
		ReadCUDoubleFromStr( _U_TH_POWER, strCapacity, &dCapacity );

		// Get RoomTemp in SI.
		TCHAR strRoomTemp[16];
		double dRoomTemp;
		m_pclSSheetConsumer->GetValue( CD_ColdConsumer_Troom, i + RD_ColdConsumer_FirstAvailRow, strRoomTemp );
		ReadCUDoubleFromStr( _U_TEMPERATURE, strRoomTemp, &dRoomTemp );

		// Get Quantity.
		TCHAR strQuantity[16];
		long lQuantity;
		m_pclSSheetConsumer->GetValue( CD_ColdConsumer_Quantity, i + RD_ColdConsumer_FirstAvailRow, strQuantity );
		ReadLongFromStr( strQuantity, &lQuantity );
		//m_pclSSheetConsumer->FormatEditDouble( CD_ColdConsumer_Quantity, i + RD_ColdConsumer_FirstAvailRow, strQuantity, SSS_ALIGN_RIGHT );

		// Get Surface in SI.
		TCHAR strSurface[16];
		double dSurface;
		m_pclSSheetConsumer->GetValue( CD_ColdConsumer_Surface, i + RD_ColdConsumer_FirstAvailRow, strSurface );
		ReadCUDoubleFromStr( _U_AREA, strSurface, &dSurface );

		double dContent = -1.0;
		double dSupplyTemp = pclPMInputUser->GetSupplyTemperature();
		double dReturnTemp = pclPMInputUser->GetReturnTemperature();
		bool bManualContent = _IsCheckBoxChecked( m_pclSSheetConsumer, CD_ColdConsumer_ManualContent, i + RD_ColdConsumer_FirstAvailRow );
		bool bManualTemp = _IsCheckBoxChecked( m_pclSSheetConsumer, CD_ColdConsumer_ManualTemp, i + RD_ColdConsumer_FirstAvailRow );

		if( true == bManualContent )
		{
			// If in manual input then get the Content in SI.
			TCHAR strContent[16];
			m_pclSSheetConsumer->GetValue( CD_ColdConsumer_Content, i + RD_ColdConsumer_FirstAvailRow, strContent );
			ReadCUDoubleFromStr( _U_VOLUME, strContent, &dContent );
			m_pclSSheetConsumer->SetBackColor( CD_ColdConsumer_Content, i + RD_ColdConsumer_FirstAvailRow, _WHITE );
		}

		if( true == bManualTemp )
		{
			// If in manual input then get the Content in SI.
			TCHAR strSupplyTemp[16];
			TCHAR strReturnTemp[16];
			m_pclSSheetConsumer->GetValue( CD_ColdConsumer_SupplyTemp, i + RD_ColdConsumer_FirstAvailRow, strSupplyTemp );
			m_pclSSheetConsumer->GetValue( CD_ColdConsumer_ReturnTemp, i + RD_ColdConsumer_FirstAvailRow, strReturnTemp );
			ReadCUDoubleFromStr( _U_TEMPERATURE, strSupplyTemp, &dSupplyTemp );
			ReadCUDoubleFromStr( _U_TEMPERATURE, strReturnTemp, &dReturnTemp );
			m_pclSSheetConsumer->SetBackColor( CD_ColdConsumer_SupplyTemp, i + RD_ColdConsumer_FirstAvailRow, _WHITE );
			m_pclSSheetConsumer->SetBackColor( CD_ColdConsumer_ReturnTemp, i + RD_ColdConsumer_FirstAvailRow, _WHITE );
		}

		int iCurrentRow = i + RD_ColdConsumer_FirstAvailRow;

		if( 0 == strConsumerID.Compare( _T("PM_C_SURFACECOOLING") ) )
		{
			// Reformat only if changed
			if( 0 != pclPMInputUser->GetColdConsumersList()->GetConsumer( iRecordIndex )->m_strConsumersTypeID.Compare( _T("PM_C_SURFACECOOLING") ) )
			{
				m_pclSSheetConsumer->FormatStaticText( CD_ColdConsumer_Troom, iCurrentRow, CD_ColdConsumer_Troom, iCurrentRow, _T( "" ), SSS_ALIGN_RIGHT );
				m_pclSSheetConsumer->SetBackColor( CD_ColdConsumer_Troom, iCurrentRow, _LIGHTGRAY );

				dSurface = pclPMInputUser->GetColdConsumersList()->GetConsumer( iRecordIndex )->m_dSurface;
				_swprintf_c( strSurface, 15, _T("%g"), dSurface );
				m_pclSSheetConsumer->FormatEditDouble( CD_ColdConsumer_Surface, iCurrentRow, strSurface, SSS_ALIGN_RIGHT );
			}
		}
		else if( 0 == strConsumerID.Compare( _T("PM_C_FANCOIL") ) )
		{
			// Reformat only if changed
			if( 0 != pclPMInputUser->GetColdConsumersList()->GetConsumer( iRecordIndex )->m_strConsumersTypeID.Compare( _T("PM_C_FANCOIL") ) )
			{
				dRoomTemp = pclPMInputUser->GetColdConsumersList()->GetConsumer(iRecordIndex)->m_dRoomTemp;
				_swprintf_c(strRoomTemp, 15, _T("%g"), dRoomTemp);
				m_pclSSheetConsumer->FormatEditDouble(CD_ColdConsumer_Troom, iCurrentRow, strRoomTemp, SSS_ALIGN_RIGHT);

				m_pclSSheetConsumer->FormatStaticText(CD_ColdConsumer_Surface, iCurrentRow, CD_ColdConsumer_Surface, iCurrentRow, _T(""), SSS_ALIGN_RIGHT);
				m_pclSSheetConsumer->SetBackColor(CD_ColdConsumer_Surface, iCurrentRow, _LIGHTGRAY);
			}
		}
		else if (0 == strConsumerID.Compare(_T("PM_C_AIRHANDLERUNIT")))
		{
			// Reformat only if changed
			if( 0 != pclPMInputUser->GetColdConsumersList()->GetConsumer( iRecordIndex )->m_strConsumersTypeID.Compare( _T("PM_C_AIRHANDLERUNIT") ) )
			{
				dRoomTemp = pclPMInputUser->GetColdConsumersList()->GetConsumer( iRecordIndex )->m_dRoomTemp;
				_swprintf_c( strRoomTemp, 15, _T("%g"), dRoomTemp );
				m_pclSSheetConsumer->FormatEditDouble( CD_ColdConsumer_Troom, iCurrentRow, strRoomTemp, SSS_ALIGN_RIGHT );

				m_pclSSheetConsumer->FormatStaticText( CD_ColdConsumer_Surface, iCurrentRow, CD_ColdConsumer_Surface, iCurrentRow, _T( "" ), SSS_ALIGN_RIGHT );
				m_pclSSheetConsumer->SetBackColor( CD_ColdConsumer_Surface, iCurrentRow, _LIGHTGRAY );
			}
		}

		// Update database.
		pclPMInputUser->GetColdConsumersList()->UpdateConsumers( 
				iRecordIndex,
				strConsumerID,
				dCapacity,
				dContent,
				dSupplyTemp,
				dReturnTemp,
				bManualTemp,
				dRoomTemp,
				lQuantity,
				dSurface );

		if( false == bManualContent )
		{
			// If not in manual input then refresh Content in spreadsheet
			dContent = pclPMInputUser->GetColdConsumersList()->GetConsumer( iRecordIndex )->m_dContent;
			
			m_pclSSheetConsumer->FormatStaticText( 
					CD_ColdConsumer_Content,
					i + RD_ColdConsumer_FirstAvailRow,
					CD_ColdConsumer_Content,
					i + RD_ColdConsumer_FirstAvailRow,
					WriteCUDouble( _U_VOLUME, dContent ),
					SSS_ALIGN_RIGHT );

			m_pclSSheetConsumer->SetBackColor( CD_ColdConsumer_Content, i + RD_ColdConsumer_FirstAvailRow, _LIGHTGRAY );
		}

		if( false == bManualTemp )
		{
			// If not in manual input then refresh Supply Temperature in spreadsheet.
			m_pclSSheetConsumer->FormatStaticText( CD_ColdConsumer_SupplyTemp,
												   i + RD_ColdConsumer_FirstAvailRow,
												   CD_ColdConsumer_SupplyTemp,
												   i + RD_ColdConsumer_FirstAvailRow,
												   WriteCUDouble( _U_TEMPERATURE, pclPMInputUser->GetSupplyTemperature() ),
												   SSS_ALIGN_RIGHT );

			m_pclSSheetConsumer->SetBackColor( CD_ColdConsumer_SupplyTemp, i + RD_ColdConsumer_FirstAvailRow, _LIGHTGRAY );

			// If not in manual input then refresh Return Temperature in spreadsheet.
			m_pclSSheetConsumer->FormatStaticText( CD_ColdConsumer_ReturnTemp,
												   i + RD_ColdConsumer_FirstAvailRow,
												   CD_ColdConsumer_ReturnTemp,
												   i + RD_ColdConsumer_FirstAvailRow,
												   WriteCUDouble( _U_TEMPERATURE, pclPMInputUser->GetReturnTemperature() ),
												   SSS_ALIGN_RIGHT );

			m_pclSSheetConsumer->SetBackColor( CD_ColdConsumer_ReturnTemp, i + RD_ColdConsumer_FirstAvailRow, _LIGHTGRAY );
		}
	}

	_UpdateStaticValue();
}

void CDlgWizardPM_RightViewInput_CoolingConsumers::_UpdateColdConsumerValues()
{
	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pclSelectPMList 
			|| NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERT_RETURN;
	}

	CPMInputUser *pclPMInputUser = m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser();

	for( int i = 0; i < m_iConsumerCount; i++ )
	{
		SS_CELLTYPE rCellType;

		// Update capacity.
		if( true == m_pclSSheetConsumer->IsCellIsStaticText( CD_ColdConsumer_Capacity, i + RD_ColdConsumer_FirstAvailRow, rCellType ) )
		{
			double dColdCapacity = pclPMInputUser->GetColdConsumersList()->GetConsumer( i )->m_dColdCapacity;
			CString str = WriteCUDouble( _U_TH_POWER, dColdCapacity );
			m_pclSSheetConsumer->SetValue( CD_ColdConsumer_Capacity, i + RD_ColdConsumer_FirstAvailRow, str );
		}

		// Update surface.
		if( true == m_pclSSheetConsumer->IsCellIsStaticText( CD_ColdConsumer_Surface, i + RD_ColdConsumer_FirstAvailRow, rCellType ) )
		{
			double dSurface = pclPMInputUser->GetColdConsumersList()->GetConsumer( i )->m_dSurface;
			CString str = WriteCUDouble( _U_AREA, dSurface );
			m_pclSSheetConsumer->SetValue( CD_ColdConsumer_Surface, i + RD_ColdConsumer_FirstAvailRow, str );
		}

		// Update content.
		if( true == m_pclSSheetConsumer->IsCellIsStaticText( CD_ColdConsumer_Content, i + RD_ColdConsumer_FirstAvailRow, rCellType ) )
		{
			double dContent = pclPMInputUser->GetColdConsumersList()->GetConsumer( i )->m_dContent;
			CString str = WriteCUDouble( _U_VOLUME, dContent );
			m_pclSSheetConsumer->SetValue( CD_ColdConsumer_Content, i + RD_ColdConsumer_FirstAvailRow, str );
		}

		// Update supply temperature.
		if( true == m_pclSSheetConsumer->IsCellIsStaticText( CD_ColdConsumer_SupplyTemp, i + RD_ColdConsumer_FirstAvailRow, rCellType ) )
		{
			double dSupplyTemperature = pclPMInputUser->GetColdConsumersList()->GetConsumer( i )->m_dSupplyTemp;
			CString str = WriteCUDouble( _U_TEMPERATURE, dSupplyTemperature );
			m_pclSSheetConsumer->SetValue( CD_ColdConsumer_SupplyTemp, i + RD_ColdConsumer_FirstAvailRow, str );
		}

		// Update return temperature.
		if( true == m_pclSSheetConsumer->IsCellIsStaticText( CD_ColdConsumer_ReturnTemp, i + RD_ColdConsumer_FirstAvailRow, rCellType ) )
		{
			double dReturnTemperature = pclPMInputUser->GetColdConsumersList()->GetConsumer( i )->m_dReturnTemp;
			CString str = WriteCUDouble( _U_TEMPERATURE, dReturnTemperature );
			m_pclSSheetConsumer->SetValue( CD_ColdConsumer_ReturnTemp, i + RD_ColdConsumer_FirstAvailRow, str );
		}

		// Update room temperature.
		if( true == m_pclSSheetConsumer->IsCellIsStaticText( CD_ColdConsumer_Troom, i + RD_ColdConsumer_FirstAvailRow, rCellType ) )
		{
			double dRoomTemperature = pclPMInputUser->GetColdConsumersList()->GetConsumer( i )->m_dRoomTemp;
			CString str = WriteCUDouble( _U_TEMPERATURE, dRoomTemperature );
			m_pclSSheetConsumer->SetValue( CD_ColdConsumer_Troom, i + RD_ColdConsumer_FirstAvailRow, str );
		}
	}
}

bool CDlgWizardPM_RightViewInput_CoolingConsumers::_ButtonPlusConsumerEnabled()
{
	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pclSelectPMList 
			|| NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERTA_RETURN( false );
	}

	CPMInputUser *pclPMInputUser = m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser();

	// avoid to add a new consumer if some input are invalid
	// not valid :
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
		m_pclSSheetConsumer->SetCellBorder( CD_ColdConsumer_Content, i, false );

		if( RD_OK != ReadCUDoubleFromStr( _U_TH_POWER, strContent, &dContent ) || dContent <= 0.0 )
		{
			if( _IsCheckBoxChecked( m_pclSSheetConsumer, CD_ColdConsumer_ManualContent, i ) )
			{
				// Red border to help the user to find errors.
				m_pclSSheetConsumer->SetCellBorder( CD_ColdConsumer_Content, i, CD_ColdConsumer_Content, i, true, border, SS_BORDERSTYLE_SOLID, _TAH_RED_LIGHT );
			}

			bEnable = false;
		}

		std::wstring strCapacity = m_pclSSheetConsumer->GetCellText( CD_ColdConsumer_Capacity, i );
		double dCapacity = 0.0;

		if( RD_OK != ReadCUDoubleFromStr( _U_VOLUME, strCapacity, &dCapacity ) || dCapacity <= 0.0 )
		{
			// Red border to help the user to find errors.
			m_pclSSheetConsumer->SetCellBorder( CD_ColdConsumer_Capacity, i, CD_ColdConsumer_Capacity, i, true, border, SS_BORDERSTYLE_SOLID, _TAH_RED_LIGHT );
			bEnable = false;
		}
		else
		{
			// Remove border if input is valid.
			m_pclSSheetConsumer->SetCellBorder( CD_ColdConsumer_Capacity, i, false );
		}

		std::wstring strSupplyTemp = m_pclSSheetConsumer->GetCellText( CD_ColdConsumer_SupplyTemp, i );
		double dSupplyTemp = 0.0;

		// HYS-1164 : Synchronize with dlg system volume
		if( RD_OK != ReadCUDoubleFromStr( _U_TEMPERATURE, strSupplyTemp, &dSupplyTemp ) 
			|| dSupplyTemp < pclPMInputUser ->GetSupplyTemperature() )
		{
			// Red border to help the user to find errors.
			m_pclSSheetConsumer->SetCellBorder( CD_ColdConsumer_SupplyTemp, i, CD_ColdConsumer_SupplyTemp, i, true, border, SS_BORDERSTYLE_SOLID, _TAH_RED_LIGHT );
			bEnable = false;
		}
		else
		{
			// Remove border if input is valid.
			m_pclSSheetConsumer->SetCellBorder( CD_ColdConsumer_SupplyTemp, i, false );
		}

		std::wstring strReturnTemp = m_pclSSheetConsumer->GetCellText( CD_ColdConsumer_ReturnTemp, i );
		double dReturnTemp = 0.0;

		// HYS-1164 : Synchronize with dlg system volume
		if( RD_OK != ReadCUDoubleFromStr( _U_TEMPERATURE, strReturnTemp, &dReturnTemp ) )
		{
			// Red border to help the user to find errors.
			m_pclSSheetConsumer->SetCellBorder( CD_ColdConsumer_ReturnTemp, i, CD_ColdConsumer_ReturnTemp, i, true, border, SS_BORDERSTYLE_SOLID, _TAH_RED_LIGHT );
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

		if( RD_OK != ReadCUDoubleFromStr( _U_TEMPERATURE, strRoomTemp, &dRoomTemp )
			&& 0 != strConsumerID.Compare( _T("PM_C_SURFACECOOLING") ) )
		{
			// red border to help the user to find errors.
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
		pclPMInputUser->GetColdConsumersList()->VerifyConsCoolingTemperatureValues( strConsumerID,
			dSupplyTemp, dReturnTemp, dRoomTemp, &bSupplyTempOK, &bReturnTempOK, &bRoomTempOK );
		if( bSupplyTempOK == false )
		{
			// Red border to help the user to find errors.
			m_pclSSheetConsumer->SetCellBorder( CD_ColdConsumer_SupplyTemp, i, CD_ColdConsumer_SupplyTemp, i, true, border, SS_BORDERSTYLE_SOLID,
				_TAH_RED_LIGHT );

			bEnable = false;
		}
		if( bReturnTempOK == false )
		{
			// Red border to help the user to find errors.
			m_pclSSheetConsumer->SetCellBorder( CD_ColdConsumer_ReturnTemp, i, CD_ColdConsumer_ReturnTemp, i, true, border,
				SS_BORDERSTYLE_SOLID, _TAH_RED_LIGHT );

			bEnable = false;
		}
		if( bRoomTempOK == false )
		{
			// Red border to help the user to find errors.
			m_pclSSheetConsumer->SetCellBorder( CD_ColdConsumer_Troom, i, CD_ColdConsumer_Troom, i, true, border, SS_BORDERSTYLE_SOLID, _TAH_RED_LIGHT );
			bEnable = false;
		}

		// Surface validation.
		std::wstring strSurface = m_pclSSheetConsumer->GetCellText( CD_ColdConsumer_Surface, i );
		double dSurface = 0.0;

		if( RD_OK != ReadCUDoubleFromStr( _U_AREA, strSurface, &dSurface )
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
		m_pclSSheetConsumer->SetBackColor( CD_ColdConsumer_FirstColumn,
										   RD_ColdConsumer_FirstAvailRow + m_iConsumerCount,
										   CD_ColdConsumer_LastColumn,
										   RD_ColdConsumer_FirstAvailRow + m_iConsumerCount,
										   _WHITE );
	}
	else
	{
		m_pclSSheetConsumer->SetBackColor( CD_ColdConsumer_FirstColumn,
										   RD_ColdConsumer_FirstAvailRow + m_iConsumerCount,
										   CD_ColdConsumer_LastColumn,
										   RD_ColdConsumer_FirstAvailRow + m_iConsumerCount,
										   _TAH_GREY_XLIGHT );
	}

	return m_bConsumerValid = bEnable;
}

void CDlgWizardPM_RightViewInput_CoolingConsumers::_CheckColdConsumerColumnWidth()
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

void CDlgWizardPM_RightViewInput_CoolingConsumers::_CheckColdConsumerSheetHeight()
{
	CRect rectClient;
	GetClientRect( &rectClient );

	CRect rectButtonReset;
	GetDlgItem( IDC_RESET )->GetWindowRect( &rectButtonReset);
	ScreenToClient( &rectButtonReset );

	CRect rectSheetClient;
	GetDlgItem( IDC_SSCOLDCONSLIST )->GetWindowRect( &rectSheetClient );
	ScreenToClient( &rectSheetClient );
	
	CRect rectSheetSize = m_pclSSheetConsumer->GetSheetSizeInPixels();
	
	if( rectSheetSize.Height() > rectSheetClient.Height() )
	{
		// Sheet with a new row becomes higher than the sheet client area.
		int iOffsetY = rectSheetSize.Height() - rectSheetClient.Height();

		if( m_ButtonGoNormalModePos.bottom + iOffsetY > rectButtonReset.top - 5 )
		{
			iOffsetY = rectButtonReset.top - 5 - m_ButtonGoNormalModePos.bottom;
		}

		m_pclSSheetConsumer->SetWindowPos( NULL, -1, -1, rectClient.Width() - 40, rectSheetClient.Height() + iOffsetY, SWP_NOMOVE | SWP_NOZORDER );
		m_ButtonGoNormalModePos.OffsetRect( 0, iOffsetY );
	}
	else if( rectSheetSize.Height() < rectSheetClient.Height() )
	{
		// Sheet with a row removed becomes taller than the sheet client area.
		int iHeight = max( m_rectSheetInitialSize.Height(), rectSheetSize.Height() );
		m_pclSSheetConsumer->SetWindowPos( NULL, -1, -1, rectClient.Width() - 40, iHeight, SWP_NOMOVE | SWP_NOZORDER );
		m_ButtonGoNormalModePos.OffsetRect( 0, -( rectSheetClient.Height() - iHeight ) );
	}
	else if( m_ButtonGoNormalModePos.bottom > rectButtonReset.top - 5 )
	{
		// Check the limit. After that the sheet has a vertical scrollbar.
		int iOffsetY = m_ButtonGoNormalModePos.bottom - rectButtonReset.top + 5;
		m_pclSSheetConsumer->SetWindowPos( NULL, -1, -1, rectClient.Width() - 40, rectSheetClient.Height() - iOffsetY, SWP_NOMOVE | SWP_NOZORDER );
		m_ButtonGoNormalModePos.OffsetRect( 0, -iOffsetY );
	}
	else
	{
		// Nothing. We just adapt the width.
		m_pclSSheetConsumer->SetWindowPos( NULL, -1, -1, rectClient.Width() - 40, rectSheetClient.Height(), SWP_NOMOVE | SWP_NOZORDER );
	}
}

bool CDlgWizardPM_RightViewInput_CoolingConsumers::_IsCheckBoxChecked( CSSheet *pclSSheet, SS_COORD col, SS_COORD row )
{
	TCHAR strCheckBox[16];
	pclSSheet->GetValue( col, row, strCheckBox );

	if( strCheckBox[0] == _T( '0' ) )
	{
		return false;
	}
	else
	{
		return true;
	}
}

void CDlgWizardPM_RightViewInput_CoolingConsumers::_VerifyResetButtons()
{
	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pclSelectPMList 
			|| NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERT_RETURN;
	}

	CPMInputUser *pclPMInputUser = m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser();

	BOOL bResetEnable = TRUE;

	if( 0 == pclPMInputUser->GetColdConsumersList()->GetConsumersCount() 
			&& 0 == pclPMInputUser->GetPipeList()->GetPipeCount() )
	{
		bResetEnable = FALSE;
	}

	GetDlgItem( IDC_RESET )->EnableWindow( bResetEnable );

	BOOL bResetAllEnable = TRUE;

	if( 0 == pclPMInputUser->GetColdGeneratorList()->GetColdGeneratorCount() 
			&& 0 == pclPMInputUser->GetColdConsumersList()->GetConsumersCount() 
			&& 0 == pclPMInputUser->GetPipeList()->GetPipeCount() )
	{
		bResetAllEnable = FALSE;
	}

	GetDlgItem( IDC_RESETALL )->EnableWindow( bResetAllEnable );
}
