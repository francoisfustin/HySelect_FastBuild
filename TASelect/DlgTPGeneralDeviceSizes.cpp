#include "stdafx.h"
#include "TASelect.h"
#include "DlgTPGeneralDeviceSizes.h"


IMPLEMENT_DYNAMIC( CDlgTPGeneralDeviceSizes, CDlgCtrlPropPage )

CDlgTPGeneralDeviceSizes::CDlgTPGeneralDeviceSizes( CWnd* pParent )
	: CDlgCtrlPropPage( pParent )
{
	m_pTADS = NULL;
	m_iSizeShift = 0;
	m_iSizeShiftAbove = 0;
	m_iSizeShiftBelow = 0;
	m_csHeaderTitle = TASApp.LoadLocalizedString( IDS_TABDLGTECHDS_HEADERTITLE );
	m_csPageName = TASApp.LoadLocalizedString( IDS_TABDLGTECHDS_PAGENAME );
}

void CDlgTPGeneralDeviceSizes::Init( bool fResetToDefault )
{
	CDS_TechnicalParameter *pTechP;
	
	if( true == fResetToDefault )
	{
		pTechP = (CDS_TechnicalParameter*)( TASApp.GetpTADB()->Get( _T("PARAM_TECH") ).MP );
	}
	else
	{
		pTechP = m_pTADS->GetpTechParams();
	}

	ASSERT( NULL != pTechP );

	if( NULL == pTechP )
	{
		return;
	}
	
	m_iSizeShiftAbove = pTechP->GetSizeShiftAbove();
	m_iSizeShiftBelow = pTechP->GetSizeShiftBelow();
	if( 0 == m_iSizeShiftAbove && 0 == m_iSizeShiftBelow )
	{
		m_RadioSameSizeAsPipe.SetCheck( BST_CHECKED );
		m_RadioSizeUpTo.SetCheck( BST_UNCHECKED );
	}
	else
	{
		m_RadioSameSizeAsPipe.SetCheck( BST_UNCHECKED );
		m_RadioSizeUpTo.SetCheck( BST_CHECKED );
	}

	// Initialize the spin controls.
	m_SpinSizeShiftBelow.SetRange( 0, pTechP->GetMaxSizeShift() );
	m_SpinSizeShiftAbove.SetRange( 0, pTechP->GetMaxSizeShift() );
	
	// Ensures the disabling of the edit and spin controls
	if( BST_UNCHECKED == m_RadioSizeUpTo.GetCheck() )
	{
		m_EditSizeShiftAbove.EnableWindow( FALSE );
		m_SpinSizeShiftAbove.EnableWindow( FALSE );
		m_EditSizeShiftBelow.EnableWindow( FALSE );
		m_SpinSizeShiftBelow.EnableWindow( FALSE );
	}

	// Initialize the edit boxes and spin control positions.
	m_SpinSizeShiftBelow.SetPos( abs( m_iSizeShiftBelow ) );
	m_SpinSizeShiftAbove.SetPos( abs( m_iSizeShiftAbove ) );
	
}

void CDlgTPGeneralDeviceSizes::Save( CString strSectionName )
{
	CDS_TechnicalParameter *pTech = TASApp.GetpTADS()->GetpTechParams();

	if( NULL != GetSafeHwnd() )
	{
		pTech->SetSizeShiftAbove( m_iSizeShiftAbove );
		pTech->SetSizeShiftBelow( m_iSizeShiftBelow );
	}
}

BEGIN_MESSAGE_MAP(CDlgTPGeneralDeviceSizes, CDlgCtrlPropPage)
	ON_BN_CLICKED( IDC_RADIOSIZE1, OnRadioSameSizeAsPipe )
	ON_BN_CLICKED( IDC_RADIOSIZE2, OnRadioSizeUpTo )
	ON_EN_CHANGE( IDC_EDITSIZESHIFTABOVE, OnChangeEditSizeShiftAbove )
	ON_EN_CHANGE( IDC_EDITSIZESHIFTBELOW, OnChangeEditSizeShiftBelow )
