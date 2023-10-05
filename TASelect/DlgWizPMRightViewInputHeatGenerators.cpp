#include "stdafx.h"
#include "TASelect.h"
#include "TASelectDoc.h"
#include "MainFrm.h"
#include "Global.h"
#include "SSheet.h"
#include "Select.h"
#include "SelectPM.h"
#include "RViewDescription.h"
#include "DlgWizPMRightViewInputHeatGenerators.h"
#include "DlgIndSelPMSysVolHeating.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


CDlgWizardPM_RightViewInput_HeatingGenerators::CDlgWizardPM_RightViewInput_HeatingGenerators( CWizardSelPMParams *pclWizardSelPMParams, CWnd *pclParent )
	: CDlgWizardPM_RightViewInput_Base( pclWizardSelPMParams, HeatingGenerators, IDD, pclParent )
{
	m_bChangesDone = false;
	m_pclSDescGenerator = NULL;
	m_pclSSheetGenerator = NULL;
	m_rectSheetInitialSize.SetRectEmpty();
	m_iHeatGeneratorCount = 0;
	m_bGeneratorValid = true;

	m_HeatGeneratorRectClient = CRect( 0, 0, 0, 0 );
	m_bGoToNormalModeMsgSent = false;
	m_bInitialized = false;
}

CDlgWizardPM_RightViewInput_HeatingGenerators::~CDlgWizardPM_RightViewInput_HeatingGenerators()
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

void CDlgWizardPM_RightViewInput_HeatingGenerators::FillPMInputUser()
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
		double dTotalContent = pclPMInputUser->GetHeatGeneratorList()->GetTotalContent();
		dTotalContent += pclPMInputUser->GetHeatConsumersList()->GetTotalContent( pclPMInputUser->GetSupplyTemperature(), pclPMInputUser->GetReturnTemperature() );
		dTotalContent += pclPMInputUser->GetPipeList()->GetTotalContent();
		pclPMInputUser->SetSystemVolume( dTotalContent );

		pclPMInputUser->SetInstalledPower( pclPMInputUser->GetHeatGeneratorList()->GetTotalHeatCapacity() );
	}
}

BOOL CDlgWizardPM_RightViewInput_HeatingGenerators::PreTranslateMessage( MSG *pMsg )
{
	EnableNextButton( m_bGeneratorValid );

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
	}

	return __super::PreTranslateMessage( pMsg ); // allow default behavior (return TRUE if you want to discard message)
}

BEGIN_MESSAGE_MAP( CDlgWizardPM_RightViewInput_HeatingGenerators, CDlgWizardPM_RightViewInput_Base )
	ON_WM_LBUTTONDOWN()
	ON_BN_CLICKED( IDC_CHECKGENPROTECT, OnBnClickedCheckGenProtect )
	ON_BN_CLICKED( IDC_RESET, OnBnClickedReset )
	ON_BN_CLICKED( IDC_RESETALL, OnBnClickedResetAll )
	ON_BN_CLICKED( IDC_BUTTON_CANCEL, OnBnClickedCancel )
	ON_MESSAGE( SSM_COMBOCLOSEUP, OnComboDropCloseUp )
	ON_MESSAGE( SSM_CLICK, OnSSClick )
	ON_MESSAGE( SSM_DBLCLK, OnSSDblClick )
	ON_MESSAGE( SSM_EDITCHANGE, OnSSEditChange )
END_MESSAGE_MAP()

void CDlgWizardPM_RightViewInput_HeatingGenerators::DoDataExchange( CDataExchange *pDX )
{
	CDlgWizardPM_RightViewInput_Base::DoDataExchange( pDX );

	DDX_Control( pDX, IDC_STATICTOTALGENHEATCAPACITY, m_StaticTotalGeneratorHeatCapacity );
	DDX_Control( pDX, IDC_STATICTOTALGENCONTENT, m_StaticTotalGeneratorContent );
	DDX_Control( pDX, IDC_STATICTOTALINSTALL, m_StaticTotalInstallation );
	DDX_Control( pDX, IDC_STATICTOTALCONTENT, m_StaticTotalContent );
	DDX_Control( pDX, IDC_CHECKGENPROTECT, m_ButtonHeatGeneratorProtect );
	DDX_Control( pDX, IDC_STATIC_GONORMALMODE2, m_ButtonGoNormalMode );
}

