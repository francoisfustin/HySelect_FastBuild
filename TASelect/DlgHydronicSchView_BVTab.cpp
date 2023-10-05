#include "stdafx.h"


#include "TASelect.h"
#include "MainFrm.h"
#include "Global.h"
#include "HydroMod.h"
#include "DlgHydronicSchView_BaseTab.h"
#include "DlgHydronicSchView_BVTab.h"
#include "DlgHydronicSchView.h"

CDlgHydronicSchView_BVTab::CDlgHydronicSchView_BVTab( CDlgHydronicSchView *pclHydronicSchView, CWnd *pParent )
	: CDlgHydronicSchView_BaseTab( pclHydronicSchView, CDlgHydronicSchView_BVTab::IDD,  pParent )
{
}

bool CDlgHydronicSchView_BVTab::IsAvailable( CDB_CircuitScheme *pCircuitScheme )
{
	if( NULL == pCircuitScheme )
	{
		return false;
	}

	bool bAvailable = false;

	if( pCircuitScheme->IsAnchorPtExist( CAnchorPt::BV_P ) || pCircuitScheme->IsAnchorPtExist( CAnchorPt::BV_S )
			|| pCircuitScheme->IsAnchorPtExist( CAnchorPt::BV_Byp ) )
	{
		bAvailable = true;
	}

	return bAvailable;
}

void CDlgHydronicSchView_BVTab::GetProducts( std::map<CAnchorPt::eFunc, CString> *pmapProducts, CDB_CircuitScheme *pCircuitScheme )
{
	if( NULL == pmapProducts || NULL == pCircuitScheme || NULL == m_pProduct )
	{
		return;
	}

	if( pCircuitScheme->IsAnchorPtExist( CAnchorPt::BV_P ) )
	{
		pmapProducts->insert( std::pair<CAnchorPt::eFunc, CString>( CAnchorPt::BV_P, m_pProduct->GetIDPtr().ID ) );
	}
	else if( pCircuitScheme->IsAnchorPtExist( CAnchorPt::BV_S ) )
	{
		pmapProducts->insert( std::pair<CAnchorPt::eFunc, CString>( CAnchorPt::BV_S, m_pProduct->GetIDPtr().ID ) );
	}
	else if( pCircuitScheme->IsAnchorPtExist( CAnchorPt::BV_Byp ) )
	{
		pmapProducts->insert( std::pair<CAnchorPt::eFunc, CString>( CAnchorPt::BV_Byp, m_pProduct->GetIDPtr().ID ) );
	}
}

void CDlgHydronicSchView_BVTab::FillComboBoxType()
{
	CRankEx ListEx;
	int iCount = m_pTADB->GetBVTypeList( &ListEx, CTADatabase::FilterSelection::NoFiltering, m_iDNMin, m_iDNMax, true );

	FillComboBox( &ListEx, &m_ComboType, iCount, &m_strType );
	FillComboBoxFamily();
}

void CDlgHydronicSchView_BVTab::FillComboBoxFamily()
{
	CRankEx ListEx;
	int iCount = m_pTADB->GetBVFamilyList( &ListEx, m_strType, CTADatabase::FilterSelection::NoFiltering, m_iDNMin, m_iDNMax );

	FillComboBox( &ListEx, &m_ComboFamily, iCount, &m_strFamily );
	FillComboBoxBodyMaterial();
}

void CDlgHydronicSchView_BVTab::FillComboBoxBodyMaterial()
{
	CRankEx ListEx;
	int iCount = m_pTADB->GetBVBdyMatList( &ListEx, m_strType, m_strFamily, CTADatabase::FilterSelection::NoFiltering, m_iDNMin, m_iDNMax );
	
	FillComboBox( &ListEx, &m_ComboBodyMaterial, iCount, &m_strBodyMaterial );
	FillComboBoxConnect();
}

void CDlgHydronicSchView_BVTab::FillComboBoxConnect()
{
	CRankEx ListEx;
	int iCount = m_pTADB->GetBVConnList( &ListEx, m_strType, m_strFamily, m_strBodyMaterial, CTADatabase::FilterSelection::NoFiltering, m_iDNMin, m_iDNMax );

	FillComboBox( &ListEx, &m_ComboConnect, iCount, &m_strConnect );
	FillComboBoxVersion();
}

void CDlgHydronicSchView_BVTab::FillComboBoxVersion()
{
	CRankEx ListEx;
	int iCount = m_pTADB->GetBVVersList( &ListEx, m_strType, m_strFamily, m_strBodyMaterial, m_strConnect, CTADatabase::FilterSelection::NoFiltering, m_iDNMin, m_iDNMax );	

	FillComboBox( &ListEx, &m_ComboVersion, iCount, &m_strVersion );
	FillComboBoxPN();
}

void CDlgHydronicSchView_BVTab::FillComboBoxPN()
{
	CRankEx ListEx;
	int iCount = m_pTADB->GetBVPNList( &ListEx, m_strType, m_strFamily, m_strBodyMaterial, m_strConnect, m_strVersion, CTADatabase::FilterSelection::NoFiltering, 
			m_iDNMin, m_iDNMax );

	FillComboBox( &ListEx, &m_ComboPN, iCount, &m_strPN );
	FillComboBoxName();
}

void CDlgHydronicSchView_BVTab::FillComboBoxName()
{
	// Reset the member variable.
	m_pProduct = NULL;

	CRankEx ListEx; 
	int iCount = m_pTADB->GetBVList( &ListEx, m_strType, m_strFamily, m_strBodyMaterial, m_strConnect, m_strVersion, CTADatabase::FilterSelection::NoFiltering, m_iDNMin, m_iDNMax );

	FillComboBoxTAProduct( &ListEx, &m_ComboName, iCount );

	// Set the first item by default.
	if( 0 != iCount )
	{
		m_ComboName.SetCurSel( 0 );
		m_pProduct = dynamic_cast<CDB_TAProduct *>( (CData *)m_ComboName.GetItemData( 0 ) );
	}
}
