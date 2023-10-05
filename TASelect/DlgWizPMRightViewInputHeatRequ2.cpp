#include "stdafx.h"


#include "TASelect.h"
#include "TASelectDoc.h"
#include "MainFrm.h"
#include "DlgWizPMRightViewInputHeatRequ2.h"
#include "RViewWizardSelBase.h"
#include "RViewWizardSelPM.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


CDlgWizardPM_RightViewInput_HeatingRequirement2::CDlgWizardPM_RightViewInput_HeatingRequirement2( CWizardSelPMParams *pclWizardSelPMParams, CWnd *pclParent )
	: CDlgWizardPM_RightViewInput_Base( pclWizardSelPMParams, HeatingRequirement2, IDD, pclParent )
{
	m_bButtonWaterMakeUpBreakTankState = false;
	m_bButtonWaterMakeUpDutyStby = false;
}

void CDlgWizardPM_RightViewInput_HeatingRequirement2::ApplyPMInputUserUpdated( bool bWaterCharUpdated, bool bShowErrorMsg )
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
	// Water make-up - Break tank.
	int iIsChecked = ( true == pclPMInputUser->GetPMWQSelectionPreferences()->IsChecked( CPMWQPrefs::WaterMakeupBreakTank ) ) ? BST_CHECKED : BST_UNCHECKED;
	BOOL bIsEnabled = ( true == pclPMInputUser->GetPMWQSelectionPreferences()->IsDisabled( CPMWQPrefs::WaterMakeupBreakTank ) ) ? FALSE : TRUE;

	if( m_clButtonWaterMakeUpBreakTank.GetCheck() != iIsChecked )
	{
		m_clButtonWaterMakeUpBreakTank.SetCheck( iIsChecked );
		m_bButtonWaterMakeUpBreakTankState = ( BST_CHECKED == iIsChecked ) ? true : false;
		int iNewImage = ( true == m_bButtonWaterMakeUpBreakTankState ) ? CRCImageManager::ILPMWQP_BreakTankSelected: CRCImageManager::ILPMWQP_BreakTank;
		m_clButtonWaterMakeUpBreakTank.SetIcon( ( HICON )pclImgListButton->ExtractIcon( iNewImage ) );
		bAtLeastOneChange = true;
	}

	// HYS-1537 : When hte option is checked and disabled. Deactivated it.
	if( FALSE == bIsEnabled && true == m_bButtonWaterMakeUpBreakTankState )
	{
		m_bButtonWaterMakeUpBreakTankState = false;
		m_clButtonWaterMakeUpBreakTank.SetIcon( (HICON)pclImgListButton->ExtractIcon( CRCImageManager::ILPMWQP_BreakTank ) );
	}

	m_clButtonWaterMakeUpBreakTank.EnableWindow( bIsEnabled );
	m_StaticWaterMakeUpBreakTank.EnableWindow( bIsEnabled );
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Water make-up - Duty stand-by.
	iIsChecked = ( true == pclPMInputUser->GetPMWQSelectionPreferences()->IsChecked( CPMWQPrefs::WaterMakeupDutyStandBy ) ) ? BST_CHECKED : BST_UNCHECKED;
	bIsEnabled = ( true == pclPMInputUser->GetPMWQSelectionPreferences()->IsDisabled( CPMWQPrefs::WaterMakeupDutyStandBy ) ) ? FALSE : TRUE;

	if( m_clButtonWaterMakeUpDutyStby.GetCheck() != iIsChecked )
	{
		m_clButtonWaterMakeUpDutyStby.SetCheck( iIsChecked );
		m_bButtonWaterMakeUpDutyStby = ( BST_CHECKED == iIsChecked ) ? true : false;
		int iNewImage = ( true == m_bButtonWaterMakeUpDutyStby ) ? CRCImageManager::ILPMWQP_DutyStandBySelected: CRCImageManager::ILPMWQP_DutyStandBy;
		m_clButtonWaterMakeUpDutyStby.SetIcon( ( HICON )pclImgListButton->ExtractIcon( iNewImage ) );
		bAtLeastOneChange = true;
	}

	// HYS-1537 : When hte option is checked and disabled. Deactivated it.
	if( FALSE == bIsEnabled && true == m_bButtonWaterMakeUpDutyStby )
	{
		m_bButtonWaterMakeUpDutyStby = false;
		m_clButtonWaterMakeUpDutyStby.SetIcon( (HICON)pclImgListButton->ExtractIcon( CRCImageManager::ILPMWQP_DutyStandBy ) );
	}

	m_clButtonWaterMakeUpDutyStby.EnableWindow( bIsEnabled );
	m_StaticWaterMakeUpDutyStby.EnableWindow( bIsEnabled );
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	if( m_ExtEditSizeMaxWidth.GetCurrentValSI() != pclPMInputUser->GetMaxWidth() )
	{
		m_ExtEditSizeMaxWidth.SetCurrentValSI( pclPMInputUser->GetMaxWidth() );
		m_ExtEditSizeMaxWidth.Update();
		bAtLeastOneChange = true;
	}

	if( m_ExtEditSizeMaxHeight.GetCurrentValSI() != pclPMInputUser->GetMaxHeight() )
	{
		m_ExtEditSizeMaxHeight.SetCurrentValSI( pclPMInputUser->GetMaxHeight() );
		m_ExtEditSizeMaxHeight.Update();
		bAtLeastOneChange = true;
	}

	if( true == bAtLeastOneChange )
	{
		_VerifyInputValues();
	}
}

