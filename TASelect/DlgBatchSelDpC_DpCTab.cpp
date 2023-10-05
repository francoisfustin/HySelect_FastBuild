#include "stdafx.h"
#include <errno.h>
#include <algorithm>

#include "Taselect.h"
#include "MainFrm.h"

#include "ProductSelectionParameters.h"
#include "DlgBatchSelDpC.h"
#include "DlgBatchSelDpC_DpCTab.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CDlgBatchSelDpC_DpCTab::CDlgBatchSelDpC_DpCTab( CBatchSelDpCParams &clBatchSelDpCParams, CWnd *pParent )
	: CDialogExt( CDlgBatchSelDpC_DpCTab::IDD, pParent )
{
	m_pclBatchSelDpCParams = &clBatchSelDpCParams;
}

void CDlgBatchSelDpC_DpCTab::SaveSelectionParameters( void )
{
	if( NULL == m_pclBatchSelDpCParams || NULL == m_pclBatchSelDpCParams->m_pTADS || NULL == m_pclBatchSelDpCParams->m_pTADS->GetpBatchSelParameter() )
	{
		return;
	}
	
	CDS_BatchSelParameter *pclBatchSelParameters = m_pclBatchSelDpCParams->m_pTADS->GetpBatchSelParameter();

	pclBatchSelParameters->SetDpCTypeBelow65ID( m_pclBatchSelDpCParams->m_strComboTypeBelow65ID );
	pclBatchSelParameters->SetDpCFamilyBelow65ID( m_pclBatchSelDpCParams->m_strComboFamilyBelow65ID );
	pclBatchSelParameters->SetDpCMaterialBelow65ID( m_pclBatchSelDpCParams->m_strComboMaterialBelow65ID );
	pclBatchSelParameters->SetDpCConnectBelow65ID( m_pclBatchSelDpCParams->m_strComboConnectBelow65ID );
	pclBatchSelParameters->SetDpCVersionBelow65ID( m_pclBatchSelDpCParams->m_strComboVersionBelow65ID );
	pclBatchSelParameters->SetDpCPNBelow65ID( m_pclBatchSelDpCParams->m_strComboPNBelow65ID );
	
	pclBatchSelParameters->SetDpCTypeAbove50ID( m_pclBatchSelDpCParams->m_strComboTypeAbove50ID );
	pclBatchSelParameters->SetDpCFamilyAbove50ID( m_pclBatchSelDpCParams->m_strComboFamilyAbove50ID );
	pclBatchSelParameters->SetDpCMaterialAbove50ID( m_pclBatchSelDpCParams->m_strComboMaterialAbove50ID );
	pclBatchSelParameters->SetDpCConnectAbove50ID( m_pclBatchSelDpCParams->m_strComboConnectAbove50ID );
	pclBatchSelParameters->SetDpCVersionAbove50ID( m_pclBatchSelDpCParams->m_strComboVersionAbove50ID );
	pclBatchSelParameters->SetDpCPNAbove50ID( m_pclBatchSelDpCParams->m_strComboPNAbove50ID );
}

void CDlgBatchSelDpC_DpCTab::EnableCombos( bool bEnable )
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

void CDlgBatchSelDpC_DpCTab::UpdateCombos()
{
	FillComboBoxType( CDlgBatchSelBase::Below65 );
	//HYS-781
	FillComboBoxFamily(CDlgBatchSelBase::AboveOrBelow::Below65);
	FillComboBoxBodyMaterial(CDlgBatchSelBase::AboveOrBelow::Below65);
	FillComboBoxConnect(CDlgBatchSelBase::AboveOrBelow::Below65);
	FillComboBoxVersion(CDlgBatchSelBase::AboveOrBelow::Below65);
	FillComboBoxPN(CDlgBatchSelBase::AboveOrBelow::Below65);

	FillComboBoxType( CDlgBatchSelBase::Above50 );
	//HYS-781
	FillComboBoxFamily(CDlgBatchSelBase::AboveOrBelow::Above50);
	FillComboBoxBodyMaterial(CDlgBatchSelBase::AboveOrBelow::Above50);
	FillComboBoxConnect(CDlgBatchSelBase::AboveOrBelow::Above50);
	FillComboBoxVersion(CDlgBatchSelBase::AboveOrBelow::Above50);
	FillComboBoxPN(CDlgBatchSelBase::AboveOrBelow::Above50);
}

