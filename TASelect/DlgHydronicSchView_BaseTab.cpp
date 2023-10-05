#include "stdafx.h"


#include "TASelect.h"
#include "MainFrm.h"
#include "Global.h"
#include "Hydromod.h"
#include "DlgHydronicSchView_BaseTab.h"
#include "DlgHydronicSchView.h"

CDlgHydronicSchView_BaseTab::CDlgHydronicSchView_BaseTab( CDlgHydronicSchView *pclHydronicSchView, UINT uiIDD, CWnd *pParent )
	: CDialogEx( uiIDD,  pParent )
{
	m_pclHydronicSchView = pclHydronicSchView;
	ASSERT( NULL != m_pclHydronicSchView );

	m_pTADB = TASApp.GetpTADB();
	ASSERT( NULL != m_pTADB );

	m_iDNMin = -1;
	m_iDNMax = -1;

	m_eMvLoc = eMvLoc::MvLocNone;
	m_eDpStab = eDpStab::DpStabNone;
	m_bBvIn3WBypass = false;
	m_eSmartValveLocation = SmartValveLocalization::SmartValveLocNone;
	m_bSelectedBySet = false;

	m_strType = _T("");
	m_strFamily = _T("");
	m_strBodyMaterial = _T("");
	m_strConnect = _T("");
	m_strVersion = _T("");
	m_strPN = _T("");

	m_pProduct = NULL;
}

void CDlgHydronicSchView_BaseTab::SetAvailable( bool bAvailable )
{
	GetDlgItem( IDC_STATICTPRODUCTSIZE )->EnableWindow( ( true == bAvailable ) ? TRUE : FALSE );
	GetDlgItem( IDC_STATICTYPE )->EnableWindow( ( true == bAvailable ) ? TRUE : FALSE );
	GetDlgItem( IDC_STATICFAM )->EnableWindow( ( true == bAvailable ) ? TRUE : FALSE );
	GetDlgItem( IDC_STATICBDYMAT )->EnableWindow( ( true == bAvailable ) ? TRUE : FALSE );
	GetDlgItem( IDC_STATICCON )->EnableWindow( ( true == bAvailable ) ? TRUE : FALSE );
	GetDlgItem( IDC_STATICVERSION )->EnableWindow( ( true == bAvailable ) ? TRUE : FALSE );
	GetDlgItem( IDC_STATICPN )->EnableWindow( ( true == bAvailable ) ? TRUE : FALSE );
	GetDlgItem( IDC_STATICNAME )->EnableWindow( ( true == bAvailable ) ? TRUE : FALSE );

	m_ComboProductSize.EnableWindow( ( true == bAvailable ) ? TRUE : FALSE );
	m_ComboType.EnableWindow( ( true == bAvailable ) ? TRUE : FALSE );
	m_ComboFamily.EnableWindow( ( true == bAvailable ) ? TRUE : FALSE );
	m_ComboBodyMaterial.EnableWindow( ( true == bAvailable ) ? TRUE : FALSE );
	m_ComboConnect.EnableWindow( ( true == bAvailable ) ? TRUE : FALSE );
	m_ComboVersion.EnableWindow( ( true == bAvailable ) ? TRUE : FALSE );
	m_ComboPN.EnableWindow( ( true == bAvailable ) ? TRUE : FALSE );
	m_ComboName.EnableWindow( ( true == bAvailable ) ? TRUE : FALSE );

	Invalidate();
	UpdateWindow();
}

void CDlgHydronicSchView_BaseTab::InitComboBox()
{
	FillComboBoxProductSize();
}

void CDlgHydronicSchView_BaseTab::ResetComboBoxStr()
{
	m_strType = _T("");
	m_strFamily = _T("");
	m_strBodyMaterial = _T("");
	m_strConnect = _T("");
	m_strVersion = _T("");
	m_strPN = _T("");
}

void CDlgHydronicSchView_BaseTab::DoDataExchange( CDataExchange *pDX )
{
	CDialogEx::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_COMBOPRODUCTSIZE, m_ComboProductSize );
	DDX_Control( pDX, IDC_COMBOTYPE, m_ComboType );
	DDX_Control( pDX, IDC_COMBOFAM, m_ComboFamily );           
	DDX_Control( pDX, IDC_COMBOBDYMAT, m_ComboBodyMaterial );
	DDX_Control( pDX, IDC_COMBOCON, m_ComboConnect );
	DDX_Control( pDX, IDC_COMBOVER, m_ComboVersion );
	DDX_Control( pDX, IDC_COMBOPN, m_ComboPN );
	DDX_Control( pDX, IDC_COMBONAME, m_ComboName );
}

