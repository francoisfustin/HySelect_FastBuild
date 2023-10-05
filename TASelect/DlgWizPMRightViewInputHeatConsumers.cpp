#include "stdafx.h"
#include "TASelect.h"
#include "TASelectDoc.h"
#include "MainFrm.h"
#include "Global.h"
#include "SelectPM.h"
#include "DlgIndSelAdditionalPiping.h"
#include "DlgWizPMRightViewInputHeatConsumers.h"
#include "DlgIndSelPMSysVolHeating.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


CDlgWizardPM_RightViewInput_HeatingConsumers::CDlgWizardPM_RightViewInput_HeatingConsumers( CWizardSelPMParams *pclWizardSelPMParams, CWnd *pclParent )
	: CDlgWizardPM_RightViewInput_Base( pclWizardSelPMParams, HeatingConsumers, IDD, pclParent )
{
	m_bChangesDone = false;
	m_pclSDescConsumer = NULL;
	m_pclSSheetConsumer = NULL;
	m_rectSheetInitialSize.SetRectEmpty();
	m_iConsumerCount = 0;
	m_bConsumerValid = true;
	m_bBufferValid = true;

	m_HeatConsumerRectClient = CRect( 0, 0, 0, 0 );
	m_bGoToNormalModeMsgSent = false;
	m_bInitialized = false;
}

CDlgWizardPM_RightViewInput_HeatingConsumers::~CDlgWizardPM_RightViewInput_HeatingConsumers()
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

void CDlgWizardPM_RightViewInput_HeatingConsumers::FillPMInputUser()
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
		// Remark: these values have been saved by 'CDlgWizardPM_RightViewInput_HeatingSystemVolumePower' before switching
		//         to this advance mode.
		pclPMInputUser->SetSystemVolume( pclPMInputUser->GetSystemVolumeBackup() );
		pclPMInputUser->SetInstalledPower( pclPMInputUser->GetInstalledPowerBackup() );
	}
	else
	{
		double dTotalContent = pclPMInputUser->GetHeatGeneratorList()->GetTotalContent();
		dTotalContent += pclPMInputUser->GetHeatConsumersList()->GetTotalContent( pclPMInputUser->GetSupplyTemperature(), pclPMInputUser->GetReturnTemperature() );
		dTotalContent += pclPMInputUser->GetPipeList()->GetTotalContent();
		pclPMInputUser->SetSystemVolume( dTotalContent );

		pclPMInputUser->SetInstalledPower( pclPMInputUser->GetHeatGeneratorList()->GetTotalHeatCapacity() );
	}
}

BOOL CDlgWizardPM_RightViewInput_HeatingConsumers::PreTranslateMessage( MSG *pMsg )
{
	EnableNextButton( m_bConsumerValid && m_bBufferValid );

	if( WM_LBUTTONDOWN == pMsg->message )
	{
		// PWU : The first click on the spread is ignored if the focus is elsewhere.
		// This code catch the first click and send it directly to the spread.
		CRect rectHeatCons;
		GetDlgItem( IDC_SSHEATCONSLIST )->GetWindowRect( &rectHeatCons );

		CRect rectCurrentFocus;
		CWnd *focusedCtrl = GetFocus();

		if( focusedCtrl )
		{
			focusedCtrl->GetWindowRect( rectCurrentFocus );
		}

		CRect rectIntersectHeatCons;
		rectIntersectHeatCons.IntersectRect( rectCurrentFocus, rectHeatCons );

		if( rectHeatCons.PtInRect( pMsg->pt ) && rectIntersectHeatCons.IsRectNull() )
		{
			CSheetDescription *pclSDesc = m_ViewDescription.GetFromSSheetID( IDC_SSHEATCONSLIST );
			CSSheet *plcSSheet = pclSDesc->GetSSheetPointer();
			plcSSheet->SendMessage( pMsg->message, pMsg->wParam, pMsg->lParam );
			GetDlgItem( IDC_SSHEATCONSLIST )->SetFocus();
		}
	}

	return __super::PreTranslateMessage( pMsg ); // allow default behavior (return TRUE if you want to discard message)
}

BEGIN_MESSAGE_MAP( CDlgWizardPM_RightViewInput_HeatingConsumers, CDlgWizardPM_RightViewInput_Base )
	ON_WM_LBUTTONDOWN()
	ON_BN_CLICKED( IDC_ADDPIPES, OnBnClickedAddPipes )
	ON_EN_KILLFOCUS( IDC_EDITBUFFER, OnKillFocusOtherVolume )
	ON_BN_CLICKED( IDC_RESET, OnBnClickedReset )
	ON_BN_CLICKED( IDC_RESETALL, OnBnClickedResetAll )
	ON_BN_CLICKED( IDC_BUTTON_CANCEL, OnBnClickedCancel )
	ON_MESSAGE( SSM_COMBOCLOSEUP, OnComboDropCloseUp )
	ON_MESSAGE( SSM_CLICK, OnSSClick )
	ON_MESSAGE( SSM_DBLCLK, OnSSDblClick )
	ON_MESSAGE( SSM_EDITCHANGE, OnSSEditChange )
	// Spread DLL messages
	ON_MESSAGE( SSM_TEXTTIPFETCH, OnTextTipFetch )
END_MESSAGE_MAP()

void CDlgWizardPM_RightViewInput_HeatingConsumers::DoDataExchange( CDataExchange *pDX )
{
	CDlgWizardPM_RightViewInput_Base::DoDataExchange( pDX );

	DDX_Control( pDX, IDC_STATICTOTALCONSHEATCAPACITY, m_StaticTotalConsumerHeatCapacity );
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
}

