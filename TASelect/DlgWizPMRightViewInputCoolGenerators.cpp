#include "stdafx.h"
#include "TASelect.h"
#include "TASelectDoc.h"
#include "MainFrm.h"
#include "Global.h"
#include "SSheet.h"
#include "Select.h"
#include "SelectPM.h"
#include "RViewDescription.h"
#include "DlgWizPMRightViewInputCoolGenerators.h"
#include "DlgIndSelPMSysVolCooling.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


CDlgWizardPM_RightViewInput_CoolingGenerators::CDlgWizardPM_RightViewInput_CoolingGenerators( CWizardSelPMParams *pclWizardSelPMParams, CWnd *pclParent )
	: CDlgWizardPM_RightViewInput_Base( pclWizardSelPMParams, CoolingGenerators, IDD, pclParent )
{
	m_bChangesDone = false;
	m_pclSDescGenerator = NULL;
	m_pclSSheetGenerator = NULL;
	m_rectSheetInitialSize.SetRectEmpty();
	m_iColdGeneratorCount = 0;
	m_bGeneratorValid = true;

	m_ColdGeneratorRectClient = CRect( 0, 0, 0, 0 );
	m_bGoToNormalModeMsgSent = false;
	m_bInitialized = false;
}

CDlgWizardPM_RightViewInput_CoolingGenerators::~CDlgWizardPM_RightViewInput_CoolingGenerators()
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

void CDlgWizardPM_RightViewInput_CoolingGenerators::FillPMInputUser()
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

BOOL CDlgWizardPM_RightViewInput_CoolingGenerators::PreTranslateMessage( MSG *pMsg )
{
	EnableNextButton( m_bGeneratorValid );

	if( WM_LBUTTONDOWN == pMsg->message )
	{
		// PWU : The first click on the spread is ignored if the focus is elsewhere.
		// This code catch the first click and send it directly to the spread.
		CRect rectColdGen;
		GetDlgItem( IDC_SSCOLDGENLIST )->GetWindowRect( &rectColdGen );

		CRect rectCurrentFocus;
		CWnd *pFocusedCtrl = GetFocus();

		if( NULL != pFocusedCtrl )
		{
			pFocusedCtrl->GetWindowRect( rectCurrentFocus );
		}

		CRect rectIntersectColdGen;
		rectIntersectColdGen.IntersectRect( rectCurrentFocus, rectColdGen );

		if( TRUE == rectColdGen.PtInRect( pMsg->pt ) && TRUE == rectIntersectColdGen.IsRectNull() )
		{
			CSheetDescription *pclSDesc = m_ViewDescription.GetFromSSheetID( IDC_SSCOLDGENLIST );
			CSSheet *plcSSheet = pclSDesc->GetSSheetPointer();
			plcSSheet->SendMessage( pMsg->message, pMsg->wParam, pMsg->lParam );
			GetDlgItem( IDC_SSCOLDGENLIST )->SetFocus();
		}
	}

	return __super::PreTranslateMessage( pMsg ); // allow default behavior (return TRUE if you want to discard message)
}

BEGIN_MESSAGE_MAP( CDlgWizardPM_RightViewInput_CoolingGenerators, CDlgWizardPM_RightViewInput_Base )
	ON_WM_LBUTTONDOWN()
	ON_BN_CLICKED( IDC_RESET, OnBnClickedReset )
	ON_BN_CLICKED( IDC_RESETALL, OnBnClickedResetAll )
	ON_BN_CLICKED( IDC_BUTTON_CANCEL, OnBnClickedCancel )
	ON_MESSAGE( SSM_COMBOCLOSEUP, OnComboDropCloseUp )
	ON_MESSAGE( SSM_CLICK, OnSSClick )
	ON_MESSAGE( SSM_DBLCLK, OnSSDblClick )
	ON_MESSAGE( SSM_EDITCHANGE, OnSSEditChange )
	ON_MESSAGE( SSM_TEXTTIPFETCH, OnTextTipFetch )
END_MESSAGE_MAP()

void CDlgWizardPM_RightViewInput_CoolingGenerators::DoDataExchange( CDataExchange *pDX )
{
	CDlgWizardPM_RightViewInput_Base::DoDataExchange( pDX );

	DDX_Control( pDX, IDC_STATICTOTALGENCOOLCAPACITY, m_StaticTotalGeneratorColdCapacity );
	DDX_Control( pDX, IDC_STATICTOTALGENCONTENT, m_StaticTotalGeneratorContent );
	DDX_Control( pDX, IDC_STATICTOTALINSTALL, m_StaticTotalInstallation );
	DDX_Control( pDX, IDC_STATICTOTALCONTENT, m_StaticTotalContent );
	DDX_Control( pDX, IDC_STATIC_GONORMALMODE2, m_ButtonGoNormalMode );
}

BOOL CDlgWizardPM_RightViewInput_CoolingGenerators::OnInitDialog()
{
	CDlgWizardPM_RightViewInput_Base::OnInitDialog();
	
	SetMainTitle( TASApp.LoadLocalizedString( IDS_DLGWIZPM_RVIEWCOLDGENERATORS_MAINTITLE ) );

	CString str;

	//////////////////////////////////////////////////////////////////////////
	// STATIC

	// Total installation content (Generators and consumers):
	str = TASApp.LoadLocalizedString( IDS_DLGWIZPM_RVIEWCOLDGENERATORS_TOTALCONTENT );
	m_StaticTotalInstallation.SetTextAndToolTip( str );

	m_StaticTotalGeneratorColdCapacity.SetBackColor( RGB( 255, 255, 255 ) );
	m_StaticTotalGeneratorContent.SetBackColor( RGB( 255, 255, 255 ) );
	m_StaticTotalInstallation.SetBackColor( RGB( 255, 255, 255 ) );
	m_StaticTotalContent.SetBackColor( RGB( 255, 255, 255 ) );

	// Set some static text in bold.
	m_StaticTotalGeneratorColdCapacity.SetFontBold( true );
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
	_LoadColdGenerator();
	_ButtonPlusGeneratorEnabled();
	_UpdateStaticValue();

	m_bInitialized = true;

	return TRUE;
}

