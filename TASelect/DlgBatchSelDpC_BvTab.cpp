#include "stdafx.h"
#include <errno.h>
#include <algorithm>

#include "Taselect.h"
#include "MainFrm.h"

#include "ProductSelectionParameters.h"

#include "DlgBatchSelDpC.h"
#include "DlgBatchSelDpC_BvTab.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CDlgBatchSelDpC_BvTab::CDlgBatchSelDpC_BvTab( CBatchSelDpCParams &clBatchSelDpCParams, CWnd *pParent )
	: CDialogExt( CDlgBatchSelDpC_BvTab::IDD, pParent )
{
	m_pclBatchSelDpCParams = &clBatchSelDpCParams;
}

void CDlgBatchSelDpC_BvTab::SaveSelectionParameters( void )
{
	if( NULL == m_pclBatchSelDpCParams || NULL == m_pclBatchSelDpCParams->m_pTADS || NULL == m_pclBatchSelDpCParams->m_pTADS->GetpBatchSelParameter() )
	{
		return;
	}

	CDS_BatchSelParameter *pclBatchSelParameters = m_pclBatchSelDpCParams->m_pTADS->GetpBatchSelParameter();

	pclBatchSelParameters->SetDpCBvTypeBelow65ID( m_pclBatchSelDpCParams->m_clBatchSelBVParams.m_strComboTypeBelow65ID );
	pclBatchSelParameters->SetDpCBvFamilyBelow65ID( m_pclBatchSelDpCParams->m_clBatchSelBVParams.m_strComboFamilyBelow65ID );
	pclBatchSelParameters->SetDpCBvMaterialBelow65ID( m_pclBatchSelDpCParams->m_clBatchSelBVParams.m_strComboMaterialBelow65ID );
	pclBatchSelParameters->SetDpCBvConnectBelow65ID( m_pclBatchSelDpCParams->m_clBatchSelBVParams.m_strComboConnectBelow65ID );
	pclBatchSelParameters->SetDpCBvVersionBelow65ID( m_pclBatchSelDpCParams->m_clBatchSelBVParams.m_strComboVersionBelow65ID );
	pclBatchSelParameters->SetDpCBvPNBelow65ID( m_pclBatchSelDpCParams->m_clBatchSelBVParams.m_strComboPNBelow65ID );
	
	pclBatchSelParameters->SetDpCBvTypeAbove50ID( m_pclBatchSelDpCParams->m_clBatchSelBVParams.m_strComboTypeAbove50ID );
	pclBatchSelParameters->SetDpCBvFamilyAbove50ID( m_pclBatchSelDpCParams->m_clBatchSelBVParams.m_strComboFamilyAbove50ID );
	pclBatchSelParameters->SetDpCBvMaterialAbove50ID( m_pclBatchSelDpCParams->m_clBatchSelBVParams.m_strComboMaterialAbove50ID );
	pclBatchSelParameters->SetDpCBvConnectAbove50ID( m_pclBatchSelDpCParams->m_clBatchSelBVParams.m_strComboConnectAbove50ID );
	pclBatchSelParameters->SetDpCBvVersionAbove50ID( m_pclBatchSelDpCParams->m_clBatchSelBVParams.m_strComboVersionAbove50ID );
	pclBatchSelParameters->SetDpCBvPNAbove50ID( m_pclBatchSelDpCParams->m_clBatchSelBVParams.m_strComboPNAbove50ID );
}

void CDlgBatchSelDpC_BvTab::EnableCombos( bool bEnable )
{
	m_clGroupValveBelow65.EnableWindow( bEnable, false );

	m_ComboTypeBelow65.EnableWindow( ( true == bEnable ) ? TRUE : FALSE );

	if( true == bEnable && m_ComboTypeBelow65.GetCount() <= 1 )
	{
		m_ComboTypeBelow65.EnableWindow( FALSE );
	}

	if( NULL != GetDlgItem( IDC_STATICTYPE1 ) )
	{
		GetDlgItem( IDC_STATICTYPE1 )->EnableWindow( ( true == bEnable ) ? TRUE : FALSE );
	}

	m_ComboFamilyBelow65.EnableWindow( ( true == bEnable ) ? TRUE : FALSE );

	if( true == bEnable && m_ComboFamilyBelow65.GetCount() <= 1 )
	{
		m_ComboFamilyBelow65.EnableWindow( FALSE );
	}

	if( NULL != GetDlgItem( IDC_STATICFAMILY1 ) )
	{
		GetDlgItem( IDC_STATICFAMILY1 )->EnableWindow( ( true == bEnable ) ? TRUE : FALSE );
	}

	m_ComboBodyMaterialBelow65.EnableWindow( ( true == bEnable ) ? TRUE : FALSE );

	if( true == bEnable && m_ComboBodyMaterialBelow65.GetCount() <= 1 )
	{
		m_ComboBodyMaterialBelow65.EnableWindow( FALSE );
	}

	if( NULL != GetDlgItem( IDC_STATICMATERIAL1 ) )
	{
		GetDlgItem( IDC_STATICMATERIAL1 )->EnableWindow( ( true == bEnable ) ? TRUE : FALSE );
	}

	m_ComboConnectBelow65.EnableWindow( ( true == bEnable ) ? TRUE : FALSE );

	if( true == bEnable && m_ComboConnectBelow65.GetCount() <= 1 )
	{
		m_ComboConnectBelow65.EnableWindow( FALSE );
	}

	if( NULL != GetDlgItem( IDC_STATICCONNECT1 ) )
	{
		GetDlgItem( IDC_STATICCONNECT1 )->EnableWindow( ( true == bEnable ) ? TRUE : FALSE );
	}

	m_ComboVersionBelow65.EnableWindow( ( true == bEnable ) ? TRUE : FALSE );

	if( true == bEnable && m_ComboVersionBelow65.GetCount() <= 1 )
	{
		m_ComboVersionBelow65.EnableWindow( FALSE );
	}

	if( NULL != GetDlgItem( IDC_STATICVERSION1 ) )
	{
		GetDlgItem( IDC_STATICVERSION1 )->EnableWindow( ( true == bEnable ) ? TRUE : FALSE );
	}

	m_ComboPNBelow65.EnableWindow( ( true == bEnable ) ? TRUE : FALSE );

	if( true == bEnable && m_ComboPNBelow65.GetCount() <= 1 )
	{
		m_ComboPNBelow65.EnableWindow( FALSE );
	}

	if( NULL != GetDlgItem( IDC_STATICPN1 ) )
	{
		GetDlgItem( IDC_STATICPN1 )->EnableWindow( ( true == bEnable ) ? TRUE : FALSE );
	}

	m_clGroupValveAbove50.EnableWindow( bEnable, false );

	m_ComboTypeAbove50.EnableWindow( ( true == bEnable ) ? TRUE : FALSE );

	if( true == bEnable && m_ComboTypeAbove50.GetCount() <= 1 )
	{
		m_ComboTypeAbove50.EnableWindow( FALSE );
	}

	if( NULL != GetDlgItem( IDC_STATICTYPE2 ) )
	{
		GetDlgItem( IDC_STATICTYPE2 )->EnableWindow( ( true == bEnable ) ? TRUE : FALSE );
	}

	m_ComboFamilyAbove50.EnableWindow( ( true == bEnable ) ? TRUE : FALSE );

	if( true == bEnable && m_ComboFamilyAbove50.GetCount() <= 1 )
	{
		m_ComboFamilyAbove50.EnableWindow( FALSE );
	}

	if( NULL != GetDlgItem( IDC_STATICFAMILY2 ) )
	{
		GetDlgItem( IDC_STATICFAMILY2 )->EnableWindow( ( true == bEnable ) ? TRUE : FALSE );
	}

	m_ComboBodyMaterialAbove50.EnableWindow( ( true == bEnable ) ? TRUE : FALSE );

	if( true == bEnable && m_ComboBodyMaterialAbove50.GetCount() <= 1 )
	{
		m_ComboBodyMaterialAbove50.EnableWindow( FALSE );
	}

	if( NULL != GetDlgItem( IDC_STATICMATERIAL2 ) )
	{
		GetDlgItem( IDC_STATICMATERIAL2 )->EnableWindow( ( true == bEnable ) ? TRUE : FALSE );
	}

	m_ComboConnectAbove50.EnableWindow( ( true == bEnable ) ? TRUE : FALSE );

	if( true == bEnable && m_ComboConnectAbove50.GetCount() <= 1 )
	{
		m_ComboConnectAbove50.EnableWindow( FALSE );
	}

	if( NULL != GetDlgItem( IDC_STATICCONNECT2 ) )
	{
		GetDlgItem( IDC_STATICCONNECT2 )->EnableWindow( ( true == bEnable ) ? TRUE : FALSE );
	}

	m_ComboVersionAbove50.EnableWindow( ( true == bEnable ) ? TRUE : FALSE );

	if( true == bEnable && m_ComboVersionAbove50.GetCount() <= 1 )
	{
		m_ComboVersionAbove50.EnableWindow( FALSE );
	}

	if( NULL != GetDlgItem( IDC_STATICVERSION2 ) )
	{
		GetDlgItem( IDC_STATICVERSION2 )->EnableWindow( ( true == bEnable ) ? TRUE : FALSE );
	}

	m_ComboPNAbove50.EnableWindow( ( true == bEnable ) ? TRUE : FALSE );

	if( true == bEnable && m_ComboPNAbove50.GetCount() <= 1 )
	{
		m_ComboPNAbove50.EnableWindow( FALSE );
	}

	if( NULL != GetDlgItem( IDC_STATICPN2 ) )
	{
		GetDlgItem( IDC_STATICPN2 )->EnableWindow( ( true == bEnable ) ? TRUE : FALSE );
	}
}

