#include "stdafx.h"
#include "TASelect.h"
#include "DlgTPGeneralPipes.h"


IMPLEMENT_DYNAMIC( CDlgTPGeneralPipes, CDlgCtrlPropPage )

CDlgTPGeneralPipes::CDlgTPGeneralPipes( CWnd* pParent )
	: CDlgCtrlPropPage( pParent )
{
	m_pTADS = NULL;
	m_dTargetLinearDp = 0.0;
	m_dMaxLinearDp = 0.0;
	m_fTargetWaterVelocity = false;
	m_dTargetWaterVelocity = 0.0;
	m_dMaxWaterVelocity = 0.0;
	// HYS-1878: 3- Consider min. velocity when changing max or target value.
	m_dMinWaterVelocity = 0.0;
	m_csHeaderTitle = TASApp.LoadLocalizedString( IDS_TABDLGTECHPIPES_HEADERTITLE );
	m_csPageName = TASApp.LoadLocalizedString( IDS_TABDLGTECHPIPES_PAGENAME );
}

void CDlgTPGeneralPipes::Init( bool fResetToDefault )
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

	TCHAR tcName[_MAXCHARS];
	CUnitDatabase *pUnitDB = CDimValue::AccessUDB();
	
	// Set the text for the linear DP units.
	GetNameOf( pUnitDB->GetDefaultUnit( _U_LINPRESSDROP ), tcName );
	SetDlgItemText( IDC_STATICLINDPUNIT1, tcName );
	SetDlgItemText( IDC_STATICLINDPUNIT2, tcName );
	
	// Set the text for the velocity units.
	GetNameOf( pUnitDB->GetDefaultUnit( _U_VELOCITY ), tcName );
	SetDlgItemText( IDC_STATICVELUNIT1, tcName );
	SetDlgItemText( IDC_STATICVELUNIT2, tcName );
	// HYS-1878: 3- Add min. pipe velocity field in dialog tech. param.
	SetDlgItemText( IDC_STATICVELUNIT3, tcName );

	m_dTargetLinearDp = pTechP->GetPipeTargDp();
	m_dMaxLinearDp = pTechP->GetPipeMaxDp();
	m_dTargetWaterVelocity = pTechP->GetPipeTargVel();
	m_dMaxWaterVelocity = pTechP->GetPipeMaxVel();
	// HYS-1878: 3- Consider min. velocity when changing max or target value.
	m_dMinWaterVelocity = pTechP->GetPipeMinVel();
	m_fTargetWaterVelocity = ( 1 == pTechP->GetPipeVtargUsed() ) ? true : false;
	
	// Initialize edit boxes.
	m_EditTargetLinearDp.SetEditType( CNumString::eEditType::eDouble, CNumString::eEditSign::ePositive );
	m_EditTargetLinearDp.SetPhysicalType( _U_DIFFPRESS );
	m_EditTargetLinearDp.SetWindowText( WriteDouble( CDimValue::SItoCU( _U_LINPRESSDROP, m_dTargetLinearDp ), 3, 1, true ) );

	m_EditMaxLinearDp.SetEditType( CNumString::eEditType::eDouble, CNumString::eEditSign::ePositive );
	m_EditMaxLinearDp.SetPhysicalType( _U_DIFFPRESS );
	m_EditMaxLinearDp.SetWindowText( WriteDouble( CDimValue::SItoCU( _U_LINPRESSDROP, m_dMaxLinearDp ), 3, 1, true ) );

	m_EditTargetWaterVelocity.SetEditType( CNumString::eEditType::eDouble, CNumString::eEditSign::ePositive );
	m_EditTargetWaterVelocity.SetPhysicalType( _U_VELOCITY );
	m_EditTargetWaterVelocity.SetWindowText( WriteDouble( CDimValue::SItoCU( _U_VELOCITY, m_dTargetWaterVelocity ), 3, 1, true ) );

	m_EditMaxWaterVelocity.SetEditType( CNumString::eEditType::eDouble, CNumString::eEditSign::ePositive );
	m_EditMaxWaterVelocity.SetPhysicalType( _U_VELOCITY );
	m_EditMaxWaterVelocity.SetWindowText( WriteDouble( CDimValue::SItoCU( _U_VELOCITY, m_dMaxWaterVelocity ), 3, 1, true ) );

	// HYS-1878: 3- Add min. pipe velocity field in dialog tech. param.
	m_EditMinWaterVelocity.SetEditType( CNumString::eEditType::eDouble, CNumString::eEditSign::ePositive );
	m_EditMinWaterVelocity.SetPhysicalType( _U_VELOCITY );
	m_EditMinWaterVelocity.SetWindowText( WriteDouble( CDimValue::SItoCU( _U_VELOCITY, m_dMinWaterVelocity ), 3, 1, true ) );
	
	m_CheckTargetWaterVelocity.SetCheck( ( true == m_fTargetWaterVelocity )? BST_CHECKED : BST_UNCHECKED );

	// Enable/disable m_EditPipeTargVel and IDC_STATICVELUNIT1.
	m_EditTargetWaterVelocity.EnableWindow( m_fTargetWaterVelocity );
	GetDlgItem( IDC_STATICVELUNIT1 )->EnableWindow( m_fTargetWaterVelocity );

	// HYS-1878: Add warning message for min velocity
	CString str1, str2, str3;
	GetNameOf( pUnitDB->GetDefaultUnit( _U_VELOCITY ), tcName );
	str3 = tcName;
	// 0.2 m/s
	str1 = WriteDouble( CDimValue::SItoCU( _U_VELOCITY, 0.2 ), 3, 1, true );
	// 0.6 m/s
	str2 = WriteDouble( CDimValue::SItoCU( _U_VELOCITY, 0.6 ), 3, 1, true );
	m_StaticWarnMinVelValue.ShowWindow( SW_HIDE );
	FormatString( m_strWarnMinVelValue, IDS_TABDLGTECHPIPES_WARNMINVEL, str1, str2, str3 );
	m_StaticWarnMinVelValue.SetWindowText( m_strWarnMinVelValue );
	m_StaticWarnMinVelValue.SetTextColor( _RED );
}