void CDlgWizardPM_RightViewInput_CoolingGenerators::OnPaint()
{
	CDlgWizardPM_RightViewInput_Base::OnPaint();

	DrawGoAdvancedNormalModeButtton( Draw_NormalMode, m_ButtonGoNormalModePos );
}

void CDlgWizardPM_RightViewInput_CoolingGenerators::OnSize( UINT nType, int cx, int cy )
{
	CDlgWizardPM_RightViewInput_Base::OnSize( nType, cx, cy );

	if( NULL == GetDlgItem( IDC_SSCOLDGENLIST ) || NULL == GetDlgItem( IDC_SSCOLDGENLIST )->GetSafeHwnd() )
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
	_CheckColdGeneratorSheetHeight();
	_CheckColdGeneratorColumnWidth();

	CRect rectSheet;
	GetDlgItem( IDC_SSCOLDGENLIST )->GetWindowRect( &rectSheet );
	ScreenToClient( &rectSheet );

	// Resize separator below sheet.
	CRect rectSeparatorBelowSheet;
	GetDlgItem( IDC_STATICBELOWSHEET )->GetClientRect( &rectSeparatorBelowSheet );
	int iNewY = rectSheet.bottom + 1;
	GetDlgItem( IDC_STATICBELOWSHEET )->SetWindowPos( NULL, rectSheet.left, iNewY, rectSheet.Width(), 2, SWP_NOZORDER );

	// Move total cold generators capacity.
	CRect rectTotalColdGeneratorCapacity;
	m_StaticTotalGeneratorColdCapacity.GetWindowRect( &rectTotalColdGeneratorCapacity );
	ScreenToClient( &rectTotalColdGeneratorCapacity );
	CRect rectCell = m_pclSSheetGenerator->GetCellCoordInPixel( CD_ColdGenerator_Capacity, RD_ColdGenerator_UnitName );
	int iNewX = 20 + rectCell.right - rectTotalColdGeneratorCapacity.Width();
	iNewY += 4;
	m_StaticTotalGeneratorColdCapacity.SetWindowPos( NULL, iNewX, iNewY, -1, -1, SWP_NOSIZE | SWP_NOZORDER );

	// Move total cold generators content.
	CRect rectTotalColdGeneratorContent;
	m_StaticTotalGeneratorContent.GetWindowRect( &rectTotalColdGeneratorContent );
	ScreenToClient( &rectTotalColdGeneratorContent );
	rectCell = m_pclSSheetGenerator->GetCellCoordInPixel( CD_ColdGenerator_Content, RD_ColdGenerator_UnitName );
	iNewX = 20 + rectCell.right - rectTotalColdGeneratorContent.Width();
	m_StaticTotalGeneratorContent.SetWindowPos( NULL, iNewX, iNewY, -1, -1, SWP_NOSIZE | SWP_NOZORDER );

	// Move separator.
	CRect rectSeparator;
	GetDlgItem( IDC_STATIC )->GetWindowRect( &rectSeparator );
	ScreenToClient( &rectSeparator );
	iNewX = rectClient.Width() - 20 - rectSeparator.Width();
	iNewY += rectTotalColdGeneratorCapacity.Height() + 20;
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

void CDlgWizardPM_RightViewInput_CoolingGenerators::OnLButtonDown( UINT nFlags, CPoint point )
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

void CDlgWizardPM_RightViewInput_CoolingGenerators::OnBnClickedReset()
{
	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pclSelectPMList 
			|| NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERT_RETURN;
	}

	CPMInputUser *pclPMInputUser = m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser();

	if( 0 != pclPMInputUser->GetColdGeneratorList()->GetColdGeneratorCount() 
			|| 0 != pclPMInputUser->GetPipeList()->GetPipeCount() )
	{
		CString str = TASApp.LoadLocalizedString( IDS_DLGWIZPM_RVIEWCOOLGENERATORS_RESET );

		if( IDYES == ::AfxMessageBox( str, MB_YESNO | MB_ICONQUESTION ) )
		{
			pclPMInputUser->GetColdGeneratorList()->Reset();
			_UpdateStaticValue();

			SS_CELLCOORD cellCoord;
			cellCoord.Col = 1;
			cellCoord.Row = RD_ColdGenerator_FirstAvailRow;

			while( m_iColdGeneratorCount )
			{
				_RemoveColdGenerator( &cellCoord );
			}

			m_bChangesDone = false;
			_ButtonPlusGeneratorEnabled();
		}
	}
}

void CDlgWizardPM_RightViewInput_CoolingGenerators::OnBnClickedResetAll()
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
			cellCoord.Row = RD_ColdGenerator_FirstAvailRow;

			while( m_iColdGeneratorCount )
			{
				_RemoveColdGenerator( &cellCoord );
			}

			m_bChangesDone = false;
			_ButtonPlusGeneratorEnabled();
		}
	}
}

void CDlgWizardPM_RightViewInput_CoolingGenerators::OnBnClickedCancel()
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

LRESULT CDlgWizardPM_RightViewInput_CoolingGenerators::OnComboDropCloseUp( WPARAM wParam, LPARAM lParam )
{
	_UpdateColdGenerator();
	return 0;
}