BOOL CDlgWizardPM_RightViewInput_HeatingConsumers::OnInitDialog()
{
	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pclSelectPMList 
			|| NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERTA_RETURN( FALSE );
	}

	CPMInputUser *pclPMInputUser = m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser();

	CDlgWizardPM_RightViewInput_Base::OnInitDialog();

	SetMainTitle( TASApp.LoadLocalizedString( IDS_DLGWIZPM_RVIEWHEATCONSUMERS_MAINTITLE ) );

	CString str;
	TCHAR name[_MAXCHARS];
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();
	
	//////////////////////////////////////////////////////////////////////////
	// EDITTEXT

	m_ExtEditOtherVolume.SetCurrentValSI( pclPMInputUser->GetHeatConsumersList()->GetBufferContent() );
	InitNumericalEdit( &m_ExtEditOtherVolume, _U_VOLUME );

	m_ExtEditAdditionalPipe.SetCurrentValSI( pclPMInputUser->GetPipeList()->GetTotalContent() );
	InitNumericalEdit( &m_ExtEditAdditionalPipe, _U_VOLUME );

	//////////////////////////////////////////////////////////////////////////
	// STATIC

	if( false == pclPMInputUser->IsNorm( ProjectType::All, PressurisationNorm::PN_SWKIHE301_01 ) )
	{
		// Other, e.g. buffer:
		str = TASApp.LoadLocalizedString( IDS_DLGWIZPM_RVIEWHEATGENERATORS_OTHERBUFFER );
	}
	else
	{
		// Other:
		str = TASApp.LoadLocalizedString( IDS_DLGWIZPM_RVIEWHEATGENERATORS_OTHER );
	}

	m_StaticOtherVolume.SetTextAndToolTip( str );

	// Other, e.g. buffer: (Unit)
	GetNameOf( pUnitDB->GetDefaultUnit( _U_VOLUME ), name );
	SetDlgItemText( IDC_STATICUNITBUFFER, name );

	// Additional piping:
	str = TASApp.LoadLocalizedString( IDS_DLGWIZPM_RVIEWHEATGENERATORS_ADDPIPES );
	m_StaticAdditionalPipe.SetTextAndToolTip( str );

	// Additional piping: (Unit)
	GetNameOf( pUnitDB->GetDefaultUnit( _U_VOLUME ), name );
	SetDlgItemText( IDC_STATICUNITADDPIPES, name );

	// Target linear Dp: xx Pa/m
	double dPipeTargetDp = m_pclTechParam->GetPipeTargDp();
	CString strTargetLinearDp = WriteCUDouble( _U_LINPRESSDROP, dPipeTargetDp );
	GetNameOf( pUnitDB->GetDefaultUnit( _U_LINPRESSDROP ), name );
	FormatString( str, IDS_DLGWIZPM_RVIEWHEATCONSUMERS_TARGETLINEARDP, strTargetLinearDp, name );
	m_StaticTargetLinearDp.SetWindowTextW( str );
	m_StaticTargetLinearDp.SetToolTip( TASApp.LoadLocalizedString( IDS_INDSELPMSYSVOLCOOLING_TARGETLINEARDPTT ) );

	// Total installation content (Generators and consumers):
	str = TASApp.LoadLocalizedString( IDS_DLGWIZPM_RVIEWHEATGENERATORS_TOTALCONTENT );
	m_StaticTotalInstallation.SetTextAndToolTip( str );

	m_StaticTotalConsumerHeatCapacity.SetBackColor( RGB( 255, 255, 255 ) );
	m_StaticTotalConsumerContent.SetBackColor( RGB( 255, 255, 255 ) );
	m_StaticOtherVolume.SetBackColor( RGB( 255, 255, 255 ) );
	m_StaticAdditionalPipe.SetBackColor( RGB( 255, 255, 255 ) );
	m_StaticTargetLinearDp.SetBackColor( RGB( 255, 255, 255 ) );
	m_StaticTotalInstallation.SetBackColor( RGB( 255, 255, 255 ) );
	m_StaticTotalContent.SetBackColor( RGB( 255, 255, 255 ) );

	// Set some static text in bold.
	m_StaticTotalConsumerHeatCapacity.SetFontBold( true );
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
	_LoadHeatConsumer();
	_ButtonPlusConsumerEnabled();
	_UpdateStaticValue();

	m_bInitialized = true;

	return TRUE;
}

void CDlgWizardPM_RightViewInput_HeatingConsumers::OnPaint()
{
	CDlgWizardPM_RightViewInput_Base::OnPaint();

	DrawGoAdvancedNormalModeButtton( Draw_NormalMode, m_ButtonGoNormalModePos );
}

void CDlgWizardPM_RightViewInput_HeatingConsumers::OnSize( UINT nType, int cx, int cy )
{
	CDlgWizardPM_RightViewInput_Base::OnSize( nType, cx, cy );

	if( NULL == GetDlgItem( IDC_SSHEATCONSLIST ) || NULL == GetDlgItem( IDC_SSHEATCONSLIST )->GetSafeHwnd() )
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
	_CheckHeatConsumerSheetHeight();
	_CheckHeatConsumerColumnWidth();

	CRect rectSheet;
	GetDlgItem( IDC_SSHEATCONSLIST )->GetWindowRect( &rectSheet );
	ScreenToClient( &rectSheet );

	// Resize separator below sheet.
	CRect rectSeparatorBelowSheet;
	GetDlgItem( IDC_STATICBELOWSHEET )->GetClientRect( &rectSeparatorBelowSheet );
	int iNewY = rectSheet.bottom + 1;
	GetDlgItem( IDC_STATICBELOWSHEET )->SetWindowPos( NULL, rectSheet.left, iNewY, rectSheet.Width(), 2, SWP_NOZORDER );

	// Move total consumers heat capacity.
	CRect rectTotalConsumerHeatCapacity;
	m_StaticTotalConsumerHeatCapacity.GetWindowRect( &rectTotalConsumerHeatCapacity );
	ScreenToClient( &rectTotalConsumerHeatCapacity );
	CRect rectCellCapacity = m_pclSSheetConsumer->GetCellCoordInPixel( CD_HeatConsumer_Capacity, RD_HeatConsumer_UnitName );
	int iNewX = 20 + rectCellCapacity.right - rectTotalConsumerHeatCapacity.Width();
	iNewY += 4;
	m_StaticTotalConsumerHeatCapacity.SetWindowPos( NULL, iNewX, iNewY, -1, -1, SWP_NOSIZE | SWP_NOZORDER );

	// Move total consumers content.
	CRect rectTotalConsumerContent;
	m_StaticTotalConsumerContent.GetWindowRect( &rectTotalConsumerContent );
	ScreenToClient( &rectTotalConsumerContent );
	CRect rectCellContent = m_pclSSheetConsumer->GetCellCoordInPixel( CD_HeatConsumer_Content, RD_HeatConsumer_UnitName );
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

void CDlgWizardPM_RightViewInput_HeatingConsumers::OnLButtonDown( UINT nFlags, CPoint point )
{
	if( false == m_bGoToNormalModeMsgSent )
	{
		CRect rectClient;
		GetClientRect( &rectClient );
		CRect rectGoButton( 20, m_ButtonGoNormalModePos.top, rectClient.Width() - 20, m_ButtonGoNormalModePos.top + 20 );

		if( TRUE == rectGoButton.PtInRect( (POINT)point ) )
		{
			m_bGoToNormalModeMsgSent = true;
			m_pclParent->PostMessage( WM_USER_RIGHTVIEWWIZPM_SWITCHMODE, WPARAM( HeatingSystemVolumePower ), 0 );
		}
	}

	CDlgWizardPM_RightViewInput_Base::OnLButtonDown( nFlags, point );
}

void CDlgWizardPM_RightViewInput_HeatingConsumers::OnBnClickedAddPipes()
{
	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pclSelectPMList 
			|| NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERT_RETURN;
	}

	CPMInputUser *pclPMInputUser = m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser();

	CDlgIndSelAdditionalPiping dlg( m_pclWizardSelPMParams, pclPMInputUser->GetPipeList() );
	dlg.DoModal();
	_UpdateStaticValue();
}

void CDlgWizardPM_RightViewInput_HeatingConsumers::OnKillFocusOtherVolume()
{
	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pclSelectPMList 
			|| NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERT_RETURN;
	}

	CPMInputUser *pclPMInputUser = m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser();

	double previousBuffer = pclPMInputUser->GetHeatConsumersList()->GetBufferContent();

	CString strBuffer;
	double dBuffer = 0.0;
	m_ExtEditOtherVolume.GetWindowTextW( strBuffer );

	// if the buffer content is invalid put 0.0
	if( RD_NOT_NUMBER == ReadDoubleFromStr( strBuffer.GetString(), &dBuffer ) || dBuffer < 0.0 )
	{
		m_bBufferValid = false;
		pclPMInputUser->GetHeatConsumersList()->SetBufferContent( 0.0 );
	}
	else
	{
		m_bBufferValid = true;
		pclPMInputUser->GetHeatConsumersList()->SetBufferContent( CDimValue::CUtoSI( _U_VOLUME, dBuffer ) );
	}

	// if the buffer is changed warn the user on cancel
	if( previousBuffer != pclPMInputUser->GetHeatConsumersList()->GetBufferContent() )
	{
		m_bChangesDone = true;
	}

	_UpdateStaticValue();
}