void CDlgTPGeneralPipes::Save( CString strSectionName )
{
	CDS_TechnicalParameter *pTech = TASApp.GetpTADS()->GetpTechParams();

	if ( NULL != GetSafeHwnd() )
	{
		pTech->SetPipeTargDp( m_dTargetLinearDp );
		pTech->SetPipeMaxDp( m_dMaxLinearDp );
		pTech->SetPipeTargVel( m_dTargetWaterVelocity );
		pTech->SetPipeMaxVel( m_dMaxWaterVelocity );
		// HYS-1878: 3- Add min. pipe velocity field in dialog tech. param.
		pTech->SetPipeMinVel( m_dMinWaterVelocity );
		pTech->SetPipeVtargUsed( ( true == m_fTargetWaterVelocity ) ? 1 : 0 );
	}
}

bool CDlgTPGeneralPipes::VerifyInfo()
{
	bool fResult = true;
	
	// Verify the edit pipe target linear Dp.
	double dMaxLinearDp;
	double dTargetLinearDp;
	ReadDouble( m_EditMaxLinearDp, &dMaxLinearDp );
	switch( ReadDouble( m_EditTargetLinearDp, &dTargetLinearDp ) )
	{
		case RD_EMPTY:
			fResult = false;
			TASApp.AfxLocalizeMessageBox( AFXMSG_NON_POSITIVE_VALUE );
			break;
		
		case RD_OK:
			if( dTargetLinearDp > dMaxLinearDp )
			{
				fResult = false;
				TASApp.AfxLocalizeMessageBox( AFXMSG_INCORRECT_TARGMAX );
			}
			else
			{
				m_EditTargetLinearDp.SetWindowText( WriteDouble( dTargetLinearDp, 3, 1, true ) );
				m_dTargetLinearDp = CDimValue::CUtoSI( _U_LINPRESSDROP, dTargetLinearDp );
			}
			break;
	}

	// Verify the edit pipe max linear Dp.
	switch( ReadDouble( m_EditMaxLinearDp, &dMaxLinearDp ) )
	{
		case RD_EMPTY:
			fResult = false;
			TASApp.AfxLocalizeMessageBox( AFXMSG_NON_POSITIVE_VALUE );
			break;
		
		case RD_OK:
			if( dMaxLinearDp < dTargetLinearDp )
				fResult = false;
			else
			{
				m_EditMaxLinearDp.SetWindowText( WriteDouble( dMaxLinearDp, 3, 1, true ) );
				m_dMaxLinearDp = CDimValue::CUtoSI( _U_LINPRESSDROP, dMaxLinearDp );
			}
			break;
	}

	// Verify the edit pipe target water velocity.
	double dMaxWaterVelocity;
	double dTargetWaterVelocity;
	// HYS-1878: 3- Add min. pipe velocity field in dialog tech. param.
	double dMinWaterVelocity;
	ReadDouble( m_EditMaxWaterVelocity, &dMaxWaterVelocity );
	switch( ReadDouble( m_EditTargetWaterVelocity, &dTargetWaterVelocity ) )
	{
		case RD_EMPTY: case RD_NOT_NUMBER:
			fResult = false;
			TASApp.AfxLocalizeMessageBox( AFXMSG_NON_POSITIVE_VALUE );
			break;
		
		case RD_OK:
			if( dTargetWaterVelocity > dMaxWaterVelocity && true == m_fTargetWaterVelocity )
			{
				fResult = false;
				TASApp.AfxLocalizeMessageBox( AFXMSG_INCORRECT_TARGMAX );
			}
			else
			{
				m_EditTargetWaterVelocity.SetWindowText( WriteDouble( dTargetWaterVelocity, 3, 1, true ) );
				m_dTargetWaterVelocity = CDimValue::CUtoSI( _U_VELOCITY, dTargetWaterVelocity );
			}
			break;
	}

	// Verify the edit pipe max water velocity.
	switch( ReadDouble( m_EditMaxWaterVelocity, &dMaxWaterVelocity ) )
	{
		case RD_EMPTY:
			fResult = false;
			TASApp.AfxLocalizeMessageBox( AFXMSG_NON_POSITIVE_VALUE );
			break;
		
		case RD_OK:
			if( dMaxWaterVelocity < dTargetWaterVelocity && true == m_fTargetWaterVelocity )
				fResult = false;
			else
			{
				m_EditMaxWaterVelocity.SetWindowText( WriteDouble( dMaxWaterVelocity, 3, 1, true ) );
				m_dMaxWaterVelocity = CDimValue::CUtoSI( _U_VELOCITY, dMaxWaterVelocity );
			}
			break;
	}

	// HYS-1878: 3- Verify the edit pipe min water velocity.
	switch( ReadDouble( m_EditMinWaterVelocity, &dMinWaterVelocity ) )
	{
		case RD_EMPTY:
			fResult = false;
			TASApp.AfxLocalizeMessageBox( AFXMSG_NON_POSITIVE_VALUE );
			break;
		
		case RD_OK:
			double dMinAccepted = CDimValue::SItoCU( _U_VELOCITY, 0.2 );
			double dMaxAccepted = CDimValue::SItoCU( _U_VELOCITY, 0.6 );
			TCHAR tcName[_MAXCHARS];
			CUnitDatabase* pUnitDB = CDimValue::AccessUDB();
			CString str, str1, str2, str3;

			if( dTargetWaterVelocity < dMinWaterVelocity && true == m_fTargetWaterVelocity )
			{
				fResult = false;
				TASApp.AfxLocalizeMessageBox( AFXMSG_INCORRECT_TARGMIN );
			}
			else if( dMaxWaterVelocity < dMinWaterVelocity )
			{
				fResult = false;
				TASApp.AfxLocalizeMessageBox( AFXMSG_INCORRECT_MINMAX );
			}
			else if( dMinWaterVelocity < dMinAccepted || dMinWaterVelocity > dMaxAccepted )
			{
				fResult = false;
				GetNameOf( pUnitDB->GetDefaultUnit( _U_VELOCITY ), tcName );
				str3 = tcName;
				str1 = WriteDouble( dMinAccepted, 3, 1, true );
				str2 = WriteDouble( dMaxAccepted, 3, 1, true );
				FormatString( str, AFXMSG_INCORRECT_MINVEL, str1, str2, str3 );
				::AfxMessageBox( str );
			}
			else
			{
				m_EditMinWaterVelocity.SetWindowText( WriteDouble( dMinWaterVelocity, 3, 1, true ) );
				m_dMinWaterVelocity = CDimValue::CUtoSI( _U_VELOCITY, dMinWaterVelocity );
			}
			break;
	}
	
	return fResult;
}