LRESULT CDlgWizardPM_RightViewInput_CoolingGenerators::OnSSClick( WPARAM wParam, LPARAM lParam )
{
	SS_CELLCOORD *pclCellCoord = (SS_CELLCOORD *)lParam;
	int iRecordIndex = pclCellCoord->Row - RD_ColdGenerator_FirstAvailRow;

	CSheetDescription *pclSDesc = m_ViewDescription.GetFromSSheetID( UINT( wParam ) );
	CSSheet *pclSSheet = pclSDesc->GetSSheetPointer();

	// Intercept click on the first column and below first row only.
	if( ( pclCellCoord->Col != CD_ColdGenerator_FirstColumn && pclCellCoord->Col != CD_ColdGenerator_Manual && pclCellCoord->Col != CD_ColdGenerator_ManualTemp )
			|| pclCellCoord->Row < RD_ColdGenerator_FirstAvailRow )
	{
		return 0;
	}

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
		return 1; // event processed
	}
	else if( CD_ColdGenerator_Manual == pclCellCoord->Col && iRecordIndex < m_iColdGeneratorCount ) // manual clicked
	{
		m_bChangesDone = true;

		if( true == _IsCheckBoxChecked( m_pclSSheetGenerator, pclCellCoord->Col, pclCellCoord->Row ) )
		{
			pclSSheet->SetCellText( CD_ColdGenerator_Manual, pclCellCoord->Row, _T("0") );
			pclSSheet->SetActiveCell( CD_ColdGenerator_Capacity, pclCellCoord->Row );
			pclSSheet->FormatStaticText( CD_ColdGenerator_Content, pclCellCoord->Row, CD_ColdGenerator_Content, pclCellCoord->Row, _T( "" ), SSS_ALIGN_RIGHT );
		}
		else
		{
			pclSSheet->SetCellText( CD_ColdGenerator_Manual, pclCellCoord->Row, _T("1") );
			pclSSheet->SetActiveCell( CD_ColdGenerator_Content, pclCellCoord->Row );
			pclSSheet->FormatEditDouble( CD_ColdGenerator_Content, pclCellCoord->Row, _T( "" ), SSS_ALIGN_RIGHT, true );
			SS_CELLTYPE rCellType;
			pclSSheet->SetTypeEdit( &rCellType, SSS_ALIGN_RIGHT, 15, SS_CHRSET_ALPHANUM, SS_CASE_NOCASE );
			pclSSheet->SetCellType( CD_ColdGenerator_Content, pclCellCoord->Row, &rCellType );
		}

		_UpdateColdGenerator();
		_ButtonPlusGeneratorEnabled();
		return 1;
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
		return 1;
	}

	return 0;
}

LRESULT CDlgWizardPM_RightViewInput_CoolingGenerators::OnSSDblClick( WPARAM wParam, LPARAM lParam )
{
	SS_CELLCOORD *pclCellCoord = ( SS_CELLCOORD * )lParam;

	// Avoid weird behavior on checkbox.
	if( CD_ColdGenerator_Manual == pclCellCoord->Col || CD_ColdGenerator_ManualTemp == pclCellCoord->Col )
	{
		return 1;
	}

	return 0;
}

LRESULT CDlgWizardPM_RightViewInput_CoolingGenerators::OnSSEditChange( WPARAM wParam, LPARAM lParam )
{
	CSheetDescription *pclSDesc = m_ViewDescription.GetFromSSheetID( UINT( wParam ) );
	CSSheet *pclSSheet = pclSDesc->GetSSheetPointer();

	SS_CELLCOORD *pclCellCoord = ( SS_CELLCOORD * )lParam;

	// Intercept below first row only.
	if( pclCellCoord->Row < RD_ColdGenerator_FirstAvailRow )
	{
		return 0;
	}

	// Intercept click on the editable columns.
	if( pclCellCoord->Col != CD_ColdGenerator_Capacity && pclCellCoord->Col != CD_ColdGenerator_Content 
		&& pclCellCoord->Col != CD_ColdGenerator_SupplyTemp && pclCellCoord->Col != CD_ColdGenerator_Quantity )
	{
		return 0;
	}

	CString strValue = pclSSheet->GetCellText( pclCellCoord->Col, pclCellCoord->Row );
	double dValue;

	// HYS-1307: Enable to use "-" sign for temperature
	bool bIsForTemperature = false;
	if( CD_ColdGenerator_SupplyTemp == pclCellCoord->Col )
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

	if( CD_ColdGenerator_Quantity == pclCellCoord->Col )
	{
		dValue = static_cast<unsigned long>( dValue );
		CString strLong;
		strLong.Format( _T("%d"), static_cast<unsigned long>( dValue ) );

		if( 0 != strLong.CompareNoCase( strValue ) )
		{
			pclSSheet->SetCellText( pclCellCoord->Col, pclCellCoord->Row, strLong );
		}
	}

	_UpdateColdGenerator();
	_ButtonPlusGeneratorEnabled();

	return 0;
}

LRESULT CDlgWizardPM_RightViewInput_CoolingGenerators::OnTextTipFetch( WPARAM wParam, LPARAM lParam )
{
	SS_TEXTTIPFETCH *pTextTipFetch = (SS_TEXTTIPFETCH *)lParam;

	CString str;
	LRESULT ReturnValue = 0;

	CPMInputUser *pclPMInputUser = m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser();

	WORD borderType = SS_BORDERTYPE_LEFT | SS_BORDERTYPE_TOP | SS_BORDERTYPE_RIGHT | SS_BORDERTYPE_BOTTOM;
	WORD borderStyle = SS_BORDERSTYLE_SOLID;
	COLORREF borderColor = _TAH_RED_LIGHT;
	WORD border;
	WORD style;
	COLORREF color;

	std::wstring strGenSupplyTemp = m_pclSSheetGenerator->GetCellText( CD_ColdGenerator_SupplyTemp, pTextTipFetch->Row );
	double dGenSupplyTemp = 0.0;

	if( pTextTipFetch->Col == CD_ColdGenerator_SupplyTemp )
	{
		if( TRUE == m_pclSSheetGenerator->GetBorder( pTextTipFetch->Col, pTextTipFetch->Row, &border, &style, &color )
				&& ( border == borderType && style == borderStyle && color == _TAH_RED_LIGHT ) )
		{
			// HYS-1612: pay attention that all temperatures read from string are in custom unit!
			if( RD_OK == ReadCUDoubleFromStr( _U_TEMPERATURE, strGenSupplyTemp, &dGenSupplyTemp )
					&& dGenSupplyTemp < pclPMInputUser->GetSupplyTemperature() )
			{
				str = TASApp.LoadLocalizedString( IDS_INDSELPMSYSVOL_TSGTSSYSL_TT );
			}
		}
	}

	if( false == str.IsEmpty() )
	{
		pTextTipFetch->hText = NULL;
		pTextTipFetch->wMultiLine = SS_TT_MULTILINE_AUTO;
		pTextTipFetch->nWidth = (SHORT)m_pclSSheetGenerator->GetTipTextWidth( str );
		wcsncpy_s( pTextTipFetch->szText, SS_TT_TEXTMAX, (LPCTSTR)str, SS_TT_TEXTMAX );
		pTextTipFetch->fShow = true;
		ReturnValue = 0;
	}

	return ReturnValue;
}

