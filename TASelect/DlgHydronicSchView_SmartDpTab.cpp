#include "stdafx.h"


#include "TASelect.h"
#include "MainFrm.h"
#include "Global.h"
#include "HydroMod.h"
#include "DlgHydronicSchView_BaseTab.h"
#include "DlgHydronicSchView_SmartDpTab.h"
#include "DlgHydronicSchView.h"

CDlgHydronicSchView_SmartDpTab::CDlgHydronicSchView_SmartDpTab( CDlgHydronicSchView *pclHydronicSchView, CWnd *pParent )
	: CDlgHydronicSchView_BaseTab( pclHydronicSchView, CDlgHydronicSchView_SmartDpTab::IDD,  pParent )
{
}

void CDlgHydronicSchView_SmartDpTab::SetAvailable( bool bAvailable )
{
	GetDlgItem( IDC_STATICTLOCATION )->EnableWindow( ( true == bAvailable ) ? TRUE : FALSE );
	GetDlgItem( IDC_CHECKSELECTEDBYSET )->EnableWindow( ( true == bAvailable ) ? TRUE : FALSE );
	
	m_ComboLocation.EnableWindow( ( true == bAvailable ) ? TRUE : FALSE );
	m_CheckSelectedBySet.EnableWindow( ( true == bAvailable ) ? TRUE : FALSE );

	CDlgHydronicSchView_BaseTab::SetAvailable( bAvailable );
}

void CDlgHydronicSchView_SmartDpTab::InitComboBox()
{
	CDlgHydronicSchView_BaseTab::InitComboBox();

	FillComboBoxLocation();
}

void CDlgHydronicSchView_SmartDpTab::FillComboBoxLocation()
{
	m_ComboLocation.ResetContent();

	m_ComboLocation.InsertString( 0, _Supply );
	m_ComboLocation.SetItemData( 0, (DWORD_PTR)SmartValveLocalization::SmartValveLocSupply );

	m_ComboLocation.InsertString( 1, _Return );
	m_ComboLocation.SetItemData( 1, (DWORD_PTR)SmartValveLocalization::SmartValveLocReturn );

	m_ComboLocation.SetCurSel( 0 );
	m_eSmartValveLocation = SmartValveLocalization::SmartValveLocSupply;
}

bool CDlgHydronicSchView_SmartDpTab::IsAvailable( CDB_CircuitScheme *pCircuitScheme )
{
	if( NULL == pCircuitScheme )
	{
		return false;
	}

	bool bAvailable = false;

	if( pCircuitScheme->IsAnchorPtExist( CAnchorPt::SmartDpC ) )
	{
		bAvailable = true;
	}

	return bAvailable;
}

void CDlgHydronicSchView_SmartDpTab::GetProducts( std::map<CAnchorPt::eFunc, CString> *pmapProducts, CDB_CircuitScheme *pCircuitScheme )
{
	if( NULL == pmapProducts || NULL == pCircuitScheme || NULL == m_pProduct )
	{
		return;
	}

	if( pCircuitScheme->IsAnchorPtExist( CAnchorPt::SmartDpC ) )
	{
		pmapProducts->insert( std::pair<CAnchorPt::eFunc, CString>( CAnchorPt::SmartDpC, m_pProduct->GetIDPtr().ID ) );
	}
}

void CDlgHydronicSchView_SmartDpTab::FillComboBoxType()
{
	CRankEx ListEx;
	int iCount = m_pTADB->GetSmartDpCTypeList( &ListEx, CTADatabase::FilterSelection::NoFiltering, m_iDNMin, m_iDNMax, true );

	FillComboBox( &ListEx, &m_ComboType, iCount, &m_strType );
	FillComboBoxFamily();
}

void CDlgHydronicSchView_SmartDpTab::FillComboBoxFamily()
{
	CRankEx ListEx;
	int iCount = m_pTADB->GetSmartDpCFamilyList( &ListEx, m_strType, CTADatabase::FilterSelection::NoFiltering, m_iDNMin, m_iDNMax );

	FillComboBox( &ListEx, &m_ComboFamily, iCount, &m_strFamily );
	FillComboBoxBodyMaterial();
}

