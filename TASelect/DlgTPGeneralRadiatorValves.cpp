#include "stdafx.h"
#include "TASelect.h"
#include "DlgTPGeneralRadiatorValves.h"


IMPLEMENT_DYNAMIC( CDlgTPGeneralRadiatorValves, CDlgCtrlPropPage )

CDlgTPGeneralRadiatorValves::CDlgTPGeneralRadiatorValves( CWnd* pParent )
	: CDlgCtrlPropPage( pParent )
{
	m_pTADS = NULL;
	m_dTrvDefDpTot = 0.0;
	m_dTrvDefRoomT = 0.0;
	m_dTrvDefSupplyT = 0.0;
	m_csHeaderTitle = TASApp.LoadLocalizedString( IDS_TABDLGTECHRAD_HEADERTITLE );
	m_csPageName = TASApp.LoadLocalizedString( IDS_TABDLGTECHRAD_PAGENAME );
}

void CDlgTPGeneralRadiatorValves::Init( bool fResetToDefault )
{
	CDS_TechnicalParameter *pTechP;

	if( true == fResetToDefault )
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
	SetDlgItemText( IDC_STATICUNITDP, tcName );
	
	GetNameOf( pUnitDB->GetDefaultUnit( _U_TEMPERATURE), tcName );
	SetDlgItemText( IDC_STATICUNITROOMT, tcName );
	SetDlgItemText( IDC_STATICUNITSUPPLYT, tcName );

	// Interfaced parameters.
	m_dTrvDefDpTot = pTechP->GetTrvDefDpTot();
	m_dTrvDefRoomT = pTechP->GetTrvDefRoomT();
	m_dTrvDefSupplyT = pTechP->GetTrvDefSupplyT();

	// Initialize the edit boxes and radio buttons.
	m_EditAvailableDp.SetWindowText( WriteDouble( CDimValue::SItoCU( _U_DIFFPRESS, m_dTrvDefDpTot ), 3, 1, true ) );
	m_EditRoomTemp.SetWindowText( WriteDouble( CDimValue::SItoCU( _U_TEMPERATURE, m_dTrvDefRoomT ), 3, 1, true ) );
	m_EditSupplyTemp.SetWindowText( WriteDouble( CDimValue::SItoCU( _U_TEMPERATURE, m_dTrvDefSupplyT ), 3, 1, true ) );
}

void CDlgTPGeneralRadiatorValves::Save( CString strSectionName )
{
	CDS_TechnicalParameter *pTech = TASApp.GetpTADS()->GetpTechParams();

	// Interfaced parameters.
	if ( NULL != GetSafeHwnd() )
	{
		pTech->SetTrvDefDpTot( m_dTrvDefDpTot );
		pTech->SetTrvDefRoomT( m_dTrvDefRoomT );
		pTech->SetTrvDefSupplyT( m_dTrvDefSupplyT );
	}
}

bool CDlgTPGeneralRadiatorValves::VerifyInfo()
{
	bool fResult = true;

	// Verify the edit available Dp.
	double dValue;
	switch( ReadDouble( m_EditAvailableDp, &dValue ) )
	{
		case RD_EMPTY:
			fResult = false;
			TASApp.AfxLocalizeMessageBox( AFXMSG_NON_POSITIVE_VALUE );
			break;
		
		case RD_OK:
			m_EditAvailableDp.SetWindowText( WriteDouble( dValue, 3, 1, true ) );
			m_dTrvDefDpTot = CDimValue::CUtoSI( _U_DIFFPRESS, dValue );
			break;
	}

	// Verify the edit room temperature.
	switch( ReadDouble( m_EditRoomTemp, &dValue ) )
	{
		case RD_EMPTY:
			fResult = false;
			TASApp.AfxLocalizeMessageBox( AFXMSG_NON_POSITIVE_VALUE );
			break;
		
		case RD_OK:
			m_EditRoomTemp.SetWindowText( WriteDouble( dValue, 3, 1, true ) );
			m_dTrvDefRoomT = CDimValue::CUtoSI( _U_TEMPERATURE, dValue );
			break;
	}

	// Verify the edit supply temperature.
	switch( ReadDouble( m_EditSupplyTemp, &dValue ) )
	{
		case RD_EMPTY:
			fResult = false;
			TASApp.AfxLocalizeMessageBox( AFXMSG_NON_POSITIVE_VALUE );
			break;
		
		case RD_OK:
			m_EditSupplyTemp.SetWindowText( WriteDouble( dValue, 3, 1, true ) );
			m_dTrvDefSupplyT = CDimValue::CUtoSI( _U_TEMPERATURE, dValue );
			break;
	}

	return fResult;
}

void CDlgTPGeneralRadiatorValves::DoDataExchange( CDataExchange* pDX )
{
	CDlgCtrlPropPage::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_EDITAVAILABLEDP, m_EditAvailableDp );
	DDX_Control( pDX, IDC_EDITROOMT, m_EditRoomTemp );
	DDX_Control( pDX, IDC_EDITSUPPLYT, m_EditSupplyTemp );
}

BOOL CDlgTPGeneralRadiatorValves::OnInitDialog() 
{
	if( FALSE == CDlgCtrlPropPage::OnInitDialog() )
	{
		return FALSE;
	}

	// Initialize edit boxes.
	m_EditAvailableDp.SetEditType( CNumString::eEditType::eDouble, CNumString::eEditSign::ePositive );
	m_EditRoomTemp.SetEditType( CNumString::eEditType::eDouble, CNumString::eEditSign::ePositive );
	m_EditSupplyTemp.SetEditType( CNumString::eEditType::eDouble, CNumString::eEditSign::ePositive );

	m_EditAvailableDp.SetPhysicalType( _U_DIFFPRESS );
	m_EditRoomTemp.SetPhysicalType( _U_TEMPERATURE );
	m_EditSupplyTemp.SetPhysicalType( _U_TEMPERATURE );
		
	// Initialize dialog strings.
	CString str;
	str = TASApp.LoadLocalizedString( IDS_TABDLGTECHRAD_AVAILABLEDP );
	GetDlgItem( IDC_STATICAVAILABLEDP )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_TABDLGTECHRAD_ROOMT );
	GetDlgItem( IDC_STATICROOMT )->SetWindowText( str );
	str = TASApp.LoadLocalizedString( IDS_TABDLGTECHRAD_SUPPLYTEMP );
	GetDlgItem( IDC_STATICSUPPLYT )->SetWindowText( str );

	m_pTADS = TASApp.GetpTADS();

	Init();
	
	return TRUE;
}