void CDlgBatchSelDpC_BvTab::UpdateCombos()
{
	FillComboBoxType( CDlgBatchSelBase::Below65 );
	OnComboSelChangeTypeBelow65();

	FillComboBoxType( CDlgBatchSelBase::Above50 );
	OnComboSelChangeTypeAbove50();
}

LRESULT CDlgBatchSelDpC_BvTab::OnNewDocument( WPARAM wParam, LPARAM lParam )
{
	if( NULL == m_pclBatchSelDpCParams || NULL == m_pclBatchSelDpCParams->m_pTADS 
			|| NULL == m_pclBatchSelDpCParams->m_pTADS->GetpBatchSelParameter() )
	{
		return -1;
	}

	CDS_BatchSelParameter *pclBatchSelParameters = m_pclBatchSelDpCParams->m_pTADS->GetpBatchSelParameter();

	if( NULL == pclBatchSelParameters )
	{
		return -1;
	}

	// Fill the combo box.
	// Remark: Corresponding combo variables in 'm_pclBatchSelDpCParams.m_clBatchSelBVParams' are updated in each of this following methods.
	FillComboBoxType( CDlgBatchSelBase::AboveOrBelow::Below65, pclBatchSelParameters->GetDpCBvTypeBelow65ID() );
	FillComboBoxFamily( CDlgBatchSelBase::AboveOrBelow::Below65, pclBatchSelParameters->GetDpCBvFamilyBelow65ID() );
	FillComboBoxBodyMaterial( CDlgBatchSelBase::AboveOrBelow::Below65, pclBatchSelParameters->GetDpCBvMaterialBelow65ID() );
	FillComboBoxConnect( CDlgBatchSelBase::AboveOrBelow::Below65, pclBatchSelParameters->GetDpCBvConnectBelow65ID() );
	FillComboBoxVersion( CDlgBatchSelBase::AboveOrBelow::Below65, pclBatchSelParameters->GetDpCBvVersionBelow65ID() );
	FillComboBoxPN( CDlgBatchSelBase::AboveOrBelow::Below65, pclBatchSelParameters->GetDpCBvPNBelow65ID() );

	FillComboBoxType( CDlgBatchSelBase::AboveOrBelow::Above50, pclBatchSelParameters->GetDpCBvTypeAbove50ID() );
	FillComboBoxFamily( CDlgBatchSelBase::AboveOrBelow::Above50, pclBatchSelParameters->GetDpCBvFamilyAbove50ID() );
	FillComboBoxBodyMaterial( CDlgBatchSelBase::AboveOrBelow::Above50, pclBatchSelParameters->GetDpCBvMaterialAbove50ID() );
	FillComboBoxConnect( CDlgBatchSelBase::AboveOrBelow::Above50, pclBatchSelParameters->GetDpCBvConnectAbove50ID() );
	FillComboBoxVersion( CDlgBatchSelBase::AboveOrBelow::Above50, pclBatchSelParameters->GetDpCBvVersionAbove50ID() );
	FillComboBoxPN( CDlgBatchSelBase::AboveOrBelow::Above50, pclBatchSelParameters->GetDpCBvPNAbove50ID() );

	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PROTECTED MEMBERS
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP( CDlgBatchSelDpC_BvTab, CDialogExt )
	ON_WM_DESTROY()

	ON_CBN_SELCHANGE( IDC_COMBOTYPE1, OnComboSelChangeTypeBelow65 )
	ON_CBN_SELCHANGE( IDC_COMBOFAMILY1, OnComboSelChangeFamilyBelow65 )
	ON_CBN_SELCHANGE( IDC_COMBOMATERIAL1, OnComboSelChangeBdyMatBelow65 )
	ON_CBN_SELCHANGE( IDC_COMBOCONNECT1, OnComboSelChangeConnectBelow65 )
	ON_CBN_SELCHANGE( IDC_COMBOVERSION1, OnComboSelChangeVersionBelow65 )
	ON_CBN_SELCHANGE( IDC_COMBOPN1, OnComboSelChangePNBelow65 )

	ON_CBN_SELCHANGE( IDC_COMBOTYPE2, OnComboSelChangeTypeAbove50 )
	ON_CBN_SELCHANGE( IDC_COMBOFAMILY2, OnComboSelChangeFamilyAbove50 )
	ON_CBN_SELCHANGE( IDC_COMBOMATERIAL2, OnComboSelChangeBdyMatAbove50 )
	ON_CBN_SELCHANGE( IDC_COMBOCONNECT2, OnComboSelChangeConnectAbove50 )
	ON_CBN_SELCHANGE( IDC_COMBOVERSION2, OnComboSelChangeVersionAbove50 )
	ON_CBN_SELCHANGE( IDC_COMBOPN2, OnComboSelChangePNAbove50 )

END_MESSAGE_MAP()

void CDlgBatchSelDpC_BvTab::DoDataExchange( CDataExchange *pDX )
{
	CDialogExt::DoDataExchange( pDX );

	DDX_Control( pDX, IDC_GROUPVALVE1, m_clGroupValveBelow65 );
	DDX_Control( pDX, IDC_GROUPVALVE2, m_clGroupValveAbove50 );

	DDX_Control( pDX, IDC_COMBOTYPE1, m_ComboTypeBelow65 );
	DDX_Control( pDX, IDC_COMBOFAMILY1, m_ComboFamilyBelow65 );
	DDX_Control( pDX, IDC_COMBOMATERIAL1, m_ComboBodyMaterialBelow65 );
	DDX_Control( pDX, IDC_COMBOCONNECT1, m_ComboConnectBelow65 );
	DDX_Control( pDX, IDC_COMBOVERSION1, m_ComboVersionBelow65 );
	DDX_Control( pDX, IDC_COMBOPN1, m_ComboPNBelow65 );

	DDX_Control( pDX, IDC_COMBOTYPE2, m_ComboTypeAbove50 );
	DDX_Control( pDX, IDC_COMBOFAMILY2, m_ComboFamilyAbove50 );
	DDX_Control( pDX, IDC_COMBOMATERIAL2, m_ComboBodyMaterialAbove50 );
	DDX_Control( pDX, IDC_COMBOCONNECT2, m_ComboConnectAbove50 );
	DDX_Control( pDX, IDC_COMBOVERSION2, m_ComboVersionAbove50 );
	DDX_Control( pDX, IDC_COMBOPN2, m_ComboPNAbove50 );
}

BOOL CDlgBatchSelDpC_BvTab::OnInitDialog()
{
	CDialogExt::OnInitDialog();
	
	// Set proper style and add icons for groups.
	CImageList *pclImgListGroupBox = TASApp.GetpRCImageManager()->GetImageList( CRCImageManager::ILN_GroupBox );
	ASSERT( NULL != pclImgListGroupBox );

	CImageList *pclImgListGroupBoxGrayed = TASApp.GetpRCImageManager()->GetImageList( CRCImageManager::ILN_GroupBoxGrayed );
	ASSERT( NULL != pclImgListGroupBoxGrayed );

	if( NULL != pclImgListGroupBox )
	{
		m_clGroupValveBelow65.SetImageList( pclImgListGroupBox, CRCImageManager::ILGB_Valve );
		m_clGroupValveAbove50.SetImageList( pclImgListGroupBox, CRCImageManager::ILGB_Valve );
	}

	if( NULL != pclImgListGroupBoxGrayed )
	{
		m_clGroupValveBelow65.SetImageList( pclImgListGroupBoxGrayed, CRCImageManager::ILGB_Valve, true );
		m_clGroupValveAbove50.SetImageList( pclImgListGroupBoxGrayed, CRCImageManager::ILGB_Valve, true );
	}

	m_clGroupValveBelow65.SetInOffice2007Mainframe( true );
	m_clGroupValveBelow65.SaveChildList();

	m_clGroupValveAbove50.SetInOffice2007Mainframe( true );
	m_clGroupValveAbove50.SaveChildList();

	CString str = TASApp.LoadLocalizedString( IDS_BATCHSELBASE_STATICBELOW65 );
	m_clGroupValveBelow65.SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_BATCHSELBASE_STATICABOVE65 );
	m_clGroupValveAbove50.SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_BATCHSELBV_STATICTYPE );
	GetDlgItem( IDC_STATICTYPE1 )->SetWindowText( str );
	GetDlgItem( IDC_STATICTYPE2 )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_BATCHSELBV_STATICFAM );
	GetDlgItem( IDC_STATICFAMILY1 )->SetWindowText( str );
	GetDlgItem( IDC_STATICFAMILY2 )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_BATCHSELBV_STATICBDYMAT );
	GetDlgItem( IDC_STATICMATERIAL1 )->SetWindowText( str );
	GetDlgItem( IDC_STATICMATERIAL2 )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_BATCHSELBV_STATICCON );
	GetDlgItem( IDC_STATICCONNECT1 )->SetWindowText( str );
	GetDlgItem( IDC_STATICCONNECT2 )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_BATCHSELBV_STATICVER );
	GetDlgItem( IDC_STATICVERSION1 )->SetWindowText( str );
	GetDlgItem( IDC_STATICVERSION2 )->SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_BATCHSELBV_STATICPN );
	GetDlgItem( IDC_STATICPN1 )->SetWindowText( str );
	GetDlgItem( IDC_STATICPN2 )->SetWindowText( str );

	return TRUE;
}