BEGIN_MESSAGE_MAP( CDlgHydronicSchView_BaseTab, CDialogEx )
	ON_CBN_SELCHANGE( IDC_COMBOPRODUCTSIZE, OnCbnSelChangeProductSize )
	ON_CBN_SELCHANGE( IDC_COMBOTYPE, OnCbnSelChangeType )
	ON_CBN_SELCHANGE( IDC_COMBOFAM, OnCbnSelChangeFamily )
	ON_CBN_SELCHANGE( IDC_COMBOBDYMAT, OnCbnSelChangeBodyMaterial )
	ON_CBN_SELCHANGE( IDC_COMBOCON, OnCbnSelChangeConnect )
	ON_CBN_SELCHANGE( IDC_COMBOVER, OnCbnSelChangeVersion )
	ON_CBN_SELCHANGE( IDC_COMBOPN, OnCbnSelChangePN )
	ON_CBN_SELCHANGE( IDC_COMBONAME, OnCbnSelChangeComboName )
END_MESSAGE_MAP()

void CDlgHydronicSchView_BaseTab::OnCbnSelChangeProductSize()
{
	if( CB_ERR == m_ComboProductSize.GetCurSel() )
	{
		return;
	}

	CTableDN *pclTableDN = (CTableDN *)( m_pTADB->Get( _T("DN_TAB") ).MP );
	ASSERT( NULL != pclTableDN );

	CString str( _T("") );
	m_ComboProductSize.GetLBText( m_ComboProductSize.GetCurSel(), str );

	if( _Blw65 == str )
	{
		m_iDNMin = 0;
		m_iDNMax = pclTableDN->GetSize( _T("DN_50") );
	}
	else if( _Abv50 == str )
	{
		m_iDNMin = pclTableDN->GetSize( _T("DN_65") );
		m_iDNMax = INT_MAX;
	}
	else if( _AllSizes == str )
	{
		m_iDNMin = 0;
		m_iDNMax = INT_MAX;
	}

	ResetComboBoxStr();
	FillComboBoxType();

	// Redraw the scheme.
	m_pclHydronicSchView->RedrawSchemes();
}

void CDlgHydronicSchView_BaseTab::OnCbnSelChangeType()
{
	CDB_StringID *pStrIDPointer = dynamic_cast<CDB_StringID *>( (CData *)m_ComboType.GetItemData( m_ComboType.GetCurSel() ) );
	
	if( NULL == pStrIDPointer )
	{
		return;
	}

	m_strType = pStrIDPointer->GetIDPtr().ID;

	// Fill the combo.
	FillComboBoxType();

	// Redraw the scheme.
	m_pclHydronicSchView->RedrawSchemes();
}

void CDlgHydronicSchView_BaseTab::OnCbnSelChangeFamily()
{
	CDB_StringID *pStrIDPointer = dynamic_cast<CDB_StringID *>( (CData *)m_ComboFamily.GetItemData( m_ComboFamily.GetCurSel() ) );
	
	if( NULL == pStrIDPointer )
	{
		return;
	}

	m_strFamily = pStrIDPointer->GetIDPtr().ID;

	// Fill the combo.
	FillComboBoxFamily();

	// Redraw the scheme.
	m_pclHydronicSchView->RedrawSchemes();
}

void CDlgHydronicSchView_BaseTab::OnCbnSelChangeBodyMaterial()
{
	CDB_StringID *pStrIDPointer = dynamic_cast<CDB_StringID *>( (CData *)m_ComboBodyMaterial.GetItemData( m_ComboBodyMaterial.GetCurSel() ) );
	
	if( NULL == pStrIDPointer )
	{
		return;
	}

	m_strBodyMaterial = pStrIDPointer->GetIDPtr().ID;

	// Fill the combo.
	FillComboBoxBodyMaterial();

	// Redraw the scheme.
	m_pclHydronicSchView->RedrawSchemes();
}

void CDlgHydronicSchView_BaseTab::OnCbnSelChangeConnect()
{
	CDB_StringID *pStrIDPointer = dynamic_cast<CDB_StringID *>( (CData *)m_ComboConnect.GetItemData( m_ComboConnect.GetCurSel() ) );

	if( NULL == pStrIDPointer )
	{
		return;
	}

	m_strConnect = pStrIDPointer->GetIDPtr().ID;

	// Fill the combo.
	FillComboBoxConnect();

	// Redraw the scheme.
	m_pclHydronicSchView->RedrawSchemes();
}

void CDlgHydronicSchView_BaseTab::OnCbnSelChangeVersion()
{
	CDB_StringID *pStrIDPointer = dynamic_cast<CDB_StringID *>( (CData *)m_ComboVersion.GetItemData( m_ComboVersion.GetCurSel() ) );

	if( NULL == pStrIDPointer )
	{
		return;
	}

	m_strVersion = pStrIDPointer->GetIDPtr().ID;

	// Fill the combo.
	FillComboBoxVersion();

	// Redraw the scheme.
	m_pclHydronicSchView->RedrawSchemes();
}

