#include "stdafx.h"


#include "TASelect.h"
#include "MainFrm.h"
#include "Global.h"
#include "HydroMod.h"
#include "DlgHydronicSchView_BaseTab.h"
#include "DlgHydronicSchView_DpCTab.h"
#include "DlgHydronicSchView.h"

CDlgHydronicSchView_DpCTab::CDlgHydronicSchView_DpCTab( CDlgHydronicSchView *pclHydronicSchView, CWnd *pParent )
	: CDlgHydronicSchView_BaseTab( pclHydronicSchView, CDlgHydronicSchView_DpCTab::IDD,  pParent )
{
}

bool CDlgHydronicSchView_DpCTab::IsAvailable( CDB_CircuitScheme *pCircuitScheme )
{
	if( NULL == pCircuitScheme )
	{
		return false;
	}

	bool bAvailable = false;

	if( pCircuitScheme->IsAnchorPtExist( CAnchorPt::DpC ) || pCircuitScheme->IsAnchorPtExist( CAnchorPt::DPCBCV ) )
	{
		bAvailable = true;
	}

	return bAvailable;
}

void CDlgHydronicSchView_DpCTab::GetProducts( std::map<CAnchorPt::eFunc, CString> *pmapProducts, CDB_CircuitScheme *pCircuitScheme )
{
	if( NULL == pmapProducts || NULL == pCircuitScheme || NULL == m_pProduct )
	{
		return;
	}

	if( pCircuitScheme->IsAnchorPtExist( CAnchorPt::DpC ) )
	{
		pmapProducts->insert( std::pair<CAnchorPt::eFunc, CString>( CAnchorPt::ControlValve, m_pProduct->GetIDPtr().ID ) );
	}
	else if( pCircuitScheme->IsAnchorPtExist( CAnchorPt::DPCBCV ) )
	{
		pmapProducts->insert( std::pair<CAnchorPt::eFunc, CString>( CAnchorPt::PICV, m_pProduct->GetIDPtr().ID ) );
	}
}

void CDlgHydronicSchView_DpCTab::SetAvailable( bool bAvailable )
{
	GetDlgItem( IDC_STATICMVPOSITION )->EnableWindow( ( true == bAvailable ) ? TRUE : FALSE );
	GetDlgItem( IDC_STATICDPCSTABON )->EnableWindow( ( true == bAvailable ) ? TRUE : FALSE );

	m_ComboMeasuringValvePosition.EnableWindow( ( true == bAvailable ) ? TRUE : FALSE );
	m_ComboDpStabilizedOn.EnableWindow( ( true == bAvailable ) ? TRUE : FALSE );

	CDlgHydronicSchView_BaseTab::SetAvailable( bAvailable );
}
void CDlgHydronicSchView_DpCTab::InitComboBox()
{
	CDlgHydronicSchView_BaseTab::InitComboBox();

	FillComboBoxMeasuringValvePosition();
	FillComboBoxDpStabilizedOn();
}

void CDlgHydronicSchView_DpCTab::FillComboBoxMeasuringValvePosition()
{
	m_ComboMeasuringValvePosition.ResetContent();

	m_ComboMeasuringValvePosition.InsertString( 0, _No );
	m_ComboMeasuringValvePosition.SetItemData( 0, (DWORD_PTR)eMvLoc::MvLocNone );

	m_ComboMeasuringValvePosition.InsertString( 1, _Primary );
	m_ComboMeasuringValvePosition.SetItemData( 1, (DWORD_PTR)eMvLoc::MvLocPrimary );

	m_ComboMeasuringValvePosition.InsertString( 2, _Secondary );
	m_ComboMeasuringValvePosition.SetItemData( 2, (DWORD_PTR)eMvLoc::MvLocSecondary );

	m_ComboMeasuringValvePosition.SetCurSel( 0 );
	m_eMvLoc = eMvLoc::MvLocNone;
}

void CDlgHydronicSchView_DpCTab::FillComboBoxDpStabilizedOn()
{
	m_ComboDpStabilizedOn.ResetContent();

	m_ComboDpStabilizedOn.InsertString( 0, _No );
	m_ComboDpStabilizedOn.SetItemData( 0, (DWORD_PTR)eDpStab::DpStabNone );

	m_ComboDpStabilizedOn.InsertString( 1, _Branch );
	m_ComboDpStabilizedOn.SetItemData( 1, (DWORD_PTR)eDpStab::DpStabOnBranch );

	m_ComboDpStabilizedOn.InsertString( 2, _Cv );
	m_ComboDpStabilizedOn.SetItemData( 2, (DWORD_PTR)eDpStab::DpStabOnCV );

	m_ComboDpStabilizedOn.SetCurSel( 0 );
	m_eDpStab = eDpStab::DpStabNone;
}

void CDlgHydronicSchView_DpCTab::FillComboBoxType()
{
	CRankEx ListEx;
	int iCount = m_pTADB->GetDpCTypeList( &ListEx, CTADatabase::FilterSelection::NoFiltering );

	FillComboBox( &ListEx, &m_ComboType, iCount, &m_strType );
	FillComboBoxFamily();
}