void CDlgBatchSelDpC_BvTab::OnComboSelChangeTypeBelow65()
{
	if( NULL == m_pclBatchSelDpCParams )
	{
		ASSERT_RETURN;
	}

	if( m_pclBatchSelDpCParams->m_clBatchSelBVParams.m_strComboTypeBelow65ID == m_ComboTypeBelow65.GetCBCurSelIDPtr().ID )
	{
		// Do nothing if it's the same value as before.
		return;
	}

	if( false == ( (CDlgBatchSelDpC*)m_pParentWnd )->DpCBvCtrlTab_BS_ComboChange() )
	{
		int iPos = max( m_ComboTypeBelow65.FindCBIDPtr( m_pclBatchSelDpCParams->m_clBatchSelBVParams.m_strComboTypeBelow65ID ), 0 );

		if( iPos != m_ComboTypeBelow65.GetCurSel() )
		{
			m_ComboTypeBelow65.SetCurSel( iPos );
		}

		return;
	}

	m_pclBatchSelDpCParams->m_clBatchSelBVParams.m_strComboTypeBelow65ID = m_ComboTypeBelow65.GetCBCurSelIDPtr().ID;

	FillComboBoxFamily( CDlgBatchSelBase::AboveOrBelow::Below65 );
	FillComboBoxBodyMaterial( CDlgBatchSelBase::AboveOrBelow::Below65 );
	FillComboBoxConnect( CDlgBatchSelBase::AboveOrBelow::Below65 );
	FillComboBoxVersion( CDlgBatchSelBase::AboveOrBelow::Below65 );
	FillComboBoxPN( CDlgBatchSelBase::AboveOrBelow::Below65 );
	( (CDlgBatchSelDpC*)m_pParentWnd )->DpCBvCtrlTab_NotificationCbnChange();
}

