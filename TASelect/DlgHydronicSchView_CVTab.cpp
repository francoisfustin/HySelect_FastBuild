#include "stdafx.h"


#include "TASelect.h"
#include "MainFrm.h"
#include "Global.h"
#include "HydroMod.h"
#include "DlgHydronicSchView_BaseTab.h"
#include "DlgHydronicSchView_CVTab.h"
#include "DlgHydronicSchView.h"

CDlgHydronicSchView_CVTab::CDlgHydronicSchView_CVTab( CDlgHydronicSchView *pclHydronicSchView, CWnd *pParent )
	: CDlgHydronicSchView_BaseTab( pclHydronicSchView, CDlgHydronicSchView_CVTab::IDD,  pParent )
{
}

void CDlgHydronicSchView_CVTab::VerifyControlValveType()
{
	if( CDB_ControlProperties::CvCtrlType::eCvNU == m_pclHydronicSchView->GetControlType() )	
	{
		SetAvailable( false );
	}
	else
	{
		SetAvailable( true );
	}
}

bool CDlgHydronicSchView_CVTab::IsAvailable( CDB_CircuitScheme *pCircuitScheme )
{
	if( NULL == pCircuitScheme )
	{
		return false;
	}

	bool bAvailable = false;

	if( pCircuitScheme->IsAnchorPtExist( CAnchorPt::ControlValve ) || true == pCircuitScheme->IsAnchorPtExist( CAnchorPt::PICV ) )
	{
		bAvailable = true;
	}

	return bAvailable;
}

void CDlgHydronicSchView_CVTab::GetProducts( std::map<CAnchorPt::eFunc, CString> *pmapProducts, CDB_CircuitScheme *pCircuitScheme )
{
	if( NULL == pmapProducts || NULL == pCircuitScheme || NULL == m_pProduct )
	{
		return;
	}

	if( pCircuitScheme->IsAnchorPtExist( CAnchorPt::ControlValve ) )
	{
		pmapProducts->insert( std::pair<CAnchorPt::eFunc, CString>( CAnchorPt::ControlValve, m_pProduct->GetIDPtr().ID ) );
	}
	else if( pCircuitScheme->IsAnchorPtExist( CAnchorPt::PICV ) )
	{
		pmapProducts->insert( std::pair<CAnchorPt::eFunc, CString>( CAnchorPt::PICV, m_pProduct->GetIDPtr().ID ) );
	}
}

void CDlgHydronicSchView_CVTab::SetAvailable( bool bAvailable )
{
	GetDlgItem( IDC_STATICBV3WBYPASS )->EnableWindow( ( true == bAvailable ) ? TRUE : FALSE );
	m_ComboBVIn3WBypass.EnableWindow( ( true == bAvailable ) ? TRUE : FALSE );

	CDlgHydronicSchView_BaseTab::SetAvailable( bAvailable );
}

void CDlgHydronicSchView_CVTab::InitComboBox()
{
	CDlgHydronicSchView_BaseTab::InitComboBox();

	FillComboBoxBVIn3WBypass();
}

void CDlgHydronicSchView_CVTab::FillComboBoxType()
{
	CRankEx ListEx;

	int iCount = m_pTADB->GetTaCVTypeList( &ListEx, CTADatabase::eForHMCv, CDB_ControlProperties::CV2W3W::LastCV2W3W, m_pclHydronicSchView->GetCvType(), 
			m_pclHydronicSchView->GetControlType(), CTADatabase::FilterSelection::NoFiltering, m_iDNMin, m_iDNMax );

	FillComboBox( &ListEx, &m_ComboType, iCount, &m_strType );
	FillComboBoxFamily();
}

void CDlgHydronicSchView_CVTab::FillComboBoxFamily()
{
	CRankEx ListEx;

	int iCount = m_pTADB->GetTaCVFamList( &ListEx, CTADatabase::eForHMCv, CDB_ControlProperties::CV2W3W::LastCV2W3W, m_strType, m_pclHydronicSchView->GetCvType(), 
			m_pclHydronicSchView->GetControlType(),	CTADatabase::FilterSelection::NoFiltering, m_iDNMin, m_iDNMax );

	FillComboBox( &ListEx, &m_ComboFamily, iCount, &m_strFamily );
	FillComboBoxBodyMaterial();
}

void CDlgHydronicSchView_CVTab::FillComboBoxBodyMaterial()
{
	CRankEx ListEx;

	int iCount = m_pTADB->GetTaCVBdyMatList( &ListEx, CTADatabase::eForHMCv, CDB_ControlProperties::CV2W3W::LastCV2W3W, m_strType, m_strFamily, 
			m_pclHydronicSchView->GetCvType(), m_pclHydronicSchView->GetControlType(), CTADatabase::FilterSelection::NoFiltering, m_iDNMin, m_iDNMax );

	FillComboBox( &ListEx, &m_ComboBodyMaterial, iCount, &m_strBodyMaterial );
	FillComboBoxConnect();
}

