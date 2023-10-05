#include "stdafx.h"
#include "TASelect.h"
#include "DlgTPGeneralStdAndAdjustableControlValves.h"


IMPLEMENT_DYNAMIC( CDlgTPGeneralStdAndAdjustableControlValves, CDlgCtrlPropPage )

CDlgTPGeneralStdAndAdjustableControlValves::CDlgTPGeneralStdAndAdjustableControlValves( CWnd* pParent )
	: CDlgCtrlPropPage( pParent )
{
	m_iUseDpCPBand = 0;
	m_dCvMinAuthority = 0.0;
	m_dCvMinDpProportional = 0.0;
	m_dCvMaxDpProportional = 0.0;
	m_iCvDefKvReynard = 0;
	m_dCvMinDpOnOff = 0.0;
	m_pTADS = NULL;
	m_dCvMinAuthCstFlow = 0.0;
	m_dCvMaxDispDp = 0.0;
	m_csHeaderTitle = TASApp.LoadLocalizedString( IDS_TABDLGTECHCV_HEADERTITLE );
	m_csPageName = TASApp.LoadLocalizedString( IDS_TABDLGTECHCV_PAGENAME );
}

void CDlgTPGeneralStdAndAdjustableControlValves::Init( bool bResetToDefault )
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
	SetDlgItemText( IDC_STATICMINDPONOFFUNIT, tcName );
	SetDlgItemText( IDC_STATICMINDPPROPUNIT, tcName );
	SetDlgItemText( IDC_STATICMAXDPPROPUNIT, tcName );
	SetDlgItemText( IDC_STATICMAXDPUNIT, tcName );
	SetDlgItemText( IDC_STATICSVDPMAXBESTSUGGESTUNIT, tcName );

	// Interfaced parameters.
	m_dCvMinAuthority = pTechP->GetCVMinAuthor();
	m_dCvMinAuthCstFlow = pTechP->GetCVMinAuthCstFlow();
	m_dCvMinDpOnOff = pTechP->GetCVMinDpOnoff();
	m_dCvMinDpProportional = pTechP->GetCVMinDpProp();
	m_dCvMaxDpProportional = pTechP->GetCVMaxDpProp();
	m_dCvMaxDispDp = pTechP->GetCVMaxDispDp();
	m_iCvDefKvReynard = pTechP->GetCVDefKvReynard();
	m_iUseDpCPBand = pTechP->GetCVUseDpCBand();

	// Initialize the edit and check boxes.
	m_EditMinAuthority.SetEditType( CNumString::eEditType::eDouble, CNumString::eEditSign::ePositive );
	m_EditMinAuthority.SetWindowText( WriteDouble( m_dCvMinAuthority, 3, 2, 1 ) );
	m_EditMinAuthority.SetMaxDblValue( 1.0 );
   
	m_EditMinAuthCstFlow.SetEditType( CNumString::eEditType::eDouble, CNumString::eEditSign::ePositive );
	m_EditMinAuthCstFlow.SetWindowText( WriteDouble( m_dCvMinAuthCstFlow, 3, 2, 1 ) );
	m_EditMinAuthCstFlow.SetMaxDblValue( 1.0 );

	m_EditMinDpOnOff.SetEditType( CNumString::eEditType::eDouble, CNumString::eEditSign::ePositive );
	m_EditMinDpOnOff.SetPhysicalType( _U_DIFFPRESS );
	m_EditMinDpOnOff.SetMinDblValue( MINDPVALUE );
	// Currently the maximum value for DpOnOff is not editable in the dialog box.
	// So by default we will not accept more than the default (see in localdb.txt) maxval for all Cv.
	m_EditMinDpOnOff.SetMaxDblValue( m_dCvMaxDispDp );
	m_EditMinDpOnOff.SetCurrentValSI( m_dCvMinDpOnOff );
	m_EditMinDpOnOff.Update();

	m_EditMinDpProportional.SetEditType( CNumString::eEditType::eDouble, CNumString::eEditSign::ePositive );
	m_EditMinDpProportional.SetPhysicalType( _U_DIFFPRESS );
	m_EditMinDpProportional.SetMinDblValue( MINDPVALUE );
	m_EditMinDpProportional.SetMaxDblValue( m_dCvMaxDispDp );
	m_EditMinDpProportional.SetCurrentValSI( m_dCvMinDpProportional );
	m_EditMinDpProportional.Update();

	m_EditMaxDpProportional.SetEditType( CNumString::eEditType::eDouble, CNumString::eEditSign::ePositive );
	m_EditMaxDpProportional.SetPhysicalType( _U_DIFFPRESS );
	m_EditMaxDpProportional.SetMinDblValue( MINDPVALUE );
	m_EditMaxDpProportional.SetMaxDblValue( m_dCvMaxDispDp );
	m_EditMaxDpProportional.SetCurrentValSI( m_dCvMaxDpProportional );
	m_EditMaxDpProportional.Update();

	m_CheckReynard.SetCheck( ( 1 == m_iCvDefKvReynard ) ? BST_CHECKED : BST_UNCHECKED );
	m_CheckDpCPBand.SetCheck( ( 1 == m_iUseDpCPBand ) ? BST_CHECKED : BST_UNCHECKED );
}