void CDlgHydronicSchView_SmartDpTab::FillComboBoxBodyMaterial()
{
	CRankEx ListEx;

	int iCount = m_pTADB->GetSmartDpCBdyMatList( &ListEx, m_strType, m_strFamily, CTADatabase::FilterSelection::NoFiltering, m_iDNMin, m_iDNMax );
	
	FillComboBox( &ListEx, &m_ComboBodyMaterial, iCount, &m_strBodyMaterial );
	FillComboBoxConnect();
}

void CDlgHydronicSchView_SmartDpTab::FillComboBoxConnect()
{
	CRankEx ListEx;

	int iCount = m_pTADB->GetSmartDpCConnList( &ListEx, m_strType, m_strFamily, m_strBodyMaterial, 
			CTADatabase::FilterSelection::NoFiltering, m_iDNMin, m_iDNMax );

	FillComboBox( &ListEx, &m_ComboConnect, iCount, &m_strConnect );
	FillComboBoxVersion();
}

void CDlgHydronicSchView_SmartDpTab::FillComboBoxVersion()
{
	CRankEx ListEx;

	int iCount = m_pTADB->GetSmartDpCVersList( &ListEx, m_strType, m_strFamily, m_strBodyMaterial, m_strConnect, 
			CTADatabase::FilterSelection::NoFiltering, m_iDNMin, m_iDNMax );	

	FillComboBox( &ListEx, &m_ComboVersion, iCount, &m_strVersion );
	FillComboBoxPN();
}

void CDlgHydronicSchView_SmartDpTab::FillComboBoxPN()
{
	CRankEx ListEx;

	int iCount = m_pTADB->GetSmartDpCPNList( &ListEx, m_strType, m_strFamily, m_strBodyMaterial, m_strConnect, m_strVersion, 
			CTADatabase::FilterSelection::NoFiltering, m_iDNMin, m_iDNMax );

	FillComboBox( &ListEx, &m_ComboPN, iCount, &m_strPN );
	FillComboBoxName();
}

void CDlgHydronicSchView_SmartDpTab::FillComboBoxName()
{
	// Reset the member variable.
	m_pProduct = NULL;

	CRankEx ListEx; 

	int iCount = m_pTADB->GetSmartDpCList( &ListEx, m_strType, m_strFamily, m_strBodyMaterial, m_strConnect, m_strVersion, m_strPN, 
			CTADatabase::FilterSelection::NoFiltering, m_iDNMin, m_iDNMax, NULL, false, m_bSelectedBySet );

	FillComboBoxTAProduct( &ListEx, &m_ComboName, iCount );

	// Set the first item by default.
	if( 0 != iCount )
	{
		m_ComboName.SetCurSel( 0 );
		m_pProduct = dynamic_cast<CDB_TAProduct *>( (CData *)m_ComboName.GetItemData( 0 ) );
	}
}

void CDlgHydronicSchView_SmartDpTab::DoDataExchange( CDataExchange *pDX )
{
	CDlgHydronicSchView_BaseTab::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_COMBOLOCATION, m_ComboLocation );
	DDX_Control( pDX, IDC_CHECKSELECTEDBYSET, m_CheckSelectedBySet );
}

BEGIN_MESSAGE_MAP( CDlgHydronicSchView_SmartDpTab, CDlgHydronicSchView_BaseTab )
	ON_CBN_SELCHANGE( IDC_COMBOLOCATION, OnCbnSelChangeLocation )
	ON_BN_CLICKED( IDC_CHECKSELECTEDBYSET, OnBnClickedCheckSelectedBySet )
END_MESSAGE_MAP()

void CDlgHydronicSchView_SmartDpTab::OnCbnSelChangeLocation()
{
	if( CB_ERR == m_ComboLocation.GetCurSel() )
	{
		return;
	}

	m_eSmartValveLocation = (SmartValveLocalization)m_ComboLocation.GetItemData( m_ComboLocation.GetCurSel() );

	m_pclHydronicSchView->VerifyCircuitSheme();

	ResetComboBoxStr();
	FillComboBoxType();

	// Redraw the scheme.
	m_pclHydronicSchView->RedrawSchemes();
}

void CDlgHydronicSchView_SmartDpTab::OnBnClickedCheckSelectedBySet()
{
	m_bSelectedBySet = ( BST_CHECKED == m_CheckSelectedBySet.GetCheck() ) ? true : false;

	// Redraw the scheme.
	m_pclHydronicSchView->RedrawSchemes();
}