void CDlgWizardPM_RightViewInput_HeatingConsumers::OnBnClickedReset()
{
	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pclSelectPMList 
			|| NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERT_RETURN;
	}

	CPMInputUser *pclPMInputUser = m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser();

	if( 0 != pclPMInputUser->GetHeatConsumersList()->GetConsumersCount() 
			|| 0 != pclPMInputUser->GetPipeList()->GetPipeCount() )
	{
		CString str = TASApp.LoadLocalizedString( IDS_DLGWIZPM_RVIEWHEATCONSUMERS_RESET );

		if( IDYES == ::AfxMessageBox( str, MB_YESNO | MB_ICONQUESTION ) )
		{
			pclPMInputUser->GetHeatConsumersList()->Reset();
			pclPMInputUser->GetPipeList()->Reset();
			_UpdateStaticValue();

			SS_CELLCOORD cellCoord;
			cellCoord.Col = 1;
			cellCoord.Row = RD_HeatConsumer_FirstAvailRow;

			while( m_iConsumerCount )
			{
				_RemoveHeatConsumer( &cellCoord );
			}

			m_ExtEditOtherVolume.SetWindowTextW( WriteCUDouble( _U_VOLUME, 0.0 ) );

			m_bChangesDone = false;
			_ButtonPlusConsumerEnabled();
		}
	}
}

void CDlgWizardPM_RightViewInput_HeatingConsumers::OnBnClickedResetAll()
{
	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pclSelectPMList 
			|| NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERT_RETURN;
	}

	CPMInputUser *pclPMInputUser = m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser();
	
	if( 0 != pclPMInputUser->GetHeatGeneratorList()->GetHeatGeneratorCount() 
			|| 0 != pclPMInputUser->GetHeatConsumersList()->GetConsumersCount() 
			|| 0 != pclPMInputUser->GetPipeList()->GetPipeCount() )
	{
		CString str = TASApp.LoadLocalizedString( IDS_DLGWIZPM_RVIEWHEATGENERATORCONSUMERS_RESETALL );

		if( IDYES == ::AfxMessageBox( str, MB_YESNO | MB_ICONQUESTION ) )
		{
			pclPMInputUser->GetHeatGeneratorList()->Reset();
			pclPMInputUser->GetHeatConsumersList()->Reset();
			pclPMInputUser->GetPipeList()->Reset();
			_UpdateStaticValue();

			SS_CELLCOORD cellCoord;
			cellCoord.Col = 1;
			cellCoord.Row = RD_HeatConsumer_FirstAvailRow;

			while( m_iConsumerCount )
			{
				_RemoveHeatConsumer( &cellCoord );
			}

			m_ExtEditOtherVolume.SetWindowTextW( WriteCUDouble( _U_VOLUME, 0.0 ) );

			m_bChangesDone = false;
			_ButtonPlusConsumerEnabled();
		}
	}
}

void CDlgWizardPM_RightViewInput_HeatingConsumers::OnBnClickedCancel()
{
	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pclSelectPMList 
			|| NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERT_RETURN;
	}

	CPMInputUser *pclPMInputUser = m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser();

	PREVENT_ENTER_KEY;

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
			// Remark: these values have been saved by 'CDlgWizardPM_RightViewInput_HeatingSystemVolumePower' before switching
			//         to this advance mode.
			pclPMInputUser->GetHeatGeneratorList()->CopyFrom( pclPMInputUser->GetpHeatGeneratorListBackup() );
			pclPMInputUser->GetHeatConsumersList()->CopyFrom( pclPMInputUser->GetpHeatConsumersListBackup() );
			pclPMInputUser->GetPipeList()->CopyFrom( pclPMInputUser->GetpPipeListBackup() );

			m_bChangesDone = false;
		}

		m_bGoToNormalModeMsgSent = true;
		m_pclParent->PostMessage( WM_USER_RIGHTVIEWWIZPM_SWITCHMODE, WPARAM( HeatingSystemVolumePower ), 0 );
	}
}

LRESULT CDlgWizardPM_RightViewInput_HeatingConsumers::OnComboDropCloseUp( WPARAM wParam, LPARAM lParam )
{
	_UpdateHeatConsumer();
	return 0;
}

LRESULT CDlgWizardPM_RightViewInput_HeatingConsumers::OnSSClick( WPARAM wParam, LPARAM lParam )
{
	SS_CELLCOORD *pclCellCoord = (SS_CELLCOORD *)lParam;
	int iRecordIndex = pclCellCoord->Row - RD_HeatConsumer_FirstAvailRow;

	CSheetDescription *pclSDesc = m_ViewDescription.GetFromSSheetID( UINT( wParam ) );
	CSSheet *pclSSheet = pclSDesc->GetSSheetPointer();

	// Intercept click on the first column and below first row only.
	if( ( pclCellCoord->Col != CD_HeatConsumer_FirstColumn && pclCellCoord->Col != CD_HeatConsumer_ManualContent 
			&& pclCellCoord->Col != CD_HeatConsumer_ManualTemp ) || pclCellCoord->Row < RD_HeatConsumer_FirstAvailRow )
	{
		return 0;
	}

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
		return 1; // event processed
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
		return 1;
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
		return 1;
	}

	return 0;
}

LRESULT CDlgWizardPM_RightViewInput_HeatingConsumers::OnSSDblClick( WPARAM wParam, LPARAM lParam )
{
	SS_CELLCOORD *pclCellCoord = ( SS_CELLCOORD * )lParam;

	// Avoid weird behavior on checkbox.
	if( pclCellCoord->Col == CD_HeatConsumer_ManualTemp )
	{
		return 1;
	}

	return 0;
}

LRESULT CDlgWizardPM_RightViewInput_HeatingConsumers::OnSSEditChange( WPARAM wParam, LPARAM lParam )
{
	CSheetDescription *pclSDesc = m_ViewDescription.GetFromSSheetID( UINT( wParam ) );
	CSSheet *pclSSheet = pclSDesc->GetSSheetPointer();

	SS_CELLCOORD *pclCellCoord = ( SS_CELLCOORD * )lParam;

	// intercept click on the first column and below first row only
	if( ( pclCellCoord->Col != CD_HeatConsumer_Capacity
		  && pclCellCoord->Col != CD_HeatConsumer_Content 
		  && pclCellCoord->Col != CD_HeatConsumer_SupplyTemp 
		  && pclCellCoord->Col != CD_HeatConsumer_ReturnTemp )
		|| pclCellCoord->Row < RD_HeatConsumer_FirstAvailRow )
	{
		return 0;
	}

	if( pclCellCoord->Col == CD_HeatConsumer_Capacity
		|| pclCellCoord->Col == CD_HeatConsumer_Content
		|| pclCellCoord->Col == CD_HeatConsumer_SupplyTemp
		|| pclCellCoord->Col == CD_HeatConsumer_ReturnTemp )
	{
		CString strValue = pclSSheet->GetCellText( pclCellCoord->Col, pclCellCoord->Row );
		double dValue;

		// HYS-1307: Enable to use "-" sign for temperature
		bool bIsForTemperature = false;
		if( CD_HeatConsumer_SupplyTemp == pclCellCoord->Col ||
			CD_HeatConsumer_ReturnTemp == pclCellCoord->Col )
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

	}

	_UpdateHeatConsumer();
	_ButtonPlusConsumerEnabled();

	return 0;
}