LRESULT CDlgWizardPM_RightViewInput_CoolingGenerators::OnUnitChange( WPARAM wParam, LPARAM lParam )
{
	CDlgWizardPM_RightViewInput_Base::OnUnitChange( wParam, lParam );

	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();

	// Update units for the columns in the sheet.
	m_pclSSheetGenerator->SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );
	m_pclSSheetGenerator->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontSize, ( LPARAM )8 );
	m_pclSSheetGenerator->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, ( LPARAM )_TAH_TITLE_MAIN );
	m_pclSSheetGenerator->SetStaticText( CD_ColdGenerator_Capacity, RD_ColdGenerator_UnitName, GetNameOf( pUnitDB->GetDefaultUnit( _U_TH_POWER ) ).c_str() );
	m_pclSSheetGenerator->SetStaticText( CD_ColdGenerator_Content, RD_ColdGenerator_UnitName, GetNameOf( pUnitDB->GetDefaultUnit( _U_VOLUME ) ).c_str() );

	m_pclSSheetGenerator->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );

	// Update all value in the sheet.
	_UpdateColdGeneratorValues();

	// Update units in the statics.
	_UpdateStaticValue();

	return 0;
}

void CDlgWizardPM_RightViewInput_CoolingGenerators::_InitializeSSheet( void )
{
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();

	// Pay attention: because in this case I use TSpread directly as a custom control (see IDD_DLGPIPEDP in the resource), if you want to
	// use 'CSSheet' possibility, we have to attach TSpread CWnd to CSSheet newly created. And the destruction of the dialog, don't forget
	// to call 'Detach' before deleting 'CSSheet'.
	m_pclSSheetGenerator = new CSSheet();
	m_pclSSheetGenerator->Attach( GetDlgItem( IDC_SSCOLDGENLIST )->GetSafeHwnd() );
	m_pclSSheetGenerator->Reset();

	// Save initial size.
	m_pclSSheetGenerator->GetClientRect( &m_rectSheetInitialSize );

	m_pclSDescGenerator = m_ViewDescription.AddSheetDescription( SD_ColdGenerator, -1, m_pclSSheetGenerator, CPoint( 0, 0 ) );

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
	m_pclSSheetGenerator->SetMaxRows( RD_ColdGenerator_FirstAvailRow );
	m_pclSSheetGenerator->SetFreeze( 0, RD_ColdGenerator_FirstAvailRow - 1 );

	// All cells are static by default and filled with _T("").
	m_pclSSheetGenerator->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
	m_pclSSheetGenerator->FormatStaticText( -1, -1, -1, -1, _T( "" ) );

	// Add columns.
	m_pclSDescGenerator->AddColumn( CD_ColdGenerator_FirstColumn, 1 );
	m_pclSDescGenerator->AddColumn( CD_ColdGenerator_Generator, 1 );
	m_pclSDescGenerator->AddColumn( CD_ColdGenerator_Capacity, 1 );
	m_pclSDescGenerator->AddColumn( CD_ColdGenerator_Content, 1 );
	m_pclSDescGenerator->AddColumn( CD_ColdGenerator_Manual, 1 );
	m_pclSDescGenerator->AddColumn( CD_ColdGenerator_SupplyTemp, 1 );
	m_pclSDescGenerator->AddColumn( CD_ColdGenerator_ManualTemp, 1 );
	m_pclSDescGenerator->AddColumn( CD_ColdGenerator_Quantity, 1 );

	// Resize columns.
	_CheckColdGeneratorColumnWidth();

	// Row name.
	m_pclSSheetGenerator->SetTextPattern( CSSheet::_SSTextPattern::TitleGroup );
	m_pclSSheetGenerator->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontSize, ( LPARAM )8 );
	m_pclSSheetGenerator->SetTextPatternProperty( CSSheet::_SSTextPatternProperty::TPP_FontBackColor, ( LPARAM )_TAH_TITLE_MAIN );
	m_pclSSheetGenerator->SetStaticText( CD_ColdGenerator_FirstColumn, RD_ColdGenerator_ColName, _T( "" ) );
	m_pclSSheetGenerator->SetStaticText( CD_ColdGenerator_FirstColumn, RD_ColdGenerator_UnitName, _T( "" ) );
	m_pclSSheetGenerator->SetStaticText( CD_ColdGenerator_Generator, RD_ColdGenerator_ColName, IDS_INDSELPMSYSVOLCOOLING_GENERATORS );
	m_pclSSheetGenerator->SetStaticText( CD_ColdGenerator_Generator, RD_ColdGenerator_UnitName, _T( "" ) );
	m_pclSSheetGenerator->SetStaticText( CD_ColdGenerator_Capacity, RD_ColdGenerator_ColName, IDS_INDSELPMSYSVOLCOOLING_CAPACITY );
	m_pclSSheetGenerator->SetStaticText( CD_ColdGenerator_Capacity, RD_ColdGenerator_UnitName, GetNameOf( pUnitDB->GetDefaultUnit( _U_TH_POWER ) ).c_str() );
	m_pclSSheetGenerator->SetStaticText( CD_ColdGenerator_SupplyTemp, RD_ColdGenerator_ColName, IDS_INDSELPMSYSVOLCOOLING_SUPPLYTEMP );
	m_pclSSheetGenerator->SetStaticText( CD_ColdGenerator_SupplyTemp, RD_ColdGenerator_UnitName, GetNameOf( pUnitDB->GetDefaultUnit( _U_TEMPERATURE ) ).c_str() );
	
	// Manual temp checkbox.
	m_pclSSheetGenerator->SetStaticText( CD_ColdGenerator_ManualTemp, RD_ColdGenerator_ColName, _T( "" ) );
	m_pclSSheetGenerator->SetPictureCellWithID( IDI_PENCILDIAGSHORT, CD_ColdGenerator_ManualTemp, RD_ColdGenerator_UnitName, CSSheet::PictureCellType::Icon );
	m_pclSSheetGenerator->SetBackColor( CD_ColdGenerator_ManualTemp, RD_ColdGenerator_UnitName, _TAH_ORANGE );

	// Content.
	m_pclSSheetGenerator->SetStaticText( CD_ColdGenerator_Content, RD_ColdGenerator_ColName, IDS_INDSELPMSYSVOLHEATING_CONTENT );
	m_pclSSheetGenerator->SetStaticText( CD_ColdGenerator_Content, RD_ColdGenerator_UnitName, GetNameOf( pUnitDB->GetDefaultUnit( _U_VOLUME ) ).c_str() );
	
	// Manual content checkbox.
	m_pclSSheetGenerator->SetStaticText( CD_ColdGenerator_Manual, RD_ColdGenerator_ColName, _T( "" ) );
	m_pclSSheetGenerator->SetPictureCellWithID( IDI_PENCILDIAGSHORT, CD_ColdGenerator_Manual, RD_ColdGenerator_UnitName, CSSheet::PictureCellType::Icon );
	m_pclSSheetGenerator->SetBackColor( CD_ColdGenerator_Manual, RD_ColdGenerator_UnitName, _TAH_ORANGE );
	
	// Quantity.
	m_pclSSheetGenerator->SetStaticText( CD_ColdGenerator_Quantity, RD_ColdGenerator_ColName, IDS_SELP_QUANTITY );
	m_pclSSheetGenerator->SetStaticText( CD_ColdGenerator_Quantity, RD_ColdGenerator_UnitName, _T( "" ) );
	
	// To fill an icon.
	m_pclSSheetGenerator->SetPictureCellWithID( IDI_PLUSGREEN, CD_ColdGenerator_FirstColumn, RD_ColdGenerator_FirstAvailRow, CSSheet::PictureCellType::Icon );

	m_pclSSheetGenerator->SetTextPattern( CSSheet::_SSTextPattern::Arial8 );
}

