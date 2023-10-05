#include "stdafx.h"
#include "TASelect.h"
#include "DlgTPGeneralBalancingValves.h"


IMPLEMENT_DYNAMIC( CDlgTPGeneralBalancingValves, CDlgCtrlPropPage )

CDlgTPGeneralBalancingValves::CDlgTPGeneralBalancingValves( CWnd* pParent )
	: CDlgCtrlPropPage( pParent )
{
	m_pTADS = NULL;
	m_dValveMinDp = 0.0;
	m_dValveMaxDp = 0.0;
	m_csHeaderTitle = TASApp.LoadLocalizedString( IDS_TABDLGTECHBV_HEADERTITLE );
	m_csPageName = TASApp.LoadLocalizedString( IDS_TABDLGTECHBV_PAGENAME );
}

void CDlgTPGeneralBalancingValves::Init( bool bResetToDefault )
{
	CDS_TechnicalParameter *pTechP;

	if( true == bResetToDefault )
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
	SetDlgItemText( IDC_STATICDPUNIT1, tcName );
	SetDlgItemText( IDC_STATICDPUNIT, tcName );

	m_dValveMinDp = pTechP->GetValvMinDp();
	m_dValveMaxDp = pTechP->GetValvMaxDp();

	// Initialize the edit boxes and radio buttons.
	m_EditValveMinDp.SetEditType( CNumString::eEditType::eDouble, CNumString::eEditSign::ePositive );
	m_EditValveMinDp.SetPhysicalType( _U_DIFFPRESS );
	m_EditValveMinDp.SetMinDblValue( MINDPVALUE );
	m_EditValveMinDp.SetCurrentValSI(m_dValveMinDp);
	m_EditValveMinDp.Update();

	m_EditValveMaxDp.SetEditType( CNumString::eEditType::eDouble, CNumString::eEditSign::ePositive );
	m_EditValveMaxDp.SetPhysicalType( _U_DIFFPRESS );
	m_EditValveMaxDp.SetMinDblValue( MINDPVALUE );
	m_EditValveMaxDp.SetCurrentValSI(m_dValveMaxDp);
	m_EditValveMaxDp.Update();

	SetCorrectDblMinMaxBackColor(&m_EditValveMinDp, &m_EditValveMaxDp, bResetToDefault);
}

void CDlgTPGeneralBalancingValves::Save( CString strSectionName )
{
	CDS_TechnicalParameter *pTech = TASApp.GetpTADS()->GetpTechParams();

	if ( NULL != GetSafeHwnd() )
	{
		pTech->SetValvMinDp( m_dValveMinDp );
		// HYS-2024: Set valvMin2Dp according to ValveMinDp because it should be greather when ValvMinDp is changed in Tech. Param.
		// We add an offset of 3 kPa.
		pTech->SetValvMin2Dp( m_dValveMinDp + 3000 );
		pTech->SetValvMaxDp( m_dValveMaxDp );
	}
}

bool CDlgTPGeneralBalancingValves::VerifyInfo()
{
	bool fResult = true;

	// Verify the edit valve maximum Dp.
	double dValue, dValueMin;
	ReadDouble( m_EditValveMinDp, &dValueMin );

	switch( ReadDouble( m_EditValveMaxDp, &dValue ) )
	{
		case RD_EMPTY:
			fResult = false;
			TASApp.AfxLocalizeMessageBox( AFXMSG_NON_POSITIVE_VALUE );
			break;
		
		case RD_OK:
			if( dValue < dValueMin )
			{
				fResult = false;
				TASApp.AfxLocalizeMessageBox( AFXMSG_INCORRECT_MINMAX );
			}
			else
			{
				m_EditValveMaxDp.SetWindowText( WriteDouble( dValue, 3, 1, true ) );
				m_dValveMaxDp = CDimValue::CUtoSI( _U_DIFFPRESS, dValue );
			}
			break;
	}

	// Verify the edit valve minimum Dp.
	double dValueMax;
	ReadDouble( m_EditValveMaxDp, &dValueMax );
	switch( ReadDouble( m_EditValveMinDp, &dValue ) )
	{
		case RD_EMPTY:
			fResult = false;
			TASApp.AfxLocalizeMessageBox( AFXMSG_NON_POSITIVE_VALUE );
			break;
		
		case RD_OK:
			if( dValue > dValueMax )
				fResult = false;
			else
			{
				m_EditValveMinDp.SetWindowText( WriteDouble( dValue, 3, 1, true ) );
				m_dValveMinDp = CDimValue::CUtoSI( _U_DIFFPRESS, dValue );
			}
			break;
	};

	return fResult;
}

BEGIN_MESSAGE_MAP( CDlgTPGeneralBalancingValves, CDlgCtrlPropPage )
	ON_EN_KILLFOCUS( IDC_EDITVALVMINDP, OnEnKillFocusValveMinDp )
	ON_EN_KILLFOCUS( IDC_EDITVALVMAXDP, OnEnKillFocusValveMaxDp )
	ON_EN_SETFOCUS( IDC_EDITVALVMINDP, OnEnSetFocusValveMinDp )
	ON_EN_SETFOCUS( IDC_EDITVALVMAXDP, OnEnSetFocusValveMaxDp )
END_MESSAGE_MAP()

void CDlgTPGeneralBalancingValves::DoDataExchange( CDataExchange* pDX )
{
	CDlgCtrlPropPage::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_EDITVALVMINDP, m_EditValveMinDp );
	DDX_Control( pDX, IDC_EDITVALVMAXDP, m_EditValveMaxDp );
}

BOOL CDlgTPGeneralBalancingValves::OnInitDialog() 
{
	if( FALSE == CDlgCtrlPropPage::OnInitDialog() )
	{
		return FALSE;
	}

	// Initialize dialog strings.
	CString str;
	str = TASApp.LoadLocalizedString( IDS_TABDLGTECHVALV_STATICVALVMINDP );
	GetDlgItem( IDC_STATICVALVMINDP )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_TABDLGTECHVALV_STATICVALVMAXDP );
	GetDlgItem( IDC_STATICVALVMAXDP )->SetWindowText( str );

	m_pTADS = TASApp.GetpTADS();
		
	Init();

	return TRUE;
}

void CDlgTPGeneralBalancingValves::OnEnKillFocusValveMinDp()
{
	SetCorrectDblMinMaxBackColor( &m_EditValveMinDp, &m_EditValveMaxDp );
}

void CDlgTPGeneralBalancingValves::OnEnKillFocusValveMaxDp()
{
	SetCorrectDblMinMaxBackColor( &m_EditValveMinDp, &m_EditValveMaxDp );
}

void CDlgTPGeneralBalancingValves::OnEnSetFocusValveMinDp()
{
	SetCorrectDblMinMaxBackColor( &m_EditValveMinDp, &m_EditValveMaxDp, true );
}

void CDlgTPGeneralBalancingValves::OnEnSetFocusValveMaxDp()
{
	SetCorrectDblMinMaxBackColor( &m_EditValveMinDp, &m_EditValveMaxDp, true );
}
