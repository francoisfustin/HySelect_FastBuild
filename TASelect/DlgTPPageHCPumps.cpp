#include "stdafx.h"
#include "TASelect.h"
#include "DlgTPPageHCPumps.h"


IMPLEMENT_DYNAMIC( CDlgTPPageHCPumps, CDlgCtrlPropPage )

CDlgTPPageHCPumps::CDlgTPPageHCPumps( CWnd* pParent )
	: CDlgCtrlPropPage( pParent )
{
	m_pTADS = NULL;
	m_dDefaultSecondaryPumpHMin = 0.0;
	m_csHeaderTitle = TASApp.LoadLocalizedString( IDS_TABDLGTECHHCPUMPS_HEADERTITLE );
	m_csPageName = TASApp.LoadLocalizedString( IDS_TABDLGTECHHCPUMPS_PAGENAME );
}

void CDlgTPPageHCPumps::Init( bool bResetToDefault )
{
	CDS_TechnicalParameter *pTechP;

	if( true == bResetToDefault )
	{
		pTechP = (CDS_TechnicalParameter *)( TASApp.GetpTADB()->Get( _T("PARAM_TECH") ).MP );
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

	m_dDefaultSecondaryPumpHMin = pTechP->GetDefaultSecondaryPumpHMin();

	// Initialize the edit boxes and radio buttons.
	m_EditDefaultSecondaryPumpHMin.SetEditType( CNumString::eEditType::eDouble, CNumString::eEditSign::ePositive );
	m_EditDefaultSecondaryPumpHMin.SetPhysicalType( _U_DIFFPRESS );
	m_EditDefaultSecondaryPumpHMin.SetMinDblValue( MINDPVALUE );
	m_EditDefaultSecondaryPumpHMin.SetCurrentValSI( m_dDefaultSecondaryPumpHMin );
	m_EditDefaultSecondaryPumpHMin.Update();
}

void CDlgTPPageHCPumps::Save( CString strSectionName )
{
	CDS_TechnicalParameter *pTech = TASApp.GetpTADS()->GetpTechParams();

	if ( NULL != GetSafeHwnd() )
	{
		pTech->SetDefaultSecondaryPumpHMin( m_dDefaultSecondaryPumpHMin );
	}
}

bool CDlgTPPageHCPumps::VerifyInfo()
{
	bool bResult = true;

	// Verify the edit default secondary pipe Hmin.
	double dMinSecondaryPumpHMin = 10000.0;
	double dDefaultSecondaryPumpHMin;

	ReadCUDouble( _U_DIFFPRESS, m_EditDefaultSecondaryPumpHMin, &dDefaultSecondaryPumpHMin );

	if( dDefaultSecondaryPumpHMin < dMinSecondaryPumpHMin )
	{
		bResult = false;
	}

	return bResult;
}

BEGIN_MESSAGE_MAP( CDlgTPPageHCPumps, CDlgCtrlPropPage )
	ON_EN_KILLFOCUS( IDC_EDITDEFAULTSECONDARYPUMPHMIN, OnEnKillFocusDefaultSecondaryPumpHMin )
	ON_EN_SETFOCUS( IDC_EDITDEFAULTSECONDARYPUMPHMIN, OnEnSetFocusDefaultSecondaryPumpHMin )
END_MESSAGE_MAP()

void CDlgTPPageHCPumps::DoDataExchange( CDataExchange* pDX )
{
	CDlgCtrlPropPage::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_EDITDEFAULTSECONDARYPUMPHMIN, m_EditDefaultSecondaryPumpHMin );
}

BOOL CDlgTPPageHCPumps::OnInitDialog() 
{
	if( FALSE == CDlgCtrlPropPage::OnInitDialog() )
	{
		return FALSE;
	}

	// Initialize dialog strings.
	CString str;
	str = TASApp.LoadLocalizedString( IDS_TABDLGTECHPAGEHCPUMPS_STATICDEFAULTSECONDARYPUMPHMIN );
	GetDlgItem( IDC_STATICDEFAULTSECONDARYPUMPHMIN )->SetWindowText( str );
	
	m_pTADS = TASApp.GetpTADS();
		
	Init();

	return TRUE;
}

void CDlgTPPageHCPumps::OnEnKillFocusDefaultSecondaryPumpHMin()
{
}

void CDlgTPPageHCPumps::OnEnSetFocusDefaultSecondaryPumpHMin()
{
}