void CDlgWizardPM_RightViewInput_CoolingGenerators::_UpdateStaticValue( void )
{
	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pclSelectPMList 
			|| NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERT_RETURN;
	}

	CPMInputUser *pclPMInputUser = m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser();

	double dTotalGenColdCapacity = pclPMInputUser->GetColdGeneratorList()->GetTotalColdCapacity();
	m_StaticTotalGeneratorColdCapacity.SetWindowText( WriteCUDouble( _U_TH_POWER, dTotalGenColdCapacity, true ) );
	double dTotalGenContent = pclPMInputUser->GetColdGeneratorList()->GetTotalContent();
	m_StaticTotalGeneratorContent.SetWindowText( WriteCUDouble( _U_VOLUME, dTotalGenContent, true ) );

	// HYS-869
	if( dTotalGenColdCapacity < pclPMInputUser->GetColdConsumersList()->GetTotalColdCapacity() )
	{
		m_StaticTotalGeneratorColdCapacity.SetTextColor( _ORANGE );
		m_StaticTotalGeneratorColdCapacity.SetToolTip( TASApp.LoadLocalizedString( IDS_INDSELPMSYSVOL_TOTALCAPACITY_TT ) );
	}
	else
	{
		m_StaticTotalGeneratorColdCapacity.ResetToolTip();
		m_StaticTotalGeneratorColdCapacity.SetTextColor( _BLACK );
	}

	double dTotalContent = pclPMInputUser->GetColdGeneratorList()->GetTotalContent();

	dTotalContent += pclPMInputUser->GetColdConsumersList()->GetTotalContent( pclPMInputUser->GetSupplyTemperature(), 
			pclPMInputUser->GetReturnTemperature() );

	dTotalContent += pclPMInputUser->GetPipeList()->GetTotalContent();
	m_StaticTotalContent.SetWindowText( WriteCUDouble( _U_VOLUME, dTotalContent, true ) );

	_VerifyResetButtons();
}

void CDlgWizardPM_RightViewInput_CoolingGenerators::_RemoveColdGenerator( SS_CELLCOORD *plcCellCoord )
{
	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pclSelectPMList 
			|| NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERT_RETURN;
	}

	CPMInputUser *pclPMInputUser = m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser();

	m_pclSSheetGenerator->SetRedraw( FALSE );
	m_iColdGeneratorCount--;
	m_pclSSheetGenerator->DelRow( plcCellCoord->Row ); // drop a row
	m_pclSSheetGenerator->SetMaxRows( RD_ColdGenerator_FirstAvailRow + m_iColdGeneratorCount ); // Set max rows.
	pclPMInputUser->GetColdGeneratorList()->RemoveColdGenerator( plcCellCoord->Row - RD_ColdGenerator_FirstAvailRow );
	m_bChangesDone = true;

	_CheckColdGeneratorColumnWidth();
	m_pclSSheetGenerator->SetRedraw( TRUE );

	// Manually call 'OnSize' method to well move all the components.
	CRect rectClient;
	GetClientRect( &rectClient );
	OnSize( SIZE_RESTORED, rectClient.Width(), rectClient.Height() );

	// To force a paint refresh.
	Invalidate();
	UpdateWindow();
}

