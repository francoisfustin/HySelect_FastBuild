#include "stdafx.h"
#include "TASelect.h"
#include "DlgTPGeneralPIBCV.h"

#define _MAXMINDP	50000.0


IMPLEMENT_DYNAMIC( CDlgTPGeneralPIBCV, CDlgCtrlPropPage )

CDlgTPGeneralPIBCV::CDlgTPGeneralPIBCV( CWnd* pParent )
	: CDlgCtrlPropPage( pParent )
{
	m_pTADS = NULL;
	m_dPICvMinDp = 0.0;
	m_iPICvHide = 0;
	m_csHeaderTitle = TASApp.LoadLocalizedString( IDS_TABDLGTECHPICV_HEADERTITLE );
	m_csPageName = TASApp.LoadLocalizedString( IDS_TABDLGTECHPICV_PAGENAME );
}

void CDlgTPGeneralPIBCV::Init( bool fResetToDefault )
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
	m_dPICvMinDp = pTechP->GetPICvMinDp();
	m_iPICvHide = pTechP->GetPICvHide();

	// Initialize the edit and check boxes.
	m_EditMinDp.SetEditType( CNumString::eEditType::eDouble, CNumString::eEditSign::ePositive );
	m_EditMinDp.SetPhysicalType( _U_DIFFPRESS );
	m_EditMinDp.SetMinDblValue( MINDPVALUE );
	m_EditMinDp.SetCurrentValSI(m_dPICvMinDp);
	m_EditMinDp.Update();


	m_Hide.SetCheck( ( 1 == m_iPICvHide ) ? BST_CHECKED : BST_UNCHECKED );
}

void CDlgTPGeneralPIBCV::Save( CString strSectionName )
{
	CDS_TechnicalParameter *pTech = TASApp.GetpTADS()->GetpTechParams();

	// Interfaced parameters.
	if ( NULL != GetSafeHwnd() )
	{
		pTech->SetPICvMinDp( m_dPICvMinDp );
		pTech->SetPICvHide( m_iPICvHide );
	}
}

bool CDlgTPGeneralPIBCV::VerifyInfo()
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
			m_EditMinDp.SetWindowText( WriteDouble( dValue, 3, 1, true ) );
			m_dPICvMinDp = CDimValue::CUtoSI( _U_DIFFPRESS, dValue );
			break;
	}

	return fResult;
}

BEGIN_MESSAGE_MAP( CDlgTPGeneralPIBCV, CDlgCtrlPropPage )
	ON_BN_CLICKED( IDC_CHECKHIDE, OnBnClickedHide )
END_MESSAGE_MAP()

void CDlgTPGeneralPIBCV::DoDataExchange( CDataExchange* pDX )
{
	CDlgCtrlPropPage::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_EDITMINDP, m_EditMinDp );
	DDX_Control( pDX, IDC_CHECKHIDE, m_Hide );
}

BOOL CDlgTPGeneralPIBCV::OnInitDialog() 
{
	if( FALSE == CDlgCtrlPropPage::OnInitDialog() )
	{
		return FALSE;
	}

	// Initialize edit boxes.
	m_EditMinDp.SetEditType( CNumString::eEditType::eDouble, CNumString::eEditSign::ePositive );
	m_EditMinDp.SetPhysicalType( _U_DIFFPRESS );
	m_EditMinDp.SetMaxDblValue( _MAXMINDP );
	
	CString str;
	str = TASApp.LoadLocalizedString( IDS_TABDLGTECHPICV_STATICMINDP );
	GetDlgItem( IDC_STATICMINDP )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_TABDLGTECHPICV_CHECKHIDE );
	GetDlgItem( IDC_CHECKHIDE )->SetWindowText( str );

	m_pTADS = TASApp.GetpTADS();

	Init();

	return TRUE;
}

void CDlgTPGeneralPIBCV::OnBnClickedHide()
{
	m_iPICvHide = ( BST_CHECKED == m_Hide.GetCheck() ) ? 1 : 0;	
}