LRESULT CDlgWizardPM_RightViewInput_HeatingConsumers::OnTextTipFetch( WPARAM wParam, LPARAM lParam )
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
	std::wstring strSupplyTemp = m_pclSSheetConsumer->GetCellText( CD_HeatConsumer_SupplyTemp, pTextTipFetch->Row );
	double dSupplyTemp = 0.0;
	std::wstring strReturnTemp = m_pclSSheetConsumer->GetCellText( CD_HeatConsumer_ReturnTemp, pTextTipFetch->Row );
	double dReturnTemp = 0.0;

	switch( pTextTipFetch->Col )
	{
		// 
		case CD_HeatConsumer_SupplyTemp:

			if( TRUE == m_pclSSheetConsumer->GetBorder( pTextTipFetch->Col, pTextTipFetch->Row, &border, &style, &color )
					&& ( border == borderType && style == borderStyle && color == _TAH_RED_LIGHT ) )
			{
				// HYS-1612: pay attention that all temperatures read from string are in custom unit!
				ReadDoubleReturn_enum eSupplyReadStatus = ReadCUDoubleFromStr( _U_TEMPERATURE, strSupplyTemp, &dSupplyTemp );
				ReadDoubleReturn_enum eReturnReadStatus = ReadCUDoubleFromStr( _U_TEMPERATURE, strReturnTemp, &dReturnTemp );

				if( RD_OK == eSupplyReadStatus )
				{
					if( dSupplyTemp > pclPMInputUser->GetSupplyTemperature() )
					{
						str = TASApp.LoadLocalizedString( IDS_INDSELPMSYSVOL_TSTUTSSYSH_TT );
					}
					else if( RD_OK == eReturnReadStatus && dSupplyTemp <= dReturnTemp )
					{
						str = TASApp.LoadLocalizedString( IDS_INDSELPMSYSVOL_TSTRH_TT );
					}
					else if( RD_OK == eReturnReadStatus && ( ( dSupplyTemp + dReturnTemp ) / 2.0 ) <= 20.0 )
					{
						// This condition is only because the interpolation equation (See in the 'CHeatConsumersList::_ComputeWaterContent' method
						// in the 'SelectPM.cpp' file).
						FormatString( str, IDS_INDSELPMSYSVOL_TSTRSUM_TT, WriteCUDouble( _U_TEMPERATURE, 20.0, true ) );
					}
				}
			}
			break;

		case CD_HeatConsumer_ReturnTemp:

			if( TRUE == m_pclSSheetConsumer->GetBorder( pTextTipFetch->Col, pTextTipFetch->Row, &border, &style, &color )
					&& ( border == borderType && style == borderStyle && color == _TAH_RED_LIGHT ) )
			{
				// HYS-1612: pay attention that all temperatures read from string are in custom unit!
				ReadDoubleReturn_enum eSupplyReadStatus = ReadCUDoubleFromStr( _U_TEMPERATURE, strSupplyTemp, &dSupplyTemp );
				ReadDoubleReturn_enum eReturnReadStatus = ReadCUDoubleFromStr( _U_TEMPERATURE, strReturnTemp, &dReturnTemp );

				if( RD_OK == eSupplyReadStatus && RD_OK == eReturnReadStatus )
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
			break;
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

LRESULT CDlgWizardPM_RightViewInput_HeatingConsumers::OnUnitChange( WPARAM wParam, LPARAM lParam )
{
	CDlgWizardPM_RightViewInput_Base::OnUnitChange( wParam, lParam );

	TCHAR name[_MAXCHARS];
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();

	// Update units for the columns in the sheet.
	m_pclSSheetConsumer->SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );
	m_pclSSheetConsumer->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontSize, ( LPARAM )8 );
	m_pclSSheetConsumer->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, ( LPARAM )_TAH_TITLE_MAIN );
	m_pclSSheetConsumer->SetStaticText( CD_HeatConsumer_Capacity, RD_HeatConsumer_UnitName, GetNameOf( pUnitDB->GetDefaultUnit( _U_TH_POWER ) ).c_str() );
	m_pclSSheetConsumer->SetStaticText( CD_HeatConsumer_Content, RD_HeatConsumer_UnitName, GetNameOf( pUnitDB->GetDefaultUnit( _U_VOLUME ) ).c_str() );
	m_pclSSheetConsumer->SetStaticText( CD_HeatConsumer_SupplyTemp, RD_HeatConsumer_UnitName, GetNameOf( pUnitDB->GetDefaultUnit( _U_TEMPERATURE ) ).c_str() );
	m_pclSSheetConsumer->SetStaticText( CD_HeatConsumer_ReturnTemp, RD_HeatConsumer_UnitName, GetNameOf( pUnitDB->GetDefaultUnit( _U_TEMPERATURE ) ).c_str() );
	m_pclSSheetConsumer->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );

	// Update all value in the sheet.
	_UpdateHeatConsumerValues();

	// Update units in the statics.
	_UpdateStaticValue();

	// Target linear Dp: xx Pa/m.
	double dPipeTargetDp = m_pclTechParam->GetPipeTargDp();
	CString strTargetLinearDp = WriteCUDouble( _U_LINPRESSDROP, dPipeTargetDp );
	GetNameOf( pUnitDB->GetDefaultUnit( _U_LINPRESSDROP ), name );
	CString str;
	FormatString( str, IDS_DLGWIZPM_RVIEWCOOLCONSUMERS_TARGETLINEARDP, strTargetLinearDp, name );
	m_StaticTargetLinearDp.SetWindowTextW( str );
	m_StaticTargetLinearDp.SetToolTip( TASApp.LoadLocalizedString( IDS_INDSELPMSYSVOLCOOLING_TARGETLINEARDPTT ) );

	return 0;
}

LRESULT CDlgWizardPM_RightViewInput_HeatingConsumers::OnTechParChange( WPARAM wParam, LPARAM lParam )
{
	TCHAR name[_MAXCHARS];
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();

	// Target linear Dp: xx Pa/m
	double dPipeTargetDp = m_pclTechParam->GetPipeTargDp();
	CString strTargetLinearDp = WriteCUDouble( _U_LINPRESSDROP, dPipeTargetDp );
	GetNameOf( pUnitDB->GetDefaultUnit( _U_LINPRESSDROP ), name );
	CString str;
	FormatString( str, IDS_DLGWIZPM_RVIEWHEATCONSUMERS_TARGETLINEARDP, strTargetLinearDp, name );
	m_StaticTargetLinearDp.SetWindowTextW( str );
	m_StaticTargetLinearDp.SetToolTip( TASApp.LoadLocalizedString( IDS_INDSELPMSYSVOLCOOLING_TARGETLINEARDPTT ) );

	return 0;
}

