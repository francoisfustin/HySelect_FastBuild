#include "stdafx.h"
#include "TASelect.h"
#include "DlgCtrlPropPage.h"
#include "DlgSearchAndReplaceSet.h"
#include "DlgSRPageSet.h"


IMPLEMENT_DYNAMIC( CDlgSRPageSet, CDlgCtrlPropPage )

CDlgSRPageSet::CDlgSRPageSet( CWnd* pParent )
	: CDlgCtrlPropPage( pParent )
{
	m_csHeaderTitle = TASApp.LoadLocalizedString( IDS_TABDLGSRSET_HEADERTITLE );
	m_csPageName = TASApp.LoadLocalizedString( IDS_TABDLGSRSET_HEADERTITLE );
	m_pNotificationHandler = NULL;
}

CDlgSRPageSet::~CDlgSRPageSet()
{
}

void CDlgSRPageSet::SetNotificationHandler( IDlgSRPageSetNotificationHandler* pNotificationHandler )
{
	m_pNotificationHandler = pNotificationHandler;
}

bool CDlgSRPageSet::IsRadioSplitChecked( void )
{
	bool fChecked = false;
	CButton* pButton = (CButton*)GetDlgItem( IDC_RADIOSPLITCVACTUATORSET );
	if( NULL != pButton )
		fChecked = ( BST_CHECKED == pButton->GetCheck() ? true : false );
	return fChecked;
}

bool CDlgSRPageSet::IsRadioGroupChecked( void )
{
	bool fChecked = false;
	CButton* pButton = (CButton*)GetDlgItem( IDC_RADIOGROUPCVACTUATOR );
	if( NULL != pButton )
		fChecked = ( BST_CHECKED == pButton->GetCheck() ? true : false );
	return fChecked;
}

void CDlgSRPageSet::Init( bool bRstToDefault )
{
}

bool CDlgSRPageSet::OnBnClickedBtnStart( std::vector<LPARAM> *pvecParams )
{
	CDlgSearchAndReplaceSet Dlg( this );
	if( false == Dlg.SetParams( pvecParams ) )
		return false;

	CRect rect;
	GetWindowRect( &rect );
	Dlg.SetWorkingMode( ( true == IsRadioGroupChecked() ) ? CDlgSearchAndReplaceSet::WorkingMode::WM_Group : CDlgSearchAndReplaceSet::WorkingMode::WM_Split );
	Dlg.DoModal();
	return true;
}

BEGIN_MESSAGE_MAP( CDlgSRPageSet, CDlgCtrlPropPage )
	ON_BN_CLICKED( IDC_RADIOSPLITCVACTUATORSET, OnRadioClickedSplitCVActrSet )
	ON_BN_CLICKED( IDC_RADIOGROUPCVACTUATOR, OnRadioClickedGroupCVActr )
	ON_MESSAGE( WM_USER_REPLACEPOPUPFINDNEXT,  OnReplacePopupHM )
END_MESSAGE_MAP()

void CDlgSRPageSet::DoDataExchange( CDataExchange* pDX )
{
	CDlgCtrlPropPage::DoDataExchange( pDX );
}

BOOL CDlgSRPageSet::OnInitDialog()
{
	CDlgCtrlPropPage::OnInitDialog();

	CString str = TASApp.LoadLocalizedString( IDS_DLGSEARCHREPLACE_SETWHATTODO );
	GetDlgItem( IDC_STATIC_WHATTODO )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_DLGSEARCHREPLACE_SETSPLIT );
	GetDlgItem( IDC_RADIOSPLITCVACTUATORSET )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_DLGSEARCHREPLACE_SETGROUP );
	GetDlgItem( IDC_RADIOGROUPCVACTUATOR )->SetWindowText( str );
	( (CButton*)GetDlgItem( IDC_RADIOSPLITCVACTUATORSET ) )->SetCheck( BST_CHECKED );

	Init();
	
	return TRUE;
}

void CDlgSRPageSet::OnRadioClickedSplitCVActrSet()
{
	if( NULL != m_pNotificationHandler )
		m_pNotificationHandler->SRPageSet_OnRadioSplitClicked();
}

void CDlgSRPageSet::OnRadioClickedGroupCVActr()
{
	if( NULL != m_pNotificationHandler )
		m_pNotificationHandler->SRPageSet_OnRadioGroupClicked();
}

LRESULT CDlgSRPageSet::OnReplacePopupHM( WPARAM wParam, LPARAM lParam )
{
	if( NULL != m_pNotificationHandler )
		m_pNotificationHandler->SRPageSet_OnReplacePopupHM( (CDS_HydroMod *)wParam );
	return TRUE;
}