void CDlgHydronicSchView_DpCTab::FillComboBoxFamily()
{
	CRankEx ListEx;
	int iCount = m_pTADB->GetDpCFamilyList( &ListEx, eDpCLoc::DpCLocNone, m_strType, CTADatabase::FilterSelection::NoFiltering, m_iDNMin, m_iDNMax );
		
	FillComboBox( &ListEx, &m_ComboFamily, iCount, &m_strFamily );
	FillComboBoxBodyMaterial();
}

void CDlgHydronicSchView_DpCTab::FillComboBoxBodyMaterial()
{
	CRankEx ListEx;
	int iCount = m_pTADB->GetDpCBdyMatList( &ListEx, eDpCLoc::DpCLocNone, m_strType, m_strFamily, CTADatabase::FilterSelection::NoFiltering, 
			m_iDNMin, m_iDNMax );

	FillComboBox( &ListEx, &m_ComboBodyMaterial, iCount, &m_strBodyMaterial );
	FillComboBoxConnect();
}

void CDlgHydronicSchView_DpCTab::FillComboBoxConnect()
{
	CRankEx ListEx;
	int iCount = m_pTADB->GetDpCConnList( &ListEx, eDpCLoc::DpCLocNone, m_strType, m_strFamily, m_strBodyMaterial, 
		CTADatabase::FilterSelection::NoFiltering, m_iDNMin, m_iDNMax );
		
	FillComboBox( &ListEx, &m_ComboConnect, iCount, &m_strConnect );
	FillComboBoxVersion();
}

void CDlgHydronicSchView_DpCTab::FillComboBoxVersion()
{
	CRankEx ListEx;
	int iCount = m_pTADB->GetDpCVersList( &ListEx, eDpCLoc::DpCLocNone, m_strType, m_strFamily, m_strBodyMaterial, 
			m_strConnect, CTADatabase::FilterSelection::NoFiltering, m_iDNMin, m_iDNMax );

	FillComboBox( &ListEx, &m_ComboVersion, iCount,  &m_strVersion );
	FillComboBoxPN();
}

void CDlgHydronicSchView_DpCTab::FillComboBoxPN()
{
	CRankEx ListEx;
	int iCount = m_pTADB->GetDpCPNList( &ListEx, eDpCLoc::DpCLocNone, m_strType, m_strFamily, m_strBodyMaterial, 
			m_strConnect, m_strVersion, CTADatabase::FilterSelection::NoFiltering, m_iDNMin, m_iDNMax );
	
	FillComboBox( &ListEx, &m_ComboPN, iCount, &m_strPN );
	FillComboBoxName();
}

void CDlgHydronicSchView_DpCTab::FillComboBoxName()
{
	// Reset the member variable.
	m_pProduct = NULL;

	CRankEx ListEx;
	int iCount = m_pTADB->GetDpCList( &ListEx, eDpCLoc::DpCLocNone, m_strType, m_strFamily, m_strBodyMaterial, m_strConnect, 
			m_strVersion, m_strPN, CTADatabase::FilterSelection::NoFiltering, m_iDNMin, m_iDNMax );

	FillComboBoxTAProduct( &ListEx, &m_ComboName, iCount );
	
	// Set the first item by default.
	if( 0 != iCount )
	{
		m_ComboName.SetCurSel( 0 );
		m_pProduct = dynamic_cast<CDB_DpController*>( (CData *)m_ComboName.GetItemData( 0 ) );
	}
}

void CDlgHydronicSchView_DpCTab::DoDataExchange( CDataExchange *pDX )
{
	CDlgHydronicSchView_BaseTab::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_COMBOMVPOSITION, m_ComboMeasuringValvePosition );
	DDX_Control( pDX, IDC_COMBODPSTAB, m_ComboDpStabilizedOn );
}

BEGIN_MESSAGE_MAP( CDlgHydronicSchView_DpCTab, CDlgHydronicSchView_BaseTab )
	ON_CBN_SELCHANGE( IDC_COMBOMVPOSITION, OnCbnSelChangeMeasuringValvePosition )
	ON_CBN_SELCHANGE( IDC_COMBODPSTAB, OnCbnSelChangeDpStabilizedOn )
END_MESSAGE_MAP()

void CDlgHydronicSchView_DpCTab::OnCbnSelChangeMeasuringValvePosition()
{
	if( CB_ERR == m_ComboMeasuringValvePosition.GetCurSel() )
	{
		return;
	}

	m_eMvLoc = (eMvLoc)m_ComboMeasuringValvePosition.GetItemData( m_ComboMeasuringValvePosition.GetCurSel() );

	m_pclHydronicSchView->VerifyCircuitSheme();

	// Redraw the scheme.
	m_pclHydronicSchView->RedrawSchemes();
}

void CDlgHydronicSchView_DpCTab::OnCbnSelChangeDpStabilizedOn()
{
	if( CB_ERR == m_ComboDpStabilizedOn.GetCurSel() )
	{
		return;
	}

	m_eDpStab = (eDpStab)m_ComboDpStabilizedOn.GetItemData( m_ComboDpStabilizedOn.GetCurSel() );

	m_pclHydronicSchView->VerifyCircuitSheme();

	// Redraw the scheme.
	m_pclHydronicSchView->RedrawSchemes();
}