BOOL CDlgWizardPM_RightViewInput_HeatingGenerators::OnInitDialog()
{
	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pclSelectPMList 
			|| NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERTA_RETURN( FALSE );
	}

	CPMInputUser *pclPMInputUser = m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser();

	CDlgWizardPM_RightViewInput_Base::OnInitDialog();
	
	SetMainTitle( TASApp.LoadLocalizedString( IDS_DLGWIZPM_RVIEWHEATGENERATORS_MAINTITLE ) );

	CString str;

	//////////////////////////////////////////////////////////////////////////
	// CHECKBOX

	m_ButtonHeatGeneratorProtect.SetCheck( pclPMInputUser->GetHeatGeneratorList()->GetHeatHeneratorProtect() );
	
	//////////////////////////////////////////////////////////////////////////
	// STATIC

	// Add individual heat generator protection:
	str = TASApp.LoadLocalizedString( IDS_DLGWIZPM_RVIEWHEATGENERATORS_CHECKGENPROTECT );
	m_ButtonHeatGeneratorProtect.SetWindowText( str );

	// Total installation content (Generators and consumers):
	str = TASApp.LoadLocalizedString( IDS_DLGWIZPM_RVIEWHEATGENERATORS_TOTALCONTENT );
	m_StaticTotalInstallation.SetTextAndToolTip( str );

	m_StaticTotalGeneratorHeatCapacity.SetBackColor( RGB( 255, 255, 255 ) );
	m_StaticTotalGeneratorContent.SetBackColor( RGB( 255, 255, 255 ) );
	m_StaticTotalInstallation.SetBackColor( RGB( 255, 255, 255 ) );
	m_StaticTotalContent.SetBackColor( RGB( 255, 255, 255 ) );

	// Set some static text in bold.
	m_StaticTotalGeneratorHeatCapacity.SetFontBold( true );
	m_StaticTotalGeneratorContent.SetFontBold( true );

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
	_LoadHeatGenerator();
	_ButtonPlusGeneratorEnabled();
	_UpdateStaticValue();

	// Vessel computing for heat protection are not well done.
	m_ButtonHeatGeneratorProtect.EnableWindow( FALSE );
	m_ButtonHeatGeneratorProtect.ShowWindow( SW_HIDE );

	m_bInitialized = true;

	return TRUE;
}

void CDlgWizardPM_RightViewInput_HeatingGenerators::OnPaint()
{
	CDlgWizardPM_RightViewInput_Base::OnPaint();

	DrawGoAdvancedNormalModeButtton( Draw_NormalMode, m_ButtonGoNormalModePos );
}

void CDlgWizardPM_RightViewInput_HeatingGenerators::OnSize( UINT nType, int cx, int cy )
{
	CDlgWizardPM_RightViewInput_Base::OnSize( nType, cx, cy );

	if( NULL == GetDlgItem( IDC_SSHEATGENLIST ) || NULL == GetDlgItem( IDC_SSHEATGENLIST )->GetSafeHwnd() )
	{
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
	_CheckHeatGeneratorSheetHeight();
	_CheckHeatGeneratorColumnWidth();

	CRect rectSheet;
	GetDlgItem( IDC_SSHEATGENLIST )->GetWindowRect( &rectSheet );
	ScreenToClient( &rectSheet );

	// Resize separator below sheet.
	CRect rectSeparatorBelowSheet;
	GetDlgItem( IDC_STATICBELOWSHEET )->GetClientRect( &rectSeparatorBelowSheet );
	int iNewY = rectSheet.bottom + 1;
	GetDlgItem( IDC_STATICBELOWSHEET )->SetWindowPos( NULL, rectSheet.left, iNewY, rectSheet.Width(), 2, SWP_NOZORDER );

	// Move total heat generators capacity.
	CRect rectTotalHeatGeneratorCapacity;
	m_StaticTotalGeneratorHeatCapacity.GetWindowRect( &rectTotalHeatGeneratorCapacity );
	ScreenToClient( &rectTotalHeatGeneratorCapacity );
	CRect rectCell = m_pclSSheetGenerator->GetCellCoordInPixel( CD_HeatGenerator_Capacity, RD_HeatGenerator_UnitName );
	int iNewX = 20 + rectCell.right - rectTotalHeatGeneratorCapacity.Width();
	iNewY += 4;
	m_StaticTotalGeneratorHeatCapacity.SetWindowPos( NULL, iNewX, iNewY, -1, -1, SWP_NOSIZE | SWP_NOZORDER );

	// Move total heat generators content.
	CRect rectTotalHeatGeneratorContent;
	m_StaticTotalGeneratorContent.GetWindowRect( &rectTotalHeatGeneratorContent );
	ScreenToClient( &rectTotalHeatGeneratorContent );
	rectCell = m_pclSSheetGenerator->GetCellCoordInPixel( CD_HeatGenerator_Content, RD_HeatGenerator_UnitName );
	iNewX = 20 + rectCell.right - rectTotalHeatGeneratorContent.Width();
	m_StaticTotalGeneratorContent.SetWindowPos( NULL, iNewX, iNewY, -1, -1, SWP_NOSIZE | SWP_NOZORDER );

	// Move separator.
	CRect rectSeparator;
	GetDlgItem( IDC_STATIC )->GetWindowRect( &rectSeparator );
	ScreenToClient( &rectSeparator );
	iNewX = rectClient.Width() - 20 - rectSeparator.Width();
	iNewY += rectTotalHeatGeneratorCapacity.Height() + 20;
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

void CDlgWizardPM_RightViewInput_HeatingGenerators::OnLButtonDown( UINT nFlags, CPoint point )
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

void CDlgWizardPM_RightViewInput_HeatingGenerators::OnBnClickedCheckGenProtect()
{
	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pclSelectPMList 
			|| NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERT_RETURN;
	}

	CPMInputUser *pclPMInputUser = m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser();

	m_bChangesDone = true;
	CButton *pcCheckGenProtect = ( CButton * )GetDlgItem( IDC_CHECKGENPROTECT );
	UINT uiCheck = pcCheckGenProtect->GetCheck();

	if( uiCheck == BST_CHECKED )
	{
		pclPMInputUser->GetHeatGeneratorList()->SetHeatGeneratorProtect( true );
	}
	else
	{
		pclPMInputUser->GetHeatGeneratorList()->SetHeatGeneratorProtect( false );
	}
}

void CDlgWizardPM_RightViewInput_HeatingGenerators::OnBnClickedReset()
{
	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pclSelectPMList 
			|| NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERT_RETURN;
	}

	CPMInputUser *pclPMInputUser = m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser();

	if( 0 != pclPMInputUser->GetHeatGeneratorList()->GetHeatGeneratorCount() 
			|| 0 != pclPMInputUser->GetPipeList()->GetPipeCount() )
	{
		CString str = TASApp.LoadLocalizedString( AFXMSG_INDSELPMSYSVOLHEATING_RESETALL );

		if( IDYES == ::AfxMessageBox( str, MB_YESNO | MB_ICONQUESTION ) )
		{
			pclPMInputUser->GetHeatGeneratorList()->Reset();
			_UpdateStaticValue();

			SS_CELLCOORD cellCoord;
			cellCoord.Col = 1;
			cellCoord.Row = RD_HeatGenerator_FirstAvailRow;

			while( m_iHeatGeneratorCount )
			{
				_RemoveHeatGenerator( &cellCoord );
			}

			m_ButtonHeatGeneratorProtect.SetCheck( BST_UNCHECKED );

			m_bChangesDone = false;
			_ButtonPlusGeneratorEnabled();
		}
	}
}