void CDlgWizardPM_RightViewInput_HeatingConsumers::_InitializeSSheet()
{
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();

	// Pay attention: because in this case I use TSpread directly as a custom control (see IDD_DLGPIPEDP in the resource), if you want to
	// use 'CSSheet' possibility, we have to attach TSpread CWnd to CSSheet newly created. And the destruction of the dialog, don't forget
	// to call 'Detach' before deleting 'CSSheet'.
	m_pclSSheetConsumer = new CSSheet();
	m_pclSSheetConsumer->Attach( GetDlgItem( IDC_SSHEATCONSLIST )->GetSafeHwnd() );
	m_pclSSheetConsumer->Reset();

	// Save initial size.
	m_pclSSheetConsumer->GetClientRect( &m_rectSheetInitialSize );

	m_pclSDescConsumer = m_ViewDescription.AddSheetDescription( SD_HeatConsumer, -1, m_pclSSheetConsumer, CPoint( 0, 0 ) );

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
	m_pclSSheetConsumer->SetMaxRows( RD_HeatConsumer_FirstAvailRow );
	m_pclSSheetConsumer->SetFreeze( 0, RD_HeatConsumer_FirstAvailRow - 1 );

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
	m_pclSSheetConsumer->SetStaticText( CD_HeatConsumer_FirstColumn, RD_HeatConsumer_ColName, _T( "" ) );
	m_pclSSheetConsumer->SetStaticText( CD_HeatConsumer_FirstColumn, RD_HeatConsumer_UnitName, _T( "" ) );
	m_pclSSheetConsumer->SetStaticText( CD_HeatConsumer_Consumer, RD_HeatConsumer_ColName, IDS_INDSELPMSYSVOLHEATING_CONSUMERS );
	m_pclSSheetConsumer->SetStaticText( CD_HeatConsumer_Consumer, RD_HeatConsumer_UnitName, _T( "" ) );
	m_pclSSheetConsumer->SetStaticText( CD_HeatConsumer_Capacity, RD_HeatConsumer_ColName, IDS_INDSELPMSYSVOLHEATING_CAPACITY );
	m_pclSSheetConsumer->SetStaticText( CD_HeatConsumer_Capacity, RD_HeatConsumer_UnitName, GetNameOf( pUnitDB->GetDefaultUnit( _U_TH_POWER ) ).c_str() );
	m_pclSSheetConsumer->SetStaticText( CD_HeatConsumer_Content, RD_HeatConsumer_ColName, IDS_INDSELPMSYSVOLHEATING_CONTENT );
	m_pclSSheetConsumer->SetStaticText( CD_HeatConsumer_Content, RD_HeatConsumer_UnitName, GetNameOf( pUnitDB->GetDefaultUnit( _U_VOLUME ) ).c_str() );
	
	// Manual content checkbox.
	m_pclSSheetConsumer->SetStaticText( CD_HeatConsumer_ManualContent, RD_HeatConsumer_ColName, _T( "" ) );
	m_pclSSheetConsumer->SetPictureCellWithID( IDI_PENCILDIAGSHORT, CD_HeatConsumer_ManualContent, RD_HeatConsumer_UnitName, CSSheet::PictureCellType::Icon );
	m_pclSSheetConsumer->SetBackColor( CD_HeatConsumer_ManualContent, RD_HeatConsumer_UnitName, _TAH_ORANGE );
	
	m_pclSSheetConsumer->SetStaticText( CD_HeatConsumer_SupplyTemp, RD_HeatConsumer_ColName, IDS_INDSELPMSYSVOLHEATING_SUPPLYTEMP );
	m_pclSSheetConsumer->SetStaticText( CD_HeatConsumer_SupplyTemp, RD_HeatConsumer_UnitName, GetNameOf( pUnitDB->GetDefaultUnit( _U_TEMPERATURE ) ).c_str() );
	m_pclSSheetConsumer->SetStaticText( CD_HeatConsumer_ReturnTemp, RD_HeatConsumer_ColName, IDS_INDSELPMSYSVOLHEATING_RETURNTEMP );
	m_pclSSheetConsumer->SetStaticText( CD_HeatConsumer_ReturnTemp, RD_HeatConsumer_UnitName, GetNameOf( pUnitDB->GetDefaultUnit( _U_TEMPERATURE ) ).c_str() );
	
	// Manual temp checkbox.
	m_pclSSheetConsumer->SetStaticText( CD_HeatConsumer_ManualTemp, RD_HeatConsumer_ColName, _T( "" ) );
	m_pclSSheetConsumer->SetPictureCellWithID( IDI_PENCILDIAGSHORT, CD_HeatConsumer_ManualTemp, RD_HeatConsumer_UnitName, CSSheet::PictureCellType::Icon );
	m_pclSSheetConsumer->SetBackColor( CD_HeatConsumer_ManualTemp, RD_HeatConsumer_UnitName, _TAH_ORANGE );

	// To fill a icon.
	m_pclSSheetConsumer->SetPictureCellWithID( IDI_PLUSGREEN, CD_HeatConsumer_FirstColumn, RD_HeatConsumer_FirstAvailRow, CSSheet::PictureCellType::Icon );

	m_pclSSheetConsumer->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
}

void CDlgWizardPM_RightViewInput_HeatingConsumers::_UpdateStaticValue()
{
	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pclSelectPMList 
			|| NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERT_RETURN;
	}

	CPMInputUser *pclPMInputUser = m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser();

	double dTotalConsHeatCapacity = pclPMInputUser->GetHeatConsumersList()->GetTotalHeatCapacity();
	m_StaticTotalConsumerHeatCapacity.SetWindowText( WriteCUDouble( _U_TH_POWER, dTotalConsHeatCapacity, true ) );
	
	double dTotalConsContent = pclPMInputUser->GetHeatConsumersList()->GetTotalContent( pclPMInputUser->GetSupplyTemperature(), 
			pclPMInputUser->GetReturnTemperature() ) - pclPMInputUser->GetHeatConsumersList()->GetBufferContent();
	
	m_StaticTotalConsumerContent.SetWindowText( WriteCUDouble( _U_VOLUME, dTotalConsContent, true ) );

	// HYS-869
	if( dTotalConsHeatCapacity > pclPMInputUser->GetHeatGeneratorList()->GetTotalHeatCapacity() )
	{
		m_StaticTotalConsumerHeatCapacity.SetTextColor( _ORANGE );
		m_StaticTotalConsumerHeatCapacity.SetToolTip( TASApp.LoadLocalizedString( IDS_INDSELPMSYSVOL_TOTALCAPACITY_TT ) );
	}
	else
	{
		m_StaticTotalConsumerHeatCapacity.ResetToolTip();
		m_StaticTotalConsumerHeatCapacity.SetTextColor( _BLACK );
	}

	double dOtherVolume = pclPMInputUser->GetHeatConsumersList()->GetBufferContent();
	m_ExtEditOtherVolume.SetCurrentValSI( dOtherVolume );
	m_ExtEditOtherVolume.Update();

	double dPipeContent = pclPMInputUser->GetPipeList()->GetTotalContent();
	m_ExtEditAdditionalPipe.SetCurrentValSI( dPipeContent );
	m_ExtEditAdditionalPipe.Update();

	double dTotalContent = pclPMInputUser->GetHeatGeneratorList()->GetTotalContent();
	dTotalContent += pclPMInputUser->GetHeatConsumersList()->GetTotalContent( pclPMInputUser->GetSupplyTemperature(), pclPMInputUser->GetReturnTemperature() );
	dTotalContent += dPipeContent;
	m_StaticTotalContent.SetWindowText( WriteCUDouble( _U_VOLUME, dTotalContent, true ) );

	_VerifyResetButtons();
}