BEGIN_MESSAGE_MAP( CDlgTPGeneralPipes, CDlgCtrlPropPage )
	ON_EN_KILLFOCUS( IDC_EDITPIPETARGDP, OnEnKillFocusTargetLinearDp )
	ON_EN_KILLFOCUS( IDC_EDITPIPEMAXDP, OnEnKillFocusMaxLinearDp )
	ON_EN_KILLFOCUS( IDC_EDITPIPETARGVEL, OnEnKillFocusTargetWaterVelocity )
	ON_EN_KILLFOCUS( IDC_EDITPIPEMAXVEL, OnEnKillFocusMaxWaterVelocity )
	ON_EN_KILLFOCUS( IDC_EDITPIPEMINVEL, OnEnKillFocusMinWaterVelocity )
	ON_EN_SETFOCUS( IDC_EDITPIPETARGDP, OnEnSetFocusTargetLinearDp )
	ON_EN_SETFOCUS( IDC_EDITPIPEMAXDP, OnEnSetFocusMaxLinearDp )
	ON_EN_SETFOCUS( IDC_EDITPIPETARGVEL, OnEnSetFocusTargetWaterVelocity )
	ON_EN_SETFOCUS( IDC_EDITPIPEMAXVEL, OnEnSetFocusMaxWaterVelocity )
	ON_EN_SETFOCUS( IDC_EDITPIPEMINVEL, OnEnSetFocusMinWaterVelocity )
	ON_BN_CLICKED( IDC_CHECKPIPETARGVEL, OnBnClickedTargetWaterVelocity )