void CDlgWizardPM_RightViewInput_HeatingGenerators::OnBnClickedResetAll()
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
			cellCoord.Row = RD_HeatGenerator_FirstAvailRow;

			while( m_iHeatGeneratorCount )
			{
				_RemoveHeatGenerator( &cellCoord );
			}

			m_ButtonHeatGeneratorProtect.SetCheck( BST_UNCHECKED );

			m_bChangesDone = false;
			_ButtonPlusGeneratorEnabled();
		}
	}
}

void CDlgWizardPM_RightViewInput_HeatingGenerators::OnBnClickedCancel()
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

LRESULT CDlgWizardPM_RightViewInput_HeatingGenerators::OnComboDropCloseUp( WPARAM wParam, LPARAM lParam )
{
	_UpdateHeatGenerator();
	return 0;
}

LRESULT CDlgWizardPM_RightViewInput_HeatingGenerators::OnSSClick( WPARAM wParam, LPARAM lParam )
{
	SS_CELLCOORD *pclCellCoord = ( SS_CELLCOORD * )lParam;
	int iRecordIndex = pclCellCoord->Row - RD_HeatGenerator_FirstAvailRow;

	CSheetDescription *pclSDesc = m_ViewDescription.GetFromSSheetID( UINT( wParam ) );
	CSSheet *pclSSheet = pclSDesc->GetSSheetPointer();

	// Intercept click on the first column and below first row only.
	if( ( pclCellCoord->Col != CD_HeatGenerator_FirstColumn && pclCellCoord->Col != CD_HeatGenerator_Manual )
			|| pclCellCoord->Row < RD_HeatGenerator_FirstAvailRow )
	{
		return 0;
	}

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
		return 1; // event processed
	}
	else if( CD_HeatGenerator_Manual == pclCellCoord->Col && iRecordIndex < m_iHeatGeneratorCount ) // manual clicked
	{
		m_bChangesDone = true;

		if( true == _IsCheckBoxChecked( m_pclSSheetGenerator, pclCellCoord->Col, pclCellCoord->Row ) )
		{
			pclSSheet->SetCellText( CD_HeatGenerator_Manual, pclCellCoord->Row, _T("0") );
			pclSSheet->SetActiveCell( CD_HeatGenerator_Capacity, pclCellCoord->Row );
			pclSSheet->FormatStaticText( CD_HeatGenerator_Content, pclCellCoord->Row, CD_HeatGenerator_Content, pclCellCoord->Row, _T( "" ), SSS_ALIGN_RIGHT );
		}
		else
		{
			pclSSheet->SetCellText( CD_HeatGenerator_Manual, pclCellCoord->Row, _T("1") );
			pclSSheet->SetActiveCell( CD_HeatGenerator_Content, pclCellCoord->Row );
			pclSSheet->FormatEditDouble( CD_HeatGenerator_Content, pclCellCoord->Row, _T( "" ), SSS_ALIGN_RIGHT, true );
			SS_CELLTYPE rCellType;
			pclSSheet->SetTypeEdit( &rCellType, SSS_ALIGN_RIGHT, 15, SS_CHRSET_ALPHANUM, SS_CASE_NOCASE );
			pclSSheet->SetCellType( CD_HeatGenerator_Content, pclCellCoord->Row, &rCellType );
		}

		_UpdateHeatGenerator();
		_ButtonPlusGeneratorEnabled();
		return 1;
	}

	return 0;
}