LRESULT CDlgBatchSelDpC_DpCTab::OnNewDocument( WPARAM wParam, LPARAM lParam )
{
	if( NULL == m_pclBatchSelDpCParams || NULL == m_pclBatchSelDpCParams->m_pTADS 
			|| NULL == m_pclBatchSelDpCParams->m_pTADS->GetpBatchSelParameter() )
	{
		return -1;
	}

	CDS_BatchSelParameter *pclBatchSelParameters = m_pclBatchSelDpCParams->m_pTADS->GetpBatchSelParameter();

	// Fill the combo box.
	// Remark: Corresponding combo variables in 'm_pclBatchSelDpCParams' are updated in each of this following methods.
	FillComboBoxType( CDlgBatchSelBase::AboveOrBelow::Below65, pclBatchSelParameters->GetDpCTypeBelow65ID() );
	FillComboBoxFamily( CDlgBatchSelBase::AboveOrBelow::Below65, pclBatchSelParameters->GetDpCFamilyBelow65ID() );
	FillComboBoxBodyMaterial( CDlgBatchSelBase::AboveOrBelow::Below65, pclBatchSelParameters->GetDpCMaterialBelow65ID() );
	FillComboBoxConnect( CDlgBatchSelBase::AboveOrBelow::Below65, pclBatchSelParameters->GetDpCConnectBelow65ID() );
	FillComboBoxVersion( CDlgBatchSelBase::AboveOrBelow::Below65, pclBatchSelParameters->GetDpCVersionBelow65ID() );
	FillComboBoxPN( CDlgBatchSelBase::AboveOrBelow::Below65, pclBatchSelParameters->GetDpCPNBelow65ID() );

	FillComboBoxType( CDlgBatchSelBase::AboveOrBelow::Above50, pclBatchSelParameters->GetDpCTypeAbove50ID() );
	FillComboBoxFamily( CDlgBatchSelBase::AboveOrBelow::Above50, pclBatchSelParameters->GetDpCFamilyAbove50ID() );
	FillComboBoxBodyMaterial( CDlgBatchSelBase::AboveOrBelow::Above50, pclBatchSelParameters->GetDpCMaterialAbove50ID() );
	FillComboBoxConnect( CDlgBatchSelBase::AboveOrBelow::Above50, pclBatchSelParameters->GetDpCConnectAbove50ID() );
	FillComboBoxVersion( CDlgBatchSelBase::AboveOrBelow::Above50, pclBatchSelParameters->GetDpCVersionAbove50ID() );
	FillComboBoxPN( CDlgBatchSelBase::AboveOrBelow::Above50, pclBatchSelParameters->GetDpCPNAbove50ID() );

	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PROTECTED MEMBERS
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP( CDlgBatchSelDpC_DpCTab, CDialogExt )
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

void CDlgBatchSelDpC_DpCTab::DoDataExchange( CDataExchange *pDX )
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

BOOL CDlgBatchSelDpC_DpCTab::OnInitDialog()
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

void CDlgBatchSelDpC_DpCTab::OnComboSelChangeTypeBelow65()
{
	if( NULL == m_pclBatchSelDpCParams )
	{
		ASSERT_RETURN;
	}

	if( m_pclBatchSelDpCParams->m_strComboTypeBelow65ID == m_ComboTypeBelow65.GetCBCurSelIDPtr().ID )
	{
		// Do nothing if it's the same value as before.
		return;
	}

	if( false == ( (CDlgBatchSelDpC*)m_pParentWnd )->DpCBvCtrlTab_BS_ComboChange() )
	{
		int iPos = max( m_ComboTypeBelow65.FindCBIDPtr( m_pclBatchSelDpCParams->m_strComboTypeBelow65ID ), 0 );

		if( iPos != m_ComboTypeBelow65.GetCurSel() )
		{
			m_ComboTypeBelow65.SetCurSel( iPos );
		}

		return;
	}

	m_pclBatchSelDpCParams->m_strComboTypeBelow65ID = m_ComboTypeBelow65.GetCBCurSelIDPtr().ID;

	FillComboBoxFamily( CDlgBatchSelBase::AboveOrBelow::Below65 );
	FillComboBoxBodyMaterial( CDlgBatchSelBase::AboveOrBelow::Below65 );
	FillComboBoxConnect( CDlgBatchSelBase::AboveOrBelow::Below65 );
	FillComboBoxVersion( CDlgBatchSelBase::AboveOrBelow::Below65 );
	FillComboBoxPN( CDlgBatchSelBase::AboveOrBelow::Below65 );
	( (CDlgBatchSelDpC*)m_pParentWnd )->DpCBvCtrlTab_NotificationCbnChange();
}

void CDlgBatchSelDpC_DpCTab::OnComboSelChangeFamilyBelow65()
{
	if( NULL == m_pclBatchSelDpCParams )
	{
		ASSERT_RETURN;
	}

	if( m_pclBatchSelDpCParams->m_strComboFamilyBelow65ID == m_ComboFamilyBelow65.GetCBCurSelIDPtr().ID )
	{
		// Do nothing if it's the same value as before.
		return;
	}

	if( false == ( (CDlgBatchSelDpC*)m_pParentWnd )->DpCBvCtrlTab_BS_ComboChange() )
	{
		int iPos = max( m_ComboFamilyBelow65.FindCBIDPtr( m_pclBatchSelDpCParams->m_strComboFamilyBelow65ID ), 0 );

		if( iPos != m_ComboFamilyBelow65.GetCurSel() )
		{
			m_ComboFamilyBelow65.SetCurSel( iPos );
		}

		return;
	}

	m_pclBatchSelDpCParams->m_strComboFamilyBelow65ID = m_ComboFamilyBelow65.GetCBCurSelIDPtr().ID;

	FillComboBoxBodyMaterial( CDlgBatchSelBase::AboveOrBelow::Below65 );
	FillComboBoxConnect( CDlgBatchSelBase::AboveOrBelow::Below65 );
	FillComboBoxVersion( CDlgBatchSelBase::AboveOrBelow::Below65 );
	FillComboBoxPN( CDlgBatchSelBase::AboveOrBelow::Below65 );
	( (CDlgBatchSelDpC*)m_pParentWnd )->DpCBvCtrlTab_NotificationCbnChange();
}

void CDlgBatchSelDpC_DpCTab::OnComboSelChangeBdyMatBelow65()
{
	if( NULL == m_pclBatchSelDpCParams )
	{
		ASSERT_RETURN;
	}

	if( m_pclBatchSelDpCParams->m_strComboMaterialBelow65ID == m_ComboBodyMaterialBelow65.GetCBCurSelIDPtr().ID )
	{
		// Do nothing if it's the same value as before.
		return;
	}

	if( false == ( (CDlgBatchSelDpC*)m_pParentWnd )->DpCBvCtrlTab_BS_ComboChange() )
	{
		int iPos = max( m_ComboBodyMaterialBelow65.FindCBIDPtr( m_pclBatchSelDpCParams->m_strComboMaterialBelow65ID ), 0 );

		if( iPos != m_ComboBodyMaterialBelow65.GetCurSel() )
		{
			m_ComboBodyMaterialBelow65.SetCurSel( iPos );
		}

		return;
	}

	m_pclBatchSelDpCParams->m_strComboMaterialBelow65ID = m_ComboBodyMaterialBelow65.GetCBCurSelIDPtr().ID;

	FillComboBoxConnect( CDlgBatchSelBase::AboveOrBelow::Below65 );
	FillComboBoxVersion( CDlgBatchSelBase::AboveOrBelow::Below65 );
	FillComboBoxPN( CDlgBatchSelBase::AboveOrBelow::Below65 );
	( (CDlgBatchSelDpC*)m_pParentWnd )->DpCBvCtrlTab_NotificationCbnChange();
}

void CDlgBatchSelDpC_DpCTab::OnComboSelChangeConnectBelow65()
{
	if( NULL == m_pclBatchSelDpCParams )
	{
		ASSERT_RETURN;
	}

	if( m_pclBatchSelDpCParams->m_strComboConnectBelow65ID == m_ComboConnectBelow65.GetCBCurSelIDPtr().ID )
	{
		// Do nothing if it's the same value as before.
		return;
	}

	if( false == ( (CDlgBatchSelDpC*)m_pParentWnd )->DpCBvCtrlTab_BS_ComboChange() )
	{
		int iPos = max( m_ComboConnectBelow65.FindCBIDPtr( m_pclBatchSelDpCParams->m_strComboConnectBelow65ID ), 0 );

		if( iPos != m_ComboConnectBelow65.GetCurSel() )
		{
			m_ComboConnectBelow65.SetCurSel( iPos );
		}

		return;
	}

	m_pclBatchSelDpCParams->m_strComboConnectBelow65ID = m_ComboConnectBelow65.GetCBCurSelIDPtr().ID;

	FillComboBoxVersion( CDlgBatchSelBase::AboveOrBelow::Below65 );
	FillComboBoxPN( CDlgBatchSelBase::AboveOrBelow::Below65 );
	( (CDlgBatchSelDpC*)m_pParentWnd )->DpCBvCtrlTab_NotificationCbnChange();
}

void CDlgBatchSelDpC_DpCTab::OnComboSelChangeVersionBelow65()
{
	if( NULL == m_pclBatchSelDpCParams )
	{
		ASSERT_RETURN;
	}

	if( m_pclBatchSelDpCParams->m_strComboVersionBelow65ID == m_ComboVersionBelow65.GetCBCurSelIDPtr().ID )
	{
		// Do nothing if it's the same value as before.
		return;
	}

	if( false == ( (CDlgBatchSelDpC*)m_pParentWnd )->DpCBvCtrlTab_BS_ComboChange() )
	{
		int iPos = max( m_ComboVersionBelow65.FindCBIDPtr( m_pclBatchSelDpCParams->m_strComboVersionBelow65ID ), 0 );

		if( iPos != m_ComboVersionBelow65.GetCurSel() )
		{
			m_ComboVersionBelow65.SetCurSel( iPos );
		}

		return;
	}

	m_pclBatchSelDpCParams->m_strComboVersionBelow65ID = m_ComboVersionBelow65.GetCBCurSelIDPtr().ID;

	FillComboBoxPN( CDlgBatchSelBase::AboveOrBelow::Below65 );
	( (CDlgBatchSelDpC*)m_pParentWnd )->DpCBvCtrlTab_NotificationCbnChange();
}

void CDlgBatchSelDpC_DpCTab::OnComboSelChangePNBelow65()
{
	if( NULL == m_pclBatchSelDpCParams )
	{
		ASSERT_RETURN;
	}

	if( m_pclBatchSelDpCParams->m_strComboPNBelow65ID == m_ComboPNBelow65.GetCBCurSelIDPtr().ID )
	{
		// Do nothing if it's the same value as before.
		return;
	}

	if( false == ( (CDlgBatchSelDpC*)m_pParentWnd )->DpCBvCtrlTab_BS_ComboChange( true ) )
	{
		int iPos = max( m_ComboPNBelow65.FindCBIDPtr( m_pclBatchSelDpCParams->m_strComboPNBelow65ID ), 0 );

		if( iPos != m_ComboPNBelow65.GetCurSel() )
		{
			m_ComboPNBelow65.SetCurSel( iPos );
		}

		return;
	}

	m_pclBatchSelDpCParams->m_strComboPNBelow65ID = m_ComboPNBelow65.GetCBCurSelIDPtr().ID;

	( (CDlgBatchSelDpC*)m_pParentWnd )->DpCBvCtrlTab_NotificationCbnChange();
}

void CDlgBatchSelDpC_DpCTab::OnComboSelChangeTypeAbove50()
{
	if( NULL == m_pclBatchSelDpCParams )
	{
		ASSERT_RETURN;
	}

	if( m_pclBatchSelDpCParams->m_strComboTypeAbove50ID == m_ComboTypeAbove50.GetCBCurSelIDPtr().ID )
	{
		// Do nothing if it's the same value as before.
		return;
	}

	if( false == ( (CDlgBatchSelDpC*)m_pParentWnd )->DpCBvCtrlTab_BS_ComboChange() )
	{
		int iPos = max( m_ComboTypeAbove50.FindCBIDPtr( m_pclBatchSelDpCParams->m_strComboTypeAbove50ID ), 0 );

		if( iPos != m_ComboTypeAbove50.GetCurSel() )
		{
			m_ComboTypeAbove50.SetCurSel( iPos );
		}

		return;
	}

	m_pclBatchSelDpCParams->m_strComboTypeAbove50ID = m_ComboTypeAbove50.GetCBCurSelIDPtr().ID;
	
	FillComboBoxFamily( CDlgBatchSelBase::AboveOrBelow::Above50 );
	FillComboBoxBodyMaterial( CDlgBatchSelBase::AboveOrBelow::Above50 );
	FillComboBoxConnect( CDlgBatchSelBase::AboveOrBelow::Above50 );
	FillComboBoxVersion( CDlgBatchSelBase::AboveOrBelow::Above50 );
	FillComboBoxPN( CDlgBatchSelBase::AboveOrBelow::Above50 );
	( (CDlgBatchSelDpC*)m_pParentWnd )->DpCBvCtrlTab_NotificationCbnChange();
}

void CDlgBatchSelDpC_DpCTab::OnComboSelChangeFamilyAbove50()
{
	if( NULL == m_pclBatchSelDpCParams )
	{
		ASSERT_RETURN;
	}

	if( m_pclBatchSelDpCParams->m_strComboFamilyAbove50ID == m_ComboFamilyAbove50.GetCBCurSelIDPtr().ID )
	{
		// Do nothing if it's the same value as before.
		return;
	}

	if( false == ( (CDlgBatchSelDpC*)m_pParentWnd )->DpCBvCtrlTab_BS_ComboChange() )
	{
		int iPos = max( m_ComboFamilyAbove50.FindCBIDPtr( m_pclBatchSelDpCParams->m_strComboFamilyAbove50ID ), 0 );

		if( iPos != m_ComboFamilyAbove50.GetCurSel() )
		{
			m_ComboFamilyAbove50.SetCurSel( iPos );
		}

		return;
	}

	m_pclBatchSelDpCParams->m_strComboFamilyAbove50ID = m_ComboFamilyAbove50.GetCBCurSelIDPtr().ID;
	
	FillComboBoxBodyMaterial( CDlgBatchSelBase::AboveOrBelow::Above50 );
	FillComboBoxConnect( CDlgBatchSelBase::AboveOrBelow::Above50 );
	FillComboBoxVersion( CDlgBatchSelBase::AboveOrBelow::Above50 );
	FillComboBoxPN( CDlgBatchSelBase::AboveOrBelow::Above50 );
	( (CDlgBatchSelDpC*)m_pParentWnd )->DpCBvCtrlTab_NotificationCbnChange();
}

void CDlgBatchSelDpC_DpCTab::OnComboSelChangeBdyMatAbove50()
{
	if( NULL == m_pclBatchSelDpCParams )
	{
		ASSERT_RETURN;
	}

	if( m_pclBatchSelDpCParams->m_strComboMaterialAbove50ID == m_ComboBodyMaterialAbove50.GetCBCurSelIDPtr().ID )
	{
		// Do nothing if it's the same value as before.
		return;
	}

	if( false == ( (CDlgBatchSelDpC*)m_pParentWnd )->DpCBvCtrlTab_BS_ComboChange() )
	{
		int iPos = max( m_ComboBodyMaterialAbove50.FindCBIDPtr( m_pclBatchSelDpCParams->m_strComboMaterialAbove50ID ), 0 );

		if( iPos != m_ComboBodyMaterialAbove50.GetCurSel() )
		{
			m_ComboBodyMaterialAbove50.SetCurSel( iPos );
		}

		return;
	}

	m_pclBatchSelDpCParams->m_strComboMaterialAbove50ID = m_ComboBodyMaterialAbove50.GetCBCurSelIDPtr().ID;
	
	FillComboBoxConnect( CDlgBatchSelBase::AboveOrBelow::Above50 );
	FillComboBoxVersion( CDlgBatchSelBase::AboveOrBelow::Above50 );
	FillComboBoxPN( CDlgBatchSelBase::AboveOrBelow::Above50 );
	( (CDlgBatchSelDpC*)m_pParentWnd )->DpCBvCtrlTab_NotificationCbnChange();
}

void CDlgBatchSelDpC_DpCTab::OnComboSelChangeConnectAbove50()
{
	if( NULL == m_pclBatchSelDpCParams )
	{
		ASSERT_RETURN;
	}

	if( m_pclBatchSelDpCParams->m_strComboConnectAbove50ID == m_ComboConnectAbove50.GetCBCurSelIDPtr().ID )
	{
		// Do nothing if it's the same value as before.
		return;
	}

	if( false == ( (CDlgBatchSelDpC*)m_pParentWnd )->DpCBvCtrlTab_BS_ComboChange() )
	{
		int iPos = max( m_ComboConnectAbove50.FindCBIDPtr( m_pclBatchSelDpCParams->m_strComboConnectAbove50ID ), 0 );

		if( iPos != m_ComboConnectAbove50.GetCurSel() )
		{
			m_ComboConnectAbove50.SetCurSel( iPos );
		}

		return;
	}

	m_pclBatchSelDpCParams->m_strComboConnectAbove50ID = m_ComboConnectAbove50.GetCBCurSelIDPtr().ID;
	
	FillComboBoxVersion( CDlgBatchSelBase::AboveOrBelow::Above50 );
	FillComboBoxPN( CDlgBatchSelBase::AboveOrBelow::Above50 );
	( (CDlgBatchSelDpC*)m_pParentWnd )->DpCBvCtrlTab_NotificationCbnChange();
}

void CDlgBatchSelDpC_DpCTab::OnComboSelChangeVersionAbove50()
{
	if( NULL == m_pclBatchSelDpCParams )
	{
		ASSERT_RETURN;
	}

	if( m_pclBatchSelDpCParams->m_strComboVersionAbove50ID == m_ComboVersionAbove50.GetCBCurSelIDPtr().ID )
	{
		// Do nothing if it's the same value as before.
		return;
	}

	if( false == ( (CDlgBatchSelDpC*)m_pParentWnd )->DpCBvCtrlTab_BS_ComboChange() )
	{
		int iPos = max( m_ComboVersionAbove50.FindCBIDPtr( m_pclBatchSelDpCParams->m_strComboVersionAbove50ID ), 0 );

		if( iPos != m_ComboVersionAbove50.GetCurSel() )
		{
			m_ComboVersionAbove50.SetCurSel( iPos );
		}

		return;
	}

	m_pclBatchSelDpCParams->m_strComboVersionAbove50ID = m_ComboVersionAbove50.GetCBCurSelIDPtr().ID;

	FillComboBoxPN( CDlgBatchSelBase::AboveOrBelow::Above50 );
	( (CDlgBatchSelDpC*)m_pParentWnd )->DpCBvCtrlTab_NotificationCbnChange();
}

void CDlgBatchSelDpC_DpCTab::OnComboSelChangePNAbove50()
{
	if( NULL == m_pclBatchSelDpCParams )
	{
		ASSERT_RETURN;
	}

	if( m_pclBatchSelDpCParams->m_strComboPNAbove50ID == m_ComboPNAbove50.GetCBCurSelIDPtr().ID )
	{
		// Do nothing if it's the same value as before.
		return;
	}

	if( false == ( (CDlgBatchSelDpC*)m_pParentWnd )->DpCBvCtrlTab_BS_ComboChange( true ) )
	{
		int iPos = max( m_ComboPNAbove50.FindCBIDPtr( m_pclBatchSelDpCParams->m_strComboPNAbove50ID ), 0 );

		if( iPos != m_ComboPNAbove50.GetCurSel() )
		{
			m_ComboPNAbove50.SetCurSel( iPos );
		}

		return;
	}

	m_pclBatchSelDpCParams->m_strComboPNAbove50ID = m_ComboPNAbove50.GetCBCurSelIDPtr().ID;

	( (CDlgBatchSelDpC*)m_pParentWnd )->DpCBvCtrlTab_NotificationCbnChange();
}

void CDlgBatchSelDpC_DpCTab::FillComboBoxType( CDlgBatchSelBase::AboveOrBelow eAboveOrBelow, CString strTypeID )
{
	if( NULL == m_pclBatchSelDpCParams || NULL == m_pclBatchSelDpCParams->m_pTADB )
	{
		ASSERT_RETURN;
	}

	int iDN;
	CExtNumEditComboBox *pCCombo;
	CRankEx TypeList;

	CTableDN *pclTableDN = (CTableDN *)( m_pclBatchSelDpCParams->m_pTADB->Get( _T("DN_TAB") ).MP );
	ASSERT( NULL != pclTableDN );

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

	if( CDlgBatchSelBase::AboveOrBelow::Above50 == eAboveOrBelow )
	{
		m_pclBatchSelDpCParams->m_pTADB->GetDpCTypeList( &TypeList, CTADatabase::FilterSelection::ForIndAndBatchSel, m_pclBatchSelDpCParams->m_eDpCLoc );
	}
	else
	{
		m_pclBatchSelDpCParams->m_pTADB->GetDpCTypeList( &TypeList, CTADatabase::FilterSelection::ForIndAndBatchSel, m_pclBatchSelDpCParams->m_eDpCLoc );
	}

	pCCombo->FillInCombo( &TypeList, strTypeID );

	if( CDlgBatchSelBase::AboveOrBelow::Above50 == eAboveOrBelow )
	{
		m_pclBatchSelDpCParams->m_strComboTypeAbove50ID = pCCombo->GetCBCurSelIDPtr().ID;
	}
	else
	{
		m_pclBatchSelDpCParams->m_strComboTypeBelow65ID = pCCombo->GetCBCurSelIDPtr().ID;
	}
}

void CDlgBatchSelDpC_DpCTab::FillComboBoxFamily( CDlgBatchSelBase::AboveOrBelow eAboveOrBelow, CString strFamilyID )
{
	if( NULL == m_pclBatchSelDpCParams || NULL == m_pclBatchSelDpCParams->m_pTADB )
	{
		ASSERT_RETURN;
	}

	int iDN;
	CExtNumEditComboBox *pCCombo;
	CRankEx FamilyList;

	CTableDN *pclTableDN = (CTableDN *)( m_pclBatchSelDpCParams->m_pTADB->Get( _T("DN_TAB") ).MP );
	ASSERT( NULL != pclTableDN );

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

	if( CDlgBatchSelBase::AboveOrBelow::Above50 == eAboveOrBelow )
	{
		m_pclBatchSelDpCParams->m_pTADB->GetDpCFamilyList(
			&FamilyList,
			m_pclBatchSelDpCParams->m_eDpCLoc,
			(LPCTSTR)m_pclBatchSelDpCParams->m_strComboTypeAbove50ID,
			CTADatabase::FilterSelection::ForIndAndBatchSel,
			iDN,
			INT_MAX );
	}
	else
	{
		m_pclBatchSelDpCParams->m_pTADB->GetDpCFamilyList(
			&FamilyList,
			m_pclBatchSelDpCParams->m_eDpCLoc,
			(LPCTSTR)m_pclBatchSelDpCParams->m_strComboTypeBelow65ID,
			CTADatabase::FilterSelection::ForIndAndBatchSel,
			0,
			iDN );
	}

	pCCombo->FillInCombo( &FamilyList, strFamilyID );

	if( CDlgBatchSelBase::AboveOrBelow::Above50 == eAboveOrBelow )
	{
		m_pclBatchSelDpCParams->m_strComboFamilyAbove50ID = pCCombo->GetCBCurSelIDPtr().ID;
	}
	else
	{
		m_pclBatchSelDpCParams->m_strComboFamilyBelow65ID = pCCombo->GetCBCurSelIDPtr().ID;
	}
}

void CDlgBatchSelDpC_DpCTab::FillComboBoxBodyMaterial( CDlgBatchSelBase::AboveOrBelow eAboveOrBelow, CString strBdyMatID )
{
	if( NULL == m_pclBatchSelDpCParams || NULL == m_pclBatchSelDpCParams->m_pTADB )
	{
		ASSERT_RETURN;
	}

	int iDN;
	CExtNumEditComboBox *pCCombo;
	CRankEx BodyMaterialList;

	CTableDN *pclTableDN = (CTableDN *)( m_pclBatchSelDpCParams->m_pTADB->Get( _T("DN_TAB") ).MP );
	ASSERT( NULL != pclTableDN );

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

	if( CDlgBatchSelBase::AboveOrBelow::Above50 == eAboveOrBelow )
	{
		m_pclBatchSelDpCParams->m_pTADB->GetDpCBdyMatList(
			&BodyMaterialList,
			m_pclBatchSelDpCParams->m_eDpCLoc,
			(LPCTSTR)m_pclBatchSelDpCParams->m_strComboTypeAbove50ID,
			(LPCTSTR)m_pclBatchSelDpCParams->m_strComboFamilyAbove50ID,
			CTADatabase::FilterSelection::ForIndAndBatchSel,
			iDN,
			INT_MAX );
	}
	else
	{
		m_pclBatchSelDpCParams->m_pTADB->GetDpCBdyMatList(
			&BodyMaterialList,
			m_pclBatchSelDpCParams->m_eDpCLoc,
			(LPCTSTR)m_pclBatchSelDpCParams->m_strComboTypeBelow65ID,
			(LPCTSTR)m_pclBatchSelDpCParams->m_strComboFamilyBelow65ID,
			CTADatabase::FilterSelection::ForIndAndBatchSel,
			0,
			iDN );
	}

	pCCombo->FillInCombo( &BodyMaterialList, strBdyMatID );

	if( CDlgBatchSelBase::AboveOrBelow::Above50 == eAboveOrBelow )
	{
		m_pclBatchSelDpCParams->m_strComboMaterialAbove50ID = pCCombo->GetCBCurSelIDPtr().ID;
	}
	else
	{
		m_pclBatchSelDpCParams->m_strComboMaterialBelow65ID = pCCombo->GetCBCurSelIDPtr().ID;
	}
}

void CDlgBatchSelDpC_DpCTab::FillComboBoxConnect( CDlgBatchSelBase::AboveOrBelow eAboveOrBelow, CString strConnectID )
{
	if( NULL == m_pclBatchSelDpCParams || NULL == m_pclBatchSelDpCParams->m_pTADB )
	{
		ASSERT_RETURN;
	}

	int iDN;
	CExtNumEditComboBox *pCCombo;
	CRankEx ConnectList;

	CTableDN *pclTableDN = (CTableDN *)( m_pclBatchSelDpCParams->m_pTADB->Get( _T("DN_TAB") ).MP );
	ASSERT( NULL != pclTableDN );

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

	if( CDlgBatchSelBase::AboveOrBelow::Above50 == eAboveOrBelow )
	{
		m_pclBatchSelDpCParams->m_pTADB->GetDpCConnList(
				&ConnectList,
				m_pclBatchSelDpCParams->m_eDpCLoc,
				(LPCTSTR)m_pclBatchSelDpCParams->m_strComboTypeAbove50ID,
				(LPCTSTR)m_pclBatchSelDpCParams->m_strComboFamilyAbove50ID,
				(LPCTSTR)m_pclBatchSelDpCParams->m_strComboMaterialAbove50ID,
				CTADatabase::FilterSelection::ForIndAndBatchSel,
				iDN, INT_MAX );
	}
	else
	{
		m_pclBatchSelDpCParams->m_pTADB->GetDpCConnList(
				&ConnectList,
				m_pclBatchSelDpCParams->m_eDpCLoc,
				(LPCTSTR)m_pclBatchSelDpCParams->m_strComboTypeBelow65ID,
				(LPCTSTR)m_pclBatchSelDpCParams->m_strComboFamilyBelow65ID,
				(LPCTSTR)m_pclBatchSelDpCParams->m_strComboMaterialBelow65ID,
				CTADatabase::FilterSelection::ForIndAndBatchSel,
				0,
				iDN );
	}

	pCCombo->FillInCombo( &ConnectList, strConnectID );

	if( CDlgBatchSelBase::AboveOrBelow::Above50 == eAboveOrBelow )
	{
		m_pclBatchSelDpCParams->m_strComboConnectAbove50ID = pCCombo->GetCBCurSelIDPtr().ID;
	}
	else
	{
		m_pclBatchSelDpCParams->m_strComboConnectBelow65ID = pCCombo->GetCBCurSelIDPtr().ID;
	}
}

void CDlgBatchSelDpC_DpCTab::FillComboBoxVersion( CDlgBatchSelBase::AboveOrBelow eAboveOrBelow, CString strVersionID )
{
	if( NULL == m_pclBatchSelDpCParams || NULL == m_pclBatchSelDpCParams->m_pTADB )
	{
		ASSERT_RETURN;
	}

	int iDN;
	CExtNumEditComboBox *pCCombo;
	CRankEx VersionList;

	CTableDN *pclTableDN = (CTableDN *)( m_pclBatchSelDpCParams->m_pTADB->Get( _T("DN_TAB") ).MP );
	ASSERT( NULL != pclTableDN );

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

	if( CDlgBatchSelBase::AboveOrBelow::Above50 == eAboveOrBelow )
	{
		m_pclBatchSelDpCParams->m_pTADB->GetDpCVersList(
				&VersionList,
				m_pclBatchSelDpCParams->m_eDpCLoc,
				(LPCTSTR)m_pclBatchSelDpCParams->m_strComboTypeAbove50ID,
				(LPCTSTR)m_pclBatchSelDpCParams->m_strComboFamilyAbove50ID,
				(LPCTSTR)m_pclBatchSelDpCParams->m_strComboMaterialAbove50ID,
				(LPCTSTR)m_pclBatchSelDpCParams->m_strComboConnectAbove50ID,
				CTADatabase::FilterSelection::ForIndAndBatchSel );
	}
	else
	{
		m_pclBatchSelDpCParams->m_pTADB->GetDpCVersList(
				&VersionList,
				m_pclBatchSelDpCParams->m_eDpCLoc,
				(LPCTSTR)m_pclBatchSelDpCParams->m_strComboTypeBelow65ID,
				(LPCTSTR)m_pclBatchSelDpCParams->m_strComboFamilyBelow65ID,
				(LPCTSTR)m_pclBatchSelDpCParams->m_strComboMaterialBelow65ID,
				(LPCTSTR)m_pclBatchSelDpCParams->m_strComboConnectBelow65ID,
				CTADatabase::FilterSelection::ForIndAndBatchSel );
	}

	pCCombo->FillInCombo( &VersionList, strVersionID );

	if( CDlgBatchSelBase::AboveOrBelow::Above50 == eAboveOrBelow )
	{
		m_pclBatchSelDpCParams->m_strComboVersionAbove50ID = pCCombo->GetCBCurSelIDPtr().ID;
	}
	else
	{
		m_pclBatchSelDpCParams->m_strComboVersionBelow65ID = pCCombo->GetCBCurSelIDPtr().ID;
	}
}

void CDlgBatchSelDpC_DpCTab::FillComboBoxPN( CDlgBatchSelBase::AboveOrBelow eAboveOrBelow, CString strPNID )
{
	if( NULL == m_pclBatchSelDpCParams || NULL == m_pclBatchSelDpCParams->m_pTADB )
	{
		ASSERT_RETURN;
	}

	int iDN;
	CExtNumEditComboBox *pCCombo;
	CRankEx PNList;

	CTableDN *pclTableDN = (CTableDN *)( m_pclBatchSelDpCParams->m_pTADB->Get( _T("DN_TAB") ).MP );
	ASSERT( NULL != pclTableDN );

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

	if( CDlgBatchSelBase::AboveOrBelow::Above50 == eAboveOrBelow )
	{
		m_pclBatchSelDpCParams->m_pTADB->GetDpCPNList(
				&PNList,
				m_pclBatchSelDpCParams->m_eDpCLoc,
				(LPCTSTR)m_pclBatchSelDpCParams->m_strComboTypeAbove50ID,
				(LPCTSTR)m_pclBatchSelDpCParams->m_strComboFamilyAbove50ID,
				(LPCTSTR)m_pclBatchSelDpCParams->m_strComboMaterialAbove50ID,
				(LPCTSTR)m_pclBatchSelDpCParams->m_strComboConnectAbove50ID,
				(LPCTSTR)m_pclBatchSelDpCParams->m_strComboVersionAbove50ID,
				CTADatabase::FilterSelection::ForIndAndBatchSel );
	}
	else
	{
		m_pclBatchSelDpCParams->m_pTADB->GetDpCPNList(
				&PNList,
				m_pclBatchSelDpCParams->m_eDpCLoc,
				(LPCTSTR)m_pclBatchSelDpCParams->m_strComboTypeBelow65ID,
				(LPCTSTR)m_pclBatchSelDpCParams->m_strComboFamilyBelow65ID,
				(LPCTSTR)m_pclBatchSelDpCParams->m_strComboMaterialBelow65ID,
				(LPCTSTR)m_pclBatchSelDpCParams->m_strComboConnectBelow65ID,
				(LPCTSTR)m_pclBatchSelDpCParams->m_strComboVersionBelow65ID,
				CTADatabase::FilterSelection::ForIndAndBatchSel );
	}

	pCCombo->FillInCombo( &PNList, strPNID );

	if( CDlgBatchSelBase::AboveOrBelow::Above50 == eAboveOrBelow )
	{
		m_pclBatchSelDpCParams->m_strComboPNAbove50ID = pCCombo->GetCBCurSelIDPtr().ID;
	}
	else
	{
		m_pclBatchSelDpCParams->m_strComboPNBelow65ID = pCCombo->GetCBCurSelIDPtr().ID;
	}
}