void CDlgWizardPM_RightViewInput_HeatingConsumers::_RemoveHeatConsumer( SS_CELLCOORD * plcCellCoord )
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
	m_pclSSheetConsumer->SetMaxRows( RD_HeatConsumer_FirstAvailRow + m_iConsumerCount ); // Set max rows.
	pclPMInputUser->GetHeatConsumersList()->RemoveConsumers( plcCellCoord->Row - RD_HeatConsumer_FirstAvailRow );
	m_bChangesDone = true;
	
	_CheckHeatConsumerColumnWidth();
	m_pclSSheetConsumer->SetRedraw( TRUE );

	// Manually call 'OnSize' method to well move all the components.
	CRect rectClient;
	GetClientRect( &rectClient );
	OnSize( SIZE_RESTORED, rectClient.Width(), rectClient.Height() );

	// To force a paint refresh.
	Invalidate();
	UpdateWindow();
}

void CDlgWizardPM_RightViewInput_HeatingConsumers::_FillConsumersCombo( CSSheet* pclSheet, long lCol, long lRow, const TCHAR* strSelect )
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
		CTable *pclTable = dynamic_cast<CTable *>( m_pclWizardSelPMParams->m_pTADB->Get( _T("PMCONSUMERS_TAB") ).MP );

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

void CDlgWizardPM_RightViewInput_HeatingConsumers::_AddHeatConsumer( CSSheet *pclSSheet, CString pstrType, double dCapacity, double dContent, 
	bool bManualContent, double dSupplyTemp, double dReturnTemp, bool bManualTemp, bool bOnlyGUI )
{
	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pclSelectPMList 
			|| NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERT_RETURN;
	}

	CPMInputUser *pclPMInputUser = m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser();

	m_iConsumerCount++;
	int iCurrentRow = RD_HeatConsumer_FirstAvailRow + m_iConsumerCount - 1;

	// Set max rows.
	pclSSheet->SetRedraw( FALSE );
	pclSSheet->SetMaxRows( RD_HeatConsumer_FirstAvailRow + m_iConsumerCount );

	// Add plus icon.
	pclSSheet->SetPictureCellWithID( IDI_PLUSGREEN, CD_HeatConsumer_FirstColumn, iCurrentRow + 1, CSSheet::PictureCellType::Icon );

	// Add checkbox.
	pclSSheet->SetCheckBox( CD_HeatConsumer_ManualContent, iCurrentRow, _T( "" ), false, true );
	pclSSheet->SetCheckBox( CD_HeatConsumer_ManualTemp, iCurrentRow, _T( "" ), false, true );

	// Add trash icon.
	pclSSheet->SetPictureCellWithID( IDI_TRASH, CD_HeatConsumer_FirstColumn, iCurrentRow, CSSheet::PictureCellType::Icon );

	// Add generator combo.
	pclSSheet->FormatComboList( CD_HeatConsumer_Consumer, iCurrentRow );
	_FillConsumersCombo( pclSSheet, CD_HeatConsumer_Consumer, iCurrentRow, pstrType );

	//HYS-869
	if( ( false == bOnlyGUI ) && ( pclPMInputUser->GetHeatGeneratorList()->GetTotalHeatCapacity() > pclPMInputUser->GetHeatConsumersList()->GetTotalHeatCapacity() ) )
	{
		dCapacity = CDimValue::SItoCU( _U_TH_POWER, ( pclPMInputUser->GetHeatGeneratorList()->GetTotalHeatCapacity() - pclPMInputUser->GetHeatConsumersList()->GetTotalHeatCapacity() ) );
	}
	// Add capacity edit.
	pclSSheet->FormatEditDouble( CD_HeatConsumer_Capacity, iCurrentRow, _T( "" ), SSS_ALIGN_RIGHT );
	
	CString strCapacity;
	strCapacity.Format( _T("%g"), dCapacity );
	pclSSheet->SetCellText( CD_HeatConsumer_Capacity, iCurrentRow, strCapacity );
	
	SS_CELLTYPE rCellType;
	pclSSheet->SetTypeEdit( &rCellType, SSS_ALIGN_RIGHT, 15, SS_CHRSET_ALPHANUM, SS_CASE_NOCASE );
	pclSSheet->SetCellType( CD_HeatConsumer_Capacity, iCurrentRow, &rCellType );

	CString strConsumerID;
	_FindConsumerID( pclSSheet, iCurrentRow, strConsumerID );

	if( false == bOnlyGUI ) // Used to load data at dialog opening
	{
		pclPMInputUser->GetHeatConsumersList()->AddConsumers( strConsumerID, dCapacity, dContent, pclPMInputUser->GetSupplyTemperature(), 
				pclPMInputUser->GetReturnTemperature(), bManualTemp );
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
		pclSSheet->SetCellText( CD_HeatConsumer_ManualContent, iCurrentRow, _T("1") );
		pclSSheet->FormatEditDouble( CD_HeatConsumer_Content, iCurrentRow, strContent, SSS_ALIGN_RIGHT );
		pclSSheet->SetCellType( CD_HeatConsumer_Content, iCurrentRow, &rCellType );
	}
	else
	{
		pclSSheet->FormatStaticText( CD_HeatConsumer_ManualContent, iCurrentRow, CD_HeatConsumer_Content, iCurrentRow, strContent, SSS_ALIGN_RIGHT );
	}

	if( true == bManualTemp ) // Manual input
	{
		pclSSheet->SetCellText( CD_HeatConsumer_ManualTemp, iCurrentRow, _T("1") );
		pclSSheet->FormatEditDouble( CD_HeatConsumer_SupplyTemp, iCurrentRow, strSupplyTemp, SSS_ALIGN_RIGHT );
		pclSSheet->FormatEditDouble( CD_HeatConsumer_ReturnTemp, iCurrentRow, strReturnTemp, SSS_ALIGN_RIGHT );
		pclSSheet->SetCellType( CD_HeatConsumer_SupplyTemp, iCurrentRow, &rCellType );
		pclSSheet->SetCellType( CD_HeatConsumer_ReturnTemp, iCurrentRow, &rCellType );
	}
	else
	{
		pclSSheet->FormatStaticText( CD_HeatConsumer_SupplyTemp, iCurrentRow, CD_HeatConsumer_SupplyTemp, iCurrentRow, strSupplyTemp, SSS_ALIGN_RIGHT );
		pclSSheet->FormatStaticText( CD_HeatConsumer_ReturnTemp, iCurrentRow, CD_HeatConsumer_ReturnTemp, iCurrentRow, strReturnTemp, SSS_ALIGN_RIGHT );
	}

	_CheckHeatConsumerColumnWidth();
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