LRESULT CDlgWizardPM_RightViewInput_HeatingGenerators::OnSSDblClick( WPARAM wParam, LPARAM lParam )
{
	SS_CELLCOORD *pclCellCoord = ( SS_CELLCOORD * )lParam;

	// Avoid weird behavior on checkbox.
	if( pclCellCoord->Col == CD_HeatGenerator_Manual )
	{
		return 1;
	}

	return 0;
}

LRESULT CDlgWizardPM_RightViewInput_HeatingGenerators::OnSSEditChange( WPARAM wParam, LPARAM lParam )
{
	CSheetDescription *pclSDesc = m_ViewDescription.GetFromSSheetID( UINT( wParam ) );
	CSSheet *pclSSheet = pclSDesc->GetSSheetPointer();

	SS_CELLCOORD *pclCellCoord = ( SS_CELLCOORD * )lParam;

	// Intercept below first row only.
	if( pclCellCoord->Row < RD_HeatGenerator_FirstAvailRow )
	{
		return 0;
	}

	// Intercept click on the needed columns.
	if( pclCellCoord->Col != CD_HeatGenerator_Capacity && pclCellCoord->Col != CD_HeatGenerator_Content )
	{
		return 0;
	}

	CString strValue = pclSSheet->GetCellText( pclCellCoord->Col, pclCellCoord->Row );
	double dValue;

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

	_UpdateHeatGenerator();
	_ButtonPlusGeneratorEnabled();

	return 0;
}

LRESULT CDlgWizardPM_RightViewInput_HeatingGenerators::OnUnitChange( WPARAM wParam, LPARAM lParam )
{
	CDlgWizardPM_RightViewInput_Base::OnUnitChange( wParam, lParam );

	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();

	// Update units for the columns in the sheet.
	m_pclSSheetGenerator->SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );
	m_pclSSheetGenerator->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontSize, ( LPARAM )8 );
	m_pclSSheetGenerator->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, ( LPARAM )_TAH_TITLE_MAIN );
	m_pclSSheetGenerator->SetStaticText( CD_HeatGenerator_Capacity, RD_HeatGenerator_UnitName, GetNameOf( pUnitDB->GetDefaultUnit( _U_TH_POWER ) ).c_str() );
	m_pclSSheetGenerator->SetStaticText( CD_HeatGenerator_Content, RD_HeatGenerator_UnitName, GetNameOf( pUnitDB->GetDefaultUnit( _U_VOLUME ) ).c_str() );

	m_pclSSheetGenerator->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );

	// Update all value in the sheet.
	_UpdateHeatGeneratorValues();

	// Update units in the statics.
	_UpdateStaticValue();

	return 0;
}

void CDlgWizardPM_RightViewInput_HeatingGenerators::_InitializeSSheet( void )
{
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();

	// Pay attention: because in this case I use TSpread directly as a custom control (see IDD_DLGPIPEDP in the resource), if you want to
	// use 'CSSheet' possibility, we have to attach TSpread CWnd to CSSheet newly created. And the destruction of the dialog, don't forget
	// to call 'Detach' before deleting 'CSSheet'.
	m_pclSSheetGenerator = new CSSheet();
	m_pclSSheetGenerator->Attach( GetDlgItem( IDC_SSHEATGENLIST )->GetSafeHwnd() );
	m_pclSSheetGenerator->Reset();

	// Save initial size.
	m_pclSSheetGenerator->GetClientRect( &m_rectSheetInitialSize );

	m_pclSDescGenerator = m_ViewDescription.AddSheetDescription( SD_HeatGenerator, -1, m_pclSSheetGenerator, CPoint( 0, 0 ) );

	if( NULL == m_pclSDescGenerator )
	{
		return;
	}

	m_pclSSheetGenerator->SetBool( SSB_ALLOWDRAGDROP, FALSE );
	m_pclSSheetGenerator->SetBool( SSB_HORZSCROLLBAR, FALSE );
	m_pclSSheetGenerator->SetBool( SSB_VERTSCROLLBAR, TRUE );
	m_pclSSheetGenerator->SetBool( SSB_MOVEACTIVEONFOCUS, FALSE );
	m_pclSSheetGenerator->SetBool( SSB_EDITMODEPERMANENT, TRUE );
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
	m_pclSDescGenerator->AddColumn( CD_HeatGenerator_Manual, 1 );

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
	m_pclSSheetGenerator->SetStaticText( CD_HeatGenerator_Manual, RD_HeatGenerator_ColName, _T( "" ) );
	m_pclSSheetGenerator->SetPictureCellWithID( IDI_PENCILDIAGSHORT, CD_HeatGenerator_Manual, RD_HeatGenerator_UnitName, CSSheet::PictureCellType::Icon );
	m_pclSSheetGenerator->SetBackColor( CD_HeatGenerator_Manual, RD_HeatGenerator_UnitName, _TAH_ORANGE );
	
	// To fill an icon.
	m_pclSSheetGenerator->SetPictureCellWithID( IDI_PLUSGREEN, CD_HeatGenerator_FirstColumn, RD_HeatGenerator_FirstAvailRow, CSSheet::PictureCellType::Icon );

	m_pclSSheetGenerator->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
}

