#include "stdafx.h"
#include "TASelect.h"
#include "DlgTPPageMainHC.h"

IMPLEMENT_DYNAMIC( CDlgTPPageMainHC, CDlgCtrlPropPage )

CDlgTPPageMainHC::CDlgTPPageMainHC( CWnd* pParent )
	: CDlgCtrlPropPage(pParent)
{
	m_pTADS = NULL;
	m_fAutoCheck = false;
	m_bCheckBuildOld = false;
	m_csHeaderTitle = TASApp.LoadLocalizedString( IDS_TABDLGTECHMAINHC_HEADERTITLE );
	m_csPageName = TASApp.LoadLocalizedString( IDS_TABDLGTECHMAINHC_PAGENAME );
}

void CDlgTPPageMainHC::Init( bool fResetToDefault )
{
	if( true == fResetToDefault )
	{
		m_pTADS->GetpProjectParams()->GetpHmCalcParams()->ResetPrjParams( true );
	}
	
	CDS_ProjectParams *pProjectParams = m_pTADS->GetpProjectParams();
	ASSERT( NULL != pProjectParams );

	m_fAutoCheck = pProjectParams->GetHNAutomaticCheck();
	m_AutoCheck.SetCheck( ( true == m_fAutoCheck ) ? BST_CHECKED : BST_UNCHECKED );
	
	// HYS-1221 : The text of the checkbox becomes red after checked
	m_bCheckBuildOld = pProjectParams->GetpHmCalcParams()->IsOldProductsAvailableForHC();
	m_BnCheckBuildOld.SetCheck( ( true == m_bCheckBuildOld ) ? BST_CHECKED : BST_UNCHECKED );
	if( true == m_bCheckBuildOld )
	{
		m_staticCheckOldProducts.SetTextColor( _RED );
	}
	else
	{
		m_staticCheckOldProducts.SetTextColor( _BLACK );
	}
}

void CDlgTPPageMainHC::Save( CString strSectionName )
{
	CDS_ProjectParams *pProjectParams = TASApp.GetpTADS()->GetpProjectParams();
	ASSERT( NULL != pProjectParams );

	if( NULL != GetSafeHwnd() )
	{
		pProjectParams->SetHNAutomaticCheck( m_fAutoCheck );
		pProjectParams->GetpHmCalcParams()->SetCheckOldProductsAvailableForHC( m_bCheckBuildOld );

		// HYS-1221: Write to the registry
		int iChecked = ( true == pProjectParams->GetpHmCalcParams()->IsOldProductsAvailableForHC() ) ? 1 : 0;
	}
}

BEGIN_MESSAGE_MAP( CDlgTPPageMainHC, CDlgCtrlPropPage )
	ON_BN_CLICKED( IDC_CHECKAUTOCHECK, OnBnClickedAutoCheck )
	ON_BN_CLICKED( IDC_CHECKBUILDOLD, OnBnCheckBuildOldProduct )
END_MESSAGE_MAP()

void CDlgTPPageMainHC::DoDataExchange( CDataExchange* pDX )
{
	CDlgCtrlPropPage::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_CHECKAUTOCHECK, m_AutoCheck );
	DDX_Control( pDX, IDC_CHECKBUILDOLD, m_BnCheckBuildOld );
	DDX_Control( pDX, IDC_STATICCHECKOLDPROD, m_staticCheckOldProducts );
}

BOOL CDlgTPPageMainHC::OnInitDialog() 
{	
	if( FALSE == CDlgCtrlPropPage::OnInitDialog() )
	{
		return FALSE;
	}

	// Initialize dialog strings.
	CString str;
	str = TASApp.LoadLocalizedString( IDS_TABDLGTECHMAINHC_CHECKAUTOCHECK );
	GetDlgItem( IDC_CHECKAUTOCHECK )->SetWindowText( str );
	// HYS-1221 :  The static text for the checkbox IDC_CHECKBUILDOLD
	str = TASApp.LoadLocalizedString( IDS_TABDLGTECHMAINHC_CHECKOLDPROD );
	GetDlgItem( IDC_STATICCHECKOLDPROD )->SetWindowText( str );
	
	m_pTADS = TASApp.GetpTADS();

	Init();

	return TRUE;
}

void CDlgTPPageMainHC::OnBnClickedAutoCheck()
{
	m_fAutoCheck = ( BST_CHECKED == m_AutoCheck.GetCheck() ) ? true : false;
}

void CDlgTPPageMainHC::OnBnCheckBuildOldProduct()
{
	if( BST_CHECKED == m_BnCheckBuildOld.GetCheck() )
	{
		m_bCheckBuildOld = true;
		m_staticCheckOldProducts.SetTextColor( _RED );
	}
	else
	{
		m_bCheckBuildOld = false;
		m_staticCheckOldProducts.SetTextColor( _BLACK );
	}
	
	// Save the value to be used when filling combo HM products preferences
	CDS_ProjectParams *pProjectParams = TASApp.GetpTADS()->GetpProjectParams();

	if( NULL != pProjectParams )
	{
		pProjectParams->GetpHmCalcParams()->SetCheckOldProductsAvailableForHC( m_bCheckBuildOld );
	}
}

