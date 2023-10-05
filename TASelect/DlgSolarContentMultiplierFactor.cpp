#include "stdafx.h"
#include "afxdialogex.h"

#include "TASelect.h"
#include "MainFrm.h"
#include "SelectPM.h"

#include "DlgSolarContentMultiplierFactor.h"

CDlgSolarContentMultiplierFactor::CDlgSolarContentMultiplierFactor( CPMInputUser *pclPMInputUser, CWnd *pParent )
	: CDialogExt( IDD, pParent )
{
	m_iCheckMultiplierFactorSaved = pclPMInputUser->GetSolarCollectorMultiplierFactorChecked();
	m_dEditMultiplierFactorSaved = pclPMInputUser->GetSolarCollectorMultiplierFactor();
	m_pclPMInputUser = pclPMInputUser;
	m_dMultiplierFactorSaved = 0.0;
}

BEGIN_MESSAGE_MAP( CDlgSolarContentMultiplierFactor, CDialogExt )
	ON_BN_CLICKED( IDC_CHECKSOLARCONTENTMULTIPLIERFACTOR, OnBtnMultiplierFactorClicked )
	ON_EN_SETFOCUS( IDC_EDITSOLARCONTENTMULTIPLIERFACTOR, OnEnSetFocusMultiplierFactor )
	ON_EN_KILLFOCUS( IDC_EDITSOLARCONTENTMULTIPLIERFACTOR, OnKillFocusMultiplierFactor )
	ON_NOTIFY( WM_USER_ENTERKEYPRESSED, IDC_EDITSOLARCONTENTMULTIPLIERFACTOR, OnEditEnterMultiplierFactor )
END_MESSAGE_MAP()

void CDlgSolarContentMultiplierFactor::DoDataExchange( CDataExchange *pDX )
{
	CDialogExt::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_STATICINPUTSOLARCONTENTEXPCOEFF, m_StaticMultiplierFactor );
	DDX_Control( pDX, IDC_CHECKSOLARCONTENTMULTIPLIERFACTOR, m_BtnCheckMultiplierFactor );
	DDX_Control( pDX, IDC_EDITSOLARCONTENTMULTIPLIERFACTOR, m_EditMultiplierFactor );
}

BOOL CDlgSolarContentMultiplierFactor::OnInitDialog()
{
	CDialogExt::OnInitDialog();

	CString str = TASApp.LoadLocalizedString( IDS_DLGSOLARCONTENTMULTIPLIERFACTOR_CAPTION );
	SetWindowText( str );

	str = TASApp.LoadLocalizedString( IDS_SOLARCONTENTMULTIPLIERFACTOR_STATIC );
	m_StaticMultiplierFactor.SetWindowText( str );

	m_BtnCheckMultiplierFactor.SetCheck( m_pclPMInputUser->GetSolarCollectorMultiplierFactorChecked() );

	m_EditMultiplierFactor.SetEditSign( CNumString::ePositive );
	m_EditMultiplierFactor.SetCurrentValSI( m_pclPMInputUser->GetSolarCollectorMultiplierFactor() );
	m_EditMultiplierFactor.Update();

	if( BST_UNCHECKED == m_pclPMInputUser->GetSolarCollectorMultiplierFactorChecked() )
	{
		m_EditMultiplierFactor.EnableWindow( FALSE );
	}

	m_ToolTip.Create( this, TTS_NOPREFIX );
	m_ToolTip.SetDelayTime( TTDT_AUTOPOP, 5000 );

	FormatString( str, IDS_SOLARCONTENTMULTIPLIERFACTOR_EDITLIMIT, _T("1.1"), _T("2") );
	m_ToolTip.AddToolWindow( &m_EditMultiplierFactor, str );

	return TRUE;
}

void CDlgSolarContentMultiplierFactor::OnOK()
{
	PREVENT_ENTER_KEY
	CDialogExt::OnOK();
}