void CDlgWizardPM_RightViewInput_CoolingGenerators::_AddColdGenerator( CSSheet *pclSSheet, CString pstrType, double dCapacity,
		double dContent, bool bManual, double dTempSupply, bool bManualTemp, long lQuantity, bool bOnlyGUI )
{
	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pclSelectPMList 
			|| NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERT_RETURN;
	}

	CPMInputUser *pclPMInputUser = m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser();

	m_iColdGeneratorCount++;
	int iCurrentRow = RD_ColdGenerator_FirstAvailRow + m_iColdGeneratorCount - 1;

	// Set max rows.
	pclSSheet->SetRedraw( FALSE );
	pclSSheet->SetMaxRows( RD_ColdGenerator_FirstAvailRow + m_iColdGeneratorCount );

	// Add plus icon.
	pclSSheet->SetPictureCellWithID( IDI_PLUSGREEN, CD_ColdGenerator_FirstColumn, RD_ColdGenerator_FirstAvailRow + m_iColdGeneratorCount,
									 CSSheet::PictureCellType::Icon );

	// Add checkbox.
	pclSSheet->SetCheckBox( CD_ColdGenerator_Manual, iCurrentRow, _T( "" ), false, true );
	pclSSheet->SetCheckBox( CD_ColdGenerator_ManualTemp, iCurrentRow, _T( "" ), false, true );

	// Add trash icon.
	pclSSheet->SetPictureCellWithID( IDI_TRASH, CD_ColdGenerator_FirstColumn, iCurrentRow, CSSheet::PictureCellType::Icon );

	// Add generator combo.
	pclSSheet->FormatComboList( CD_ColdGenerator_Generator, iCurrentRow );
	_FillColdGeneratorCombo( pclSSheet, CD_ColdGenerator_Generator, iCurrentRow, pstrType );

	// Add capacity edit.
	pclSSheet->FormatEditDouble( CD_ColdGenerator_Capacity, iCurrentRow, _T( "" ), SSS_ALIGN_RIGHT );
	
	CString strCapacity;
	strCapacity.Format( _T("%g"), dCapacity );
	pclSSheet->SetCellText( CD_ColdGenerator_Capacity, iCurrentRow, strCapacity );
	
	SS_CELLTYPE rCellType;
	pclSSheet->SetTypeEdit( &rCellType, SSS_ALIGN_RIGHT, 15, SS_CHRSET_ALPHANUM, SS_CASE_NOCASE );
	pclSSheet->SetCellType( CD_ColdGenerator_Capacity, iCurrentRow, &rCellType );

	CString strGeneratorID;
	_FindGeneratorID( pclSSheet, iCurrentRow, strGeneratorID );

	if( false == bOnlyGUI ) // Used to load data at dialog opening
	{
		pclPMInputUser->GetColdGeneratorList()->AddColdGenerator( strGeneratorID, dCapacity, dContent, pclPMInputUser->GetSupplyTemperature() );
		m_bChangesDone = true;
	}

	CString strContent;
	strContent.Format( _T("%g"), dContent );

	if( bManual ) // Manual input
	{
		pclSSheet->SetCellText( CD_ColdGenerator_Manual, iCurrentRow, _T("1") );
		pclSSheet->FormatEditDouble( CD_ColdGenerator_Content, iCurrentRow, strContent, SSS_ALIGN_RIGHT );
		pclSSheet->SetCellType( CD_ColdGenerator_Content, iCurrentRow, &rCellType );
	}
	else
	{
		pclSSheet->FormatStaticText( CD_ColdGenerator_Content, iCurrentRow, CD_ColdGenerator_Content, iCurrentRow, strContent, SSS_ALIGN_RIGHT );
	}

	CString strSupplyTemp;
	strSupplyTemp.Format( _T("%g"), bManualTemp ? dTempSupply : pclPMInputUser->GetSupplyTemperature() );

	if( true == bManualTemp ) // Manual Supply Temp input
	{
		pclSSheet->SetCellText( CD_ColdGenerator_ManualTemp, iCurrentRow, _T("1") );
		pclSSheet->FormatEditDouble( CD_ColdGenerator_SupplyTemp, iCurrentRow, strSupplyTemp, SSS_ALIGN_RIGHT );
		pclSSheet->SetCellType( CD_ColdGenerator_SupplyTemp, iCurrentRow, &rCellType );
	}
	else
	{
		pclSSheet->FormatStaticText( CD_ColdGenerator_SupplyTemp, iCurrentRow, CD_ColdGenerator_SupplyTemp, iCurrentRow, strSupplyTemp, SSS_ALIGN_RIGHT );
	}

	// Add quantity edit.
	pclSSheet->FormatEditDouble( CD_ColdGenerator_Quantity, iCurrentRow, _T( "" ), SSS_ALIGN_RIGHT );
	CString strQuantity;
	strQuantity.Format( _T("%d"), lQuantity );
	pclSSheet->SetCellText( CD_ColdGenerator_Quantity, iCurrentRow, strQuantity );
	pclSSheet->SetCellType( CD_ColdGenerator_Quantity, iCurrentRow, &rCellType );

	_CheckColdGeneratorColumnWidth();
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

void CDlgWizardPM_RightViewInput_CoolingGenerators::_FillColdGeneratorCombo( CSSheet *pclSheet, long lCol, long lRow, const TCHAR *strSelect )
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
		CTable *pclTable = dynamic_cast<CTable *>( m_pclWizardSelPMParams->m_pTADB->Get( _T("PMCOLDGENERATOR_TAB") ).MP );

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

void CDlgWizardPM_RightViewInput_CoolingGenerators::_FindGeneratorID( CSSheet *plcSSheet, int row, CString &strID )
{
	strID = _T( "" );
	TCHAR strComboValue[256];
	int iCurrentSelection = plcSSheet->ComboBoxSendMessage( CD_ColdGenerator_Generator, row, SS_CBM_GETCURSEL, 0, 0 );
	plcSSheet->ComboBoxSendMessage( CD_ColdGenerator_Generator, row, SS_CBM_GETLBTEXT, iCurrentSelection, ( LPARAM )strComboValue );

	for( std::map<int, CDB_StringID *>::iterator mapIter = m_mapGeneratorType.begin(); mapIter != m_mapGeneratorType.end(); ++mapIter )
	{
		if( CString( mapIter->second->GetString() ).CompareNoCase( strComboValue ) == 0 )
		{
			strID = mapIter->second->GetIDPtr().ID;
			return;
		}
	}
}

bool CDlgWizardPM_RightViewInput_CoolingGenerators::_IsCheckBoxChecked( CSSheet *pclSSheet, SS_COORD col, SS_COORD row )
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

void CDlgWizardPM_RightViewInput_CoolingGenerators::_LoadColdGenerator()
{
	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pclSelectPMList 
			|| NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERT_RETURN;
	}

	CPMInputUser *pclPMInputUser = m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser();

	for( int i = 0; i < pclPMInputUser->GetColdGeneratorList()->GetColdGeneratorCount(); i++ )
	{
		_AddColdGenerator( 
				m_pclSSheetGenerator,
				pclPMInputUser->GetColdGeneratorList()->GetColdGenerator( i )->m_strColdGeneratorTypeID,
				CDimValue::SItoCU( _U_TH_POWER, pclPMInputUser->GetColdGeneratorList()->GetColdGenerator( i )->m_dColdCapacity ),
				CDimValue::SItoCU( _U_VOLUME, pclPMInputUser->GetColdGeneratorList()->GetColdGenerator( i )->m_dContent ),
				pclPMInputUser->GetColdGeneratorList()->GetColdGenerator( i )->m_bManual,
				CDimValue::SItoCU( _U_TEMPERATURE, pclPMInputUser->GetColdGeneratorList()->GetColdGenerator( i )->m_dSupplyTemp ),
				pclPMInputUser->GetColdGeneratorList()->GetColdGenerator( i )->m_bManualTemp,
				pclPMInputUser->GetColdGeneratorList()->GetColdGenerator( i )->m_lQuantity,
				true );
	}

	_UpdateColdGenerator();
}