END_MESSAGE_MAP()

void CDlgTPGeneralPipes::DoDataExchange( CDataExchange* pDX )
{
	CDlgCtrlPropPage::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_EDITPIPETARGDP, m_EditTargetLinearDp );
	DDX_Control( pDX, IDC_EDITPIPEMAXDP, m_EditMaxLinearDp );
	DDX_Control( pDX, IDC_CHECKPIPETARGVEL, m_CheckTargetWaterVelocity );
	DDX_Control( pDX, IDC_EDITPIPETARGVEL, m_EditTargetWaterVelocity );
	DDX_Control( pDX, IDC_EDITPIPEMAXVEL, m_EditMaxWaterVelocity );
	DDX_Control( pDX, IDC_EDITPIPEMINVEL, m_EditMinWaterVelocity );
	DDX_Control( pDX, IDC_STATIC_WARN, m_StaticWarnMinVelValue );
}

BOOL CDlgTPGeneralPipes::OnInitDialog() 
{
	if( FALSE == CDlgCtrlPropPage::OnInitDialog() )
	{
		return FALSE;
	}
	
	// Initialize dialog strings.
	CString str;
	str = TASApp.LoadLocalizedString( IDS_TABDLGTECHPIPE_STATICPIPETARGDP );
	GetDlgItem( IDC_STATICPIPETARGDP )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_TABDLGTECHPIPE_STATICPIPEMAXDP );
	GetDlgItem( IDC_STATICPIPEMAXDP )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_TABDLGTECHPIPE_CHECKPIPETARGVEL );
	GetDlgItem( IDC_CHECKPIPETARGVEL )->SetWindowText( str );
	
	str = TASApp.LoadLocalizedString( IDS_TABDLGTECHPIPE_STATICPIPEMAXVEL );
	GetDlgItem( IDC_STATICPIPEMAXVEL )->SetWindowText( str );

	// HYS-1878: 3- Add min. pipe velocity field in dialog tech. param.
	str = TASApp.LoadLocalizedString( IDS_TABDLGTECHPIPE_STATICPIPEMINVEL );
	GetDlgItem( IDC_STATICPIPEMINVEL )->SetWindowText( str );

	m_pTADS = TASApp.GetpTADS();

	Init();
	
	return TRUE;
}

void CDlgTPGeneralPipes::OnEnKillFocusTargetLinearDp()
{
	SetCorrectDblMinMaxBackColor( &m_EditTargetLinearDp, &m_EditMaxLinearDp );
}

void CDlgTPGeneralPipes::OnEnKillFocusMaxLinearDp()
{
	SetCorrectDblMinMaxBackColor( &m_EditTargetLinearDp, &m_EditMaxLinearDp );
}