void CDlgSolarContentMultiplierFactor::OnCancel()
{
	m_pclPMInputUser->SetSolarCollectorMultiplierFactorChecked( m_iCheckMultiplierFactorSaved );
	m_pclPMInputUser->SetSolarCollectorMultiplierFactor( m_dEditMultiplierFactorSaved );

	CDialogExt::OnCancel();
}

void CDlgSolarContentMultiplierFactor::OnBtnMultiplierFactorClicked()
{
	bool bEnable = false;

	if( BST_CHECKED == m_BtnCheckMultiplierFactor.GetCheck() )
	{
		if( false == m_pclPMInputUser->GetSolarContentMultiplierFactorWarningDisplayed() )
		{
			CString str = TASApp.LoadLocalizedString( IDS_DLGINDSELPMPANELS_SOLARCONTENTEXPFACTORMANUAL );

			if( IDYES == AfxMessageBox( str, MB_YESNO | MB_ICONEXCLAMATION ) )
			{
				m_pclPMInputUser->SetSolarContentMultiplierFactorWarningDisplayed( true );
				bEnable = true;
			}
		}
		else
		{
			bEnable = true;	
		}
	}

	if( true == bEnable )
	{
		m_BtnCheckMultiplierFactor.SetCheck( BST_CHECKED );
		m_EditMultiplierFactor.EnableWindow( TRUE );
	}
	else
	{
		m_BtnCheckMultiplierFactor.SetCheck( BST_UNCHECKED );
		m_EditMultiplierFactor.SetCurrentValSI( 2.0 );
		m_EditMultiplierFactor.Update();
		m_pclPMInputUser->SetSolarCollectorMultiplierFactor( 2.0 );
		m_EditMultiplierFactor.EnableWindow( FALSE );
	}

	m_pclPMInputUser->SetSolarCollectorMultiplierFactorChecked(  m_BtnCheckMultiplierFactor.GetCheck() );
}

void CDlgSolarContentMultiplierFactor::OnEnSetFocusMultiplierFactor()
{
	m_EditMultiplierFactor.SetSel( 0, -1 );
	m_dMultiplierFactorSaved = m_EditMultiplierFactor.GetCurrentValSI();
}

void CDlgSolarContentMultiplierFactor::OnKillFocusMultiplierFactor()
{
	// Do nothing if no change.
	if( m_dMultiplierFactorSaved == m_EditMultiplierFactor.GetCurrentValSI() )
	{
		return;
	}

	if( m_EditMultiplierFactor.GetCurrentValSI() < 1.1 || m_EditMultiplierFactor.GetCurrentValSI() > 2.0 )
	{
		m_EditMultiplierFactor.SetDrawBorder( true, _RED );
		GetDlgItem( IDOK )->EnableWindow( FALSE );
	}
	else
	{
		m_pclPMInputUser->SetSolarCollectorMultiplierFactor( m_EditMultiplierFactor.GetCurrentValSI() );

		m_EditMultiplierFactor.ResetDrawBorder();
		GetDlgItem( IDOK )->EnableWindow( TRUE );
	}
}

void CDlgSolarContentMultiplierFactor::OnEditEnterMultiplierFactor( NMHDR *pNMHDR, LRESULT *pResult )
{
	if( IDC_EDITSOLARCONTENTMULTIPLIERFACTOR == pNMHDR->idFrom )
	{
		if( m_EditMultiplierFactor.GetCurrentValSI() < 1.1 || m_EditMultiplierFactor.GetCurrentValSI() > 2.0 )
		{
			m_EditMultiplierFactor.SetDrawBorder( true, _RED );
			GetDlgItem( IDOK )->EnableWindow( FALSE );
			OnEnSetFocusMultiplierFactor();
		}
		else
		{
			m_pclPMInputUser->SetSolarCollectorMultiplierFactor( m_EditMultiplierFactor.GetCurrentValSI() );
			CDialogExt::OnOK();
		}
	}
}