BEGIN_MESSAGE_MAP( CDlgWizardPM_RightViewInput_HeatingRequirement2, CDlgWizardPM_RightViewInput_Base )
	ON_BN_CLICKED( IDC_BUTTON_BREAKTANK, OnBnClickedWaterMakeUpBreakTank )
	ON_BN_CLICKED( IDC_BUTTON_STANDBY, OnBnClickedWaterMakeUpDutyStby )
	ON_EN_SETFOCUS( IDC_EDIT_MAXWIDTH, OnEnSetFocusMaxWidth )
	ON_EN_SETFOCUS( IDC_EDIT_MAXHEIGHT, OnEnSetFocusMaxHeight )
	ON_EN_KILLFOCUS( IDC_EDIT_MAXWIDTH, OnKillFocusMaxWidth )
	ON_EN_KILLFOCUS( IDC_EDIT_MAXHEIGHT, OnKillFocusMaxHeight )
END_MESSAGE_MAP()

void CDlgWizardPM_RightViewInput_HeatingRequirement2::DoDataExchange( CDataExchange *pDX )
{
	CDlgWizardPM_RightViewInput_Base::DoDataExchange( pDX );

	DDX_Control( pDX, IDC_GROUPWATERMAKEUP, m_GroupWaterMakeUp );
	DDX_Control( pDX, IDC_GROUPSIZE, m_GroupSize );

	DDX_Control( pDX, IDC_STATIC_BREAKTANK, m_StaticWaterMakeUpBreakTank );
	DDX_Control( pDX, IDC_STATIC_STANDBY, m_StaticWaterMakeUpDutyStby );
	DDX_Control( pDX, IDC_STATIC_MAXWIDTH, m_StaticSizeMaxWidth );
	DDX_Control( pDX, IDC_STATIC_MAXHEIGHT, m_StaticSizeMaxHeight );

	DDX_Control( pDX, IDC_BUTTON_BREAKTANK, m_clButtonWaterMakeUpBreakTank );
	DDX_Control( pDX, IDC_BUTTON_STANDBY, m_clButtonWaterMakeUpDutyStby );
	DDX_Control( pDX, IDC_EDIT_MAXWIDTH, m_ExtEditSizeMaxWidth );
	DDX_Control( pDX, IDC_EDIT_MAXHEIGHT, m_ExtEditSizeMaxHeight );

	DDX_Control( pDX, IDC_STATIC_MAXWIDTHUNIT, m_StaticSizeMaxWidthUnit );
	DDX_Control( pDX, IDC_STATIC_MAXHEIGHTUNIT, m_StaticSizeMaxHeightUnit );
}