void CDlgBatchSelDpC_BvTab::OnComboSelChangeFamilyBelow65()
{
	if( m_pclBatchSelDpCParams->m_clBatchSelBVParams.m_strComboFamilyBelow65ID == m_ComboFamilyBelow65.GetCBCurSelIDPtr().ID )
	{
		// Do nothing if it's the same value as before.
		return;
	}

	if( NULL == m_pclBatchSelDpCParams )
	{
		ASSERT_RETURN;
	}

	if( false == ( (CDlgBatchSelDpC*)m_pParentWnd )->DpCBvCtrlTab_BS_ComboChange() )
	{
		int iPos = max( m_ComboFamilyBelow65.FindCBIDPtr( m_pclBatchSelDpCParams->m_clBatchSelBVParams.m_strComboFamilyBelow65ID ), 0 );

		if( iPos != m_ComboFamilyBelow65.GetCurSel() )
		{
			m_ComboFamilyBelow65.SetCurSel( iPos );
		}

		return;
	}

	m_pclBatchSelDpCParams->m_clBatchSelBVParams.m_strComboFamilyBelow65ID = m_ComboFamilyBelow65.GetCBCurSelIDPtr().ID;
	
	FillComboBoxBodyMaterial( CDlgBatchSelBase::AboveOrBelow::Below65 );
	FillComboBoxConnect( CDlgBatchSelBase::AboveOrBelow::Below65 );
	FillComboBoxVersion( CDlgBatchSelBase::AboveOrBelow::Below65 );
	FillComboBoxPN( CDlgBatchSelBase::AboveOrBelow::Below65 );
	( (CDlgBatchSelDpC*)m_pParentWnd )->DpCBvCtrlTab_NotificationCbnChange();
}

void CDlgBatchSelDpC_BvTab::OnComboSelChangeBdyMatBelow65()
{
	if( m_pclBatchSelDpCParams->m_clBatchSelBVParams.m_strComboMaterialBelow65ID == m_ComboBodyMaterialBelow65.GetCBCurSelIDPtr().ID )
	{
		// Do nothing if it's the same value as before.
		return;
	}

	if( NULL == m_pclBatchSelDpCParams )
	{
		ASSERT_RETURN;
	}

	if( false == ( (CDlgBatchSelDpC*)m_pParentWnd )->DpCBvCtrlTab_BS_ComboChange() )
	{
		int iPos = max( m_ComboBodyMaterialBelow65.FindCBIDPtr( m_pclBatchSelDpCParams->m_clBatchSelBVParams.m_strComboMaterialBelow65ID ), 0 );

		if( iPos != m_ComboBodyMaterialBelow65.GetCurSel() )
		{
			m_ComboBodyMaterialBelow65.SetCurSel( iPos );
		}

		return;
	}

	m_pclBatchSelDpCParams->m_clBatchSelBVParams.m_strComboMaterialBelow65ID = m_ComboBodyMaterialBelow65.GetCBCurSelIDPtr().ID;

	FillComboBoxConnect( CDlgBatchSelBase::AboveOrBelow::Below65 );
	FillComboBoxVersion( CDlgBatchSelBase::AboveOrBelow::Below65 );
	FillComboBoxPN( CDlgBatchSelBase::AboveOrBelow::Below65 );
	( (CDlgBatchSelDpC*)m_pParentWnd )->DpCBvCtrlTab_NotificationCbnChange();
}

void CDlgBatchSelDpC_BvTab::OnComboSelChangeConnectBelow65()
{
	if( m_pclBatchSelDpCParams->m_clBatchSelBVParams.m_strComboConnectBelow65ID == m_ComboConnectBelow65.GetCBCurSelIDPtr().ID )
	{
		// Do nothing if it's the same value as before.
		return;
	}

	if( NULL == m_pclBatchSelDpCParams )
	{
		ASSERT_RETURN;
	}

	if( false == ( (CDlgBatchSelDpC*)m_pParentWnd )->DpCBvCtrlTab_BS_ComboChange() )
	{
		int iPos = max( m_ComboConnectBelow65.FindCBIDPtr( m_pclBatchSelDpCParams->m_clBatchSelBVParams.m_strComboConnectBelow65ID ), 0 );

		if( iPos != m_ComboConnectBelow65.GetCurSel() )
		{
			m_ComboConnectBelow65.SetCurSel( iPos );
		}

		return;
	}

	m_pclBatchSelDpCParams->m_clBatchSelBVParams.m_strComboConnectBelow65ID = m_ComboConnectBelow65.GetCBCurSelIDPtr().ID;

	FillComboBoxVersion( CDlgBatchSelBase::AboveOrBelow::Below65 );
	FillComboBoxPN( CDlgBatchSelBase::AboveOrBelow::Below65 );
	( (CDlgBatchSelDpC*)m_pParentWnd )->DpCBvCtrlTab_NotificationCbnChange();
}

void CDlgBatchSelDpC_BvTab::OnComboSelChangeVersionBelow65()
{
	if( m_pclBatchSelDpCParams->m_clBatchSelBVParams.m_strComboVersionBelow65ID == m_ComboVersionBelow65.GetCBCurSelIDPtr().ID )
	{
		// Do nothing if it's the same value as before.
		return;
	}

	if( NULL == m_pclBatchSelDpCParams )
	{
		ASSERT_RETURN;
	}

	if( false == ( (CDlgBatchSelDpC*)m_pParentWnd )->DpCBvCtrlTab_BS_ComboChange() )
	{
		int iPos = max( m_ComboVersionBelow65.FindCBIDPtr( m_pclBatchSelDpCParams->m_clBatchSelBVParams.m_strComboVersionBelow65ID ), 0 );

		if( iPos != m_ComboVersionBelow65.GetCurSel() )
		{
			m_ComboVersionBelow65.SetCurSel( iPos );
		}

		return;
	}

	m_pclBatchSelDpCParams->m_clBatchSelBVParams.m_strComboVersionBelow65ID = m_ComboVersionBelow65.GetCBCurSelIDPtr().ID;

	FillComboBoxPN( CDlgBatchSelBase::AboveOrBelow::Below65 );
	( (CDlgBatchSelDpC*)m_pParentWnd )->DpCBvCtrlTab_NotificationCbnChange();
}

void CDlgBatchSelDpC_BvTab::OnComboSelChangePNBelow65()
{
	if( m_pclBatchSelDpCParams->m_clBatchSelBVParams.m_strComboPNBelow65ID == m_ComboPNBelow65.GetCBCurSelIDPtr().ID )
	{
		// Do nothing if it's the same value as before.
		return;
	}

	if( NULL == m_pclBatchSelDpCParams )
	{
		ASSERT_RETURN;
	}

	if( false == ( (CDlgBatchSelDpC*)m_pParentWnd )->DpCBvCtrlTab_BS_ComboChange( true ) )
	{
		int iPos = max( m_ComboPNBelow65.FindCBIDPtr( m_pclBatchSelDpCParams->m_clBatchSelBVParams.m_strComboPNBelow65ID ), 0 );

		if( iPos != m_ComboPNBelow65.GetCurSel() )
		{
			m_ComboPNBelow65.SetCurSel( iPos );
		}

		return;
	}

	m_pclBatchSelDpCParams->m_clBatchSelBVParams.m_strComboPNBelow65ID = m_ComboPNBelow65.GetCBCurSelIDPtr().ID;

	( (CDlgBatchSelDpC*)m_pParentWnd )->DpCBvCtrlTab_NotificationCbnChange();
}