void CDlgWizardPM_RightViewInput_HeatingConsumers::_FindConsumerID( CSSheet * plcSSheet, int row, CString &strID )
{
	strID = _T( "" );
	TCHAR strComboValue[256];
	int iCurrentSelection = plcSSheet->ComboBoxSendMessage( CD_HeatConsumer_Consumer, row, SS_CBM_GETCURSEL, 0, 0 );
	plcSSheet->ComboBoxSendMessage( CD_HeatConsumer_Consumer, row, SS_CBM_GETLBTEXT, iCurrentSelection, ( LPARAM )strComboValue );

	for( std::map<int, CDB_StringID *>::iterator mapIter = m_mapConsumerType.begin(); mapIter != m_mapConsumerType.end(); ++mapIter )
	{
		if( CString( mapIter->second->GetString() ).CompareNoCase( strComboValue ) == 0 )
		{
			strID = mapIter->second->GetIDPtr().ID;
			return;
		}
	}
}

void CDlgWizardPM_RightViewInput_HeatingConsumers::_LoadHeatConsumer()
{
	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pclSelectPMList 
			|| NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERT_RETURN;
	}

	CPMInputUser *pclPMInputUser = m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser();
	
	for( int i = 0; i < pclPMInputUser->GetHeatConsumersList()->GetConsumersCount(); i++ )
	{
		_AddHeatConsumer( 
				m_pclSSheetConsumer,
				pclPMInputUser->GetHeatConsumersList()->GetConsumers( i )->m_strConsumersTypeID,
				CDimValue::SItoCU( _U_TH_POWER, pclPMInputUser->GetHeatConsumersList()->GetConsumers( i )->m_dHeatCapacity ),
				CDimValue::SItoCU( _U_VOLUME, pclPMInputUser->GetHeatConsumersList()->GetConsumers( i )->m_dContent ),
				pclPMInputUser->GetHeatConsumersList()->GetConsumers( i )->m_bManualContent,
				CDimValue::SItoCU( _U_TEMPERATURE, pclPMInputUser->GetHeatConsumersList()->GetConsumers( i )->m_dSupplyTemp ),
				CDimValue::SItoCU( _U_TEMPERATURE, pclPMInputUser->GetHeatConsumersList()->GetConsumers( i )->m_dReturnTemp ),
				pclPMInputUser->GetHeatConsumersList()->GetConsumers( i )->m_bManualTemp,
				true );
	}

	_UpdateHeatConsumer();
}

void CDlgWizardPM_RightViewInput_HeatingConsumers::_UpdateHeatConsumer()
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
		_FindConsumerID( m_pclSSheetConsumer, i + RD_HeatConsumer_FirstAvailRow, strConsumerID );

		// Get Capacity in SI
		TCHAR strCapacity[16];
		double dCapacity;
		m_pclSSheetConsumer->GetValue( CD_HeatConsumer_Capacity, i + RD_HeatConsumer_FirstAvailRow, strCapacity );
		ReadCUDoubleFromStr( _U_TH_POWER, strCapacity, &dCapacity );

		double dContent = -1.0;
		double dSupplyTemp = pclPMInputUser->GetSupplyTemperature();
		double dReturnTemp = pclPMInputUser->GetReturnTemperature();
		bool bManualContent = _IsCheckBoxChecked( m_pclSSheetConsumer, CD_HeatConsumer_ManualContent, i + RD_HeatConsumer_FirstAvailRow );
		bool bManualTemp = _IsCheckBoxChecked( m_pclSSheetConsumer, CD_HeatConsumer_ManualTemp, i + RD_HeatConsumer_FirstAvailRow );

		if( true == bManualContent )
		{
			// If in manual input then get the Content in SI.
			TCHAR strContent[16];
			m_pclSSheetConsumer->GetValue( CD_HeatConsumer_Content, i + RD_HeatConsumer_FirstAvailRow, strContent );
			ReadCUDoubleFromStr( _U_VOLUME, strContent, &dContent );
			m_pclSSheetConsumer->SetBackColor( CD_HeatConsumer_Content, i + RD_HeatConsumer_FirstAvailRow, _WHITE );
		}

		if( true == bManualTemp )
		{
			// If in manual input then get the Content in SI.
			TCHAR strSupplyTemp[16];
			TCHAR strReturnTemp[16];
			m_pclSSheetConsumer->GetValue( CD_HeatConsumer_SupplyTemp, i + RD_HeatConsumer_FirstAvailRow, strSupplyTemp );
			m_pclSSheetConsumer->GetValue( CD_HeatConsumer_ReturnTemp, i + RD_HeatConsumer_FirstAvailRow, strReturnTemp );
			ReadCUDoubleFromStr( _U_TEMPERATURE, strSupplyTemp, &dSupplyTemp );
			ReadCUDoubleFromStr( _U_TEMPERATURE, strReturnTemp, &dReturnTemp );
			m_pclSSheetConsumer->SetBackColor( CD_HeatConsumer_SupplyTemp, i + RD_HeatConsumer_FirstAvailRow, _WHITE );
			m_pclSSheetConsumer->SetBackColor( CD_HeatConsumer_ReturnTemp, i + RD_HeatConsumer_FirstAvailRow, _WHITE );
		}

		// Update database.
		pclPMInputUser->GetHeatConsumersList()->UpdateConsumers( iRecordIndex, strConsumerID, dCapacity, dContent, dSupplyTemp, dReturnTemp, bManualTemp );

		if( false == bManualContent )
		{
			// If not in manual input then refresh Content in spreadsheet
			dContent = pclPMInputUser->GetHeatConsumersList()->GetConsumers( iRecordIndex )->m_dContent;
			m_pclSSheetConsumer->FormatStaticText( CD_HeatConsumer_Content,
												   i + RD_HeatConsumer_FirstAvailRow,
												   CD_HeatConsumer_Content,
												   i + RD_HeatConsumer_FirstAvailRow,
												   WriteCUDouble( _U_VOLUME, dContent ),
												   SSS_ALIGN_RIGHT );

			m_pclSSheetConsumer->SetBackColor( CD_HeatConsumer_Content, i + RD_HeatConsumer_FirstAvailRow, _LIGHTGRAY );
		}

		if( false == bManualTemp )
		{
			// If not in manual input then refresh Supply Temperature in spreadsheet.
			m_pclSSheetConsumer->FormatStaticText( CD_HeatConsumer_SupplyTemp,
												   i + RD_HeatConsumer_FirstAvailRow,
												   CD_HeatConsumer_SupplyTemp,
												   i + RD_HeatConsumer_FirstAvailRow,
												   WriteCUDouble( _U_TEMPERATURE, pclPMInputUser->GetSupplyTemperature() ),
												   SSS_ALIGN_RIGHT );

			m_pclSSheetConsumer->SetBackColor( CD_HeatConsumer_SupplyTemp, i + RD_HeatConsumer_FirstAvailRow, _LIGHTGRAY );

			// If not in manual input then refresh Return Temperature in spreadsheet.
			m_pclSSheetConsumer->FormatStaticText( CD_HeatConsumer_ReturnTemp,
												   i + RD_HeatConsumer_FirstAvailRow,
												   CD_HeatConsumer_ReturnTemp,
												   i + RD_HeatConsumer_FirstAvailRow,
												   WriteCUDouble( _U_TEMPERATURE, pclPMInputUser->GetReturnTemperature() ),
												   SSS_ALIGN_RIGHT );

			m_pclSSheetConsumer->SetBackColor( CD_HeatConsumer_ReturnTemp, i + RD_HeatConsumer_FirstAvailRow, _LIGHTGRAY );
		}
	}

	_UpdateStaticValue();
}