void CDlgWizardPM_RightViewInput_CoolingGenerators::_UpdateColdGenerator()
{
	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pclSelectPMList 
			|| NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERT_RETURN;
	}

	CPMInputUser *pclPMInputUser = m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser();

	for( int i = 0; i < m_iColdGeneratorCount; i++ )
	{
		int iRecordIndex = i;

		// Get generator ID.
		CString strGeneratorID;
		_FindGeneratorID( m_pclSSheetGenerator, i + RD_ColdGenerator_FirstAvailRow, strGeneratorID );

		// Get Capacity in SI.
		TCHAR strCapacity[16];
		double dCapacity;
		m_pclSSheetGenerator->GetValue( CD_ColdGenerator_Capacity, i + RD_ColdGenerator_FirstAvailRow, strCapacity );
		ReadCUDoubleFromStr( _U_TH_POWER, strCapacity, &dCapacity );

		double dContent = -1.0;
		double dTemp = -1.0;
		bool bManual = _IsCheckBoxChecked( m_pclSSheetGenerator, CD_ColdGenerator_Manual, i + RD_ColdGenerator_FirstAvailRow );
		bool bManualTemp = _IsCheckBoxChecked( m_pclSSheetGenerator, CD_ColdGenerator_ManualTemp, i + RD_ColdGenerator_FirstAvailRow );

		if( true == bManual )
		{
			// If in manual input then get the Content in SI.
			TCHAR strContent[16];
			m_pclSSheetGenerator->GetValue( CD_ColdGenerator_Content, i + RD_ColdGenerator_FirstAvailRow, strContent );
			ReadCUDoubleFromStr( _U_VOLUME, strContent, &dContent );
			m_pclSSheetGenerator->SetBackColor( CD_ColdGenerator_Content, i + RD_ColdGenerator_FirstAvailRow, _WHITE );
		}

		if( true == bManualTemp )
		{
			// If in manual input then get the Content in SI
			TCHAR strContent[16];
			m_pclSSheetGenerator->GetValue( CD_ColdGenerator_SupplyTemp, i + RD_ColdGenerator_FirstAvailRow, strContent );
			ReadCUDoubleFromStr( _U_TEMPERATURE, strContent, &dTemp );
			m_pclSSheetGenerator->SetBackColor( CD_ColdGenerator_SupplyTemp, i + RD_ColdGenerator_FirstAvailRow, _WHITE );
		}
		else
		{
			dTemp = pclPMInputUser->GetSupplyTemperature();
		}

		// Get Quantity
		TCHAR strQuantity[16];
		long lQuantity;
		m_pclSSheetGenerator->GetValue( CD_ColdGenerator_Quantity, i + RD_ColdGenerator_FirstAvailRow, strQuantity );
		ReadLongFromStr( strQuantity, &lQuantity );

		// Update database.
		pclPMInputUser->GetColdGeneratorList()->UpdateColdGenerator( iRecordIndex, strGeneratorID, dCapacity, dContent, dTemp, bManualTemp, lQuantity );

		if( false == bManual )
		{
			// If not in manual input then refresh Content in spreadsheet.
			dContent = pclPMInputUser->GetColdGeneratorList()->GetColdGenerator( iRecordIndex )->m_dContent;
			m_pclSSheetGenerator->FormatStaticText( CD_ColdGenerator_Content,
													i + RD_ColdGenerator_FirstAvailRow,
													CD_ColdGenerator_Content,
													i + RD_ColdGenerator_FirstAvailRow,
													WriteCUDouble( _U_VOLUME, dContent ),
													SSS_ALIGN_RIGHT );
			m_pclSSheetGenerator->SetBackColor( CD_ColdGenerator_Content, i + RD_ColdGenerator_FirstAvailRow, _LIGHTGRAY );
		}

		if( false == bManualTemp )
		{
			// If not in manual input then refresh Content in spreadsheet.
			dTemp = pclPMInputUser->GetColdGeneratorList()->GetColdGenerator( iRecordIndex )->m_dSupplyTemp;
			m_pclSSheetGenerator->FormatStaticText( CD_ColdGenerator_SupplyTemp,
													i + RD_ColdGenerator_FirstAvailRow,
													CD_ColdGenerator_SupplyTemp,
													i + RD_ColdGenerator_FirstAvailRow,
													WriteCUDouble( _U_TEMPERATURE, dTemp ),
													SSS_ALIGN_RIGHT );
			m_pclSSheetGenerator->SetBackColor( CD_ColdGenerator_SupplyTemp, i + RD_ColdGenerator_FirstAvailRow, _LIGHTGRAY );
		}
	}

	_UpdateStaticValue();
}

void CDlgWizardPM_RightViewInput_CoolingGenerators::_UpdateColdGeneratorValues()
{
	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pclSelectPMList 
			|| NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERT_RETURN;
	}

	CPMInputUser *pclPMInputUser = m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser();

	for( int i = 0; i < m_iColdGeneratorCount; i++ )
	{
		// Update capacity.
		double dColdCapacity = pclPMInputUser->GetColdGeneratorList()->GetColdGenerator( i )->m_dColdCapacity;
		CString str = WriteCUDouble( _U_TH_POWER, dColdCapacity );
		m_pclSSheetGenerator->SetValue( CD_ColdGenerator_Capacity, i + RD_ColdGenerator_FirstAvailRow, str );

		// Update content.
		double dContent = pclPMInputUser->GetColdGeneratorList()->GetColdGenerator( i )->m_dContent;
		str = WriteCUDouble( _U_VOLUME, dContent );
		m_pclSSheetGenerator->SetValue( CD_ColdGenerator_Content, i + RD_ColdGenerator_FirstAvailRow, str );

		// Update supply temperature.
		double dSupplyTemperature = pclPMInputUser->GetColdGeneratorList()->GetColdGenerator( i )->m_dSupplyTemp;
		str = WriteCUDouble( _U_TEMPERATURE, dContent );
		m_pclSSheetGenerator->SetValue( CD_ColdGenerator_SupplyTemp, i + RD_ColdGenerator_FirstAvailRow, str );
	}
}