void CDlgTPGeneralPipes::OnEnKillFocusTargetWaterVelocity()
{
	// HYS-1878: We reset the back color to consider new change. 
	if( true == m_fTargetWaterVelocity )
	{
		m_EditTargetWaterVelocity.SetBackColor( ::GetSysColor( COLOR_WINDOW ) );
		m_EditMaxWaterVelocity.SetBackColor( ::GetSysColor( COLOR_WINDOW ) );
		m_EditMinWaterVelocity.SetBackColor( ::GetSysColor( COLOR_WINDOW ) );

		SetCorrectDblMinMaxBackColor( &m_EditTargetWaterVelocity, &m_EditMaxWaterVelocity );
		SetCorrectDblMinMaxBackColor( &m_EditMinWaterVelocity, &m_EditTargetWaterVelocity );
		SetCorrectDblMinMaxBackColor( &m_EditMinWaterVelocity, &m_EditMaxWaterVelocity );
	}
}

void CDlgTPGeneralPipes::OnEnKillFocusMaxWaterVelocity()
{
	if( false == m_fTargetWaterVelocity )
	{
		double dMaxWaterVelocity;
		ReadDouble( m_EditMaxWaterVelocity, &dMaxWaterVelocity );
		m_dMaxWaterVelocity = CDimValue::CUtoSI( _U_VELOCITY, dMaxWaterVelocity );
		// HYS-1878: 3- Don't set target value if the max. value is not valid.
		if( m_dMaxWaterVelocity < m_dTargetWaterVelocity && m_dMaxWaterVelocity >= m_dMinWaterVelocity )
		{
			m_dTargetWaterVelocity = m_dMaxWaterVelocity;
			m_EditTargetWaterVelocity.SetWindowText( WriteDouble( CDimValue::SItoCU( _U_VELOCITY, m_dTargetWaterVelocity ), 3, 1, true ) );
		}

		m_EditMaxWaterVelocity.SetBackColor( ::GetSysColor( COLOR_WINDOW ) );
		m_EditMinWaterVelocity.SetBackColor( ::GetSysColor( COLOR_WINDOW ) );
		SetCorrectDblMinMaxBackColor( &m_EditMinWaterVelocity, &m_EditMaxWaterVelocity );
	}
	else
	{
		m_EditTargetWaterVelocity.SetBackColor( ::GetSysColor( COLOR_WINDOW ) );
		m_EditMaxWaterVelocity.SetBackColor( ::GetSysColor( COLOR_WINDOW ) );
		m_EditMinWaterVelocity.SetBackColor( ::GetSysColor( COLOR_WINDOW ) );

		SetCorrectDblMinMaxBackColor( &m_EditTargetWaterVelocity, &m_EditMaxWaterVelocity );
		SetCorrectDblMinMaxBackColor( &m_EditMinWaterVelocity, &m_EditTargetWaterVelocity );
		SetCorrectDblMinMaxBackColor( &m_EditMinWaterVelocity, &m_EditMaxWaterVelocity );
	}
}

void CDlgTPGeneralPipes::OnEnKillFocusMinWaterVelocity()
{
	m_StaticWarnMinVelValue.ShowWindow( SW_HIDE );
	if( false == m_fTargetWaterVelocity )
	{
		double dMinWaterVelocity;
		ReadDouble( m_EditMinWaterVelocity, &dMinWaterVelocity );
		m_dMinWaterVelocity = CDimValue::CUtoSI( _U_VELOCITY, dMinWaterVelocity );

		if( m_dMinWaterVelocity > m_dTargetWaterVelocity && m_dMaxWaterVelocity >= m_dMinWaterVelocity )
		{
			m_dTargetWaterVelocity = m_dMinWaterVelocity;
			m_EditTargetWaterVelocity.SetWindowText( WriteDouble( CDimValue::SItoCU( _U_VELOCITY, m_dTargetWaterVelocity ), 3, 1, true ) );
		}

		m_EditMaxWaterVelocity.SetBackColor( ::GetSysColor( COLOR_WINDOW ) );
		m_EditMinWaterVelocity.SetBackColor( ::GetSysColor( COLOR_WINDOW ) );
		SetCorrectDblMinMaxBackColor( &m_EditMinWaterVelocity, &m_EditMaxWaterVelocity );
	}
	else
	{
		m_EditTargetWaterVelocity.SetBackColor( ::GetSysColor( COLOR_WINDOW ) );
		m_EditMaxWaterVelocity.SetBackColor( ::GetSysColor( COLOR_WINDOW ) );
		m_EditMinWaterVelocity.SetBackColor( ::GetSysColor( COLOR_WINDOW ) );

		SetCorrectDblMinMaxBackColor( &m_EditTargetWaterVelocity, &m_EditMaxWaterVelocity );
		SetCorrectDblMinMaxBackColor( &m_EditMinWaterVelocity, &m_EditTargetWaterVelocity );
		SetCorrectDblMinMaxBackColor( &m_EditMinWaterVelocity, &m_EditMaxWaterVelocity );
	}
}

