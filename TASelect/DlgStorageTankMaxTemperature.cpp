#include "stdafx.h"
#include "afxdialogex.h"

#include "TASelect.h"
#include "MainFrm.h"
#include "SelectPM.h"

#include "DlgStorageTankMaxTemperature.h"

CDlgStorageTankMaxTemperature::CDlgStorageTankMaxTemperature( CPMInputUser *pclPMInputUser, CWnd *pParent )
	: CDialogExt( IDD, pParent )
{
	m_dEditMaxTemperatureSaved = pclPMInputUser->GetStorageTankMaxTemp();
	m_pclPMInputUser = pclPMInputUser;
	m_dMaxTemperatureSaved = 0.0;
}

BEGIN_MESSAGE_MAP( CDlgStorageTankMaxTemperature, CDialogExt )
	ON_EN_SETFOCUS( IDC_EDITSTORAGETANKMAXTEMPERATURE, OnEnSetFocusMaxTemperature )
	ON_EN_KILLFOCUS( IDC_EDITSTORAGETANKMAXTEMPERATURE, OnKillFocusMaxTemperature )
	ON_NOTIFY( WM_USER_ENTERKEYPRESSED, IDC_EDITSTORAGETANKMAXTEMPERATURE, OnEditEnterMaxTemperature )
END_MESSAGE_MAP()

void CDlgStorageTankMaxTemperature::DoDataExchange( CDataExchange *pDX )
{
	CDialogExt::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_STATICMAXTEMPERATURE, m_StaticMaxTemperature );
	DDX_Control( pDX, IDC_EDITSTORAGETANKMAXTEMPERATURE, m_EditMaxTemperature );
}

BOOL CDlgStorageTankMaxTemperature::OnInitDialog()
{
	CDialogExt::OnInitDialog();

	CString str = TASApp.LoadLocalizedString( IDS_DLGSTORAGETANKMAXTEMPERATURE_CAPTION );
	SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_DLGSTORAGETANKMAXTEMPERATURE_STATIC );
	m_StaticMaxTemperature.SetWindowText( str );

	m_EditMaxTemperature.SetEditSign( CNumString::ePositive );
	m_EditMaxTemperature.SetCurrentValSI( m_pclPMInputUser->GetStorageTankMaxTemp() );
	m_EditMaxTemperature.Update();

	m_ToolTip.Create( this, TTS_NOPREFIX );
	m_ToolTip.SetDelayTime( TTDT_AUTOPOP, 5000 );

	return TRUE;
}

void CDlgStorageTankMaxTemperature::OnOK()
{
	PREVENT_ENTER_KEY
	CDialogExt::OnOK();
}

void CDlgStorageTankMaxTemperature::OnCancel()
{
	m_pclPMInputUser->SetStorageTankMaxTemp( m_dEditMaxTemperatureSaved );

	CDialogExt::OnCancel();
}

void CDlgStorageTankMaxTemperature::OnEnSetFocusMaxTemperature()
{
	m_EditMaxTemperature.SetSel( 0, -1 );
	m_dMaxTemperatureSaved = m_EditMaxTemperature.GetCurrentValSI();
}

void CDlgStorageTankMaxTemperature::OnKillFocusMaxTemperature()
{
	// Do nothing if no change.
	if( m_dMaxTemperatureSaved == m_EditMaxTemperature.GetCurrentValSI() )
	{
		return;
	}

	if( m_EditMaxTemperature.GetCurrentValSI() < m_pclPMInputUser->GetReturnTemperature() )
	{
		CString str;
		FormatString( str, IDS_DLGSTORAGETANKMAXTEMPERATURE_MAXTEMPCANTBELOWERTHANRETURNTEMP, WriteCUDouble( _U_TEMPERATURE, m_pclPMInputUser->GetReturnTemperature(), true ) );
		m_ToolTip.AddToolWindow( &m_EditMaxTemperature, str );

		m_EditMaxTemperature.SetDrawBorder( true, _RED );
		GetDlgItem( IDOK )->EnableWindow( FALSE );
	}
	else
	{
		m_pclPMInputUser->SetStorageTankMaxTemp( m_EditMaxTemperature.GetCurrentValSI() );

		m_ToolTip.DelTool( &m_EditMaxTemperature );
		m_EditMaxTemperature.ResetDrawBorder();
		GetDlgItem( IDOK )->EnableWindow( TRUE );
	}
}

void CDlgStorageTankMaxTemperature::OnEditEnterMaxTemperature( NMHDR *pNMHDR, LRESULT *pResult )
{
	if( IDC_EDITSTORAGETANKMAXTEMPERATURE == pNMHDR->idFrom )
	{
		if( m_EditMaxTemperature.GetCurrentValSI() < m_pclPMInputUser->GetReturnTemperature() )
		{
			CString str;
			FormatString( str, IDS_DLGSTORAGETANKMAXTEMPERATURE_MAXTEMPCANTBELOWERTHANRETURNTEMP, WriteCUDouble( _U_TEMPERATURE, m_pclPMInputUser->GetReturnTemperature(), true ) );
			m_ToolTip.AddToolWindow( &m_EditMaxTemperature, str );

			m_EditMaxTemperature.SetDrawBorder( true, _RED );
			GetDlgItem( IDOK )->EnableWindow( FALSE );
		}
		else
		{
			m_pclPMInputUser->SetStorageTankMaxTemp( m_EditMaxTemperature.GetCurrentValSI() );
			CDialogExt::OnOK();
		}
	}
}