bool CDlgWizardPM_RightViewInput_CoolingGenerators::_ButtonPlusGeneratorEnabled()
{
	// avoid to add a new generator if some input are invalid
	// not valid :
	// * content or capacity <= 0
	// * NAN
	// * Empty Cell
	bool bEnable = true;
	int iLastDataRow = m_iColdGeneratorCount + RD_ColdGenerator_FirstAvailRow - 1;
	WORD border = SS_BORDERTYPE_LEFT | SS_BORDERTYPE_TOP | SS_BORDERTYPE_RIGHT | SS_BORDERTYPE_BOTTOM;
	CPMInputUser *pclPMInputUser = m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser();

	for( int i = iLastDataRow; i >= RD_ColdGenerator_FirstAvailRow; i-- )
	{
		std::wstring strContent = m_pclSSheetGenerator->GetCellText( CD_ColdGenerator_Content, i );
		double dContent = 0.0;

		// Border need to be removed if (manual && good) or (auto).
		m_pclSSheetGenerator->SetCellBorder( CD_ColdGenerator_Content, i, false );

		if( RD_OK != ReadCUDoubleFromStr( _U_TH_POWER, strContent, &dContent ) || dContent <= 0.0 )
		{
			if( _IsCheckBoxChecked( m_pclSSheetGenerator, CD_ColdGenerator_Manual, i ) )
			{
				// Red border to help the user to find errors.
				m_pclSSheetGenerator->SetCellBorder( CD_ColdGenerator_Content, i, CD_ColdGenerator_Content, i, true, border, SS_BORDERSTYLE_SOLID, _TAH_RED_LIGHT );
			}

			bEnable = false;
		}

		std::wstring strCapacity = m_pclSSheetGenerator->GetCellText( CD_ColdGenerator_Capacity, i );
		double dCapacity = 0.0;

		if( RD_OK != ReadCUDoubleFromStr( _U_VOLUME, strCapacity, &dCapacity ) || dCapacity <= 0.0 )
		{
			// Red border to help the user to find errors.
			m_pclSSheetGenerator->SetCellBorder( CD_ColdGenerator_Capacity, i, CD_ColdGenerator_Capacity, i, true, border, SS_BORDERSTYLE_SOLID, _TAH_RED_LIGHT );
			bEnable = false;
		}
		else
		{
			// Remove border if input is valid.
			m_pclSSheetGenerator->SetCellBorder( CD_ColdGenerator_Capacity, i, false );
		}
		
		// HYS-1164 : Add generator supply temp.
		std::wstring strGenSupplyTemp = m_pclSSheetGenerator->GetCellText( CD_ColdGenerator_SupplyTemp, i );
		double dGenSupplyTemp = 0.0;

		// HYS-1612: pay attention that all temperatures read from string are in custom unit!
		if( RD_OK != ReadCUDoubleFromStr( _U_TEMPERATURE, strGenSupplyTemp, &dGenSupplyTemp )
				|| dGenSupplyTemp < pclPMInputUser->GetSupplyTemperature() )
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
		m_pclSSheetGenerator->SetBackColor( CD_ColdGenerator_FirstColumn,
											RD_ColdGenerator_FirstAvailRow + m_iColdGeneratorCount,
											CD_ColdGenerator_LastColumn,
											RD_ColdGenerator_FirstAvailRow + m_iColdGeneratorCount,
											_WHITE );
	}
	else
	{
		m_pclSSheetGenerator->SetBackColor( CD_ColdGenerator_FirstColumn,
											RD_ColdGenerator_FirstAvailRow + m_iColdGeneratorCount,
											CD_ColdGenerator_LastColumn,
											RD_ColdGenerator_FirstAvailRow + m_iColdGeneratorCount,
											_TAH_GREY_XLIGHT );
	}

	return m_bGeneratorValid = bEnable;
}

void CDlgWizardPM_RightViewInput_CoolingGenerators::_CheckColdGeneratorColumnWidth()
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
	double dTotalColWidth = CGCW_FirstColumn + CGCW_Generator + CGCW_Capacity + CGCW_Content + CGCW_Manual;
	dTotalColWidth += CGCW_SupplyTemp + CGCW_ManualTemp + CGCW_Quantity;

	double dRatio = dAvailableWidth / dTotalColWidth * dStandardWidthInPixel;

	// Change column size.
	long lWidthInPixel = (long)( dRatio * CGCW_FirstColumn );
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
	m_pclSSheetGenerator->SetColWidthInPixels( CD_ColdGenerator_Manual, lWidthInPixel );

	lWidthInPixel = (long)( dRatio * CGCW_SupplyTemp );
	lTotalWidthInPixel += lWidthInPixel;
	m_pclSSheetGenerator->SetColWidthInPixels( CD_ColdGenerator_SupplyTemp, lWidthInPixel );

	lWidthInPixel = (long)( dRatio * CGCW_ManualTemp );
	lTotalWidthInPixel += lWidthInPixel;
	m_pclSSheetGenerator->SetColWidthInPixels( CD_ColdGenerator_ManualTemp, lWidthInPixel );
	
	m_pclSSheetGenerator->SetColWidthInPixels( CD_ColdGenerator_Quantity, m_ColdGeneratorRectClient.Width() - lTotalWidthInPixel );
}

void CDlgWizardPM_RightViewInput_CoolingGenerators::_CheckColdGeneratorSheetHeight()
{
	CRect rectClient;
	GetClientRect( &rectClient );

	CRect rectButtonReset;
	GetDlgItem( IDC_RESET )->GetWindowRect( &rectButtonReset);
	ScreenToClient( &rectButtonReset );

	CRect rectSheetClient;
	GetDlgItem( IDC_SSCOLDGENLIST )->GetWindowRect( &rectSheetClient );
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

void CDlgWizardPM_RightViewInput_CoolingGenerators::_VerifyResetButtons()
{
	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pclSelectPMList 
			|| NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERT_RETURN;
	}

	CPMInputUser *pclPMInputUser = m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser();

	BOOL bResetEnable = TRUE;

	if( 0 == pclPMInputUser->GetColdGeneratorList()->GetColdGeneratorCount() )
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