void CDlgHydronicSchView_CVTab::FillComboBoxConnect()
{
	CRankEx ListEx;

	int iCount = m_pTADB->GetTaCVConnList( &ListEx, CTADatabase::eForHMCv, CDB_ControlProperties::CV2W3W::LastCV2W3W, m_strType, m_strFamily, m_strBodyMaterial, 
			m_pclHydronicSchView->GetCvType(), m_pclHydronicSchView->GetControlType(), CTADatabase::FilterSelection::NoFiltering, m_iDNMin, m_iDNMax );

	FillComboBox( &ListEx, &m_ComboConnect, iCount, &m_strConnect );
	FillComboBoxVersion();
}

void CDlgHydronicSchView_CVTab::FillComboBoxVersion()
{
	CRankEx ListEx;

	int iCount = m_pTADB->GetTaCVVersList( &ListEx, CTADatabase::eForHMCv, CDB_ControlProperties::CV2W3W::LastCV2W3W, m_strType, m_strFamily, m_strBodyMaterial, 
			m_strConnect, m_pclHydronicSchView->GetCvType(), m_pclHydronicSchView->GetControlType(), CTADatabase::FilterSelection::NoFiltering, m_iDNMin, m_iDNMax );

	FillComboBox( &ListEx, &m_ComboVersion, iCount, &m_strVersion );
	FillComboBoxPN();
}

void CDlgHydronicSchView_CVTab::FillComboBoxPN()
{
	CRankEx ListEx;

	int iCount = m_pTADB->GetTaCVPNList( &ListEx, CTADatabase::eForHMCv, CDB_ControlProperties::CV2W3W::LastCV2W3W, m_strType, m_strFamily, m_strBodyMaterial, 
			m_strConnect, m_strVersion, m_pclHydronicSchView->GetCvType(), m_pclHydronicSchView->GetControlType(), CTADatabase::FilterSelection::NoFiltering, m_iDNMin,m_iDNMax );

	FillComboBox( &ListEx, &m_ComboPN, iCount, &m_strPN );
	FillComboBoxName();
}

void CDlgHydronicSchView_CVTab::FillComboBoxName()
{
	// Reset the member variable.
	m_pProduct = NULL;

	CRankEx ListEx;
	int iCount = m_pTADB->GetTaCVList( &ListEx, CTADatabase::eForHMCv, false, CDB_ControlProperties::CV2W3W::LastCV2W3W, m_strType, m_strFamily, m_strBodyMaterial, 
			m_strConnect, m_strVersion, m_strPN, m_pclHydronicSchView->GetCvType(), m_pclHydronicSchView->GetControlType(), CTADatabase::FilterSelection::NoFiltering, m_iDNMin, m_iDNMax );

	FillComboBoxTAProduct( &ListEx, &m_ComboName, iCount );
	
	// Set the first item by default.
	if( 0 != iCount )
	{
		m_ComboName.SetCurSel( 0 );
		m_pProduct = dynamic_cast<CDB_ControlValve*>( (CData*)m_ComboName.GetItemData( 0 ) );
	}
}

void CDlgHydronicSchView_CVTab::DoDataExchange( CDataExchange *pDX )
{
	CDlgHydronicSchView_BaseTab::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_COMBOBVIN3WBYPASS, m_ComboBVIn3WBypass );
}

BEGIN_MESSAGE_MAP( CDlgHydronicSchView_CVTab, CDlgHydronicSchView_BaseTab )
	ON_CBN_SELCHANGE( IDC_COMBOBVIN3WBYPASS, OnCbnSelChangeBVIn3wBypass )
END_MESSAGE_MAP()

void CDlgHydronicSchView_CVTab::OnCbnSelChangeBVIn3wBypass()
{
	if( CB_ERR == m_ComboBVIn3WBypass.GetCurSel() )
	{
		return;
	}

	// I pass by an int to avoid compilation warning message.
	m_bBvIn3WBypass = ( 0 == (int)m_ComboBVIn3WBypass.GetItemData( m_ComboBVIn3WBypass.GetCurSel() ) ) ? false : true;

	m_pclHydronicSchView->VerifyCircuitSheme();

	// Redraw the scheme.
	m_pclHydronicSchView->RedrawSchemes();
}

void CDlgHydronicSchView_CVTab::FillComboBoxBVIn3WBypass()
{
	m_ComboBVIn3WBypass.ResetContent();

	m_ComboBVIn3WBypass.InsertString( 0, _No );
	m_ComboBVIn3WBypass.SetItemData( 0, (DWORD_PTR)false );

	m_ComboBVIn3WBypass.InsertString( 1, _Yes );
	m_ComboBVIn3WBypass.SetItemData( 1, (DWORD_PTR)true );

	m_ComboBVIn3WBypass.SetCurSel( 0 );
	m_bBvIn3WBypass = false;
}