void CDlgTPGeneralStdAndAdjustableControlValves::Save( CString strSectionName )
{
	CDS_TechnicalParameter *pTech = TASApp.GetpTADS()->GetpTechParams();
	
	if ( NULL != GetSafeHwnd() )
	{
		pTech->SetCVMinAuthor( m_dCvMinAuthority );
		pTech->SetCVMinAuthCstFlow( m_dCvMinAuthCstFlow );
		pTech->SetCVMinDpOnoff( m_dCvMinDpOnOff );
		pTech->SetCVMinDpProp( m_dCvMinDpProportional );
		pTech->SetCVMaxDpProp( m_dCvMaxDpProportional );
		pTech->SetCVDefKvReynard( m_iCvDefKvReynard );
		pTech->SetCVUseDpCBand( m_iUseDpCPBand );
	}
}

bool CDlgTPGeneralStdAndAdjustableControlValves::VerifyInfo()
{
	bool bResult = true;

	// Verify the edit minimum authority.
	double dValue;

	switch( ReadDouble( m_EditMinAuthority, &dValue ) )
	{
		case RD_EMPTY:
			bResult = false;
			TASApp.AfxLocalizeMessageBox( AFXMSG_NON_POSITIVE_VALUE );
			break;

		case RD_OK:
			m_EditMinAuthority.SetWindowText( WriteDouble( dValue, 3, 1, true ) );
			m_dCvMinAuthority = dValue;
			break;
	}

	switch( ReadDouble( m_EditMinAuthCstFlow, &dValue ) )
	{
		case RD_EMPTY:
			bResult = false;
			TASApp.AfxLocalizeMessageBox( AFXMSG_NON_POSITIVE_VALUE );
			break;

		case RD_OK:
			m_EditMinAuthCstFlow.SetWindowText( WriteDouble( dValue, 3, 1, true ) );
			m_dCvMinAuthCstFlow = dValue;
			break;
	}

	// Verify the edit minimum Dp On-Off.
	switch( ReadCUDouble( _U_DIFFPRESS, m_EditMinDpOnOff, &dValue ) )
	{
		case RD_EMPTY:
			bResult = false;
			TASApp.AfxLocalizeMessageBox( AFXMSG_NON_POSITIVE_VALUE );
			break;
	
		case RD_OK:
			// TODO : Currently commented until we add the edit for the max value.
			//if( dValue > m_CvMaxDispDp )
			//{
			//	bResult = false;
			//	TASApp.AfxLocalizeMessageBox( AFXMSG_INCORRECT_MINMAX );
			//}
			//else
			{
				m_dCvMinDpOnOff = dValue;
			}
			break;
	}

	// Verify the edit minimum Dp proportional.
	switch( ReadCUDouble( _U_DIFFPRESS, m_EditMinDpProportional, &dValue ) )
	{
		case RD_EMPTY:
			bResult = false;
			TASApp.AfxLocalizeMessageBox( AFXMSG_NON_POSITIVE_VALUE );
			break;
		
		case RD_OK:
			// the edit for the max value.
			if( dValue > m_dCvMaxDpProportional )
			{
				bResult = false;
				TASApp.AfxLocalizeMessageBox( AFXMSG_INCORRECT_MINMAX );
			}
			else
			{
				m_dCvMinDpProportional = dValue;
			}
			break;
	}

	// Verify the edit maximum Dp proportional.
	switch( ReadCUDouble( _U_DIFFPRESS, m_EditMaxDpProportional, &dValue ) )
	{
		case RD_EMPTY:
			bResult = false;
			TASApp.AfxLocalizeMessageBox( AFXMSG_NON_POSITIVE_VALUE );
			break;
		
		case RD_OK:
			// the edit for the max value.
			if( dValue < m_dCvMinDpProportional )
			{
				bResult = false;
				TASApp.AfxLocalizeMessageBox( AFXMSG_INCORRECT_MINMAX );
			}
			else
			{
				m_dCvMaxDpProportional = dValue;
			}
			break;
	}

	return bResult;
}