void CDlgBatchSelDpC_BvTab::OnComboSelChangeTypeAbove50()
{
	if( m_pclBatchSelDpCParams->m_clBatchSelBVParams.m_strComboTypeAbove50ID == m_ComboTypeAbove50.GetCBCurSelIDPtr().ID )
	{
		// Do nothing if it's the same value as before.
		return;
	}

	if( NULL == m_pclBatchSelDpCParams )
	{
		ASSERT_RETURN;
	}

	if( false == ( (CDlgBatchSelDpC*)m_pParentWnd )->DpCBvCtrlTab_BS_ComboChange() )
	{
		int iPos = max( m_ComboTypeAbove50.FindCBIDPtr( m_pclBatchSelDpCParams->m_clBatchSelBVParams.m_strComboTypeAbove50ID ), 0 );

		if( iPos != m_ComboTypeAbove50.GetCurSel() )
		{
			m_ComboTypeAbove50.SetCurSel( iPos );
		}

		return;
	}

	m_pclBatchSelDpCParams->m_clBatchSelBVParams.m_strComboTypeAbove50ID = m_ComboTypeAbove50.GetCBCurSelIDPtr().ID;

	FillComboBoxFamily( CDlgBatchSelBase::AboveOrBelow::Above50 );
	FillComboBoxBodyMaterial( CDlgBatchSelBase::AboveOrBelow::Above50 );
	FillComboBoxConnect( CDlgBatchSelBase::AboveOrBelow::Above50 );
	FillComboBoxVersion( CDlgBatchSelBase::AboveOrBelow::Above50 );
	FillComboBoxPN( CDlgBatchSelBase::AboveOrBelow::Above50 );
	( (CDlgBatchSelDpC*)m_pParentWnd )->DpCBvCtrlTab_NotificationCbnChange();
}

void CDlgBatchSelDpC_BvTab::OnComboSelChangeFamilyAbove50()
{
	if( m_pclBatchSelDpCParams->m_clBatchSelBVParams.m_strComboFamilyAbove50ID == m_ComboFamilyAbove50.GetCBCurSelIDPtr().ID )
	{
		// Do nothing if it's the same value as before.
		return;
	}

	if( NULL == m_pclBatchSelDpCParams )
	{
		ASSERT_RETURN;
	}

	if( false == ( (CDlgBatchSelDpC*)m_pParentWnd )->DpCBvCtrlTab_BS_ComboChange() )
	{
		int iPos = max( m_ComboFamilyAbove50.FindCBIDPtr( m_pclBatchSelDpCParams->m_clBatchSelBVParams.m_strComboFamilyAbove50ID ), 0 );

		if( iPos != m_ComboFamilyAbove50.GetCurSel() )
		{
			m_ComboFamilyAbove50.SetCurSel( iPos );
		}

		return;
	}

	m_pclBatchSelDpCParams->m_clBatchSelBVParams.m_strComboFamilyAbove50ID = m_ComboFamilyAbove50.GetCBCurSelIDPtr().ID;

	FillComboBoxBodyMaterial( CDlgBatchSelBase::AboveOrBelow::Above50 );
	FillComboBoxConnect( CDlgBatchSelBase::AboveOrBelow::Above50 );
	FillComboBoxVersion( CDlgBatchSelBase::AboveOrBelow::Above50 );
	FillComboBoxPN( CDlgBatchSelBase::AboveOrBelow::Above50 );
	( (CDlgBatchSelDpC*)m_pParentWnd )->DpCBvCtrlTab_NotificationCbnChange();
}

void CDlgBatchSelDpC_BvTab::OnComboSelChangeBdyMatAbove50()
{
	if( m_pclBatchSelDpCParams->m_clBatchSelBVParams.m_strComboMaterialAbove50ID == m_ComboBodyMaterialAbove50.GetCBCurSelIDPtr().ID )
	{
		// Do nothing if it's the same value as before.
		return;
	}

	if( NULL == m_pclBatchSelDpCParams )
	{
		ASSERT_RETURN;
	}

	if( false == ( (CDlgBatchSelDpC*)m_pParentWnd )->DpCBvCtrlTab_BS_ComboChange() )
	{
		int iPos = max( m_ComboBodyMaterialAbove50.FindCBIDPtr( m_pclBatchSelDpCParams->m_clBatchSelBVParams.m_strComboMaterialAbove50ID ), 0 );

		if( iPos != m_ComboBodyMaterialAbove50.GetCurSel() )
		{
			m_ComboBodyMaterialAbove50.SetCurSel( iPos );
		}

		return;
	}

	m_pclBatchSelDpCParams->m_clBatchSelBVParams.m_strComboMaterialAbove50ID = m_ComboBodyMaterialAbove50.GetCBCurSelIDPtr().ID;

	FillComboBoxConnect( CDlgBatchSelBase::AboveOrBelow::Above50 );
	FillComboBoxVersion( CDlgBatchSelBase::AboveOrBelow::Above50 );
	FillComboBoxPN( CDlgBatchSelBase::AboveOrBelow::Above50 );
	( (CDlgBatchSelDpC*)m_pParentWnd )->DpCBvCtrlTab_NotificationCbnChange();
}

void CDlgBatchSelDpC_BvTab::OnComboSelChangeConnectAbove50()
{
	if( m_pclBatchSelDpCParams->m_clBatchSelBVParams.m_strComboConnectAbove50ID == m_ComboConnectAbove50.GetCBCurSelIDPtr().ID )
	{
		// Do nothing if it's the same value as before.
		return;
	}

	if( NULL == m_pclBatchSelDpCParams )
	{
		ASSERT_RETURN;
	}

	if( false == ( (CDlgBatchSelDpC*)m_pParentWnd )->DpCBvCtrlTab_BS_ComboChange() )
	{
		int iPos = max( m_ComboConnectAbove50.FindCBIDPtr( m_pclBatchSelDpCParams->m_clBatchSelBVParams.m_strComboConnectAbove50ID ), 0 );

		if( iPos != m_ComboConnectAbove50.GetCurSel() )
		{
			m_ComboConnectAbove50.SetCurSel( iPos );
		}

		return;
	}

	m_pclBatchSelDpCParams->m_clBatchSelBVParams.m_strComboConnectAbove50ID = m_ComboConnectAbove50.GetCBCurSelIDPtr().ID;

	FillComboBoxVersion( CDlgBatchSelBase::AboveOrBelow::Above50 );
	FillComboBoxPN( CDlgBatchSelBase::AboveOrBelow::Above50 );
	( (CDlgBatchSelDpC*)m_pParentWnd )->DpCBvCtrlTab_NotificationCbnChange();
}