BOOL CDlgWizardPM_RightViewInput_HeatingRequirement2::OnInitDialog()
{
	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pclSelectPMList 
			|| NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERTA_RETURN( FALSE );
	}

	CPMInputUser *pclPMInputUser = m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser();

	CDlgWizardPM_RightViewInput_Base::OnInitDialog();

	SetMainTitle( TASApp.LoadLocalizedString( IDS_DLGWIZPM_RVIEWHEATREQU2_MAINTITLE ) );

	CString str;
	TCHAR name[_MAXCHARS];
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();

	CImageList *pclImgListButton = TASApp.GetpRCImageManager()->GetImageList( CRCImageManager::ILN_PMWQPrefs );

	//////////////////////////////////////////////////////////////////////////
	// GROUP
	m_GroupWaterMakeUp.SetWindowTextW( TASApp.LoadLocalizedString( IDS_DLGWIZPM_RVIEWINPUTALLREQU_GROUPWATERMAKEUP ) );
	m_GroupWaterMakeUp.SetBckgndColor( RGB( 255, 255, 255 ) );

	m_GroupSize.SetWindowTextW( TASApp.LoadLocalizedString( IDS_DLGWIZPM_RVIEWINPUTALLREQU_GROUPSIZE ) );
	m_GroupSize.SetBckgndColor( RGB( 255, 255, 255 ) );

	//////////////////////////////////////////////////////////////////////////
	// BUTTON

	// Water make-up-based: Break tank required.
	m_clButtonWaterMakeUpBreakTank.SetIcon( ( HICON )pclImgListButton->ExtractIcon( CRCImageManager::ILPMWQP_BreakTank ) );
	m_bButtonWaterMakeUpBreakTankState = false;

	// Water make-up-based: Duty and standby of make-up pumps.
	m_clButtonWaterMakeUpDutyStby.SetIcon( ( HICON )pclImgListButton->ExtractIcon( CRCImageManager::ILPMWQP_DutyStandBy ) );
	m_bButtonWaterMakeUpDutyStby = false;

	//////////////////////////////////////////////////////////////////////////
	// EDITTEXT

	m_ExtEditSizeMaxWidth.SetCurrentValSI( pclPMInputUser->GetMaxWidth() );
	InitNumericalEdit( &m_ExtEditSizeMaxWidth, _U_LENGTH );

	m_ExtEditSizeMaxHeight.SetCurrentValSI( pclPMInputUser->GetMaxHeight() );
	InitNumericalEdit( &m_ExtEditSizeMaxHeight, _U_LENGTH );

	//////////////////////////////////////////////////////////////////////////
	// STATIC

	// Break tank required.
	str = TASApp.LoadLocalizedString( IDS_DLGWIZPM_RVIEWINPUTALLREQU2_BREAKTANK );
	m_StaticWaterMakeUpBreakTank.SetTextAndToolTip( str );

	// Duty and standby of make-up pumps.
	str = TASApp.LoadLocalizedString( IDS_DLGWIZPM_RVIEWINPUTALLREQU2_DUTYSTDBY );
	m_StaticWaterMakeUpDutyStby.SetTextAndToolTip( str );

	// Max. width:
	str = TASApp.LoadLocalizedString( IDS_DLGWIZPM_RVIEWINPUTALLREQU2_MAXWIDTH );
	m_StaticSizeMaxWidth.SetTextAndToolTip( str );

	// Max. width: (Unit)
	GetNameOf( pUnitDB->GetDefaultUnit( _U_LENGTH ), name );
	SetDlgItemText( IDC_STATIC_MAXWIDTHUNIT, name );

	// Max. height:
	str = TASApp.LoadLocalizedString( IDS_DLGWIZPM_RVIEWINPUTALLTEMP2_MAXHEIGHT );
	m_StaticSizeMaxHeight.SetTextAndToolTip( str );

	// Max. height: (Unit)
	GetNameOf( pUnitDB->GetDefaultUnit( _U_LENGTH ), name );
	SetDlgItemText( IDC_STATIC_MAXHEIGHTUNIT, name );

	m_StaticWaterMakeUpBreakTank.SetBackColor( RGB( 255, 255, 255 ) );
	m_StaticWaterMakeUpDutyStby.SetBackColor( RGB( 255, 255, 255 ) );
	m_StaticSizeMaxWidth.SetBackColor( RGB( 255, 255, 255 ) );
	m_StaticSizeMaxHeight.SetBackColor( RGB( 255, 255, 255 ) );

	m_ExtEditSizeMaxWidth.ActiveSpecialValidation( true, this );
	m_ExtEditSizeMaxHeight.ActiveSpecialValidation( true, this );

	m_StaticSizeMaxWidthUnit.SetBackColor( RGB( 255, 255, 255 ) );
	m_StaticSizeMaxHeightUnit.SetBackColor( RGB( 255, 255, 255 ) );

	if( 0 == CString( pclPMInputUser->GetWaterMakeUpTypeID() ).Compare( _T("WMUP_TYPE_NONE") ) )
	{
		GetDlgItem( IDC_GROUPWATERMAKEUP )->EnableWindow( FALSE );

		m_clButtonWaterMakeUpBreakTank.EnableWindow( FALSE );
		m_StaticWaterMakeUpBreakTank.EnableWindow( FALSE );

		m_clButtonWaterMakeUpDutyStby.EnableWindow( FALSE );
		m_StaticWaterMakeUpDutyStby.EnableWindow( FALSE );
	}

	ApplyPMInputUserUpdated();

	return TRUE;
}