void CDlgTPGeneralPipes::OnEnSetFocusTargetLinearDp()
{
	SetCorrectDblMinMaxBackColor( &m_EditTargetLinearDp, &m_EditMaxLinearDp, true );
}

void CDlgTPGeneralPipes::OnEnSetFocusMaxLinearDp()
{
	SetCorrectDblMinMaxBackColor( &m_EditTargetLinearDp, &m_EditMaxLinearDp, true );
}

void CDlgTPGeneralPipes::OnEnSetFocusTargetWaterVelocity()
{
	if( true == m_fTargetWaterVelocity )
	{
		SetCorrectDblMinMaxBackColor( &m_EditTargetWaterVelocity, &m_EditMaxWaterVelocity, true );
		// HYS-1878: 3- Check min. velocity.
		if( _RED != m_EditTargetWaterVelocity.GetBackColor() )
		{
			SetCorrectDblMinMaxBackColor( &m_EditMinWaterVelocity, &m_EditTargetWaterVelocity, true );
		}
	}
}

void CDlgTPGeneralPipes::OnEnSetFocusMaxWaterVelocity()
{
	if( true == m_fTargetWaterVelocity )
	{
		SetCorrectDblMinMaxBackColor( &m_EditTargetWaterVelocity, &m_EditMaxWaterVelocity, true );
		// HYS-1878: 3- Check min. velocity.
		if( _RED != m_EditMaxWaterVelocity.GetBackColor() )
		{
			SetCorrectDblMinMaxBackColor( &m_EditMinWaterVelocity, &m_EditMaxWaterVelocity, true );
		}
	}
	else
	{
		SetCorrectDblMinMaxBackColor( &m_EditMinWaterVelocity, &m_EditMaxWaterVelocity, true );
	}
}

void CDlgTPGeneralPipes::OnEnSetFocusMinWaterVelocity()
{
	m_StaticWarnMinVelValue.ShowWindow( SW_SHOW );
	if( true == m_fTargetWaterVelocity )
	{
		SetCorrectDblMinMaxBackColor( &m_EditMinWaterVelocity, &m_EditTargetWaterVelocity, true );
		if( _RED != m_EditMinWaterVelocity.GetBackColor() )
		{
			SetCorrectDblMinMaxBackColor( &m_EditMinWaterVelocity, &m_EditMaxWaterVelocity, true );
		}
	}
	else
	{
		SetCorrectDblMinMaxBackColor( &m_EditMinWaterVelocity, &m_EditMaxWaterVelocity, true );
	}
}

void CDlgTPGeneralPipes::OnBnClickedTargetWaterVelocity()
{
	m_fTargetWaterVelocity = ( BST_CHECKED == m_CheckTargetWaterVelocity.GetCheck() ) ? true : false;	
	m_EditTargetWaterVelocity.EnableWindow( ( true == m_fTargetWaterVelocity ) ? TRUE : FALSE );
	m_EditTargetWaterVelocity.SetBackColor( ::GetSysColor( COLOR_WINDOW ) );
	// HYS-1878: 3- Check min. and max. velocity.
	if( true == m_fTargetWaterVelocity )
	{
		SetCorrectDblMinMaxBackColor( &m_EditTargetWaterVelocity, &m_EditMaxWaterVelocity, true );
		if( _RED != m_EditTargetWaterVelocity.GetBackColor() )
		{
			SetCorrectDblMinMaxBackColor( &m_EditMinWaterVelocity, &m_EditTargetWaterVelocity, true );
		}
	}
	GetDlgItem( IDC_STATICVELUNIT1 )->EnableWindow( ( true == m_fTargetWaterVelocity ) ? TRUE : FALSE );
}
