#include "stdafx.h"
#include "TASelect.h"
#include "DlgTPGeneralSmartValves.h"


IMPLEMENT_DYNAMIC( CDlgTPGeneralSmartValves, CDlgCtrlPropPage )

CDlgTPGeneralSmartValves::CDlgTPGeneralSmartValves( CWnd* pParent )
	: CDlgCtrlPropPage( pParent )
{
	m_dDpMaxForBestSmartValveSuggestion = 0.0;
	m_csHeaderTitle = TASApp.LoadLocalizedString( IDS_TABDLGTECHSMARTVALVES_HEADERTITLE );
	m_csPageName = TASApp.LoadLocalizedString( IDS_TABDLGTECHSMARTVALVE_PAGENAME );
}

void CDlgTPGeneralSmartValves::Init( bool bResetToDefault )
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
	SetDlgItemText( IDC_STATICSVDPMAXBESTSUGGESTUNIT, tcName );

	// Interfaced parameters.
	m_dDpMaxForBestSmartValveSuggestion = pTechP->GetSmartValveDpMaxForBestSuggestion();

	// Initialize the edit.
	m_EditDpMaxForBestSmartValveSuggestion.SetEditType( CNumString::eEditType::eDouble, CNumString::eEditSign::ePositive );
	m_EditDpMaxForBestSmartValveSuggestion.SetPhysicalType( _U_DIFFPRESS );
	m_EditDpMaxForBestSmartValveSuggestion.SetMinDblValue( MINDPVALUE );
	m_EditDpMaxForBestSmartValveSuggestion.SetMaxDblValue( DBL_MAX );
	m_EditDpMaxForBestSmartValveSuggestion.SetCurrentValSI (m_dDpMaxForBestSmartValveSuggestion );
	m_EditDpMaxForBestSmartValveSuggestion.Update();
}

void CDlgTPGeneralSmartValves::Save( CString strSectionName )
{
	CDS_TechnicalParameter *pTech = TASApp.GetpTADS()->GetpTechParams();
	
	if ( NULL != GetSafeHwnd() )
	{
		pTech->SetSmartValveDpMaxForBestSuggestion( m_dDpMaxForBestSmartValveSuggestion );
	}
}

bool CDlgTPGeneralSmartValves::VerifyInfo()
{
	bool bResult = true;

	// Verify the edit minimum authority.
	double dValue;

	// Verify the edit max. dp for best smart control valve suggestion.
	switch( ReadCUDouble( _U_DIFFPRESS, m_EditDpMaxForBestSmartValveSuggestion, &dValue ) )
	{
		case RD_EMPTY:
			bResult = false;
			TASApp.AfxLocalizeMessageBox( AFXMSG_NON_POSITIVE_VALUE );
			break;
	
		case RD_OK:
			m_dDpMaxForBestSmartValveSuggestion = dValue;
			break;
	}

	return bResult;
}

void CDlgTPGeneralSmartValves::DoDataExchange( CDataExchange* pDX )
{
	CDlgCtrlPropPage::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_EDITSVDPMAXBESTSUGGEST, m_EditDpMaxForBestSmartValveSuggestion );
}

BOOL CDlgTPGeneralSmartValves::OnInitDialog() 
{
	if( FALSE == CDlgCtrlPropPage::OnInitDialog() )
	{
		return FALSE;
	}

	// Initialize dialog strings.
	CString str = TASApp.LoadLocalizedString( IDS_TABDLGTECHHCSMCV_STATICDPMAXBESTSUGGEST );
	GetDlgItem( IDC_STATICSSVMAXDPBESTSUGGEST )->SetWindowText( str );

	m_pTADS = TASApp.GetpTADS();

	Init();

	return TRUE;
}