void CDlgWizardPM_RightViewInput_HeatingConsumers::_UpdateHeatConsumerValues()
{
	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pclSelectPMList 
			|| NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERT_RETURN;
	}

	CPMInputUser *pclPMInputUser = m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser();

	for( int i = 0; i < m_iConsumerCount; i++ )
	{
		// Update capacity.
		double dHeatCapacity = pclPMInputUser->GetHeatConsumersList()->GetConsumers( i )->m_dHeatCapacity;
		CString str = WriteCUDouble( _U_TH_POWER, dHeatCapacity );
		m_pclSSheetConsumer->SetValue( CD_HeatConsumer_Capacity, i + RD_HeatConsumer_FirstAvailRow, str );

		// Update content.
		double dContent = pclPMInputUser->GetHeatConsumersList()->GetConsumers( i )->m_dContent;
		str = WriteCUDouble( _U_VOLUME, dContent );
		m_pclSSheetConsumer->SetValue( CD_HeatConsumer_Content, i + RD_HeatConsumer_FirstAvailRow, str );

		// Update supply temperature.
		double dSupplyTemperature = pclPMInputUser->GetHeatConsumersList()->GetConsumers( i )->m_dSupplyTemp;
		str = WriteCUDouble( _U_TEMPERATURE, dSupplyTemperature );
		m_pclSSheetConsumer->SetValue( CD_HeatConsumer_SupplyTemp, i + RD_HeatConsumer_FirstAvailRow, str );

		// Update return temperature.
		double dReturnTemperature = pclPMInputUser->GetHeatConsumersList()->GetConsumers( i )->m_dReturnTemp;
		str = WriteCUDouble( _U_TEMPERATURE, dReturnTemperature );
		m_pclSSheetConsumer->SetValue( CD_HeatConsumer_ReturnTemp, i + RD_HeatConsumer_FirstAvailRow, str );
	}
}

bool CDlgWizardPM_RightViewInput_HeatingConsumers::_ButtonPlusConsumerEnabled()
{
	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pclSelectPMList 
			|| NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERTA_RETURN ( false );
	}

	CPMInputUser *pclPMInputUser = m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser();
	
	// avoid to add a new consumer if some input are invalid
	// not valid :
	// * content or capacity <= 0
	// * NAN
	// * Empty Cell
	bool bEnable = true;
	int iLastDataRow = m_iConsumerCount + RD_HeatConsumer_FirstAvailRow - 1;
	WORD border = SS_BORDERTYPE_LEFT | SS_BORDERTYPE_TOP | SS_BORDERTYPE_RIGHT | SS_BORDERTYPE_BOTTOM;

	for( int i = iLastDataRow; i >= RD_HeatConsumer_FirstAvailRow; i-- )
	{
		std::wstring strContent = m_pclSSheetConsumer->GetCellText( CD_HeatConsumer_Content, i );
		double dContent = 0.0;

		// Border need to be removed if (manual && good) or (auto).
		m_pclSSheetConsumer->SetCellBorder( CD_HeatConsumer_Content, i, false );

		if( RD_OK != ReadCUDoubleFromStr( _U_TH_POWER, strContent, &dContent ) || dContent <= 0.0 )
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

		if( RD_OK != ReadCUDoubleFromStr( _U_VOLUME, strCapacity, &dCapacity ) || dCapacity <= 0.0 )
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

		// HYS - 1131: We compared the supply temperature of the TU with the return temp.of the system.This is wrong, we have to compare with the return temp.of the TU
		if( RD_OK != ReadCUDoubleFromStr( _U_TEMPERATURE, strSupplyTemp, &dSupplyTemp ) || dSupplyTemp > pclPMInputUser->GetSupplyTemperature() )
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

		pclPMInputUser->GetHeatConsumersList()->VerifyConsHeatingTemperatureValues( dSupplyTemp, dReturnTemp, &pbSupplyTempOK, &pbReturnTempOK );

		if( pbSupplyTempOK == false )
		{
			// red border to help the user to find errors
			m_pclSSheetConsumer->SetCellBorder( CD_HeatConsumer_SupplyTemp, i, CD_HeatConsumer_SupplyTemp, i, true, border, SS_BORDERSTYLE_SOLID, _TAH_RED_LIGHT );
			bEnable = false;
		}

		if( pbReturnTempOK == false )
		{
			// red border to help the user to find errors
			m_pclSSheetConsumer->SetCellBorder( CD_HeatConsumer_ReturnTemp, i, CD_HeatConsumer_ReturnTemp, i, true, border, SS_BORDERSTYLE_SOLID, _TAH_RED_LIGHT );
			bEnable = false;
		}
	}

	if( true == bEnable )
	{
		m_pclSSheetConsumer->SetBackColor( CD_HeatConsumer_FirstColumn,
										   RD_HeatConsumer_FirstAvailRow + m_iConsumerCount,
										   CD_HeatConsumer_LastColumn,
										   RD_HeatConsumer_FirstAvailRow + m_iConsumerCount,
										   _WHITE );
	}
	else
	{
		m_pclSSheetConsumer->SetBackColor( CD_HeatConsumer_FirstColumn,
										   RD_HeatConsumer_FirstAvailRow + m_iConsumerCount,
										   CD_HeatConsumer_LastColumn,
										   RD_HeatConsumer_FirstAvailRow + m_iConsumerCount,
										   _TAH_GREY_XLIGHT );
	}

	return m_bConsumerValid = bEnable;
}

void CDlgWizardPM_RightViewInput_HeatingConsumers::_CheckHeatConsumerColumnWidth()
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

void CDlgWizardPM_RightViewInput_HeatingConsumers::_CheckHeatConsumerSheetHeight()
{
	CRect rectClient;
	GetClientRect( &rectClient );

	CRect rectButtonReset;
	GetDlgItem( IDC_RESET )->GetWindowRect( &rectButtonReset);
	ScreenToClient( &rectButtonReset );

	CRect rectSheetClient;
	GetDlgItem( IDC_SSHEATCONSLIST )->GetWindowRect( &rectSheetClient );
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

bool CDlgWizardPM_RightViewInput_HeatingConsumers::_IsCheckBoxChecked( CSSheet *pclSSheet, SS_COORD col, SS_COORD row )
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

void CDlgWizardPM_RightViewInput_HeatingConsumers::_VerifyResetButtons()
{
	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pclSelectPMList 
			|| NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERT_RETURN;
	}
	
	CPMInputUser *pclPMInputUser = m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser();

	BOOL bResetEnable = TRUE;

	if( 0 == pclPMInputUser->GetHeatConsumersList()->GetConsumersCount() 
			&& 0 == pclPMInputUser->GetPipeList()->GetPipeCount() )
	{
		bResetEnable = FALSE;
	}

	GetDlgItem( IDC_RESET )->EnableWindow( bResetEnable );

	BOOL bResetAllEnable = TRUE;

	if( 0 == pclPMInputUser->GetHeatGeneratorList()->GetHeatGeneratorCount() 
			&& 0 == pclPMInputUser->GetHeatConsumersList()->GetConsumersCount() 
			&& 0 == pclPMInputUser->GetPipeList()->GetPipeCount() )
	{
		bResetAllEnable = FALSE;
	}

	GetDlgItem( IDC_RESETALL )->EnableWindow( bResetAllEnable );
}