void CDlgBatchSelDpC_BvTab::OnComboSelChangeVersionAbove50()
{
	if( m_pclBatchSelDpCParams->m_clBatchSelBVParams.m_strComboVersionAbove50ID == m_ComboVersionAbove50.GetCBCurSelIDPtr().ID )
	{
		// Do nothing if it's the same value as before.
		return;
	}

	if( NULL == m_pclBatchSelDpCParams )
	{
		ASSERT_RETURN;
	}

	if( false == ( (CDlgBatchSelDpC*)m_pParentWnd )->DpCBvCtrlTab_BS_ComboChange() )
	{
		int iPos = max( m_ComboVersionAbove50.FindCBIDPtr( m_pclBatchSelDpCParams->m_clBatchSelBVParams.m_strComboVersionAbove50ID ), 0 );

		if( iPos != m_ComboVersionAbove50.GetCurSel() )
		{
			m_ComboVersionAbove50.SetCurSel( iPos );
		}

		return;
	}

	m_pclBatchSelDpCParams->m_clBatchSelBVParams.m_strComboVersionAbove50ID = m_ComboVersionAbove50.GetCBCurSelIDPtr().ID;

	FillComboBoxPN( CDlgBatchSelBase::AboveOrBelow::Above50 );
	( (CDlgBatchSelDpC*)m_pParentWnd )->DpCBvCtrlTab_NotificationCbnChange();
}

void CDlgBatchSelDpC_BvTab::OnComboSelChangePNAbove50()
{
	if( m_pclBatchSelDpCParams->m_clBatchSelBVParams.m_strComboPNAbove50ID == m_ComboPNAbove50.GetCBCurSelIDPtr().ID )
	{
		// Do nothing if it's the same value as before.
		return;
	}

	if( NULL == m_pclBatchSelDpCParams )
	{
		ASSERT_RETURN;
	}

	if( false == ( (CDlgBatchSelDpC*)m_pParentWnd )->DpCBvCtrlTab_BS_ComboChange( true ) )
	{
		int iPos = max( m_ComboPNAbove50.FindCBIDPtr( m_pclBatchSelDpCParams->m_clBatchSelBVParams.m_strComboPNAbove50ID ), 0 );

		if( iPos != m_ComboPNAbove50.GetCurSel() )
		{
			m_ComboPNAbove50.SetCurSel( iPos );
		}

		return;
	}

	m_pclBatchSelDpCParams->m_clBatchSelBVParams.m_strComboPNAbove50ID = m_ComboPNAbove50.GetCBCurSelIDPtr().ID;

	( (CDlgBatchSelDpC*)m_pParentWnd )->DpCBvCtrlTab_NotificationCbnChange();
}

void CDlgBatchSelDpC_BvTab::FillComboBoxType( CDlgBatchSelBase::AboveOrBelow eAboveOrBelow, CString strTypeID )
{
	if( NULL == m_pclBatchSelDpCParams || NULL == m_pclBatchSelDpCParams->m_pTADB )
	{
		ASSERT_RETURN;
	}

	int iDN;
	CExtNumEditComboBox *pCCombo;
	CRankEx TypeList;

	CTableDN *pclTableDN = (CTableDN *)( m_pclBatchSelDpCParams->m_pTADB->Get( _T("DN_TAB") ).MP );

	if( NULL == pclTableDN )
	{
		ASSERT_RETURN;
	}

	if( CDlgBatchSelBase::AboveOrBelow::Above50 == eAboveOrBelow )
	{
		iDN = pclTableDN->GetSize( _T("DN_65") );
		pCCombo = &m_ComboTypeAbove50;
	}
	else
	{
		iDN = pclTableDN->GetSize( _T("DN_50") );
		pCCombo = &m_ComboTypeBelow65;
	}
	
	CRankEx MeasValveList;

	if( CDlgBatchSelBase::AboveOrBelow::Above50 == eAboveOrBelow )
	{
		m_pclBatchSelDpCParams->m_pTADB->GetBVList( &MeasValveList, _T(""), _T(""), _T(""), _T(""), _T(""),
				m_pclBatchSelDpCParams->m_eFilterSelection, iDN, INT_MAX, NULL, true);
	}
	else
	{
		m_pclBatchSelDpCParams->m_pTADB->GetBVList( &MeasValveList, _T(""), _T(""), _T(""), _T(""), _T(""),
				m_pclBatchSelDpCParams->m_eFilterSelection, 0, iDN, NULL, true);
	}
	
	CRankEx TempList;
	m_pclBatchSelDpCParams->m_pTADB->GetBVTypeList( &MeasValveList, &TempList );

	// TO REMOVE -> Fixed orifice are not yet implemented in the batch selection.
	_string str;
	LPARAM lParam;

	for( BOOL fContinue = TempList.GetFirst( str, lParam ); TRUE == fContinue; fContinue = TempList.GetNext( str, lParam ) )
	{
		CDB_StringID *pTypeID = (CDB_StringID *)lParam;

		if( 0 != CString( pTypeID->GetIDPtr().ID ).Compare( _T("RVTYPE_FO") ) )
		{
			TypeList.Add( pTypeID->GetString(), _ttoi( pTypeID->GetIDstr() ) - 1, ( LPARAM )pTypeID );
		}
	}

	pCCombo->FillInCombo( &TypeList, strTypeID );

	if( CDlgBatchSelBase::AboveOrBelow::Above50 == eAboveOrBelow )
	{
		m_pclBatchSelDpCParams->m_clBatchSelBVParams.m_strComboTypeAbove50ID = pCCombo->GetCBCurSelIDPtr().ID;
	}
	else
	{
		m_pclBatchSelDpCParams->m_clBatchSelBVParams.m_strComboTypeBelow65ID = pCCombo->GetCBCurSelIDPtr().ID;
	}
}

void CDlgBatchSelDpC_BvTab::FillComboBoxFamily( CDlgBatchSelBase::AboveOrBelow eAboveOrBelow, CString strFamilyID )
{
	if( NULL == m_pclBatchSelDpCParams || NULL == m_pclBatchSelDpCParams->m_pTADB )
	{
		ASSERT_RETURN;
	}

	int iDN;
	CExtNumEditComboBox *pCCombo;
	CRankEx FamilyList;

	CTableDN *pclTableDN = (CTableDN *)( m_pclBatchSelDpCParams->m_pTADB->Get( _T("DN_TAB") ).MP );
	
	if( NULL == pclTableDN )
	{
		ASSERT_RETURN;
	}

	if( CDlgBatchSelBase::AboveOrBelow::Above50 == eAboveOrBelow )
	{
		iDN = pclTableDN->GetSize( _T("DN_65") );
		pCCombo = &m_ComboFamilyAbove50;
	}
	else
	{
		iDN = pclTableDN->GetSize( _T("DN_50") );
		pCCombo = &m_ComboFamilyBelow65;
	}

	CRankEx MeasValveList;

	if( CDlgBatchSelBase::AboveOrBelow::Above50 == eAboveOrBelow )
	{
		m_pclBatchSelDpCParams->m_pTADB->GetBVList( &MeasValveList, (LPCTSTR)m_pclBatchSelDpCParams->m_clBatchSelBVParams.m_strComboTypeAbove50ID, 
			_T(""), _T(""), _T(""), _T(""), m_pclBatchSelDpCParams->m_eFilterSelection, iDN, INT_MAX, NULL, true );
	}
	else
	{
		m_pclBatchSelDpCParams->m_pTADB->GetBVList( &MeasValveList, (LPCTSTR)m_pclBatchSelDpCParams->m_clBatchSelBVParams.m_strComboTypeBelow65ID, 
			_T(""), _T(""), _T(""), _T(""), m_pclBatchSelDpCParams->m_eFilterSelection, 0, iDN, NULL, true);
	}
	
	m_pclBatchSelDpCParams->m_pTADB->GetBVFamilyList( &MeasValveList, &FamilyList, _T("") );
	pCCombo->FillInCombo( &FamilyList, strFamilyID );

	if( CDlgBatchSelBase::AboveOrBelow::Above50 == eAboveOrBelow )
	{
		m_pclBatchSelDpCParams->m_clBatchSelBVParams.m_strComboFamilyAbove50ID = pCCombo->GetCBCurSelIDPtr().ID;
	}
	else
	{
		m_pclBatchSelDpCParams->m_clBatchSelBVParams.m_strComboFamilyBelow65ID = pCCombo->GetCBCurSelIDPtr().ID;
	}
}