void CDlgHydronicSchView_BaseTab::OnCbnSelChangePN()
{
	CDB_StringID *pStrIDPointer = dynamic_cast<CDB_StringID *>( (CData *)m_ComboPN.GetItemData( m_ComboPN.GetCurSel() ) );

	if( NULL == pStrIDPointer )
	{
		return;
	}

	m_strPN = pStrIDPointer->GetIDPtr().ID;

	// Fill the combo.
	FillComboBoxPN();

	// Redraw the scheme.
	m_pclHydronicSchView->RedrawSchemes();
}

void CDlgHydronicSchView_BaseTab::OnCbnSelChangeComboName()
{
	m_pProduct = dynamic_cast<CDB_TAProduct *>( (CData *)m_ComboName.GetItemData( m_ComboName.GetCurSel() ) );

	// Redraw the scheme.
	m_pclHydronicSchView->RedrawSchemes();
}

void CDlgHydronicSchView_BaseTab::FillComboBoxProductSize()
{
	m_ComboProductSize.ResetContent();
	m_ComboProductSize.InsertString( 0, _Blw65 );
	m_ComboProductSize.InsertString( 1, _Abv50 );
	m_ComboProductSize.InsertString( 2, _AllSizes );
	
	m_ComboProductSize.SetCurSel( 0 );
	CTableDN *pclTableDN = (CTableDN *)( m_pTADB->Get( _T("DN_TAB") ).MP );
	ASSERT( NULL != pclTableDN );

	m_iDNMax = pclTableDN->GetSize( _T("DN_50") );
	m_iDNMin = 0;
}

void CDlgHydronicSchView_BaseTab::FillComboBox( CRankEx *pListEx, CComboBox *pCCombo, int iCount, _string *pCstr )
{
	_string str1;
	LPARAM lpParam;

	if( 0 != iCount && true == pListEx->GetFirst( str1, lpParam ) )
	{
		ASSERT( (LPARAM)0 != lpParam);
		pListEx->Transfer( pCCombo );
		int iNbre = pCCombo->GetCount();
		int iPos = -1;

		if( NULL != pCstr )
		{
			for( int i = 0; i < iNbre && iPos == -1; i++ )
			{
				CDB_StringID *pStrIDPointer = dynamic_cast<CDB_StringID *>( (CData *)pCCombo->GetItemData( i ) );

				if( NULL != pStrIDPointer && pStrIDPointer->GetIDPtr().ID == *pCstr )
				{
					pCCombo->SetCurSel( i );
					iPos = i;
				}
			}
		}

		if( -1 == iPos && 0 != iNbre )
		{
			pCCombo->SetCurSel( 0 );
			CDB_StringID *pStrIDPointer = dynamic_cast<CDB_StringID *>( (CData *)pCCombo->GetItemData( 0 ) );
			*pCstr = pStrIDPointer->GetIDPtr().ID;
		}
	}

	// Add this test to put white string to understand that no product exist
	if( iCount < 1 )
	{
		pCCombo->ResetContent();
	}

	if( iCount < 2 )
	{
		pCCombo->EnableWindow( FALSE );
	}
	else
	{
		pCCombo->EnableWindow( TRUE );
	}

	pListEx->PurgeAll();
}


void CDlgHydronicSchView_BaseTab::FillComboBoxTAProduct( CRankEx *pListEx, CComboBox *pCCombo, int iCount )
{
	_string str;
	LPARAM lpParam;
	int i = 0;

	if( 0 != iCount && true == pListEx->GetFirst( str, lpParam ) )
	{
		// Clear the content before adding new items.
		pCCombo->ResetContent();

		for( bool bContinue = pListEx->GetFirst( str, lpParam ); true == bContinue; bContinue = pListEx->GetNext( str, lpParam ) )
		{
			ASSERT( (LPARAM)0 != lpParam );
			CDB_TAProduct *pTAP = dynamic_cast<CDB_TAProduct *>( (CData *)lpParam );
			
			pCCombo->InsertString( i, pTAP->GetName() );
			pCCombo->SetItemData( i, lpParam );
			i++;
		}
	}

	// Add this test to put white string to understand that no product exist.
	if( iCount < 1 )
	{
		pCCombo->ResetContent();
	}
	
	if( i < 2 )
	{
		pCCombo->EnableWindow( FALSE );
	}
	else
	{
		pCCombo->EnableWindow( TRUE );
	}

	pListEx->PurgeAll();
}
