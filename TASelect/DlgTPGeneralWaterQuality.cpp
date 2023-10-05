#include "stdafx.h"
#include "TASelect.h"
#include "DlgTPGeneralWaterQuality.h"


IMPLEMENT_DYNAMIC( DlgTPGeneralWaterQuality, CDlgCtrlPropPage )

DlgTPGeneralWaterQuality::DlgTPGeneralWaterQuality( CWnd *pParent )
	: CDlgCtrlPropPage( pParent )
{
	m_csHeaderTitle = TASApp.LoadLocalizedString( IDS_TABDLGTECHWQ_HEADERTITLE );
	m_csPageName = TASApp.LoadLocalizedString( IDS_TABDLGTECHWQ_PAGENAME );
}

void DlgTPGeneralWaterQuality::Init( bool fResetToDefault )
{
	CDS_TechnicalParameter *pTechP;

	if( true == fResetToDefault )
	{
		pTechP = ( CDS_TechnicalParameter * )( TASApp.GetpTADB()->Get( _T("PARAM_TECH") ).MP );
	}
	else
	{
		pTechP = TASApp.GetpTADS()->GetpTechParams();
	}

	// Set the text for the 'pressure.volume' unit.
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();
	SetDlgItemText( IDC_STATICWATERHARDUNIT, pUnitDB->GetNameOfDefaultUnit( _U_WATERHARDNESS ).c_str() );

	// Initialize the edit box.
	m_clEditWaterHardness.SetEditType( CNumString::eEditType::eDouble, CNumString::eEditSign::ePositive );
	m_clEditWaterHardness.SetPhysicalType( _U_WATERHARDNESS );
	m_clEditWaterHardness.SetMinDblValue( 0.0 );
	m_clEditWaterHardness.SetMaxDblValue( 280.39479587 ); // 50°dH
	m_clEditWaterHardness.SetCurrentValSI( pTechP->GetDefaultWaterHardness() );
	m_clEditWaterHardness.Update();
}

void DlgTPGeneralWaterQuality::Save( CString strSectionName )
{
	CDS_TechnicalParameter *pTech = TASApp.GetpTADS()->GetpTechParams();

	if( NULL != GetSafeHwnd() )
	{
		pTech->SetDefaultWaterHardness( m_clEditWaterHardness.GetCurrentValSI() );
	}
}

bool DlgTPGeneralWaterQuality::VerifyInfo()
{
	bool fResult = true;

	// Verify the edit.
	double dValue;

	switch( ReadDouble( m_clEditWaterHardness, &dValue ) )
	{
		case RD_EMPTY:
			fResult = false;
			TASApp.AfxLocalizeMessageBox( AFXMSG_NON_POSITIVE_VALUE );
			break;

		case RD_OK:
			break;
	}

	return fResult;
}

void DlgTPGeneralWaterQuality::DoDataExchange( CDataExchange *pDX )
{
	CDlgCtrlPropPage::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_EDITWATERHARDNESS, m_clEditWaterHardness );
}

BOOL DlgTPGeneralWaterQuality::OnInitDialog()
{
	CDlgCtrlPropPage::OnInitDialog();

	// Initialize dialog strings.
	CString str = TASApp.LoadLocalizedString( IDS_TABDLGTECHWQ_STATICWATERHARDNESS );
	GetDlgItem( IDC_STATICWATERHARDNESS )->SetWindowText( str );

	Init();

	return TRUE;
}