void CDlgBatchSelDpC_BvTab::FillComboBoxBodyMaterial( CDlgBatchSelBase::AboveOrBelow eAboveOrBelow, CString strBdyMatID )
{
	if( NULL == m_pclBatchSelDpCParams || NULL == m_pclBatchSelDpCParams->m_pTADB )
	{
		ASSERT_RETURN;
	}

	int iDN;
	CExtNumEditComboBox *pCCombo;
	CRankEx BodyMaterialList;

	CTableDN *pclTableDN = (CTableDN *)( m_pclBatchSelDpCParams->m_pTADB->Get( _T("DN_TAB") ).MP );
	
	if( NULL == pclTableDN )
	{
		ASSERT_RETURN;
	}

	if( CDlgBatchSelBase::AboveOrBelow::Above50 == eAboveOrBelow )
	{
		iDN = pclTableDN->GetSize( _T("DN_65") );
		pCCombo = &m_ComboBodyMaterialAbove50;
	}
	else
	{
		iDN = pclTableDN->GetSize( _T("DN_50") );
		pCCombo = &m_ComboBodyMaterialBelow65;
	}

	CRankEx MeasValveList;

	if( CDlgBatchSelBase::AboveOrBelow::Above50 == eAboveOrBelow )
	{
		m_pclBatchSelDpCParams->m_pTADB->GetBVList( &MeasValveList, (LPCTSTR)m_pclBatchSelDpCParams->m_clBatchSelBVParams.m_strComboTypeAbove50ID, 
				(LPCTSTR)m_pclBatchSelDpCParams->m_clBatchSelBVParams.m_strComboFamilyAbove50ID, _T(""), _T(""), _T(""),
				m_pclBatchSelDpCParams->m_eFilterSelection, iDN, INT_MAX, NULL, true);
	}
	else
	{
		m_pclBatchSelDpCParams->m_pTADB->GetBVList( &MeasValveList, (LPCTSTR)m_pclBatchSelDpCParams->m_clBatchSelBVParams.m_strComboTypeBelow65ID, 
				(LPCTSTR)m_pclBatchSelDpCParams->m_clBatchSelBVParams.m_strComboFamilyBelow65ID, _T(""), _T(""), _T(""),
				m_pclBatchSelDpCParams->m_eFilterSelection, 0, iDN, NULL, true);
	}

	m_pclBatchSelDpCParams->m_pTADB->GetBVBdyMatList( &MeasValveList, &BodyMaterialList, _T(""), _T("") );
	pCCombo->FillInCombo( &BodyMaterialList, strBdyMatID );

	if( CDlgBatchSelBase::AboveOrBelow::Above50 == eAboveOrBelow )
	{
		m_pclBatchSelDpCParams->m_clBatchSelBVParams.m_strComboMaterialAbove50ID = pCCombo->GetCBCurSelIDPtr().ID;
	}
	else
	{
		m_pclBatchSelDpCParams->m_clBatchSelBVParams.m_strComboMaterialBelow65ID = pCCombo->GetCBCurSelIDPtr().ID;
	}
}

void CDlgBatchSelDpC_BvTab::FillComboBoxConnect( CDlgBatchSelBase::AboveOrBelow eAboveOrBelow, CString strConnectID )
{
	if( NULL == m_pclBatchSelDpCParams || NULL == m_pclBatchSelDpCParams->m_pTADB )
	{
		ASSERT_RETURN;
	}

	int iDN;
	CExtNumEditComboBox *pCCombo;
	CRankEx ConnectList;

	CTableDN *pclTableDN = (CTableDN *)( m_pclBatchSelDpCParams->m_pTADB->Get( _T("DN_TAB") ).MP );
	
	if( NULL == pclTableDN )
	{
		ASSERT_RETURN;
	}
	
	if( CDlgBatchSelBase::AboveOrBelow::Above50 == eAboveOrBelow )
	{
		iDN = pclTableDN->GetSize( _T("DN_65") );
		pCCombo = &m_ComboConnectAbove50;
	}
	else
	{
		iDN = pclTableDN->GetSize( _T("DN_50") );
		pCCombo = &m_ComboConnectBelow65;
	}

	CRankEx MeasValveList;

	if( CDlgBatchSelBase::AboveOrBelow::Above50 == eAboveOrBelow )
	{
		m_pclBatchSelDpCParams->m_pTADB->GetBVList( &MeasValveList, (LPCTSTR)m_pclBatchSelDpCParams->m_clBatchSelBVParams.m_strComboTypeAbove50ID, 
				(LPCTSTR)m_pclBatchSelDpCParams->m_clBatchSelBVParams.m_strComboFamilyAbove50ID, 
				(LPCTSTR)m_pclBatchSelDpCParams->m_clBatchSelBVParams.m_strComboMaterialAbove50ID, _T(""), _T(""),
				m_pclBatchSelDpCParams->m_eFilterSelection, iDN, INT_MAX, NULL, true );
	}
	else
	{
		m_pclBatchSelDpCParams->m_pTADB->GetBVList( &MeasValveList, (LPCTSTR)m_pclBatchSelDpCParams->m_clBatchSelBVParams.m_strComboTypeBelow65ID, 
				(LPCTSTR)m_pclBatchSelDpCParams->m_clBatchSelBVParams.m_strComboFamilyBelow65ID, 
				(LPCTSTR)m_pclBatchSelDpCParams->m_clBatchSelBVParams.m_strComboMaterialBelow65ID, _T(""), _T(""),
				m_pclBatchSelDpCParams->m_eFilterSelection, 0, iDN, NULL, true );
	}

	m_pclBatchSelDpCParams->m_pTADB->GetBVConnList( &MeasValveList, &ConnectList, _T(""), _T(""), _T("") );
	pCCombo->FillInCombo( &ConnectList, strConnectID );

	if( CDlgBatchSelBase::AboveOrBelow::Above50 == eAboveOrBelow )
	{
		m_pclBatchSelDpCParams->m_clBatchSelBVParams.m_strComboConnectAbove50ID = pCCombo->GetCBCurSelIDPtr().ID;
	}
	else
	{
		m_pclBatchSelDpCParams->m_clBatchSelBVParams.m_strComboConnectBelow65ID = pCCombo->GetCBCurSelIDPtr().ID;
	}
}