BEGIN_MESSAGE_MAP( CDlgTPGeneralStdAndAdjustableControlValves, CDlgCtrlPropPage )
	ON_EN_KILLFOCUS( IDC_EDITMINDPPROP, OnEnKillFocusMinDpProportional )
	ON_EN_KILLFOCUS( IDC_EDITMAXDPPROP, OnEnKillFocusMaxDpProportional )
	ON_EN_SETFOCUS( IDC_EDITMAXDPPROP, OnEnSetFocusMaxDpProportional )
	ON_EN_SETFOCUS( IDC_EDITMINDPPROP, OnEnSetFocusMinDpProportional )
	ON_BN_CLICKED( IDC_CHECKREYNARD, OnBnClickedReynard )
	ON_BN_CLICKED(IDC_CHECKDPCPBAND, OnBnClickedCheckdpcpband)
END_MESSAGE_MAP()

void CDlgTPGeneralStdAndAdjustableControlValves::DoDataExchange( CDataExchange* pDX )
{
	CDlgCtrlPropPage::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_EDITMINAUTHO, m_EditMinAuthority );
	DDX_Control( pDX, IDC_EDITMINAUTHOCSTFLOW, m_EditMinAuthCstFlow );
	DDX_Control( pDX, IDC_EDITMINDPONOFF, m_EditMinDpOnOff );
	DDX_Control( pDX, IDC_EDITMINDPPROP, m_EditMinDpProportional );
	DDX_Control( pDX, IDC_EDITMAXDPPROP, m_EditMaxDpProportional );
	DDX_Control( pDX, IDC_CHECKREYNARD, m_CheckReynard );
	DDX_Control(pDX, IDC_CHECKDPCPBAND, m_CheckDpCPBand);
}