END_MESSAGE_MAP()

void CDlgTPGeneralDeviceSizes::DoDataExchange( CDataExchange* pDX )
{
	CDlgCtrlPropPage::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_RADIOSIZE1, m_RadioSameSizeAsPipe );
	DDX_Control( pDX, IDC_RADIOSIZE2, m_RadioSizeUpTo );
	DDX_Control( pDX, IDC_EDITSIZESHIFTABOVE, m_EditSizeShiftAbove );
	DDX_Control( pDX, IDC_EDITSIZESHIFTBELOW, m_EditSizeShiftBelow );
	DDX_Control( pDX, IDC_SPINSIZESHIFTABOVE, m_SpinSizeShiftAbove );
	DDX_Control( pDX, IDC_SPINSIZESHIFTBELOW, m_SpinSizeShiftBelow );
}

BOOL CDlgTPGeneralDeviceSizes::OnInitDialog() 
{	
	if( FALSE == CDlgCtrlPropPage::OnInitDialog() )
	{
		return FALSE;
	}

	// Initialize dialog strings.
	CString str;
	str = TASApp.LoadLocalizedString( IDS_TABDLGTECHDEVSIZE_STATICVALV );
	GetDlgItem( IDC_STATICVALV )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_TABDLGTECHDEVSIZE_RADIOSIZE1 );
	GetDlgItem( IDC_RADIOSIZE1 )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_TABDLGTECHDEVSIZE_RADIOSIZE2 );
	GetDlgItem( IDC_RADIOSIZE2 )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_TABDLGTECHDEVSIZE_STATICABOVEPIPESIZE );
	GetDlgItem( IDC_STATICABOVEPIPESIZE )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_TABDLGTECHDEVSIZE_STATICBELOWPIPESIZE );
	GetDlgItem( IDC_STATICBELOWPIPESIZE )->SetWindowText( str );
	
	m_pTADS = TASApp.GetpTADS();
	Init();

	return TRUE;
}

void CDlgTPGeneralDeviceSizes::OnRadioSameSizeAsPipe()
{
	m_EditSizeShiftAbove.EnableWindow( FALSE );
	m_SpinSizeShiftAbove.EnableWindow( FALSE );
	m_EditSizeShiftBelow.EnableWindow( FALSE );
	m_SpinSizeShiftBelow.EnableWindow( FALSE );
	m_iSizeShiftAbove = 0;
	m_iSizeShiftBelow = 0;
}

void CDlgTPGeneralDeviceSizes::OnRadioSizeUpTo() 
{
	m_EditSizeShiftAbove.EnableWindow( TRUE );
	m_SpinSizeShiftAbove.EnableWindow( TRUE );
	m_EditSizeShiftBelow.EnableWindow( TRUE );
	m_SpinSizeShiftBelow.EnableWindow( TRUE );
	OnChangeEditSizeShiftAbove();
	OnChangeEditSizeShiftBelow();
}

void CDlgTPGeneralDeviceSizes::OnChangeEditSizeShiftAbove() 
{
	CString str;
	TCHAR *endptr;

	if( m_RadioSizeUpTo.GetSafeHwnd()  != NULL && BST_CHECKED == m_RadioSizeUpTo.GetCheck() )
	{
		m_EditSizeShiftAbove.GetWindowText( str );
		m_iSizeShiftAbove = (int)_tcstol( str, &endptr, 10 );
		ASSERT( '\0' == *endptr );
	}
}

void CDlgTPGeneralDeviceSizes::OnChangeEditSizeShiftBelow() 
{
	CString str;
	TCHAR *endptr;

	if( m_RadioSizeUpTo.GetSafeHwnd() != NULL  && BST_CHECKED == m_RadioSizeUpTo.GetCheck() )
	{
		m_EditSizeShiftBelow.GetWindowText( str );
		m_iSizeShiftBelow = -(int)_tcstol( str, &endptr, 10 );
		ASSERT( '\0' == *endptr );
	}
}
