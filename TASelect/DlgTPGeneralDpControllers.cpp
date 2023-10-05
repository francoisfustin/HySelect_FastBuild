#include "stdafx.h"
#include "TASelect.h"
#include "DlgTPGeneralDpControllers.h"

#define _MAXDPCMINDP	50000.0

IMPLEMENT_DYNAMIC( CDlgTPGeneralDpControllers, CDlgCtrlPropPage )

CDlgTPGeneralDpControllers::CDlgTPGeneralDpControllers( CWnd* pParent )
	: CDlgCtrlPropPage( pParent )
{
	m_pTADS = NULL;
	m_dDpCMinDp = 0.0;
	m_iDpCHide = 0;
	m_iDpCMvWithSameSize = 0;
	m_csHeaderTitle = TASApp.LoadLocalizedString( IDS_TABDLGTECHDPC_HEADERTITLE );
	m_csPageName = TASApp.LoadLocalizedString( IDS_TABDLGTECHDPC_PAGENAME );
}

void CDlgTPGeneralDpControllers::Init( bool fResetToDefault )
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

	// Fill all unit static controls.
	TCHAR tcName[_MAXCHARS];
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();
	
	// Set the text for the Dp units.
	GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ), tcName );
	SetDlgItemText( IDC_STATICDPUNIT, tcName );
	
	// Interfaced parameters.
	m_dDpCMinDp = pTechP->GetDpCMinDp();
	m_iDpCHide = pTechP->GetDpCHide();
	m_iDpCMvWithSameSize = pTechP->GetDpCMvWithSameSizeOnly();

	// Initialize the edit and check boxes.
	m_EditMinDp.SetMinDblValue( MINDPVALUE );
	m_EditMinDp.SetCurrentValSI(m_dDpCMinDp);// SetWindowText( WriteCUDouble( CDimValue::SItoCU( _U_DIFFPRESS, m_dDpCMinDp ), 3, 1, 2 ) );
	m_EditMinDp.Update();
	m_CheckHide.SetCheck( ( 1 == m_iDpCHide ) ? BST_CHECKED : BST_UNCHECKED );
	m_CheckMvWithSameSize.SetCheck( ( 1 == m_iDpCMvWithSameSize ) ? BST_CHECKED : BST_UNCHECKED );
}

void CDlgTPGeneralDpControllers::Save( CString strSectionName )
{
	CDS_TechnicalParameter *pTech = TASApp.GetpTADS()->GetpTechParams();

	// Interfaced parameters.
	if( NULL != GetSafeHwnd() )
	{
		pTech->SetDpCMinDp( m_dDpCMinDp );
		pTech->SetDpCHide( m_iDpCHide );
		pTech->SetDpCfDpCMvWithSameSizeOnly( m_iDpCMvWithSameSize );
	}
}

bool CDlgTPGeneralDpControllers::VerifyInfo()
{
	bool fResult = true;

	// Verify the edit valve minimum Dp.
	double dValue;
	switch( ReadDouble( m_EditMinDp, &dValue ) )
	{
		case RD_EMPTY:
			fResult = false;
			TASApp.AfxLocalizeMessageBox( AFXMSG_NON_POSITIVE_VALUE );
			break;
		
		case RD_OK:
			m_EditMinDp.SetWindowText( WriteDouble( dValue, 3, 1, true ));
			m_dDpCMinDp = CDimValue::CUtoSI( _U_DIFFPRESS, dValue );
			break;
	}

	return fResult;
}

BEGIN_MESSAGE_MAP( CDlgTPGeneralDpControllers, CDlgCtrlPropPage )
	ON_BN_CLICKED( IDC_CHECKHIDE, OnCheckHide )
	ON_BN_CLICKED( IDC_CHECKSAMESIZE, OnBnClickedSameSize )
END_MESSAGE_MAP()

void CDlgTPGeneralDpControllers::DoDataExchange( CDataExchange* pDX )
{
	CDlgCtrlPropPage::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_EDITMINDP, m_EditMinDp );
	DDX_Control( pDX, IDC_CHECKHIDE, m_CheckHide );
	DDX_Control( pDX, IDC_CHECKSAMESIZE, m_CheckMvWithSameSize );
}

BOOL CDlgTPGeneralDpControllers::OnInitDialog() 
{
	if( FALSE == CDlgCtrlPropPage::OnInitDialog() )
	{
		return FALSE;
	}

	// Initialize edit boxes.
	m_EditMinDp.SetEditType( CNumString::eEditType::eDouble, CNumString::eEditSign::ePositive );
	m_EditMinDp.SetPhysicalType( _U_DIFFPRESS );
	m_EditMinDp.SetMaxDblValue( _MAXDPCMINDP );
	
	CString str;
	str = TASApp.LoadLocalizedString( IDS_TABDLGTECHDPC_STATICMINDP );
	GetDlgItem( IDC_STATICMINDP )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_TABDLGTECHDPC_CHECKHIDE );
	GetDlgItem( IDC_CHECKHIDE )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_TABDLGTECHDPC_CHECKMVWITHSAMESIZE );
	GetDlgItem( IDC_CHECKSAMESIZE )->SetWindowText( str );

	m_pTADS = TASApp.GetpTADS();

	Init();
	
	return TRUE;
}

void CDlgTPGeneralDpControllers::OnCheckHide() 
{
	m_iDpCHide = ( BST_CHECKED == m_CheckHide.GetCheck() ) ? 1 : 0;	
}

void CDlgTPGeneralDpControllers::OnBnClickedSameSize()
{
	m_iDpCMvWithSameSize = ( BST_CHECKED == m_CheckMvWithSameSize.GetCheck() ) ? 1 : 0;	
}
