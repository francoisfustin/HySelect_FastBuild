#include "stdafx.h"
#include "TASelect.h"
#include "DlgTPGeneralFixedOrifices.h"


IMPLEMENT_DYNAMIC( CDlgTPGeneralFixedOrifices, CDlgCtrlPropPage )

CDlgTPGeneralFixedOrifices::CDlgTPGeneralFixedOrifices( CWnd* pParent )
	: CDlgCtrlPropPage( pParent )
{
	m_pTADS = NULL;
	m_dFoMinDp = 0.0;
	m_dFoMaxDp = 0.0;
	m_csHeaderTitle = TASApp.LoadLocalizedString( IDS_TABDLGTECHFO_HEADERTITLE );
	m_csPageName = TASApp.LoadLocalizedString( IDS_TABDLGTECHFO_PAGENAME );
}

void CDlgTPGeneralFixedOrifices::Init( bool fResetToDefault )
{
	CDS_TechnicalParameter *pTechP;

	if( true == fResetToDefault )
	{
		pTechP = (CDS_TechnicalParameter*)( TASApp.GetpTADB()->Get( _T("PARAM_TECH") ).MP );
	}
	else
	{
		pTechP = TASApp.GetpTADS()->GetpTechParams();
	}

	m_dFoMinDp = pTechP->GetFoMinDp();
	m_EditFoMinDp.SetEditType( CNumString::eEditType::eDouble, CNumString::eEditSign::ePositive );
	m_EditFoMinDp.SetPhysicalType( _U_DIFFPRESS );
	m_EditFoMinDp.SetMinDblValue( MINDPVALUE );
	m_EditFoMinDp.SetCurrentValSI(m_dFoMinDp);
	m_EditFoMinDp.Update();

	m_dFoMaxDp = pTechP->GetFoMaxDp();
	m_EditFoMaxDp.SetEditType( CNumString::eEditType::eDouble, CNumString::eEditSign::ePositive );
	m_EditFoMaxDp.SetPhysicalType( _U_DIFFPRESS );
	m_EditFoMaxDp.SetMinDblValue( MINDPVALUE );
	m_EditFoMaxDp.SetCurrentValSI(m_dFoMaxDp);
	m_EditFoMaxDp.Update();

	// Fill all unit static controls.
	TCHAR tcName[_MAXCHARS];
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();
	GetNameOf( pUnitDB->GetDefaultUnit( _U_DIFFPRESS ), tcName );
	SetDlgItemText( IDC_STATICDPUNIT3, tcName );
	SetDlgItemText( IDC_STATICDPUNIT4, tcName );
}

void CDlgTPGeneralFixedOrifices::Save( CString strSectionName )
{
	CDS_TechnicalParameter *pTech = TASApp.GetpTADS()->GetpTechParams();

	if ( NULL != GetSafeHwnd() )
	{
		pTech->SetFoMinDp( m_dFoMinDp );
		pTech->SetFoMaxDp( m_dFoMaxDp );
	}
}

bool CDlgTPGeneralFixedOrifices::VerifyInfo()
{
	bool bResult = true;

	// Verify the edit valve minimum Dp.
	double dValue, dValueMaxDp;
	ReadDouble( m_EditFoMaxDp, &dValueMaxDp );

	switch( ReadDouble( m_EditFoMinDp, &dValue ) )
	{
		case RD_EMPTY:
			bResult = false;
			TASApp.AfxLocalizeMessageBox( AFXMSG_NON_POSITIVE_VALUE );
			break;
		
		case RD_OK:
			if( dValue > dValueMaxDp )
			{
				bResult = false;
				TASApp.AfxLocalizeMessageBox( AFXMSG_INCORRECT_MINMAX );
			}
			else
			{
				m_EditFoMinDp.SetWindowText( WriteDouble( dValue, 3, 1, true ) );
				m_dFoMinDp = CDimValue::CUtoSI( _U_DIFFPRESS, dValue );
			}
			break;
	}

	// Verify the edit valve maximum Dp.
	double dValueMinDp;
	ReadDouble( m_EditFoMinDp, &dValueMinDp );
	switch( ReadDouble( m_EditFoMaxDp, &dValue ) )
	{
		case RD_EMPTY:
			bResult = false;
			TASApp.AfxLocalizeMessageBox( AFXMSG_NON_POSITIVE_VALUE );
			break;
		
		case RD_OK:
			if( dValue < dValueMinDp )
				bResult = false;
			else
			{
				m_EditFoMaxDp.SetWindowText( WriteDouble( dValue, 3, 1, true ) );
				m_dFoMaxDp = CDimValue::CUtoSI( _U_DIFFPRESS, dValue );
			}
			break;
	}

	return bResult;
}

BEGIN_MESSAGE_MAP( CDlgTPGeneralFixedOrifices, CDlgCtrlPropPage )
	ON_EN_KILLFOCUS( IDC_EDITFOMINDP, OnEnKillFocusFOMinDp )
	ON_EN_KILLFOCUS( IDC_EDITFOMAXDP, OnEnKillFocusFOMaxDp )
	ON_EN_SETFOCUS( IDC_EDITFOMINDP, OnEnSetFocusFOMinDp )
	ON_EN_SETFOCUS( IDC_EDITFOMAXDP, OnEnSetFocusFOMaxDp )
END_MESSAGE_MAP()

void CDlgTPGeneralFixedOrifices::DoDataExchange( CDataExchange* pDX )
{
	CDlgCtrlPropPage::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_EDITFOMINDP, m_EditFoMinDp );
	DDX_Control( pDX, IDC_EDITFOMAXDP, m_EditFoMaxDp );
}

BOOL CDlgTPGeneralFixedOrifices::OnInitDialog() 
{
	if( FALSE == CDlgCtrlPropPage::OnInitDialog() )
	{
		return FALSE;
	}

	// Initialize edit boxes
	m_EditFoMinDp.SetEditType( CNumString::eDouble, CNumString::ePositive );
	m_EditFoMaxDp.SetEditType( CNumString::eDouble, CNumString::ePositive );
	m_EditFoMinDp.SetPhysicalType( _U_DIFFPRESS );
	m_EditFoMaxDp.SetPhysicalType( _U_DIFFPRESS );

	// Initialize dialog strings
	CString str;
	str = TASApp.LoadLocalizedString( IDS_TABDLGTECHFIXORIFICE_STATICMINFODP );
	GetDlgItem( IDC_STATICMINFODP )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_TABDLGTECHFIXORIFICE_STATICMAXFODP );
	GetDlgItem( IDC_STATICMAXFODP )->SetWindowText( str );

	m_pTADS = TASApp.GetpTADS();
	
	Init();

	return TRUE;
}

void CDlgTPGeneralFixedOrifices::OnEnKillFocusFOMinDp()
{
	SetCorrectDblMinMaxBackColor( &m_EditFoMinDp, &m_EditFoMaxDp );
}

void CDlgTPGeneralFixedOrifices::OnEnKillFocusFOMaxDp()
{
	SetCorrectDblMinMaxBackColor( &m_EditFoMinDp, &m_EditFoMaxDp );
}

void CDlgTPGeneralFixedOrifices::OnEnSetFocusFOMinDp()
{
	SetCorrectDblMinMaxBackColor( &m_EditFoMinDp, &m_EditFoMaxDp, true );
}

void CDlgTPGeneralFixedOrifices::OnEnSetFocusFOMaxDp()
{
	SetCorrectDblMinMaxBackColor( &m_EditFoMinDp, &m_EditFoMaxDp, true );
}