void CDlgWizardPM_RightViewInput_HeatingGenerators::_UpdateStaticValue( void )
{
	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pclSelectPMList 
			|| NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERT_RETURN;
	}

	CPMInputUser *pclPMInputUser = m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser();

	double dTotalGenHeatCapacity = pclPMInputUser->GetHeatGeneratorList()->GetTotalHeatCapacity();
	m_StaticTotalGeneratorHeatCapacity.SetWindowText( WriteCUDouble( _U_TH_POWER, dTotalGenHeatCapacity, true ) );
	double dTotalGenContent = pclPMInputUser->GetHeatGeneratorList()->GetTotalContent();
	m_StaticTotalGeneratorContent.SetWindowText( WriteCUDouble( _U_VOLUME, dTotalGenContent, true ) );
	
	// HYS-869
	if( dTotalGenHeatCapacity < pclPMInputUser->GetHeatConsumersList()->GetTotalHeatCapacity() )
	{
		m_StaticTotalGeneratorHeatCapacity.SetTextColor( _ORANGE );
		m_StaticTotalGeneratorHeatCapacity.SetToolTip( TASApp.LoadLocalizedString( IDS_INDSELPMSYSVOL_TOTALCAPACITY_TT ) );
	}
	else
	{
		m_StaticTotalGeneratorHeatCapacity.ResetToolTip();
		m_StaticTotalGeneratorHeatCapacity.SetTextColor( _BLACK );
	}

	double dTotalContent = pclPMInputUser->GetHeatGeneratorList()->GetTotalContent();
	dTotalContent += pclPMInputUser->GetHeatConsumersList()->GetTotalContent( pclPMInputUser->GetSupplyTemperature(), pclPMInputUser->GetReturnTemperature() );
	dTotalContent += pclPMInputUser->GetPipeList()->GetTotalContent();
	m_StaticTotalContent.SetWindowText( WriteCUDouble( _U_VOLUME, dTotalContent, true ) );

	_VerifyResetButtons();
}

void CDlgWizardPM_RightViewInput_HeatingGenerators::_RemoveHeatGenerator( SS_CELLCOORD *plcCellCoord )
{
	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pclSelectPMList 
			|| NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERT_RETURN;
	}

	CPMInputUser *pclPMInputUser = m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser();

	m_pclSSheetGenerator->SetRedraw( FALSE );
	m_iHeatGeneratorCount--;
	m_pclSSheetGenerator->DelRow( plcCellCoord->Row ); // drop a row
	m_pclSSheetGenerator->SetMaxRows( RD_HeatGenerator_FirstAvailRow + m_iHeatGeneratorCount ); // Set max rows.
	pclPMInputUser->GetHeatGeneratorList()->RemoveHeatGenerator( plcCellCoord->Row - RD_HeatGenerator_FirstAvailRow );
	m_bChangesDone = true;

	_CheckHeatGeneratorColumnWidth();
	m_pclSSheetGenerator->SetRedraw( TRUE );

	// Manually call 'OnSize' method to well move all the components.
	CRect rectClient;
	GetClientRect( &rectClient );
	OnSize( SIZE_RESTORED, rectClient.Width(), rectClient.Height() );

	// To force a paint refresh.
	Invalidate();
	UpdateWindow();
}