void CDlgWizardPM_RightViewInput_HeatingRequirement2::OnSize( UINT nType, int cx, int cy )
{
	CDlgWizardPM_RightViewInput_Base::OnSize( nType, cx, cy );

	if( NULL == GetDlgItem( IDC_GROUPWATERMAKEUP ) || NULL == GetDlgItem( IDC_GROUPWATERMAKEUP )->GetSafeHwnd() )
	{
		// Not yet ready.
		return;
	}

	CRect rectClient;
	GetClientRect( &rectClient );

	// Resize 'Water make-up-based' group.
	CRect rectGroup;
	GetDlgItem( IDC_GROUPWATERMAKEUP )->GetClientRect( &rectGroup );
	GetDlgItem( IDC_GROUPWATERMAKEUP )->SetWindowPos( NULL, -1, -1, rectClient.Width() - 8, rectGroup.Height(), SWP_NOMOVE | SWP_NOZORDER );

	// Resize 'Size-based' group.
	GetDlgItem( IDC_GROUPSIZE )->GetClientRect( &rectGroup );
	GetDlgItem( IDC_GROUPSIZE )->SetWindowPos( NULL, -1, -1, rectClient.Width() - 8, rectGroup.Height(), SWP_NOMOVE | SWP_NOZORDER );
}

void CDlgWizardPM_RightViewInput_HeatingRequirement2::OnBnClickedWaterMakeUpBreakTank()
{
	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pclSelectPMList || NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser() 
			|| NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetPMWQSelectionPreferences()  )
	{
		ASSERT_RETURN;
	}

	CImageList *pclImgListButton = TASApp.GetpRCImageManager()->GetImageList( CRCImageManager::ILN_PMWQPrefs );
	m_bButtonWaterMakeUpBreakTankState = !m_bButtonWaterMakeUpBreakTankState;
	int iNewImage = ( true == m_bButtonWaterMakeUpBreakTankState ) ? CRCImageManager::ILPMWQP_BreakTankSelected: CRCImageManager::ILPMWQP_BreakTank;
	m_clButtonWaterMakeUpBreakTank.SetIcon( ( HICON )pclImgListButton->ExtractIcon( iNewImage ) );
	m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetPMWQSelectionPreferences()->SetChecked( CPMWQPrefs::WaterMakeupBreakTank, m_bButtonWaterMakeUpBreakTankState );
}

void CDlgWizardPM_RightViewInput_HeatingRequirement2::OnBnClickedWaterMakeUpDutyStby()
{
	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pclSelectPMList || NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser() 
			|| NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetPMWQSelectionPreferences()  )
	{
		ASSERT_RETURN;
	}

	CImageList *pclImgListButton = TASApp.GetpRCImageManager()->GetImageList( CRCImageManager::ILN_PMWQPrefs );
	m_bButtonWaterMakeUpDutyStby = !m_bButtonWaterMakeUpDutyStby;
	int iNewImage = ( true == m_bButtonWaterMakeUpDutyStby ) ? CRCImageManager::ILPMWQP_DutyStandBySelected: CRCImageManager::ILPMWQP_DutyStandBy;
	m_clButtonWaterMakeUpDutyStby.SetIcon( ( HICON )pclImgListButton->ExtractIcon( iNewImage ) );
	m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->GetPMWQSelectionPreferences()->SetChecked( CPMWQPrefs::WaterMakeupDutyStandBy, m_bButtonWaterMakeUpDutyStby );
}