BOOL CDlgTPGeneralStdAndAdjustableControlValves::OnInitDialog() 
{
	if( FALSE == CDlgCtrlPropPage::OnInitDialog() )
	{
		return FALSE;
	}

	// Initialize dialog strings.
	CString str;
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();
	str = TASApp.LoadLocalizedString( IDS_TABDLGTECHCV_STATICMINAUTHO );
	GetDlgItem( IDC_STATICMINAUTHO )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_TABDLGTECHCV_STATICMINAUTHOFORCSTFLOWSYSTEMS );
	GetDlgItem( IDC_STATICMINAUTHOCSTFLOW )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_TABDLGTECHCV_STATICMINDPONOFF );
	GetDlgItem( IDC_STATICMINDPONOFF )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_TABDLGTECHCV_STATICMINDPPROP );
	GetDlgItem( IDC_STATICMINDPPROP )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_TABDLGTECHCV_STATICMAXDPPROP );
	GetDlgItem( IDC_STATICMAXDPPROP )->SetWindowText( str );

	if( !pUnitDB->GetDefaultUnitIndex( _C_KVCVCOEFF ) )
	{
		str = TASApp.LoadLocalizedString( IDS_TABDLGTECHCV_CHECKBOXREYNARD );
	}
	else
	{
		str = TASApp.LoadLocalizedString( IDS_TABDLGTECHCV_CHECKBOXREYNARDCV );
	}

	GetDlgItem( IDC_CHECKREYNARD )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_TECHPARAM_CV_PBAND );
	GetDlgItem( IDC_CHECKDPCPBAND )->SetWindowText( str );

	m_pTADS = TASApp.GetpTADS();

	Init();
	
	// Initialize edit boxes.
	m_EditMinAuthority.SetEditType( CNumString::eEditType::eDouble, CNumString::eEditSign::ePositive );
	
	// The authority : [0 to 1].
	m_EditMinAuthority.SetMaxDblValue( 1.0 );
	
	m_EditMinDpOnOff.SetEditType( CNumString::eEditType::eDouble, CNumString::eEditSign::ePositive );
	m_EditMinDpOnOff.SetPhysicalType( _U_DIFFPRESS );
	
	// Currently the maximum value for DpOnOff is not editable in the dialog box.
	// So by default we will not accept more than the default (see in localdb.txt) maxval for all Cv.
	m_EditMinDpOnOff.SetMaxDblValue( m_dCvMaxDispDp );
	
	m_EditMinDpProportional.SetEditType( CNumString::eEditType::eDouble, CNumString::eEditSign::ePositive );
	m_EditMaxDpProportional.SetEditType( CNumString::eEditType::eDouble, CNumString::eEditSign::ePositive );
	m_EditMinDpProportional.SetPhysicalType( _U_DIFFPRESS );
	m_EditMaxDpProportional.SetPhysicalType( _U_DIFFPRESS );
	
	// See comment just above.
	m_EditMinDpProportional.SetMaxDblValue( m_dCvMaxDispDp );
	m_EditMaxDpProportional.SetMaxDblValue( m_dCvMaxDispDp );

	return TRUE;
}

void CDlgTPGeneralStdAndAdjustableControlValves::OnEnKillFocusMinDpProportional()
{
	SetCorrectDblMinMaxBackColor( &m_EditMinDpProportional, &m_EditMaxDpProportional );
}

void CDlgTPGeneralStdAndAdjustableControlValves::OnEnKillFocusMaxDpProportional()
{
	SetCorrectDblMinMaxBackColor( &m_EditMinDpProportional, &m_EditMaxDpProportional );
}

void CDlgTPGeneralStdAndAdjustableControlValves::OnEnSetFocusMaxDpProportional()
{
	SetCorrectDblMinMaxBackColor( &m_EditMinDpProportional, &m_EditMaxDpProportional, true );
}

void CDlgTPGeneralStdAndAdjustableControlValves::OnEnSetFocusMinDpProportional()
{
	SetCorrectDblMinMaxBackColor( &m_EditMinDpProportional, &m_EditMaxDpProportional, true );
}

void CDlgTPGeneralStdAndAdjustableControlValves::OnBnClickedReynard()
{
	m_iCvDefKvReynard = ( BST_CHECKED == m_CheckReynard.GetCheck() ) ? 1 : 0;	
}

void CDlgTPGeneralStdAndAdjustableControlValves::OnBnClickedCheckdpcpband()
{
	m_iUseDpCPBand  = ( BST_CHECKED == m_CheckDpCPBand.GetCheck() ) ? 1 : 0;
}