void CDlgWizardPM_RightViewInput_HeatingGenerators::_AddHeatGenerator( CSSheet *pclSSheet, CString pstrType, double dCapacity, double dContent,
		bool bManual, bool bOnlyGUI )
{
	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pclSelectPMList 
			|| NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERT_RETURN;
	}

	CPMInputUser *pclPMInputUser = m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser();

	m_iHeatGeneratorCount++;
	int iCurrentRow = RD_HeatGenerator_FirstAvailRow + m_iHeatGeneratorCount - 1;

	// Set max rows.
	pclSSheet->SetRedraw( FALSE );
	pclSSheet->SetMaxRows( RD_HeatGenerator_FirstAvailRow + m_iHeatGeneratorCount );

	// Add plus icon.
	pclSSheet->SetPictureCellWithID( IDI_PLUSGREEN, CD_HeatGenerator_FirstColumn, RD_HeatGenerator_FirstAvailRow + m_iHeatGeneratorCount,
									 CSSheet::PictureCellType::Icon );

	// Add checkbox.
	pclSSheet->SetCheckBox( CD_HeatGenerator_Manual, iCurrentRow, _T( "" ), false, true );

	// Add trash icon.
	pclSSheet->SetPictureCellWithID( IDI_TRASH, CD_HeatGenerator_FirstColumn, iCurrentRow, CSSheet::PictureCellType::Icon );

	// Add generator combo.
	pclSSheet->FormatComboList( CD_HeatGenerator_Generator, iCurrentRow );
	_FillHeatGeneratorCombo( pclSSheet, CD_HeatGenerator_Generator, iCurrentRow, pstrType );

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
		pclPMInputUser->GetHeatGeneratorList()->AddHeatGenerator( strGeneratorID, dCapacity, dContent );
		m_bChangesDone = true;
	}

	CString strContent;
	strContent.Format( _T("%g"), dContent );

	if( bManual ) // Manual input
	{
		pclSSheet->SetCellText( CD_HeatGenerator_Manual, iCurrentRow, _T("1") );
		pclSSheet->FormatEditDouble( CD_HeatGenerator_Content, iCurrentRow, strContent, SSS_ALIGN_RIGHT );
		pclSSheet->SetCellType( CD_HeatGenerator_Content, iCurrentRow, &rCellType );
	}
	else
	{
		pclSSheet->FormatStaticText( CD_HeatGenerator_Content, iCurrentRow, CD_HeatGenerator_Content, iCurrentRow, strContent, SSS_ALIGN_RIGHT );
	}

	_CheckHeatGeneratorColumnWidth();
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

void CDlgWizardPM_RightViewInput_HeatingGenerators::_FillHeatGeneratorCombo( CSSheet *pclSheet, long lCol, long lRow, const TCHAR *strSelect )
{
	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pTADB )
	{
		ASSERT_RETURN;
	}

	if( NULL == pclSheet || lCol < 1 || lCol > pclSheet->GetMaxCols() || lRow < 1 || lRow > pclSheet->GetMaxRows() )
	{
		return;
	}

	if( m_mapGeneratorType.empty() )
	{
		CTable *pclTable = dynamic_cast<CTable *>( m_pclWizardSelPMParams->m_pTADB->Get( _T("PMHEATGENERATOR_TAB") ).MP );

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

	if( m_mapGeneratorType.empty() )
	{
		return;
	}

	pclSheet->ComboBoxSendMessage( lCol, lRow, SS_CBM_RESETCONTENT, 0, 0 );
	int iCount = 0;
	int iSelectIndex = 0;

	for( std::map<int, CDB_StringID *>::iterator mapIter = m_mapGeneratorType.begin(); mapIter != m_mapGeneratorType.end(); ++mapIter )
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

void CDlgWizardPM_RightViewInput_HeatingGenerators::_FindGeneratorID( CSSheet *plcSSheet, int row, CString &strID )
{
	strID = _T( "" );
	TCHAR strComboValue[256];
	int iCurrentSelection = plcSSheet->ComboBoxSendMessage( CD_HeatGenerator_Generator, row, SS_CBM_GETCURSEL, 0, 0 );
	plcSSheet->ComboBoxSendMessage( CD_HeatGenerator_Generator, row, SS_CBM_GETLBTEXT, iCurrentSelection, ( LPARAM )strComboValue );

	for( std::map<int, CDB_StringID *>::iterator mapIter = m_mapGeneratorType.begin(); mapIter != m_mapGeneratorType.end(); ++mapIter )
	{
		if( CString( mapIter->second->GetString() ).CompareNoCase( strComboValue ) == 0 )
		{
			strID = mapIter->second->GetIDPtr().ID;
			return;
		}
	}
}

bool CDlgWizardPM_RightViewInput_HeatingGenerators::_IsCheckBoxChecked( CSSheet *pclSSheet, SS_COORD col, SS_COORD row )
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

void CDlgWizardPM_RightViewInput_HeatingGenerators::_LoadHeatGenerator()
{
	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pclSelectPMList 
			|| NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERT_RETURN;
	}

	CPMInputUser *pclPMInputUser = m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser();

	for( int i = 0; i < pclPMInputUser->GetHeatGeneratorList()->GetHeatGeneratorCount(); i++ )
	{
		_AddHeatGenerator( 
				m_pclSSheetGenerator,
				pclPMInputUser->GetHeatGeneratorList()->GetHeatGenerator( i )->m_strHeatGeneratorTypeID,
				CDimValue::SItoCU( _U_TH_POWER, pclPMInputUser->GetHeatGeneratorList()->GetHeatGenerator( i )->m_dHeatCapacity ),
				CDimValue::SItoCU( _U_VOLUME, pclPMInputUser->GetHeatGeneratorList()->GetHeatGenerator( i )->m_dContent ),
				pclPMInputUser->GetHeatGeneratorList()->GetHeatGenerator( i )->m_bManual,
				true );
	}

	_UpdateHeatGenerator();
}

