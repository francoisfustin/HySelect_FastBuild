#include "stdafx.h"
#include "TASelect.h"
#include "DlgTPPageHCSmartCV.h"


IMPLEMENT_DYNAMIC( CDlgTPPageHCSmartCV, CDlgCtrlPropPage )

CDlgTPPageHCSmartCV::CDlgTPPageHCSmartCV( CWnd* pParent )
	: CDlgCtrlPropPage( pParent )
{
	m_csHeaderTitle = TASApp.LoadLocalizedString( IDS_TABDLGTECHSMCV_HEADERTITLE );
	m_csPageName = TASApp.LoadLocalizedString( IDS_TABDLGTECHSMCV_PAGENAME );
	m_iLocation = -1;
}

void CDlgTPPageHCSmartCV::Init( bool bResetToDefault )
{
	CDS_TechnicalParameter *pTechP;

	if( true == bResetToDefault )
	{
		pTechP = (CDS_TechnicalParameter *)( TASApp.GetpTADB()->Get( _T( "PARAM_TECH" ) ).MP );
	}
	else
	{
		pTechP = TASApp.GetpTADS()->GetpTechParams();
	}


	// Initialize the combo box box.
	m_iLocation = pTechP->GetDefaultSmartCVLoc();
	FillComboBoxSMCVLocation();
}

void CDlgTPPageHCSmartCV::Save( CString strSectionName )
{
	CDS_TechnicalParameter *pTech = TASApp.GetpTADS()->GetpTechParams();

	if( NULL != GetSafeHwnd() )
	{
		pTech->SetDefaultSmartCVLoc( m_iLocation );
	}
}

BEGIN_MESSAGE_MAP( CDlgTPPageHCSmartCV, CDlgCtrlPropPage )
	ON_CBN_SELCHANGE( IDC_COMBOSMCVLOC, OnCbnSelChangeLocation )
END_MESSAGE_MAP()

void CDlgTPPageHCSmartCV::DoDataExchange( CDataExchange* pDX )
{
	CDlgCtrlPropPage::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_COMBOSMCVLOC, m_ComboBoxLocation );
}

BOOL CDlgTPPageHCSmartCV::OnInitDialog()
{
	CDlgCtrlPropPage::OnInitDialog();

	// Initialize dialog strings.
	CString str = TASApp.LoadLocalizedString( IDS_TABDLGTECHHCSMCV_STATICLOCATION );
	GetDlgItem( IDC_STATICSMCVLOC )->SetWindowText( str );

	Init();

	return TRUE;
}

void CDlgTPPageHCSmartCV::OnCbnSelChangeLocation()
{
	m_iLocation = m_ComboBoxLocation.GetItemData( m_ComboBoxLocation.GetCurSel() );
}

void CDlgTPPageHCSmartCV::FillComboBoxSMCVLocation()
{
	CRank List( false );
	CString str = TASApp.LoadLocalizedString( IDS_SMARTVALVE_LOCATION_RETURN );
	List.Add( str, SmartValveLocalization::SmartValveLocReturn, (LPARAM)(void*)SmartValveLocalization::SmartValveLocReturn );
	
	str = TASApp.LoadLocalizedString( IDS_SMARTVALVE_LOCATION_SUPPLY );
	List.Add( str, SmartValveLocalization::SmartValveLocSupply, (LPARAM)(void*)SmartValveLocalization::SmartValveLocSupply );

	List.Transfer( &m_ComboBoxLocation );

	for( int i = 0; i < m_ComboBoxLocation.GetCount(); i++ )
	{
		if( m_ComboBoxLocation.GetItemData( i ) == m_iLocation )
		{
			m_ComboBoxLocation.SetCurSel( i );
			break;
		}
	}
}