void CDlgBatchSelDpC_BvTab::FillComboBoxVersion( CDlgBatchSelBase::AboveOrBelow eAboveOrBelow, CString strVersionID )
{
	if( NULL == m_pclBatchSelDpCParams || NULL == m_pclBatchSelDpCParams->m_pTADB )
	{
		ASSERT_RETURN;
	}

	// Sanity incorrect version stored, clean it
	if (L"VERS_NODRAIN" == strVersionID)
	{
		strVersionID.Empty();
	}

	int iDN;
	CExtNumEditComboBox *pCCombo;
	CRankEx VersionList;

	CTableDN *pclTableDN = (CTableDN *)( m_pclBatchSelDpCParams->m_pTADB->Get( _T("DN_TAB") ).MP );
	
	if( NULL == pclTableDN )
	{
		ASSERT_RETURN;
	}
	
	if( CDlgBatchSelBase::AboveOrBelow::Above50 == eAboveOrBelow )
	{
		iDN = pclTableDN->GetSize( _T("DN_65") );
		pCCombo = &m_ComboVersionAbove50;
	}
	else
	{
		iDN = pclTableDN->GetSize( _T("DN_50") );
		pCCombo = &m_ComboVersionBelow65;
	}

	CRankEx MeasValveList;

	if( CDlgBatchSelBase::AboveOrBelow::Above50 == eAboveOrBelow )
	{
		m_pclBatchSelDpCParams->m_pTADB->GetBVList( &MeasValveList, (LPCTSTR)m_pclBatchSelDpCParams->m_clBatchSelBVParams.m_strComboTypeAbove50ID, 
				(LPCTSTR)m_pclBatchSelDpCParams->m_clBatchSelBVParams.m_strComboFamilyAbove50ID, 
				(LPCTSTR)m_pclBatchSelDpCParams->m_clBatchSelBVParams.m_strComboMaterialAbove50ID, 
				(LPCTSTR)m_pclBatchSelDpCParams->m_clBatchSelBVParams.m_strComboConnectAbove50ID, (LPCTSTR)strVersionID, 
				m_pclBatchSelDpCParams->m_eFilterSelection, iDN, INT_MAX, NULL, true );
	}
	else
	{
		m_pclBatchSelDpCParams->m_pTADB->GetBVList( &MeasValveList, (LPCTSTR)m_pclBatchSelDpCParams->m_clBatchSelBVParams.m_strComboTypeBelow65ID, 
				(LPCTSTR)m_pclBatchSelDpCParams->m_clBatchSelBVParams.m_strComboFamilyBelow65ID, 
				(LPCTSTR)m_pclBatchSelDpCParams->m_clBatchSelBVParams.m_strComboMaterialBelow65ID, 
				(LPCTSTR)m_pclBatchSelDpCParams->m_clBatchSelBVParams.m_strComboConnectBelow65ID, (LPCTSTR)strVersionID, 
				m_pclBatchSelDpCParams->m_eFilterSelection, 0, iDN, NULL, true );
	}

	m_pclBatchSelDpCParams->m_pTADB->GetBVVersList( &MeasValveList, &VersionList, _T(""), _T(""), _T(""), _T("") );
	pCCombo->FillInCombo( &VersionList, strVersionID );

	if( CDlgBatchSelBase::AboveOrBelow::Above50 == eAboveOrBelow )
	{
		m_pclBatchSelDpCParams->m_clBatchSelBVParams.m_strComboVersionAbove50ID = pCCombo->GetCBCurSelIDPtr().ID;
	}
	else
	{
		m_pclBatchSelDpCParams->m_clBatchSelBVParams.m_strComboVersionBelow65ID = pCCombo->GetCBCurSelIDPtr().ID;
	}
}

void CDlgBatchSelDpC_BvTab::FillComboBoxPN( CDlgBatchSelBase::AboveOrBelow eAboveOrBelow, CString strPNID )
{
	if( NULL == m_pclBatchSelDpCParams || NULL == m_pclBatchSelDpCParams->m_pTADB )
	{
		ASSERT_RETURN;
	}

	int iDN;
	CExtNumEditComboBox *pCCombo;
	CRankEx PNList;

	CTableDN *pclTableDN = (CTableDN *)( m_pclBatchSelDpCParams->m_pTADB->Get( _T("DN_TAB") ).MP );
	
	if( NULL == pclTableDN )
	{
		ASSERT_RETURN;
	}
	
	if( CDlgBatchSelBase::AboveOrBelow::Above50 == eAboveOrBelow )
	{
		iDN = pclTableDN->GetSize( _T("DN_65") );
		pCCombo = &m_ComboPNAbove50;
	}
	else
	{
		iDN = pclTableDN->GetSize( _T("DN_50") );
		pCCombo = &m_ComboPNBelow65;
	}

	CRankEx MeasValveList;

	if( CDlgBatchSelBase::AboveOrBelow::Above50 == eAboveOrBelow )
	{
		m_pclBatchSelDpCParams->m_pTADB->GetBVList( &MeasValveList, (LPCTSTR)m_pclBatchSelDpCParams->m_clBatchSelBVParams.m_strComboTypeAbove50ID, 
				(LPCTSTR)m_pclBatchSelDpCParams->m_clBatchSelBVParams.m_strComboFamilyAbove50ID, 
				(LPCTSTR)m_pclBatchSelDpCParams->m_clBatchSelBVParams.m_strComboMaterialAbove50ID, 
				(LPCTSTR)m_pclBatchSelDpCParams->m_clBatchSelBVParams.m_strComboConnectAbove50ID, 
				(LPCTSTR)m_pclBatchSelDpCParams->m_clBatchSelBVParams.m_strComboVersionAbove50ID, 
				m_pclBatchSelDpCParams->m_eFilterSelection, iDN, INT_MAX, NULL, true);
	}
	else
	{
		m_pclBatchSelDpCParams->m_pTADB->GetBVList( &MeasValveList, (LPCTSTR)m_pclBatchSelDpCParams->m_clBatchSelBVParams.m_strComboTypeBelow65ID, 
				(LPCTSTR)m_pclBatchSelDpCParams->m_clBatchSelBVParams.m_strComboFamilyBelow65ID, 
				(LPCTSTR)m_pclBatchSelDpCParams->m_clBatchSelBVParams.m_strComboMaterialBelow65ID, 
				(LPCTSTR)m_pclBatchSelDpCParams->m_clBatchSelBVParams.m_strComboConnectBelow65ID, 
				(LPCTSTR)m_pclBatchSelDpCParams->m_clBatchSelBVParams.m_strComboVersionBelow65ID, 
				m_pclBatchSelDpCParams->m_eFilterSelection, 0, iDN, NULL, true);
	}

	m_pclBatchSelDpCParams->m_pTADB->GetBVPNList( &MeasValveList, &PNList, _T(""), _T(""), _T(""), _T(""), _T("") );
	pCCombo->FillInCombo( &PNList, strPNID );

	if( CDlgBatchSelBase::AboveOrBelow::Above50 == eAboveOrBelow )
	{
		m_pclBatchSelDpCParams->m_clBatchSelBVParams.m_strComboPNAbove50ID = pCCombo->GetCBCurSelIDPtr().ID;
	}
	else
	{
		m_pclBatchSelDpCParams->m_clBatchSelBVParams.m_strComboPNBelow65ID = pCCombo->GetCBCurSelIDPtr().ID;
	}
}