void CDlgWizardPM_RightViewInput_HeatingGenerators::_UpdateHeatGenerator()
{
	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pclSelectPMList 
			|| NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERT_RETURN;
	}

	CPMInputUser *pclPMInputUser = m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser();

	for( int i = 0; i < m_iHeatGeneratorCount; i++ )
	{
		// Get generator ID.
		CString strGeneratorID;
		_FindGeneratorID( m_pclSSheetGenerator, i + RD_HeatGenerator_FirstAvailRow, strGeneratorID );

		// Get Capacity in SI.
		TCHAR strCapacity[16];
		double dCapacity;
		m_pclSSheetGenerator->GetValue( CD_HeatGenerator_Capacity, i + RD_HeatGenerator_FirstAvailRow, strCapacity );
		ReadCUDoubleFromStr( _U_TH_POWER, strCapacity, &dCapacity );

		double dContent = -1.0;
		bool bManual = _IsCheckBoxChecked( m_pclSSheetGenerator, CD_HeatGenerator_Manual, i + RD_HeatGenerator_FirstAvailRow );

		if( true == bManual )
		{
			// If in manual input then get the Content in SI.
			TCHAR strContent[16];
			m_pclSSheetGenerator->GetValue( CD_HeatGenerator_Content, i + RD_HeatGenerator_FirstAvailRow, strContent );
			ReadCUDoubleFromStr( _U_VOLUME, strContent, &dContent );
			m_pclSSheetGenerator->SetBackColor( CD_HeatGenerator_Content, i + RD_HeatGenerator_FirstAvailRow, _WHITE );
		}

		// Update database.
		pclPMInputUser->GetHeatGeneratorList()->UpdateHeatGenerator( i, strGeneratorID, dCapacity, dContent );

		if( false == bManual )
		{
			// If not in manual input then refresh Content in spreadsheet.
			dContent = pclPMInputUser->GetHeatGeneratorList()->GetHeatGenerator( i )->m_dContent;
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

void CDlgWizardPM_RightViewInput_HeatingGenerators::_UpdateHeatGeneratorValues()
{
	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pclSelectPMList 
			|| NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERT_RETURN;
	}

	CPMInputUser *pclPMInputUser = m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser();

	for( int i = 0; i < m_iHeatGeneratorCount; i++ )
	{
		// Update capacity.
		double dHeatCapacity = pclPMInputUser->GetHeatGeneratorList()->GetHeatGenerator( i )->m_dHeatCapacity;
		CString str = WriteCUDouble( _U_TH_POWER, dHeatCapacity );
		m_pclSSheetGenerator->SetValue( CD_HeatGenerator_Capacity, i + RD_HeatGenerator_FirstAvailRow, str );

		// Update content.
		double dContent = pclPMInputUser->GetHeatGeneratorList()->GetHeatGenerator( i )->m_dContent;
		str = WriteCUDouble( _U_VOLUME, dContent );
		m_pclSSheetGenerator->SetValue( CD_HeatGenerator_Content, i + RD_HeatGenerator_FirstAvailRow, str );
	}
}

bool CDlgWizardPM_RightViewInput_HeatingGenerators::_ButtonPlusGeneratorEnabled()
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

		// border need to be removed if (manual && good) or (auto)
		m_pclSSheetGenerator->SetCellBorder( CD_HeatGenerator_Content, i, false );

		if( RD_OK != ReadCUDoubleFromStr( _U_TH_POWER, strContent, &dContent ) || dContent <= 0.0 )
		{
			if( _IsCheckBoxChecked( m_pclSSheetGenerator, CD_HeatGenerator_Manual, i ) )
			{
				// red border to help the user to find errors
				m_pclSSheetGenerator->SetCellBorder( CD_HeatGenerator_Content, i, CD_HeatGenerator_Content, i, true, border, SS_BORDERSTYLE_SOLID, _TAH_RED_LIGHT );
			}

			bEnable = false;
		}

		std::wstring strCapacity = m_pclSSheetGenerator->GetCellText( CD_HeatGenerator_Capacity, i );
		double dCapacity = 0.0;

		if( RD_OK != ReadCUDoubleFromStr( _U_VOLUME, strCapacity, &dCapacity ) || dCapacity <= 0.0 )
		{
			// red border to help the user to find errors
			m_pclSSheetGenerator->SetCellBorder( CD_HeatGenerator_Capacity, i, CD_HeatGenerator_Capacity, i, true, border, SS_BORDERSTYLE_SOLID, _TAH_RED_LIGHT );
			bEnable = false;
		}
		else
		{
			// remove border if input is valid
			m_pclSSheetGenerator->SetCellBorder( CD_HeatGenerator_Capacity, i, false );
		}
	}

	if( bEnable )
	{
		m_pclSSheetGenerator->SetBackColor( CD_HeatGenerator_FirstColumn,
											RD_HeatGenerator_FirstAvailRow + m_iHeatGeneratorCount,
											CD_HeatGenerator_LastColumn,
											RD_HeatGenerator_FirstAvailRow + m_iHeatGeneratorCount,
											_WHITE );
	}
	else
	{
		m_pclSSheetGenerator->SetBackColor( CD_HeatGenerator_FirstColumn,
											RD_HeatGenerator_FirstAvailRow + m_iHeatGeneratorCount,
											CD_HeatGenerator_LastColumn,
											RD_HeatGenerator_FirstAvailRow + m_iHeatGeneratorCount,
											_TAH_GREY_XLIGHT );
	}

	return m_bGeneratorValid = bEnable;
}