void CDlgWizardPM_RightViewInput_HeatingRequirement2::OnEnSetFocusMaxWidth()
{
	m_ExtEditSizeMaxWidth.SetSel( 0, -1 );
	m_dMaxWidthSaved = m_ExtEditSizeMaxWidth.GetCurrentValSI();
}

void CDlgWizardPM_RightViewInput_HeatingRequirement2::OnEnSetFocusMaxHeight()
{
	m_ExtEditSizeMaxHeight.SetSel( 0, -1 );
	m_dMaxHeightSaved = m_ExtEditSizeMaxHeight.GetCurrentValSI();
}

void CDlgWizardPM_RightViewInput_HeatingRequirement2::OnKillFocusMaxWidth()
{
	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pclSelectPMList 
			|| NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERT_RETURN;
	}

	double dMaxWidth = m_ExtEditSizeMaxWidth.GetCurrentValSI();

	// Do nothing if no change.
	if( m_dMaxWidthSaved == dMaxWidth )
	{
		return;
	}

	m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->SetMaxWidth( dMaxWidth );
}

void CDlgWizardPM_RightViewInput_HeatingRequirement2::OnKillFocusMaxHeight()
{
	if( NULL == m_pclWizardSelPMParams || NULL == m_pclWizardSelPMParams->m_pclSelectPMList 
			|| NULL == m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser() )
	{
		ASSERT_RETURN;
	}

	double dMaxHeight = m_ExtEditSizeMaxHeight.GetCurrentValSI();

	// Do nothing if no change.
	if( m_dMaxHeightSaved == dMaxHeight )
	{
		return;
	}

	m_pclWizardSelPMParams->m_pclSelectPMList->GetpclPMInputUser()->SetMaxHeight( dMaxHeight );
}

LRESULT CDlgWizardPM_RightViewInput_HeatingRequirement2::OnUnitChange( WPARAM wParam, LPARAM lParam )
{
	CDlgWizardPM_RightViewInput_Base::OnUnitChange( wParam, lParam );

	CUnitDatabase *m_pUnitDB = CDimValue::AccessUDB();

	m_StaticSizeMaxWidthUnit.SetWindowText( m_pUnitDB->GetNameOfDefaultUnit( _U_LENGTH ).c_str() );
	InitNumericalEdit( &m_ExtEditSizeMaxWidth, _U_LENGTH );

	m_StaticSizeMaxHeightUnit.SetWindowText( m_pUnitDB->GetNameOfDefaultUnit( _U_LENGTH ).c_str() );
	InitNumericalEdit( &m_ExtEditSizeMaxHeight, _U_LENGTH );

	return 0;
}

void CDlgWizardPM_RightViewInput_HeatingRequirement2::GetExtEditList( CArray<CExtNumEdit*> &arExtEditList )
{
	arExtEditList.Add( &m_ExtEditSizeMaxWidth );
	arExtEditList.Add( &m_ExtEditSizeMaxHeight );
}

void CDlgWizardPM_RightViewInput_HeatingRequirement2::InitToolTips( CWnd *pWnd )
{
if( NULL != m_ToolTip.GetSafeHwnd() )
	{
		CString TTstr;

		if( NULL == pWnd || pWnd == &m_ExtEditSizeMaxWidth )
		{
			TTstr = TASApp.LoadLocalizedString( IDS_PM_TTMAXDIAMETER );
			m_ToolTip.AddToolWindow( &m_ExtEditSizeMaxWidth, TTstr );
		}

		if( NULL == pWnd || pWnd == &m_ExtEditSizeMaxHeight )
		{
			TTstr = TASApp.LoadLocalizedString( IDS_PM_TTMAXHEIGHT );
			m_ToolTip.AddToolWindow( &m_ExtEditSizeMaxHeight, TTstr );
		}
	}

	CDlgWizardPM_RightViewInput_Base::InitToolTips( pWnd );
}

void CDlgWizardPM_RightViewInput_HeatingRequirement2::_VerifyInputValues()
{
	// Nothing to verify for the moment.
}