void CDlgWizardPM_RightViewInput_HeatingGenerators::_CheckHeatGeneratorColumnWidth()
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
	
	m_pclSSheetGenerator->SetColWidthInPixels( CD_HeatGenerator_Manual, m_HeatGeneratorRectClient.Width() - lTotalWidthInPixel );
}

void CDlgWizardPM_RightViewInput_HeatingGenerators::_CheckHeatGeneratorSheetHeight()
{
	CRect rectClient;
	GetClientRect( &rectClient );

	CRect rectButtonReset;
	GetDlgItem( IDC_RESET )->GetWindowRect( &rectButtonReset);
	ScreenToClient( &rectButtonReset );

	CRect rectSheetClient;
	GetDlgItem( IDC_SSHEATGENLIST )->GetWindowRect( &rectSheetClient );
	ScreenToClient( &rectSheetClient );
	
	CRect rectSheetSize = m_pclSSheetGenerator->GetSheetSizeInPixels();
	
	if( rectSheetSize.Height() > rectSheetClient.Height() )
	{
		// Sheet with a new row becomes higher than the sheet client area.
		int iOffsetY = rectSheetSize.Height() - rectSheetClient.Height();

		if( m_ButtonGoNormalModePos.bottom + iOffsetY > rectButtonReset.top - 5 )
		{
			iOffsetY = rectButtonReset.top - 5 - m_ButtonGoNormalModePos.bottom;
		}

		m_pclSSheetGenerator->SetWindowPos( NULL, -1, -1, rectClient.Width() - 40, rectSheetClient.Height() + iOffsetY, SWP_NOMOVE | SWP_NOZORDER );
		m_ButtonGoNormalModePos.OffsetRect( 0, iOffsetY );
	}
	else if( rectSheetSize.Height() < rectSheetClient.Height() )
	{
		// Sheet with a row removed becomes taller than the sheet client area.
		int iHeight = max( m_rectSheetInitialSize.Height(), rectSheetSize.Height() );
		m_pclSSheetGenerator->SetWindowPos( NULL, -1, -1, rectClient.Width() - 40, iHeight, SWP_NOMOVE | SWP_NOZORDER );
		m_ButtonGoNormalModePos.OffsetRect( 0, -( rectSheetClient.Height() - iHeight ) );
	}
	else if( m_ButtonGoNormalModePos.bottom > rectButtonReset.top - 5 )
	{
		// Check the limit. After that the sheet has a vertical scrollbar.
		int iOffsetY = m_ButtonGoNormalModePos.bottom - rectButtonReset.top + 5;
		m_pclSSheetGenerator->SetWindowPos( NULL, -1, -1, rectClient.Width() - 40, rectSheetClient.Height() - iOffsetY, SWP_NOMOVE | SWP_NOZORDER );
		m_ButtonGoNormalModePos.OffsetRect( 0, -iOffsetY );
	}
	else
	{
		// Nothing. We just adapt the width.
		m_pclSSheetGenerator->SetWindowPos( NULL, -1, -1, rectClient.Width() - 40, rectSheetClient.Height(), SWP_NOMOVE | SWP_NOZORDER );
	}
}

void CDlgWizardPM_RightViewInput_HeatingGenerators::_VerifyResetButtons()
{
	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pclSelectPMList 
			|| NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERT_RETURN;
	}

	CPMInputUser *pclPMInputUser = m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser();

	BOOL bResetEnable = TRUE;

	if( 0 == pclPMInputUser->GetHeatGeneratorList()->GetHeatGeneratorCount() )
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
